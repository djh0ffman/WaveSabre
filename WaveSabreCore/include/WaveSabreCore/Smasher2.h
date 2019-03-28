#ifndef __WAVESABRECORE_SMASHER2_H__
#define __WAVESABRECORE_SMASHER2_H__

#include "Device.h"
#include "DelayBuffer.h"
#include "GainReduction.h"

namespace WaveSabreCore
{
	class Smasher2 : public Device
	{
	public:
		enum class ParamIndices
		{
			Sidechain,
			InputGain,
			Threshold,
			Ratio,
			Attack,
			Release,
			OutputGain,
			Knee,
			Rms,

			NumParams,
		};

		Smasher2();
		virtual ~Smasher2();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		const static float lookaheadMs;

		

		bool sidechain;
		float inputGain;
		float threshold, ratio, knee;
		float attack, release;
		float outputGain;
		bool rms;

		GainReduction compressor;

		DelayBuffer leftBuffer;
		DelayBuffer rightBuffer;
		float peak;
	};
}

#endif
