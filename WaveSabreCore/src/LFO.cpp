#include <WaveSabreCore/LFO.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	LFO::LFO()
	{
		phase = 0.0f;
		phaseInc = 0.0f;
		LFOWave = LFOWave::Sin;
	}

	// tempo sync
	void LFO::SetTempoRate(int rate)
	{ 
		rate++;															// do this or we start on a tripplet
		double frequency = (Helpers::CurrentTempo / 60.0f) * 8.0f;		// base frequency at 1/32
		if (rate % 2 == 0)												// tripplets inbetween each rate value
			frequency = frequency * 3 / 2;
		rate = rate / 2;
		frequency = frequency / (1 << rate);							// log divide downwards
		phaseInc = frequency / (Helpers::CurrentSampleRate);
	}

	// free running
	void LFO::SetFreeRate(double rate)
	{
		double frequency = 20.0f * rate * rate;
		phaseInc = frequency / (Helpers::CurrentSampleRate);
	}

	void LFO::Trigger(float phaseStart)
	{
		phase = phaseStart;
	}

	float LFO::Next()
	{
		float output = 0;

		switch (LFOWave)
		{
		case LFOWave::Sin:
			output = (float)Helpers::FastSin((2.0f * 3.141592 * phase));
			break;
		case LFOWave::Square:
			output = (float)phase >= 0.5f ? -1.0f : 1.0f;
			break;
		case LFOWave::SawUp:
			output = (float)phase * 2.0f - 1.0f;
			break;
		case LFOWave::SawDown:
			output = (float)-(phase * 2.0f - 1.0f);
			break;
		default:
			output = 0;
		}

		// calculate next phase position
		phase += (phaseInc);
		if (phase >= 1.0f)
		{
			phase -= 1.0f;
		}

		return output;
	}

}
