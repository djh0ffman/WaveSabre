#include <WaveSabreCore/Glitcher.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Glitcher::Glitcher()
		: VoiceEffect((int)ParamIndices::NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new GlitcherVoice(this);
		
		gateAttack = 1.0f;
		gateDecay = 1.0f;
		gateSustain = 1.0f;
		gateRelease = 1.0f;
		 
		filterType = 0;
		filterFreq = 0.5f;
		filterQ = 1.0f;
		filterLfoAmount = 0.3f;
		filterLfoRate = 0.3f;
		filterLfoPhase = 0.0f;

		filterVeloAmount = 0.0f;
		filterVeloRate = 0.0f;

		filterTempoSync = true;
		filterWave = LFOWave::Sin;
		
		dryActive = true;

		repeatLoopLength = 1.0f;
		repeatLoopMode = LoopMode::Repeat;
		repeatPitch = 0.0f;

		reverseLeft.SetLength(ReverseBufferLength);
		reverseRight.SetLength(ReverseBufferLength);

		// trigger continuous voice for dry signal
		voices[0]->NoteOn((int)GlitchMode::DrySignal, 0, 0, 0);

		master = 1.0f;
	}

	float Glitcher::calcLength(int velocity)
	{
		double delayScalar = 120.0 / (double)Helpers::CurrentTempo / 64.0 * 1000.0;
		velocity = velocity / 8;
		if (velocity % 2 == 0)
			delayScalar = delayScalar * 2 / 3;
		else
			velocity--;
		velocity = (velocity / 2) + 1;
		float bufferLength = (float)(delayScalar * (1<<velocity));
		return bufferLength;
	}

	Glitcher::GlitcherVoice::GlitcherVoice(Glitcher *glitcher)
	{
		this->glitcher = glitcher;
	}

	void Glitcher::GlitcherVoice::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		float freq = 0.0f;
		float velo = (float)Velocity / 127.0f;
		float lfoValue = 0.0f;

		switch (glitchMode)
		{
		case GlitchMode::DrySignal:
			for (int i = 0; i < numSamples; i++)
			{
				glitcher->reverseLeft.WriteSample(inputs[0][i]);
				glitcher->reverseRight.WriteSample(inputs[1][i]);
			}
			if (glitcher->dryActive)
			{
				for (int i = 0; i < numSamples; i++)
				{
					outputs[0][i] += inputs[0][i];
					outputs[1][i] += inputs[1][i];
				}
			}
			break;
		case GlitchMode::ManualGate:
			for (int i = 0; i < numSamples; i++)
			{
				float gateAmp = gateEnv.GetValue();
				outputs[0][i] += inputs[0][i] * gateAmp;
				outputs[1][i] += inputs[1][i] * gateAmp;
				gateEnv.Next();
				if (gateEnv.State == EnvelopeState::Finished)
				{
					IsOn = false;
				}
			}
			break;
		case GlitchMode::Reverse:
			for (int i = 0; i < numSamples; i++)
			{
				outputs[0][i] += glitcher->reverseLeft.ReadSample();
				outputs[1][i] += glitcher->reverseRight.ReadSample();
				gateEnv.Next();
			} 
			break;
		case GlitchMode::Repeat:
 			for (int j = 0; j < 2; j++)
			{
				samplePlayer[j].LoopLength = glitcher->repeatLoopLength;
				samplePlayer[j].CalcPitch(glitcher->repeatPitch);
				samplePlayer[j].RunPrep();
			}
			for (int i = 0; i < numSamples; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					recordBuffer[j].WriteSample(inputs[j][i]);
					outputs[j][i] += samplePlayer[j].Next();
				}
			}
			break;
		case GlitchMode::AutoFilter:
			lfo.LFOWave = glitcher->filterWave;
			if (glitcher->filterTempoSync)
				lfo.SetTempoRate((int)((glitcher->filterLfoRate + (velo * glitcher->filterVeloRate)) * 15.0f));
			else
				lfo.SetFreeRate(1.0f - glitcher->filterLfoRate + (velo * glitcher->filterVeloRate));
			for (int i = 0; i < numSamples; i++)
			{
				lfoValue = lfo.Next(); 
				freq = glitcher->filterFreq + (lfoValue * glitcher->filterLfoAmount) + (lfoValue * velo * glitcher->filterVeloAmount);
				freq = Helpers::ParamToFrequency(Helpers::Clamp(freq, 0.0f, 1.0f));

				for (int j = 0; j < 2; j++)
				{
					filter[j].SetType((BiquadFilterType)glitcher->filterType);
					filter[j].SetFreq(freq);
					filter[j].SetQ(glitcher->filterQ);
					outputs[j][i] += filter[j].Next(inputs[j][i]);
				}
			}
			break;
		default:
			break;
		}
	}

	void Glitcher::GlitcherVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		glitchMode = GlitchMode(note);

		gateEnv.Attack = glitcher->gateAttack;
		gateEnv.Decay = glitcher->gateDecay;
		gateEnv.Sustain = glitcher->gateSustain;
		gateEnv.Release = glitcher->gateRelease;
		gateEnv.Trigger();

		float bufferLength = 0.0f;

		switch (glitchMode)
		{
		case GlitchMode::DrySignal:
			glitcher->dryActive = true;
			Voice::NoteOn(note, velocity, detune, pan);
			break;
		case GlitchMode::DryOn:
			glitcher->dryActive = true;
			break;
		case GlitchMode::DryOff:
			glitcher->dryActive = false;
			break;
		case GlitchMode::ManualGate:
			glitcher->dryActive = false;
			Voice::NoteOn(note, velocity, detune, pan);
			break;
		case GlitchMode::Reverse:
			glitcher->dryActive = false;
			glitcher->reverseLeft.Trigger();
			glitcher->reverseRight.Trigger();
			Voice::NoteOn(note, velocity, detune, pan);
			break;
		case GlitchMode::Repeat: 
			for (int i = 0; i < 2; i++)
			{
				recordBuffer[i].SetLength(glitcher->calcLength(velocity));

				samplePlayer[i].SampleData = recordBuffer[i].GetBuffer();
				samplePlayer[i].SampleLength = recordBuffer[i].GetLength();
				samplePlayer[i].SampleLoopStart = 0;
				samplePlayer[i].SampleLoopLength = recordBuffer[i].GetLength();

				samplePlayer[i].SampleStart = 0.0f;
				samplePlayer[i].LoopStart = 0.0f;
				samplePlayer[i].LoopLength = glitcher->repeatLoopLength;
				samplePlayer[i].LoopMode = glitcher->repeatLoopMode;
				samplePlayer[i].LoopBoundaryMode = LoopBoundaryMode::Manual;
				samplePlayer[i].InterpolationMode = InterpolationMode::Linear;
				samplePlayer[i].Reverse = false;

				samplePlayer[i].CalcPitch(0.0);
				samplePlayer[i].InitPos();
				samplePlayer[i].RunPrep();
				glitcher->dryActive = false;
			}
			Voice::NoteOn(note, velocity, detune, pan);
			break;
		case GlitchMode::AutoFilter:
			for (int i = 0; i < 2; i++)
			{
				filter[i].SetType((BiquadFilterType)glitcher->filterType);
				filter[i].SetFreq(Helpers::ParamToFrequency(glitcher->filterFreq));
				filter[i].SetQ(glitcher->filterQ);
			}
			lfo.LFOWave = glitcher->filterWave;
			lfo.Trigger(glitcher->filterLfoPhase);
			glitcher->dryActive = false;
			Voice::NoteOn(note, velocity, detune, pan);
			break;
		default:
			break;
		}
	}

	void Glitcher::GlitcherVoice::NoteOff()
	{
		// ensure dry signal is never switched off
		if (glitchMode == GlitchMode::DrySignal)
			return;

		switch (glitchMode)
		{
		case GlitchMode::ManualGate:
			gateEnv.Off();
			break;
		case GlitchMode::Reverse:
			glitcher->dryActive = true;
			IsOn = false;
			break;
		case GlitchMode::Repeat:
			glitcher->dryActive = true;
			IsOn = false;
			break;
		case GlitchMode::AutoFilter:
			glitcher->dryActive = true;
			IsOn = false;
			break;
		default:
			break;
		}
	}

	void Glitcher::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::GateAttack: gateAttack = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::GateDecay: gateDecay = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::GateSustain: gateSustain = value; break;
		case ParamIndices::GateRelease: gateRelease = Helpers::ScalarToEnvValue(value); break;

		case ParamIndices::FilterType: filterType = (int)(value * 2.0f); break;
		case ParamIndices::FilterFreq: filterFreq = value; break;
		case ParamIndices::FilterQ: filterQ = Helpers::ParamToQ(value); break;
		case ParamIndices::FilterWave: filterWave = Helpers::ParamToLFOWave(value); break;
		case ParamIndices::FilterLfoAmount: filterLfoAmount = value; break;
		case ParamIndices::FilterLfoRate: filterLfoRate = value; break;
		case ParamIndices::FilterLfoPhase: filterLfoPhase = value; break;
		case ParamIndices::FilterVeloAmount: filterVeloAmount = value; break;
		case ParamIndices::FilterVeloRate: filterVeloRate = value; break;
		case ParamIndices::FilterTempoSync: filterTempoSync = Helpers::ParamToBoolean(value); break;

		case ParamIndices::RepeatLoopLength: repeatLoopLength = value; break;
		case ParamIndices::RepeatLoopMode: repeatLoopMode = (LoopMode)(int)(value * (float)((int)LoopMode::NumLoopModes - 1)); break;
		case ParamIndices::RepeatPitch: repeatPitch = (value - 0.5f) * 24.0f;
		}
	}

	float Glitcher::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::GateAttack: default: 
			return Helpers::EnvValueToScalar(gateAttack);
		case ParamIndices::GateDecay: return Helpers::EnvValueToScalar(gateDecay);
		case ParamIndices::GateSustain: return gateSustain;
		case ParamIndices::GateRelease: return Helpers::EnvValueToScalar(gateRelease);

		case ParamIndices::FilterType: return (float)(filterType / 2.0f);
		case ParamIndices::FilterFreq: return filterFreq;
		case ParamIndices::FilterQ: return Helpers::QToParam(filterQ);
		case ParamIndices::FilterWave: return Helpers::LFOWaveToParam(filterWave);
		case ParamIndices::FilterLfoAmount: return filterLfoAmount;
		case ParamIndices::FilterLfoRate: return filterLfoRate;
		case ParamIndices::FilterLfoPhase: return filterLfoPhase;
		case ParamIndices::FilterVeloAmount: return filterVeloAmount;
		case ParamIndices::FilterVeloRate: return filterVeloRate;
		case ParamIndices::FilterTempoSync: return Helpers::BooleanToParam(filterTempoSync);

		case ParamIndices::RepeatLoopLength: return repeatLoopLength;
		case ParamIndices::RepeatLoopMode: return (float)repeatLoopMode / (float)((int)LoopMode::NumLoopModes - 1);
		case ParamIndices::RepeatPitch: return (repeatPitch / 24.0f) + 0.5f;
		}
	}
}
