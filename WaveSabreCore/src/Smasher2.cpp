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
		case ParamIndices::Threshold: threshold = Helpers::ParamToDbScale(value, 0.000976563, 1.0); break;
		case ParamIndices::Attack: attack = Helpers::ParamToLogScale(value, 0.01, 2000); break;
		case ParamIndices::Release: release = Helpers::ParamToLogScale(value, 0.01, 2000); break;
		case ParamIndices::Ratio: ratio = Helpers::ParamToLogScale(value, 1.0, 20); break;
		case ParamIndices::OutputGain: outputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Knee: knee = Helpers::ParamToDbScale(value, 1.0, 8.0); break;
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
		case ParamIndices::Threshold: return Helpers::DbScaleToParam(threshold, 0.000976563, 1.0);
		case ParamIndices::Attack: return Helpers::LogScaleToParam(attack, 0.01, 2000);
		case ParamIndices::Release: return Helpers::LogScaleToParam(release, 0.01, 2000);
		case ParamIndices::Ratio: return Helpers::LogScaleToParam(ratio, 1.0, 2.0);
		case ParamIndices::OutputGain: return Helpers::DbToParam(outputGain, 12.0f);
		case ParamIndices::Knee: return Helpers::DbScaleToParam(knee, 1.0, 8.0);
		case ParamIndices::Rms: return Helpers::BooleanToParam(rms);
		}
		return 0.0f;
	}
}
