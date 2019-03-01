#include <WaveSabreCore/VoiceEffect.h>

#include <WaveSabreCore/Helpers.h>

#include <Windows.h>

namespace WaveSabreCore
{
	VoiceEffect::VoiceEffect(int numParams)
		: Device(numParams)
	{
		VoicesUnisono = 1;
		VoicesDetune = 0.0f;
		VoicesPan = .5f;

		VibratoFreq = Helpers::ParamToVibratoFreq(0.0f);
		VibratoAmount = 0.0f;

		Rise = 0.0f;

		voices = new Voice *[maxVoices];
		events = new Event[maxEvents];
		clearEvents();
	}

	VoiceEffect::~VoiceEffect()
	{
		for (int i = 0; i < maxVoices; i++) delete voices[i];
		delete [] voices;
		delete [] events;
	}

	void VoiceEffect::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		int originalNumSamples = numSamples;

		auto leftBuffer = new float[numSamples];
		auto rightBuffer = new float[numSamples];
		
		float *runningOutputs[2];
		runningOutputs[0] = outputs[0];
		runningOutputs[1] = outputs[1];
		
		float *runningInputs[2];
		runningInputs[0] = leftBuffer;
		runningInputs[1] = rightBuffer;

		copyBuffers(inputs, runningInputs, numSamples);

		clearOutputs(outputs, numSamples);

		while (numSamples)
		{
			int samplesToNextEvent = numSamples;
			for (int i = 0; i < maxEvents; i++)
			{
				Event *e = &events[i];
				if (e->Type != EventType::None)
				{
					if (!e->DeltaSamples)
					{
						switch (e->Type)
						{
							case EventType::NoteOn:
							{
								int j = VoicesUnisono;
								for (int k = 0; j && k < maxVoices; k++)
								{
									if (!voices[k]->IsOn)
									{
										j--;
										float f = (float)j / (VoicesUnisono > 1 ? (float)(VoicesUnisono - 1) : 1.0f);
										voices[k]->NoteOn(e->Note, e->Velocity, f * VoicesDetune, (f - .5f) * (VoicesPan * 2.0f - 1.0f) + .5f);
									}
								}
							}
							break;

						case EventType::NoteOff:
							for (int j = 0; j < maxVoices; j++)
							{
								if (voices[j]->IsOn && voices[j]->Note == e->Note) voices[j]->NoteOff();
							}
						}
						events[i].Type = EventType::None;
					}
					else if (e->DeltaSamples < samplesToNextEvent)
					{
						samplesToNextEvent = e->DeltaSamples;
					}
				}
			}

			for (int i = 0; i < maxVoices; i++)
			{
				if (voices[i]->IsOn) voices[i]->Run(songPosition, runningInputs, runningOutputs, samplesToNextEvent);
			}
			for (int i = 0; i < maxEvents; i++)
			{
				if (events[i].Type != EventType::None) events[i].DeltaSamples -= samplesToNextEvent;
			}
			songPosition += (double)samplesToNextEvent / Helpers::CurrentSampleRate;
			runningOutputs[0] += samplesToNextEvent;
			runningOutputs[1] += samplesToNextEvent;
			runningInputs[0] += samplesToNextEvent;
			runningInputs[1] += samplesToNextEvent;
			numSamples -= samplesToNextEvent;
		}

		if (leftBuffer) delete[] leftBuffer;
		if (rightBuffer) delete[] rightBuffer;
	}

	void VoiceEffect::AllNotesOff()
	{
		for (int i = 0; i < maxVoices; i++)
		{
			if (voices[i]->IsOn) voices[i]->NoteOff();
		}
		clearEvents();
	}

	void VoiceEffect::NoteOn(int note, int velocity, int deltaSamples)
	{
		for (int i = 0; i < maxEvents; i++)
		{
			if (events[i].Type == EventType::None)
			{
				events[i].Type = EventType::NoteOn;
				events[i].DeltaSamples = deltaSamples;
				events[i].Note = note;
				events[i].Velocity = velocity;
				break;
			}
		}
	}

	void VoiceEffect::NoteOff(int note, int deltaSamples)
	{
		for (int i = 0; i < maxEvents; i++)
		{
			if (events[i].Type == EventType::None)
			{
				events[i].Type = EventType::NoteOff;
				events[i].DeltaSamples = deltaSamples;
				events[i].Note = note;
				break;
			}
		}
	}

	VoiceEffect::Voice::Voice()
	{
		IsOn = false;
		vibratoPhase = 0.0;
	}

	VoiceEffect::Voice::~Voice() { }

	void VoiceEffect::Voice::NoteOn(int note, int velocity, float detune, float pan)
	{
		IsOn = true;
		Note = note;
		Detune = detune;
		Pan = pan;
	}

	void VoiceEffect::Voice::NoteOff()
	{
	}

	void VoiceEffect::clearEvents()
	{
		for (int i = 0; i < maxEvents; i++) events[i].Type = EventType::None;
	}
}
