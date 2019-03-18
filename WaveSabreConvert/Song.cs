﻿using System;
using System.Collections.Generic;

namespace WaveSabreConvert
{
    public class Song
    {
        private int midiLaneDupes = 0; 

        public enum DeviceId
        {
            Falcon,
            Slaughter,
            Thunder,
            Scissor,
            Leveller,
            Crusher,
            Echo,
            Smasher,
            Chamber,
            Twister,
            Cathedral,
            Adultery,
            Specimen,
            Glitcher
        }

        public class Receive
        {
            public int SendingTrackIndex;
            public int ReceivingChannelIndex;
            public float Volume;
        }

        public class Device
        {
            public DeviceId Id;
            public byte[] Chunk;
        }

        public enum EventType
        {
            NoteOn,
            NoteOff,
        }

        public class Event
        {
            public int TimeStamp;
            public EventType Type;
            public byte Note;
            public byte Velocity;
        }

        public class DeltaCodedEvent
        {
            public int TimeFromLastEvent;
            public EventType Type;
            public byte Note;
            public byte Velocity;
        }

        public class Point
        {
            public int TimeStamp;
            public float Value;
        }

        public class DeltaCodedPoint
        {
            public int TimeFromLastPoint;
            public byte Value;
        }

        public class Automation
        {
            public int DeviceIndex;
            public int ParamId;
            public List<Point> Points = new List<Point>();
            /// <summary>
            /// Auto populated by converter, do not populate
            /// </summary>
            public List<DeltaCodedPoint> DeltaCodedPoints = new List<DeltaCodedPoint>();
        }

        public class Track
        {
            public string Name;
            public float Volume;
            public List<Receive> Receives = new List<Receive>();
            public List<Device> Devices = new List<Device>();
            public List<MidiEvents> MidiEvents = new List<MidiEvents>();
            public List<Automation> Automations = new List<Automation>();
            /// <summary>
            /// Auto populated by converter, do not populate
            /// </summary>
            public List<DeltaCodedMidiEvents> DeltaCodedEvents = new List<DeltaCodedMidiEvents>();
            /// <summary>
            /// Auto populated by converter, do not populate
            /// </summary>
            public List<int> DeviceIndicies = new List<int>();
            /// <summary>
            /// Auto populated by converter, do not populate
            /// </summary>
            public List<MidiSend> MidiSends = new List<MidiSend>();

            public int DataSize;

            public override string ToString()
            {
                return this.Name;
            }
        }

        public class MidiEvents
        {
            public int DeviceId;
            public List<Event> Events = new List<Event>();
        }
        
        public class MidiSend
        {
            public int DeviceId;
            public int MidiLaneId;
        }

        public class DeltaCodedMidiEvents
        {
            public int DeviceId;
            public List<DeltaCodedEvent> MidiEvents = new List<DeltaCodedEvent>();
        }
        
        public class MidiLane
        {
            public List<DeltaCodedEvent> MidiEvents = new List<DeltaCodedEvent>();
        }

        public int Tempo;
        public int SampleRate;
        public double Length;

        public List<Track> Tracks = new List<Track>();
        /// <summary>
        /// Auto populated by converter, do not populate
        /// </summary>
        public List<Device> Devices = new List<Device>();
        /// <summary>
        /// Auto populated by converter, do not populate
        /// </summary>
        public List<MidiLane> MidiLanes = new List<MidiLane>();

        // detect things which could cause a problem within the song
        public void DetectWarnings(ILog logger)
        {
            // detect multiple side-chain useage on tracks
            foreach (var t in Tracks)
            {
                var recCount = 0;
                foreach (var r in t.Receives)
                {
                    if (r.ReceivingChannelIndex > 0)
                    {
                        recCount++;
                    }
                }

                if (recCount > 1)
                {
                    logger.WriteLine("WARNING: Track {0} has {1} side-chain recieves, audio will be summed for all devices", t.Name, recCount);
                }
            }
        }

        // restructures song elements int indexes lists
        public void Restructure(ILog logger)
        {
            // make list of all devices
            foreach (var t in Tracks)
            {
                foreach (var d in t.Devices)
                {
                    Devices.Add(d);
                }
            }

            // sort by devices type, keeping similar chunk data together, should be better for compressor
            Devices.Sort((x, y) => x.Id.CompareTo(y.Id));

            // link device id back to track
            foreach (var t in Tracks)
            {
                foreach (var d in t.Devices)
                {
                    t.DeviceIndicies.Add(Devices.IndexOf(d));
                }
            }

            // create midi lanes from tracks and index them
            foreach (var t in Tracks)
            {
                foreach (var dce in t.DeltaCodedEvents)
                {
                    var midiLane = new MidiLane() { MidiEvents = dce.MidiEvents };
                    var midiLaneId = AddMidiLane(midiLane);
                    t.MidiSends.Add(new MidiSend() { DeviceId = dce.DeviceId, MidiLaneId = midiLaneId });
                }
            }

            if (midiLaneDupes > 0)
                logger.WriteLine("Found {0} duplicate midi lanes", midiLaneDupes);
        }

        // determines if this midi lane is exactly the same as an existing one
        // otherwise add it
        private int AddMidiLane(MidiLane midiLane)
        {
            foreach (var m in MidiLanes)
            {
                var duplicate = false;
                if (m.MidiEvents.Count == midiLane.MidiEvents.Count)
                {
                    duplicate = true;
                    for (var i = 0; i < m.MidiEvents.Count; i++)
                    {
                        if (m.MidiEvents[i].Note != midiLane.MidiEvents[i].Note
                            || m.MidiEvents[i].Velocity != midiLane.MidiEvents[i].Velocity
                            || m.MidiEvents[i].Type != midiLane.MidiEvents[i].Type
                            || m.MidiEvents[i].TimeFromLastEvent != midiLane.MidiEvents[i].TimeFromLastEvent)
                        {
                            duplicate = false;
                            break;
                        }
                    }
                }

                if (duplicate)
                {
                    if (midiLane.MidiEvents.Count > 0)
                        midiLaneDupes++;

                    return MidiLanes.IndexOf(m);
                }
            }

            MidiLanes.Add(midiLane);
            return MidiLanes.IndexOf(midiLane);
        }

        // performs delta encoding on midi and automation events
        public void DeltaEncode()
        {
            int lastTime;
            foreach (var t in Tracks)
            {
                foreach (var a in t.Automations)
                {
                    lastTime = 0;
                    foreach (var p in a.Points)
                    {
                        a.DeltaCodedPoints.Add(new DeltaCodedPoint()
                        {
                            TimeFromLastPoint = p.TimeStamp - lastTime,
                            Value = FloatToByte(p.Value)
                        });

                        lastTime = p.TimeStamp;
                    }
                }

                foreach (var midi in t.MidiEvents)
                {
                    lastTime = 0;
                    var dce = new DeltaCodedMidiEvents();
                    dce.DeviceId = midi.DeviceId;

                    foreach (var e in midi.Events)
                    {
                        dce.MidiEvents.Add(new DeltaCodedEvent()
                        {
                            TimeFromLastEvent = e.TimeStamp - lastTime,
                            Type = e.Type,
                            Note = e.Note,
                            Velocity = e.Velocity
                        });

                        lastTime = e.TimeStamp;
                    }
                    t.DeltaCodedEvents.Add(dce);
                }
            }
        }

        private byte FloatToByte(float value)
        {
            var temp = (int)Math.Floor((double)value * 255.0);
            if (temp < 0)
                temp = 0;
            if (temp > 255)
                temp = 255;
            return (byte) temp;
        }
    }
}
