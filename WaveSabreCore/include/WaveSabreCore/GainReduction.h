#ifndef __WAVESABRECORE_GAINREDUCTION_H__
#define __WAVESABRECORE_GAINREDUCTION_H__

namespace WaveSabreCore
{
	class GainReduction
	{
	public:
		GainReduction();
		virtual ~GainReduction();
		
		void set_params(float att, float rel, float thr, float rat, float kn, float mak, float det, float stl, float byp, float mu);
		void SetAttack(float attack);
		void SetRelease(float release);

		void UpdateCurve();
		void Process(float &left, float &right);

	private:
		float linSlope, detected, kneeSqrt, kneeStart, linKneeStart, kneeStop;
		float compressedKneeStop, adjKneeStart, thres;
		float attack, release, threshold, ratio, knee, makeup, detection, stereo_link;

		mutable bool redraw_graph;
		bool is_active;
		inline float outputLevel(float slope) const;
		inline float outputGain(float linSlope, bool rms) const;
		inline float hermiteInterpolation(float x, float x0, float x1, float p0, float p1, float m0, float m1) const;
	};
}

#endif
