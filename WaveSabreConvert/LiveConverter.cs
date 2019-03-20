using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WaveSabreConvert
{
    public class LiveConverter
    {
        ILog logger;

        class Receive
        {
            public LiveProject.Track SendingTrack;
            public int ReceivingChannelIndex;
            public double Volume;
            public Receive(LiveProject.Track sendingTrack, int receivingChannelIndex, double volume)
            {
                SendingTrack = sendingTrack;
                ReceivingChannelIndex = receivingChannelIndex;
                Volume = volume;
            }
        }

        class Event
        {
            public double Time;
            public Song.EventType Type;
            public byte Note;
            public byte Velocity;
        }

        class MidiDestination
        {
            public Song.Track Track;
            public LiveProject.Device Device;
        }

        Dictionary<LiveProject.Track, List<Receive>> trackReceives;

        List<LiveProject.Track> visitedTracks, orderedTracks;

        double? minEventTime;
        double? maxEventTime;

        public Song Process(LiveProject project, ILog logger)
        {
            this.logger = logger;

            var song = new Song();

            song.Tempo = (int)project.Tempo;
            song.SampleRate = 44100;

            var projectLoopEnd = project.LoopStart + project.LoopLength;

            trackReceives = new Dictionary<LiveProject.Track, List<Receive>>();
            foreach (var projectTrack in project.Tracks) trackReceives.Add(projectTrack, new List<Receive>());
            foreach (var projectTrack in project.Tracks)
            {
                foreach (var send in projectTrack.Sends)
                {
                    if (send.IsActive) trackReceives[send.ReceivingTrack].Add(new Receive(projectTrack, send.ReceivingChannelIndex - 1, send.Volume));
                }
            }

            project.MasterTrack.Name = project.MasterTrack.Name == "" ? "Master" : project.MasterTrack.Name;
            
            visitedTracks = new List<LiveProject.Track>();
            orderedTracks = new List<LiveProject.Track>();

            visitTrack(project.MasterTrack);

            var projectTracksToSongTracks = new Dictionary<LiveProject.Track, Song.Track>();
            var songTrackEvents = new Dictionary<MidiDestination, List<Event>>();

            var deviceLink = new Dictionary<LiveProject.Device, Song.Device>();

            foreach (var projectTrack in orderedTracks)
            {
                var track = new Song.Track();
                track.Name = projectTrack.Name;
                track.Volume = (float)projectTrack.Volume;

                foreach (var projectDevice in projectTrack.Devices)
                {
                    Song.Device device = null;

                    Song.DeviceId deviceId;
                    if (Enum.TryParse<Song.DeviceId>(projectDevice.PluginDll.Replace(".dll", "").Replace(".64",""), out deviceId))
                    {
                        device = new Song.Device();
                        device.Id = deviceId;
                        device.Chunk = projectDevice.RawData != null ? (byte[])projectDevice.RawData.Clone() : new byte[0];
                    }
                    if (device == null)
                    {
                        logger.WriteLine("WARNING: Device skipped (unsupported plugin): " + projectDevice.PluginDll);
                    }
                    else if (projectDevice.Bypass)
                    {
                        logger.WriteLine("WARNING: Device skipped (bypass enabled): " + projectDevice.PluginDll);
                    }
                    else
                    {
                        track.Devices.Add(device);
                        deviceLink.Add(projectDevice, device);  // store these for liking later

                        foreach (var floatParameter in projectDevice.FloatParameters)
                        {
                            if (floatParameter.Id >= 0)
                            {
                                var automation = new Song.Automation();
                                automation.DeviceIndex = track.Devices.IndexOf(device);
                                automation.ParamId = floatParameter.Id;
                                foreach (var e in floatParameter.Events)
                                {
                                    if (e.Time >= 0.0)
                                    {
                                        var point = new Song.Point();
                                        point.TimeStamp = secondsToSamples(e.Time, (double)song.Tempo, (double)song.SampleRate);
                                        point.Value = e.Value;
                                        automation.Points.Add(point);
                                    }
                                }
                                if (automation.Points.Count > 0) track.Automations.Add(automation);
                            }
                        }
                    }
                }

                var events = ConvertEvents(projectTrack, project);
                projectTracksToSongTracks.Add(projectTrack, track);

                if (projectTrack.Devices.Count > 0)
                {
                    var midiDest = new MidiDestination()
                    {
                        Track = track,
                        Device = projectTrack.Devices[0]
                    };
                    songTrackEvents.Add(midiDest, events);
                }

                foreach (var findTrack in project.Tracks)
                {
                    if (findTrack.MidiDestinationTrack == projectTrack)
                    {
                        // FOUND IT
                        events = new List<Event>();
                        events = ConvertEvents(findTrack, project);
                        var midiDest = new MidiDestination()
                        {
                            Track = track,
                            Device = findTrack.MidiDestinationDevice
                        };
                        songTrackEvents.Add(midiDest, events);

                    }
                }

                song.Tracks.Add(track);
            }

            double songStartTime, songEndTime;
            if (project.IsLoopOn)
            {
                songStartTime = project.LoopStart;
                songEndTime = projectLoopEnd;
            }
            else if (minEventTime.HasValue && maxEventTime.HasValue)
            {
                songStartTime = minEventTime.Value;
                songEndTime = maxEventTime.Value;
            }
            else
            {
                throw new Exception("Couldn't find song start/end times");
            }
            song.Length = (songEndTime - songStartTime) * 60.0 / (double)song.Tempo;

            foreach (var kvp in songTrackEvents)
            {
                var midiDest = kvp.Key;
                var track = midiDest.Track;
                var events = kvp.Value;

                if (events.Count > 0)
                {
                    var midiEvents = new Song.MidiEvents();
                    Song.Device device = null;
                    deviceLink.TryGetValue(midiDest.Device, out device);
                    if (device == null)
                        throw new Exception("Unable to find midi destination device");
                    var deviceId = track.Devices.IndexOf(device);
                    if (deviceId < 0)
                        throw new Exception("Unable to find midi destination device");
                    midiEvents.DeviceId = deviceId;

                    int lastTimeStamp = 0;
                    foreach (var e in events)
                    {
                        var songEvent = new Song.Event();
                        var time = e.Time - songStartTime;
                        int timeStamp = Math.Max(secondsToSamples(time, (double)song.Tempo, (double)song.SampleRate), lastTimeStamp);

                        songEvent.TimeStamp = secondsToSamples(time, (double)song.Tempo, (double)song.SampleRate);
                        songEvent.Type = e.Type;
                        songEvent.Note = e.Note;
                        songEvent.Velocity = e.Velocity;
                        midiEvents.Events.Add(songEvent);
                        lastTimeStamp = timeStamp;
                    }
                    track.MidiEvents.Add(midiEvents);
                }
            }

            // TODO: Clip all of this instead of just offsetting
            // adjust automation start times based on song start
            foreach (var track in song.Tracks)
            {
                foreach (var automation in track.Automations)
                {
                    foreach (var point in automation.Points)
                    {
                        point.TimeStamp -= secondsToSamples(songStartTime, (double)song.Tempo, (double)song.SampleRate);
                    }
                }
            }

            foreach (var kvp in projectTracksToSongTracks)
            {
                foreach (var projectReceive in trackReceives[kvp.Key])
                {
                    if (projectTracksToSongTracks.ContainsKey(projectReceive.SendingTrack))
                    {
                        var receive = new Song.Receive();
                        receive.SendingTrackIndex = song.Tracks.IndexOf(projectTracksToSongTracks[projectReceive.SendingTrack]);
                        receive.ReceivingChannelIndex = projectReceive.ReceivingChannelIndex;
                        receive.Volume = (float)projectReceive.Volume;
                        kvp.Value.Receives.Add(receive);
                    }
                }
            }

            return song;
        }

        List<Event> ConvertEvents(LiveProject.Track projectTrack, LiveProject project)
        {
            var projectLoopEnd = project.LoopStart + project.LoopLength;
            var events = new List<Event>();
            foreach (var midiClip in projectTrack.MidiClips)
            {
                if (!midiClip.IsDisabled)
                {
                    var loopLength = midiClip.LoopEnd - midiClip.LoopStart;
                    for (var currentTime = midiClip.CurrentStart; currentTime < midiClip.CurrentEnd; currentTime += loopLength)
                    {
                        foreach (var keyTrack in midiClip.KeyTracks)
                        {
                            foreach (var note in keyTrack.Notes)
                            {
                                if (note.IsEnabled)
                                {
                                    var startTime = note.Time - (currentTime - midiClip.CurrentStart) - midiClip.LoopStartRelative;
                                    while (startTime < 0.0) startTime += loopLength;
                                    startTime = currentTime + startTime - midiClip.LoopStart;
                                    var endTime = startTime + note.Duration;

                                    if ((startTime >= midiClip.CurrentStart && startTime < midiClip.CurrentEnd) &&
                                        (!project.IsLoopOn || (
                                            startTime >= project.LoopStart && startTime < projectLoopEnd)))
                                    {
                                        endTime = Math.Min(endTime, midiClip.CurrentEnd);
                                        if (project.IsLoopOn) endTime = Math.Min(endTime, projectLoopEnd);
                                        if (endTime > startTime)
                                        {
                                            var startEvent = new Event();
                                            startEvent.Time = startTime;
                                            startEvent.Type = Song.EventType.NoteOn;
                                            startEvent.Note = (byte)keyTrack.MidiKey;
                                            startEvent.Velocity = (byte)note.Velocity;
                                            events.Add(startEvent);

                                            var endEvent = new Event();
                                            endEvent.Time = endTime;
                                            endEvent.Type = Song.EventType.NoteOff;
                                            endEvent.Note = (byte)keyTrack.MidiKey;
                                            events.Add(endEvent);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            events.Sort((a, b) =>
            {
                if (a.Time > b.Time) return 1;
                if (a.Time < b.Time) return -1;
                if (a.Type == Song.EventType.NoteOn && b.Type == Song.EventType.NoteOff) return 1;
                if (a.Type == Song.EventType.NoteOff && b.Type == Song.EventType.NoteOn) return -1;
                return 0;
            });

            foreach (var e in events)
            {
                if (!minEventTime.HasValue || e.Time < minEventTime.Value) minEventTime = e.Time;
                if (!maxEventTime.HasValue || e.Time > maxEventTime.Value) maxEventTime = e.Time;
            }

            return events;
        }

        void visitTrack(LiveProject.Track projectTrack)
        {
            if (visitedTracks.Contains(projectTrack) || !projectTrack.IsSpeakerOn) return;
            visitedTracks.Add(projectTrack);
            foreach (var projectReceive in trackReceives[projectTrack])
            {
                if (projectReceive.Volume > 0.0) visitTrack(projectReceive.SendingTrack);
            }
            orderedTracks.Add(projectTrack);
        }

        static int secondsToSamples(double time, double tempo, double sampleRate)
        {
            return (int)(time * 60.0 / tempo * sampleRate);
        }
    }
}
