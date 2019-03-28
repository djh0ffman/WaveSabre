#ifndef __WAVESABRECORE_GAINREDUCTION_H__
#define __WAVESABRECORE_GAINREDUCTION_H__

namespace WaveSabreCore
{
	class GainReduction
	{
	public:
		GainReduction();
		virtual ~GainReduction();
		
		void SetAttack(float attack);
		void SetRelease(float release);
		void SetThreshold(float threshold);
		void SetRatio(float ratio);
		void SetKnee(float knee);
		void SetRms(bool rms);

		void Process(float &left, float &right, float &detLeft, float &detRight);

	private:
		float linSlope, detected, kneeSqrt, kneeStart, linKneeStart, kneeStop;
		float compressedKneeStop, adjKneeStart, thres;
		float attack, release, threshold, ratio, knee;
		bool rms;

		bool recalc;
		void updateCurve();
		inline float outputGain(float linSlope) const;
		inline float hermiteInterpolation(float x, float x0, float x1, float p0, float p1, float m0, float m1) const;
	};
}

#endif
