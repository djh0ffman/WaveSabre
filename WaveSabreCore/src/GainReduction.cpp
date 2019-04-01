#include <WaveSabreCore/GainReduction.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	GainReduction::GainReduction()
	{
		recalc = true;

		linSlope = 0.0f;
		attack = 20.0f;
		release = 250.0f;
		threshold = 0.125;
		ratio = 2.0;
		knee = 2.828427125;
		rms = true;
	}

	GainReduction::~GainReduction()
	{

	}

	void GainReduction::SetAttack(float attack)
	{
		this->attack = attack;
	}

	void GainReduction::SetRelease(float release)
	{
		this->release = release;
	}

	void GainReduction::SetThreshold(float threshold)
	{
		if (this->threshold != threshold)
		{
			this->threshold = threshold;
			recalc = true;
		}
	}

	void GainReduction::SetRatio(float ratio)
	{
		if (this->ratio != ratio)
		{
			this->ratio = ratio;
			recalc = true;
		}
	}

	void GainReduction::SetKnee(float knee)
	{
		if (this->knee != knee)
		{
			this->knee = knee;
			recalc = true;
		}
	}

	void GainReduction::SetRms(bool rms)
	{
		this->rms = rms;
	}

	void GainReduction::updateCurve()
	{
		float linThreshold = threshold;
		float linKneeSqrt = sqrt(knee);
		linKneeStart = linThreshold / linKneeSqrt;
		adjKneeStart = linKneeStart*linKneeStart;
		float linKneeStop = linThreshold * linKneeSqrt;
		thres = log(linThreshold);
		kneeStart = log(linKneeStart);
		kneeStop = log(linKneeStop);
		compressedKneeStop = (kneeStop - thres) / ratio + thres;
	}

	void GainReduction::Process(float &left, float &right, float &detLeft, float &detRight)
	{
		if (recalc)
		{
			updateCurve();
			recalc = false;
		}

		float gain = 1.0f;
		
		bool average = false;
		float attack_coeff = Helpers::Min(1.f, 1.f / (attack * (float)Helpers::CurrentSampleRate / 4000.f));
		float release_coeff = Helpers::Min(1.f, 1.f / (release * (float)Helpers::CurrentSampleRate / 4000.f));
		
		float absample = average ? (fabs(detLeft) + fabs(detRight)) * 0.5f : Helpers::Max(fabs(detLeft), fabs(detRight));
		if (rms) absample *= absample;

		linSlope += (absample - linSlope) * (absample > linSlope ? attack_coeff : release_coeff);

		if (linSlope > 0.f) 
		{
			gain = outputGain(linSlope);
		}
		left *= gain;
		right *= gain;
	}

	float GainReduction::outputGain(float linSlope) const
	{
		//this calculation is also thor's work
		if (linSlope > (rms ? adjKneeStart : linKneeStart)) 
		{
			float slope = log(linSlope);
			if (rms) slope *= 0.5f;

			float gain = 0.f;
			float delta = 0.f;

			gain = (slope - thres) / ratio + thres;
			delta = 1.f / ratio;

			if (knee > 1.f && slope < kneeStop) 
			{
				gain = hermiteInterpolation(slope, kneeStart, kneeStop, kneeStart, compressedKneeStop, 1.f, delta);
			}

			return exp(gain - slope);
		}

		return 1.f;
	}

	/// Hermite interpolation between two points and slopes 
	/// @arg x point within interval (x0 <= x <= x1)
	/// @arg x0 interval start
	/// @arg x1 interval end
	/// @arg p0 value at x0
	/// @arg p1 value at x1
	/// @arg m0 slope (steepness, tangent) at x0
	/// @arg m1 slope at x1
	float GainReduction::hermiteInterpolation(float x, float x0, float x1, float p0, float p1, float m0, float m1) const
	{
		float width = x1 - x0;
		float t = (x - x0) / width;
		m0 *= width;
		m1 *= width;
		float t2 = t*t;
		float t3 = t2*t;

		float ct0 = p0;
		float ct1 = m0;
		float ct2 = -3 * p0 - 2 * m0 + 3 * p1 - m1;
		float ct3 = 2 * p0 + m0 - 2 * p1 + m1;

		return ct3 * t3 + ct2 * t2 + ct1 * t + ct0;
	}

}
