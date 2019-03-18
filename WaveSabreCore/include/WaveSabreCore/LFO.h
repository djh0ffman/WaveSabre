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

	class LFO
	{
	public:
		LFO();

		LFOWave LFOWave;

		void SetTempoRate(int rate);
		void SetFreeRate(double rate);
		void Trigger(float startPhase);

		float Next();

	private:

		double phase;
		double phaseInc;
	};
}

#endif
