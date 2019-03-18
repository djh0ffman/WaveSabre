﻿using System.Collections.Generic;

namespace WaveSabreConvert
{
    public class LiveProject
    {
        public class Send
        {
            public Track ReceivingTrack;
            public int ReceivingChannelIndex;
            public double Volume;
            public bool IsActive;
            public Send(Track receivingTrack, int receivingChannelIndex, double volume, bool isActive)
            {
                ReceivingTrack = receivingTrack;
                ReceivingChannelIndex = receivingChannelIndex;
                Volume = volume;
                IsActive = isActive;
            }
        }

        public class FloatParameter
        {
            public class Event
            {
                public double Time;
                public float Value;
            }

            public int Id;
            public List<Event> Events = new List<Event>();
        }

        public class MidiRouting
        {
            public Track MidiSource;
            public Track Destination;
            public Device DestinationDevice;
        }

        public class Device
        {
            public string Id;
            public string PluginDll;
            public bool Bypass;
            public byte[] RawData;
            public List<FloatParameter> FloatParameters = new List<FloatParameter>();
        }

        public class Note
        {
            public double Time;
            public double Duration;
            public int Velocity;
            public bool IsEnabled;
        }

        public class KeyTrack
        {
            public int MidiKey;
            public List<Note> Notes = new List<Note>();
        }

        public class MidiClip
        {
            public double CurrentStart, CurrentEnd;
            public double LoopStart, LoopEnd, LoopStartRelative;
            public bool IsDisabled;
            public List<KeyTrack> KeyTracks = new List<KeyTrack>();
        }

        public class Track
        {
            public string Id;
            public string Name;
            public double Volume;
            public bool IsSpeakerOn;
            public List<Send> Sends = new List<Send>();
            public List<Device> Devices = new List<Device>();
            public List<MidiClip> MidiClips = new List<MidiClip>();
            public string TrackGroupId;
            public Track MidiDestinationTrack;
            public Device MidiDestinationDevice;
        }

        public List<Track> Tracks = new List<Track>();
        public List<Track> ReturnTracks = new List<Track>();
        public Track MasterTrack;

        public double Tempo;

        public bool IsLoopOn;
        public double LoopStart, LoopLength;
    }
}
