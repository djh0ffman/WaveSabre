#include <WaveSabreCore/RecordBuffer.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	RecordBuffer::RecordBuffer(float lengthMs)
	{
		buffer = nullptr;
		SetLength(lengthMs);
		Complete = false;
	}

	RecordBuffer::~RecordBuffer()
	{
		if (buffer) delete[] buffer;
	}

	void RecordBuffer::SetLength(float lengthMs)
	{
		int newLength = (int)((double)lengthMs * Helpers::CurrentSampleRate / 1000.0);
		if (newLength < 1) newLength = 1;
		if (newLength != length || !buffer)
		{
			auto newBuffer = new float[newLength];
			auto oldBuffer = buffer;
			buffer = newBuffer;
			length = newLength;
			if (oldBuffer) delete[] oldBuffer;
		}
		for (int i = 0; i < newLength; i++) buffer[i] = 0.0f;
		currentPosition = 0;
		Complete = false;
	}

	void RecordBuffer::WriteSample(float sample)
	{
		if (!Complete)
		{
			buffer[currentPosition] = sample;
			currentPosition++;
			if (currentPosition == length)
				Complete = true;
		}
	}

	int RecordBuffer::GetLength() const
	{
		return length;
	}

	float *RecordBuffer::GetBuffer() const
	{
		return buffer;
	}
}