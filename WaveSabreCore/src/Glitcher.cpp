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

		filterFreq = 0.5f;
		filterLfoAmount = 0.5f;
		filterLfoRateAdjust = 1.0f;
		filterWave = LFOWave::Sin;
		
		dryActive = true;

		reverseLeft.SetLength(ReverseBufferLength);
		reverseRight.SetLength(ReverseBufferLength);

		// trigger continuous voice for dry signal
		voices[0]->NoteOn((int)GlitchMode::DrySignal, 0, 0, 0);

		master = 1.0f;
	}

	Glitcher::GlitcherVoice::GlitcherVoice(Glitcher *glitcher)
	{
		this->glitcher = glitcher;
	}

	void Glitcher::GlitcherVoice::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		float freq = 0.0f;

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
		case GlitchMode::AutoFilter:
			lfo.LFOWave = glitcher->filterWave;
			lfo.SetRate((int)((float)Velocity / 128.0f * 15.0f));
			lfo.SetRatePercent(glitcher->filterLfoRateAdjust);
			for (int i = 0; i < numSamples; i++)
			{
				freq = glitcher->filterFreq;  //lfo.Next();
				freq = freq + (lfo.Next() * glitcher->filterLfoAmount);
				freq = Helpers::ParamToFrequency(Helpers::Clamp(freq, 0.0f, 1.0f));
				for (int j = 0; j < 2; j++)
				{
					filter[j].SetFreq(freq);
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
		case GlitchMode::AutoFilter:
			for (int i = 0; i < 2; i++)
			{
				filter[0].SetType(BiquadFilterType::Lowpass);
				filter[0].SetFreq(3000.0f);
				filter[0].SetQ(1.0f);
			}
			lfo.SetRate(8);
			lfo.LFOWave = LFOWave::Sin;
			lfo.Trigger(0.0f);
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

		case ParamIndices::FilterFreq: filterFreq = value; break;
		case ParamIndices::FilterLfoAmount: filterLfoAmount = value; break;
		case ParamIndices::FilterLfoRateAdjust: filterLfoRateAdjust; break;
		case ParamIndices::FilterWave: filterWave = Helpers::ParamToLFOWave(value); break;
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

		case ParamIndices::FilterFreq: return filterFreq;
		case ParamIndices::FilterLfoAmount: return filterLfoAmount; break;
		case ParamIndices::FilterLfoRateAdjust: return filterLfoRateAdjust; break;
		case ParamIndices::FilterWave: return Helpers::LFOWaveToParam(filterWave);
		}
	}
}
