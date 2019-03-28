#include <WaveSabreCore/Smasher2.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	const float Smasher2::lookaheadMs = 2.0f;

	Smasher2::Smasher2()
		: Device((int)ParamIndices::NumParams)
	{
		sidechain = false;
		inputGain = 0.0f;
		threshold = 0.0f;
		ratio = 2.0f;
		attack = 1.0f;
		release = 200.0f;
		outputGain = 0.0f;
		knee = 1.0f;
		rms = true;
	}

	Smasher2::~Smasher2()
	{
	}

	void Smasher2::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		leftBuffer.SetLength(lookaheadMs);
		rightBuffer.SetLength(lookaheadMs);

		compressor.SetAttack(attack);
		compressor.SetRelease(release);
		compressor.SetThreshold(threshold);
		compressor.SetRatio(ratio);
		compressor.SetKnee(knee);
		compressor.SetRms(rms);

		float inputGainScalar = Helpers::DbToScalar(inputGain);
		float outputGainScalar = Helpers::DbToScalar(outputGain);
		int inputChannelOffset = sidechain ? 2 : 0;

		float envCoeff = (float)(1000.0 / Helpers::CurrentSampleRate);
		float attackScalar = envCoeff / attack;
		float releaseScalar = envCoeff / release;

		float thresholdScalar = Helpers::DbToScalar(threshold);

		for (int i = 0; i < numSamples; i++)
		{
			leftBuffer.WriteSample(inputs[0][i] * inputGainScalar);
			rightBuffer.WriteSample(inputs[1][i] * inputGainScalar);

			float keyLeft = inputs[inputChannelOffset][i] * inputGainScalar;
			float keyRight = inputs[inputChannelOffset + 1][i] * inputGainScalar;
			float inputLeft = leftBuffer.ReadSample();
			float inputRight = rightBuffer.ReadSample();

			compressor.Process(inputLeft, inputRight, keyLeft, keyRight);

			outputs[0][i] = inputLeft * outputGainScalar;
			outputs[1][i] = inputRight * outputGainScalar;
		}
	}

	void Smasher2::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Sidechain: sidechain = Helpers::ParamToBoolean(value); break;
		case ParamIndices::InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Threshold: threshold = Helpers::ParamToDb(value / 2.0f, 36.0f); break;
		case ParamIndices::Attack: attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
		case ParamIndices::Release: release = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::Ratio: ratio = value * value * 18.0f + 2.0f; break;
		case ParamIndices::OutputGain: outputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Knee: knee = Helpers::ParamToDb(value / 2.0f, 36.0f); break;
		case ParamIndices::Rms: rms = Helpers::ParamToBoolean(value); break;
		}
	}

	float Smasher2::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Sidechain:
		default:
			return Helpers::BooleanToParam(sidechain);

		case ParamIndices::InputGain: return Helpers::DbToParam(inputGain, 12.0f);
		case ParamIndices::Threshold: return Helpers::DbToParam(threshold, 36.0f) * 2.0f;
		case ParamIndices::Attack: return Helpers::EnvValueToScalar(attack * 5.0f);
		case ParamIndices::Release: return Helpers::EnvValueToScalar(release);
		case ParamIndices::Ratio: return sqrtf((ratio - 2.0f) / 18.0f);
		case ParamIndices::OutputGain: return Helpers::DbToParam(outputGain, 12.0f);
		case ParamIndices::Knee: return Helpers::DbToParam(knee, 36.0f) * 2.0f;
		case ParamIndices::Rms: return Helpers::BooleanToParam(rms);
		}
		return 0.0f;
	}
}
