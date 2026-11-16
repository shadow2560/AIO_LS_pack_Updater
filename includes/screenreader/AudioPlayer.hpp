#pragma once

#include <switch.h>
#include <atomic>
#include <cstdint>
#include <cstddef>

class AudioPlayer
{
public:
	static bool Initialize(AudioRendererOutputRate sampleRate = AudioRendererOutputRate_32kHz);
	static void Exit();

	static void Play(const int16_t* samples, size_t count, int inputSampleRate, const std::atomic<bool>* cancelRequested = nullptr);
	static void Stop();
};
