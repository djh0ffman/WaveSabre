#include <WaveSabreCore/Glitcher.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Glitcher::Glitcher()
		: VoiceEffect((int)ParamIndices::NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new GlitcherVoice(this);

		lowCutFreq = 20.0f;
		lowCutQ = 1.0f;

		peak1Freq = 1000.0f;
		peak1Gain = 0.0f;
		peak1Q = 1.0f;

		peak2Freq = 3000.0f;
		peak2Gain = 0.0f;
		peak2Q = 1.0f;

		peak3Freq = 7000.0f;
		peak3Gain = 0.0f;
		peak3Q = 1.0f;

		highCutFreq = 20000.0f;
		highCutQ = 1.0f;

		for (int i = 0; i < 2; i++)
		{
			highpass[i].SetType(BiquadFilterType::Highpass);			
			peak1[i].SetType(BiquadFilterType::Peak);
			peak2[i].SetType(BiquadFilterType::Peak);
			peak3[i].SetType(BiquadFilterType::Peak);
		}

		gateAttack = 1.0f;
		gateDecay = 1.0f;
		gateSustain = 1.0f;
		gateRelease = 1.0f;

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
			break;
		default:
			break;
		}

		IsOn = false;
	}

	void Glitcher::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::GateAttack: gateAttack = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::GateDecay: gateDecay = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::GateSustain: gateSustain = value; break;
		case ParamIndices::GateRelease: gateRelease = Helpers::ScalarToEnvValue(value); break;
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
		}
	}
}
