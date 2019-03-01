#ifndef __WAVESABRECORE_GLITCHER_H__
#define __WAVESABRECORE_GLITCHER_H__

#include "VoiceEffect.h"
#include "Device.h"
#include "BiquadFilter.h"
#include "Envelope.h"
#include "RecordBuffer.h"

namespace WaveSabreCore
{
	class Glitcher : public VoiceEffect
	{
	public:
		enum class ParamIndices
		{
			GateAttack,
			GateDecay,
			GateSustain,
			GateRelease,

			NumParams,
		};

		enum class GlitchMode
		{
			DrySignal = -1,
			DryOn = 36,
			DryOff = 37,
			ManualGate = 38,
			AutoGate = 39,
			Reverse = 48,
		};

		Glitcher();

		const int ReverseBufferLength = 20 * 1000;

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	protected:
		class GlitcherVoice : public Voice
		{
		public:
			GlitcherVoice(Glitcher *glitcher);

			virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			Glitcher *glitcher;

			GlitchMode glitchMode;

			Envelope gateEnv;
		};
	private:
		float lowCutFreq, lowCutQ;
		float peak1Freq, peak1Gain, peak1Q;
		float peak2Freq, peak2Gain, peak2Q;
		float peak3Freq, peak3Gain, peak3Q;
		float highCutFreq, highCutQ;
		float master;

		bool dryActive;
		float gateAttack, gateDecay, gateSustain, gateRelease;

		RecordBuffer reverseLeft;
		RecordBuffer reverseRight;

		BiquadFilter highpass[2];
		BiquadFilter peak1[2];
		BiquadFilter peak2[2];
		BiquadFilter peak3[2];
		BiquadFilter lowpass[2];
	};
}

#endif
