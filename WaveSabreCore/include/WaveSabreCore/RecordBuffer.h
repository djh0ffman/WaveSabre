#ifndef __WAVESABRECORE_RECORDBUFFER_H__
#define __WAVESABRECORE_RECORDBUFFER_H__

namespace WaveSabreCore
{
	class RecordBuffer
	{
	public:
		RecordBuffer(float lengthMs = 0.0f);
		~RecordBuffer();

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