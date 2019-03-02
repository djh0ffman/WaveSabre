#ifndef __WAVESABRECORE_GLITCHER_H__
#define __WAVESABRECORE_GLITCHER_H__

#include "VoiceEffect.h"
#include "Device.h"
#include "BiquadFilter.h"
#include "Envelope.h"
#include "ReverseBuffer.h"
#include "LFO.h"

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

			FilterFreq,
			FilterLfoAmount,
			FilterLfoRateAdjust,
			FilterWave,

			NumParams,
		};

		enum class GlitchMode
		{
			DrySignal = -1,

			// 1st octave
			DryOn = 36,
			DryOff = 37,
			ManualGate = 38,
			AutoGate = 39,
			
			// 2nd octave
			Reverse = 48,

			// 3rd octave
			AutoFilter = 60,
		};

		Glitcher();

		const float ReverseBufferLength = 21 * 1000;

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
			BiquadFilter filter[2];
			LFO lfo;
		};
	private:
		float master;

		bool dryActive;
		float gateAttack, gateDecay, gateSustain, gateRelease;
		
		LFOWave filterWave;
		float filterFreq;
		float filterLfoAmount;
		float filterLfoRateAdjust;

		ReverseBuffer reverseLeft;
		ReverseBuffer reverseRight;
	};
}

#endif
