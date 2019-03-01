#include <WaveSabreCore/ReverseBuffer.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	ReverseBuffer::ReverseBuffer(float lengthMs)
	{
		buffer = nullptr;
		SetLength(lengthMs);
	}

	ReverseBuffer::~ReverseBuffer()
	{
		if (buffer) delete[] buffer;
	}

	void ReverseBuffer::SetLength(float lengthMs)
	{
		int newLength = (int)((double)lengthMs * Helpers::CurrentSampleRate / 1000.0);
		if (newLength < 1) newLength = 1;
		if (newLength != length || !buffer)
		{
			auto newBuffer = new float[newLength];
			for (int i = 0; i < newLength; i++) newBuffer[i] = 0.0f;
			currentPosition = 0;
			auto oldBuffer = buffer;
			buffer = newBuffer;
			length = newLength;
			if (oldBuffer) delete[] oldBuffer;
		}
	}

	void ReverseBuffer::WriteSample(float sample)
	{
		buffer[currentPosition] = sample;
		currentPosition = (currentPosition + 1) % length;
	}

	void ReverseBuffer::Trigger()
	{
		playbackPostion = currentPosition;
	}

	float ReverseBuffer::ReadSample()
	{
		float sample = buffer[playbackPostion];
		playbackPostion = (playbackPostion - 1) % length;
		return sample;
	}
}