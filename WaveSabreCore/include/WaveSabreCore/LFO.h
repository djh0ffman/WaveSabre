#ifndef __WAVESABRECORE_LFO_H__
#define __WAVESABRECORE_LFO_H__

namespace WaveSabreCore
{
	enum class LFOWave
	{
		Sin,
		Square,
		SawUp,
		SawDown
	};

	const double NoteDivisionScalers[] = { 
		1.0 / 256.0 * 8.0,				// whole
		(1.0 / 128.0 * 8.0) * 2.0 / 3.0,	// tripplet
		1.0 / 128.0 * 8.0,
		(1.0 / 64.0 * 8.0) * 2.0 / 3.0,
		1.0 / 64.0 * 8.0,
		(1.0 / 32.0 * 8.0) * 2.0 / 3.0,
		1.0 / 32.0 * 8.0,
		(1.0 / 16.0 * 8.0) * 2.0 / 3.0,
		1.0 / 16.0 * 8.0,
		(1.0 / 8.0 * 8.0) * 2.0 / 3.0,
		1.0 / 8.0 * 8.0,
		(1.0 / 4.0 * 8.0) * 2.0 / 3.0,
		1.0 / 4.0 * 8.0,
		(1.0 / 2.0 * 8.0) * 2.0 / 3.0,
		1.0 / 2.0 * 8.0,
		(1.0 / 1.0 * 8.0) * 2.0 / 3.0 };

	class LFO
	{
	public:
		LFO();

		LFOWave LFOWave;

		void SetRate(int rate);
		void SetRate(int rate, double songPosition);
		void SetRatePercent(float percent);
		void Trigger(float startPhase);

		float Next();

	private:

		double phase;
		double phaseInc;
		double ratePercent;
	};
}

#endif
