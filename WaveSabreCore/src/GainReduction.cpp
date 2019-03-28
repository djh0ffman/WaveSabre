#include <WaveSabreCore/GainReduction.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	GainReduction::GainReduction()
	{
		linSlope = 0.f;
		attack = -1;
		release = -1;
		detection = -1;
		stereo_link = -1;
		threshold = -1;
		ratio = -1;
		knee = -1;
		makeup = -1;
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

	void GainReduction::UpdateCurve()
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

	void GainReduction::Process(float &left, float &right)
	{
		float gain = 1.f;
		
		bool rms = (detection == 0);
		bool average = (stereo_link == 0);
		float attack_coeff = Helpers::Min(1.f, 1.f / (attack * (float)Helpers::CurrentSampleRate / 4000.f));
		float release_coeff = Helpers::Min(1.f, 1.f / (release * (float)Helpers::CurrentSampleRate / 4000.f));
		
		float absample = average ? (fabs(left) + fabs(right)) * 0.5f : Helpers::Max(fabs(left), fabs(right));
		if (rms) absample *= absample;

		linSlope += (absample - linSlope) * (absample > linSlope ? attack_coeff : release_coeff);

		if (linSlope > 0.f) 
		{
			gain = outputGain(linSlope, rms);
		}
		left *= gain * makeup;
		right *= gain * makeup;

		// used for meter gui
		//meter_out = std::max(fabs(left), fabs(right));;
		//meter_comp = gain;
		//detected = rms ? sqrt(linSlope) : linSlope;
	}

	float GainReduction::outputGain(float linSlope, bool rms) const
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
