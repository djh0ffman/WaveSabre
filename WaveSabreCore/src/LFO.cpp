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
		ratePercent = 1.0f;
	}

	void LFO::SetRate(int rate)
	{ 
		double frequency = Helpers::CurrentTempo > 0.0f ? 60.0f / Helpers::CurrentTempo : 60.0f / 120.0f;
		double scaler = 1.0f / NoteDivisionScalers[rate];
		double syncRequency = frequency * scaler;
		phaseInc = syncRequency / (Helpers::CurrentSampleRate);
	}
	  
	void LFO::SetRate(int rate, double songPosition)
	{
		double frequency = Helpers::CurrentTempo > 0.0f ? 60.0f / Helpers::CurrentTempo : 60.0f / 120.0f;
		double scaler = 1.0f / NoteDivisionScalers[rate];
		double syncRequency = frequency * scaler;
		phaseInc = syncRequency / Helpers::CurrentSampleRate;
		double p = songPosition * scaler;
		phase = p - floor(p);
	}

	void LFO::SetRatePercent(float percent)
	{
		ratePercent = percent;
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
		phase += (phaseInc * ratePercent);
		if (phase >= 1.0f)
		{
			phase -= 1.0f;
		}

		return output;
	}

}
