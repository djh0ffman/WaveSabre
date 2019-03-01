#ifndef __WAVESABRECORE_REVERSEBUFFER_H__
#define __WAVESABRECORE_REVERSEBUFFER_H__

namespace WaveSabreCore
{
	class ReverseBuffer
	{
	public:
		ReverseBuffer(float lengthMs = 0.0f);
		~ReverseBuffer();

		void SetLength(float lengthMs);

		void WriteSample(float sample);
		void Trigger();
		float ReadSample();

	private:
		int length;
		float *buffer;
		int currentPosition;
		int playbackPostion;
	};
}

#endif