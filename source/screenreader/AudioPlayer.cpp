#include <switch.h>
#include "ScreenReaderDebug.hpp"
#include "AudioPlayer.hpp"
#include "../main_util.h"

#include <cstring>
#include <cstdlib>
#include <malloc.h>
#include <mutex>

static AudioDriver driver;
static int voiceId = 0;
static int sinkId = -1;
static int playbackMemPoolId = -1;
static bool init = false;
static bool rendererInitialized = false;
static bool rendererStarted = false;
static std::mutex audioMutex;
static int16_t* playbackBuffer = nullptr;
static size_t playbackBufferSize = 0;
static size_t playbackBufferAllocSize = 0;
static AudioDriverWaveBuf waveBuffer{};
static int outputSampleRate = 32000;
static const u8 finalMixChannels[] = {0, 1};

static void freePlaybackMemory()
{
	if (playbackBuffer)
		free(playbackBuffer);

	playbackBuffer = nullptr;
	playbackBufferSize = 0;
	playbackBufferAllocSize = 0;
}

static bool releasePlaybackMemoryPool()
{
	if (playbackMemPoolId >= 0)
	{
		AudioRendererMemPoolState state = driver.in_mempools[playbackMemPoolId].state;

		if (state == AudioRendererMemPoolState_Attached || state == AudioRendererMemPoolState_RequestAttach)
		{
			if (!audrvMemPoolDetach(&driver, playbackMemPoolId))
			{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer: échec du détachement du mempool (id=%d, état=%d).\n",
					playbackMemPoolId, (int)state);
#endif
				return false;
			}
			state = driver.in_mempools[playbackMemPoolId].state;
		}

		if (state == AudioRendererMemPoolState_RequestDetach)
		{
			const Result result = audrvUpdate(&driver);
			if (R_FAILED(result))
			{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer: audrvUpdate pendant le détachement du mempool a échoué (0x%08X).\n",
					(unsigned int)result);
#endif
				return false;
			}
			state = driver.in_mempools[playbackMemPoolId].state;

			/* A running renderer may need another frame to acknowledge the detach. */
			if (state == AudioRendererMemPoolState_RequestDetach && rendererStarted)
			{
				for (int frame = 0; frame < 4 && state == AudioRendererMemPoolState_RequestDetach; ++frame)
				{
					const Result waitResult = eventWait(audrenGetFrameEvent(), 100000000ULL);
					if (R_FAILED(waitResult))
					{
#if SCREENREADER_DEBUG
						debug_log_write("AudioPlayer: attente du détachement interrompue (frame=%d, rc=0x%08X, état=%d).\n",
							frame + 1, (unsigned int)waitResult, (int)state);
#endif
						break;
					}

					const Result updateResult = audrvUpdate(&driver);
					if (R_FAILED(updateResult))
					{
#if SCREENREADER_DEBUG
						debug_log_write("AudioPlayer: mise à jour pendant l'attente du détachement échouée (frame=%d, rc=0x%08X).\n",
							frame + 1, (unsigned int)updateResult);
#endif
						break;
					}

					state = driver.in_mempools[playbackMemPoolId].state;
#if SCREENREADER_DEBUG
					debug_log_write("AudioPlayer: état du mempool après frame de détachement %d = %d.\n",
						frame + 1, (int)state);
#endif
				}
			}
		}

		if (state != AudioRendererMemPoolState_Detached && state != AudioRendererMemPoolState_New && state != AudioRendererMemPoolState_Released)
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer: mempool toujours utilisé, conservation du buffer (id=%d, état=%d).\n",
				playbackMemPoolId, (int)state);
#endif
			return false;
		}

		if (state != AudioRendererMemPoolState_Released && !audrvMemPoolRemove(&driver, playbackMemPoolId))
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer: audrvMemPoolRemove a échoué (id=%d, état=%d).\n",
				playbackMemPoolId, (int)state);
#endif
			return false;
		}

#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer: mempool libéré (id=%d).\n", playbackMemPoolId);
#endif
		playbackMemPoolId = -1;
	}

	freePlaybackMemory();
	return true;
}

bool AudioPlayer::Initialize(AudioRendererOutputRate sampleRate)
{
	std::lock_guard<std::mutex> lock(audioMutex);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: début (fréquence=%d).\n", (int)sampleRate);
#endif

	if (init)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: déjà initialisé.\n");
#endif
		return true;
	}

	outputSampleRate = sampleRate == AudioRendererOutputRate_48kHz ? 48000 : 32000;

	AudioRendererConfig config;
	memset(&config, 0, sizeof(config));
	config.output_rate = sampleRate;
	config.num_voices = 1;
	config.num_sinks = 1;
	config.num_mix_objs = 1;
	config.num_mix_buffers = 2;

	Result result = audrenInitialize(&config);
	if (R_FAILED(result))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: audrenInitialize a échoué (0x%08X).\n", (unsigned int)result);
#endif
		return false;
	}
	rendererInitialized = true;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: AudioRenderer initialisé (%d Hz).\n", outputSampleRate);
#endif

	result = audrvCreate(&driver, &config, 2);
	if (R_FAILED(result))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: audrvCreate a échoué (0x%08X).\n", (unsigned int)result);
#endif
		audrenExit();
		rendererInitialized = false;
		return false;
	}

	voiceId = 0;
	if (!audrvVoiceInit(&driver, voiceId, 1, PcmFormat_Int16, outputSampleRate))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: audrvVoiceInit a échoué (fréquence=%d).\n", outputSampleRate);
#endif
		audrvClose(&driver);
		audrenExit();
		rendererInitialized = false;
		return false;
	}

	audrvVoiceSetDestinationMix(&driver, voiceId, AUDREN_FINAL_MIX_ID);
	audrvVoiceSetMixFactor(&driver, voiceId, 1.0f, 0, 0);
	audrvVoiceSetMixFactor(&driver, voiceId, 1.0f, 0, 1);
	audrvVoiceSetVolume(&driver, voiceId, 1.0f);

	sinkId = audrvDeviceSinkAdd(
		&driver,
		AUDREN_DEFAULT_DEVICE_NAME,
		2,
		finalMixChannels);
	if (sinkId < 0)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: audrvDeviceSinkAdd a échoué.\n");
#endif
		audrvVoiceDrop(&driver, voiceId);
		audrvClose(&driver);
		audrenExit();
		rendererInitialized = false;
		return false;
	}
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: sortie audio créée (sink=%d, canaux=2).\n", sinkId);
#endif

	result = audrvUpdate(&driver);
	if (R_FAILED(result))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: audrvUpdate initial a échoué (0x%08X).\n", (unsigned int)result);
#endif
		audrvVoiceDrop(&driver, voiceId);
		audrvClose(&driver);
		audrenExit();
		sinkId = -1;
		rendererInitialized = false;
		return false;
	}
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: configuration AudioRenderer publiée.\n");
#endif

	rendererStarted = false;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: démarrage AudioRenderer différé avant la première lecture du buffer PCM.\n");
#endif

	init = true;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Initialize: OK (sortie=%d Hz).\n", outputSampleRate);
#endif
	return true;
}

void AudioPlayer::Exit()
{
	std::lock_guard<std::mutex> lock(audioMutex);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Exit: début (initialisé=%d).\n", init ? 1 : 0);
#endif

	if (!init)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Exit: déjà arrêté.\n");
#endif
		return;
	}

	audrvVoiceStop(&driver, voiceId);
	// Keep the AudioRenderer update even when debug logging is disabled.
#if SCREENREADER_DEBUG
	Result result = audrvUpdate(&driver);
debug_log_write("AudioPlayer::Exit: audrvUpdate après arrêt = 0x%08X.\n", (unsigned int)result);
#else
	audrvUpdate(&driver);
#endif

	if (rendererStarted)
	{
#if SCREENREADER_DEBUG
		Result result = audrenStopAudioRenderer();
debug_log_write("AudioPlayer::Exit: audrenStopAudioRenderer = 0x%08X.\n", (unsigned int)result);
#else
		audrenStopAudioRenderer();
#endif
	}

	audrvClose(&driver);
	audrenExit();
	playbackMemPoolId = -1;
	freePlaybackMemory();
	init = false;
	rendererInitialized = false;
	rendererStarted = false;
	sinkId = -1;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Exit: terminé.\n");
#endif
}

void AudioPlayer::Play(const int16_t* samples, size_t count, int inputSampleRate, const std::atomic<bool>* cancelRequested)
{
	if (!samples || count == 0 || inputSampleRate <= 0)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: paramètres invalides (samples=%d, count=%d, fréquence=%d).\n",
			samples ? 1 : 0, (int)count, inputSampleRate);
#endif
		return;
	}

	std::lock_guard<std::mutex> lock(audioMutex);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: début (échantillons=%d, fréquence_entrée=%d, fréquence_sortie=%d, initialisé=%d).\n",
		(int)count, inputSampleRate, outputSampleRate, init ? 1 : 0);
#endif

	if (!init || !rendererInitialized)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: moteur audio non initialisé.\n");
#endif
		return;
	}

	if (cancelRequested && cancelRequested->load())
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: lecture annulée avant préparation.\n");
#endif
		return;
	}

	audrvVoiceStop(&driver, voiceId);
	Result result = audrvUpdate(&driver);
	if (R_FAILED(result))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvUpdate avant lecture a échoué (0x%08X).\n", (unsigned int)result);
#endif
		return;
	}

	if (!releasePlaybackMemoryPool())
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: impossible de libérer le précédent buffer audio.\n");
#endif
		return;
	}

	playbackBufferSize = inputSampleRate == outputSampleRate
		? count
		: (count * (size_t)outputSampleRate) / (size_t)inputSampleRate;

	if (playbackBufferSize == 0 || playbackBufferSize > 0x7FFFFFFF)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: taille PCM de sortie invalide (%d échantillons).\n", (int)playbackBufferSize);
#endif
		playbackBufferSize = 0;
		return;
	}

	const size_t byteCount = playbackBufferSize * sizeof(int16_t);
	if (byteCount > (size_t)-1 - 0xFFF)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: taille PCM trop importante (%d octets).\n", (int)byteCount);
#endif
		playbackBufferSize = 0;
		return;
	}

	playbackBufferAllocSize = (byteCount + 0xFFF) & ~(size_t)0xFFF;
	playbackBuffer = (int16_t*)memalign(0x1000, playbackBufferAllocSize);

	if (!playbackBuffer)
	{
		playbackBufferSize = 0;
		playbackBufferAllocSize = 0;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: échec allocation PCM (%d octets).\n", (int)byteCount);
#endif
		return;
	}
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: buffer PCM alloué (échantillons=%d, octets=%d, allocation=%d).\n",
		(int)playbackBufferSize, (int)byteCount, (int)playbackBufferAllocSize);
#endif

	memset(playbackBuffer, 0, playbackBufferAllocSize);

	if (inputSampleRate == outputSampleRate)
	{
		memcpy(playbackBuffer, samples, byteCount);
	}
	else
	{
		for (size_t i = 0; i < playbackBufferSize; i++)
		{
			const uint64_t sourcePosition = (uint64_t)i * (uint64_t)inputSampleRate;
			const size_t sourceIndex = (size_t)(sourcePosition / (uint64_t)outputSampleRate);
			const uint32_t fraction = (uint32_t)(sourcePosition % (uint64_t)outputSampleRate);
			const size_t nextIndex = sourceIndex + 1 < count ? sourceIndex + 1 : sourceIndex;

			const int32_t first = samples[sourceIndex];
			const int32_t second = samples[nextIndex];
			const int32_t interpolated = first + (int32_t)(((int64_t)(second - first) * fraction) / outputSampleRate);
			playbackBuffer[i] = (int16_t)interpolated;
		}
	}

	armDCacheFlush(playbackBuffer, playbackBufferAllocSize);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: cache PCM vidé (%d octets).\n", (int)playbackBufferAllocSize);
#endif

	playbackMemPoolId = audrvMemPoolAdd(&driver, playbackBuffer, playbackBufferAllocSize);
	if (playbackMemPoolId < 0)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvMemPoolAdd a échoué (taille=%d).\n", (int)playbackBufferAllocSize);
#endif
		freePlaybackMemory();
		return;
	}

	if (!audrvMemPoolAttach(&driver, playbackMemPoolId))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvMemPoolAttach a échoué (id=%d).\n", playbackMemPoolId);
#endif
		releasePlaybackMemoryPool();
		return;
	}

	result = audrvUpdate(&driver);
	if (R_FAILED(result))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvUpdate après attachement du mempool a échoué (0x%08X).\n", (unsigned int)result);
#endif
		releasePlaybackMemoryPool();
		return;
	}

	const AudioRendererMemPoolState memPoolState = driver.in_mempools[playbackMemPoolId].state;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: mempool attaché (id=%d, état=%d).\n", playbackMemPoolId, (int)memPoolState);
#endif
	if (memPoolState != AudioRendererMemPoolState_Attached)
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: état du mempool inattendu après attachement.\n");
#endif
		releasePlaybackMemoryPool();
		return;
	}

	if (cancelRequested && cancelRequested->load())
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: lecture annulée après attachement du mempool.\n");
#endif
		releasePlaybackMemoryPool();
		return;
	}

	if (!rendererStarted)
	{
		result = audrenStartAudioRenderer();
		if (R_FAILED(result))
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrenStartAudioRenderer a échoué (0x%08X).\n", (unsigned int)result);
#endif
			releasePlaybackMemoryPool();
			return;
		}
		rendererStarted = true;
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: AudioRenderer démarré après attachement du mempool.\n");
#endif
	}

	waveBuffer = {};
	waveBuffer.data_pcm16 = playbackBuffer;
	waveBuffer.size = byteCount;
	waveBuffer.start_sample_offset = 0;
	waveBuffer.end_sample_offset = (s32)playbackBufferSize;
	waveBuffer.state = AudioDriverWaveBufState_Free;

	if (!audrvVoiceAddWaveBuf(&driver, voiceId, &waveBuffer))
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvVoiceAddWaveBuf a échoué.\n");
#endif
		releasePlaybackMemoryPool();
		return;
	}
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: wave buffer ajouté (état=%d, taille=%d, début=%d, fin=%d).\n",
		(int)waveBuffer.state, (int)waveBuffer.size,
		(int)waveBuffer.start_sample_offset, (int)waveBuffer.end_sample_offset);
#endif

	if (cancelRequested && cancelRequested->load())
	{
		audrvVoiceStop(&driver, voiceId);
		result = audrvUpdate(&driver);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: lecture annulée avant démarrage (update=0x%08X).\n", (unsigned int)result);
#endif
		releasePlaybackMemoryPool();
		return;
	}

	audrvVoiceStart(&driver, voiceId);
	result = audrvUpdate(&driver);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvUpdate après démarrage = 0x%08X.\n", (unsigned int)result);
#endif
	if (R_FAILED(result))
	{
		audrvVoiceStop(&driver, voiceId);
		audrvUpdate(&driver);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: lecture annulée après échec de publication.\n");
#endif
		releasePlaybackMemoryPool();
		return;
	}

#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: lecture publiée (état_buffer=%d, playing=%d, sortie=%d Hz).\n",
		(int)waveBuffer.state, audrvVoiceIsPlaying(&driver, voiceId) ? 1 : 0, outputSampleRate);
#endif

	AudioDriverWaveBufState previousState = waveBuffer.state;
	bool playbackCancelled = false;
	bool updateFailed = false;
	size_t renderedFrames = 0;
	const size_t samplesPerFrame = outputSampleRate == 32000
		? AUDREN_SAMPLES_PER_FRAME_32KHZ
		: AUDREN_SAMPLES_PER_FRAME_48KHZ;
	const size_t expectedFrames = (playbackBufferSize + samplesPerFrame - 1) / samplesPerFrame;
	const size_t maxFrames = expectedFrames + 400;

	while (waveBuffer.state != AudioDriverWaveBufState_Done)
	{
		if (cancelRequested && cancelRequested->load())
		{
			playbackCancelled = true;
			break;
		}

		result = eventWait(audrenGetFrameEvent(), 100000000ULL);
		if (R_FAILED(result))
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: attente de frame AudioRenderer échouée (0x%08X).\n", (unsigned int)result);
#endif
			updateFailed = true;
			break;
		}

		if (++renderedFrames > maxFrames)
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: délai de lecture dépassé (frames=%d, attendues=%d).\n",
				(int)renderedFrames, (int)expectedFrames);
#endif
			updateFailed = true;
			break;
		}

		result = audrvUpdate(&driver);
		if (R_FAILED(result))
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: audrvUpdate pendant lecture a échoué (0x%08X).\n", (unsigned int)result);
#endif
			updateFailed = true;
			break;
		}

		if (waveBuffer.state != previousState)
		{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: état du wave buffer %d -> %d (échantillons lus=%d).\n",
				(int)previousState, (int)waveBuffer.state,
				(int)audrvVoiceGetPlayedSampleCount(&driver, voiceId));
#endif
			previousState = waveBuffer.state;
		}
	}

	if (playbackCancelled || updateFailed)
	{
		audrvVoiceStop(&driver, voiceId);
		result = audrvUpdate(&driver);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: voix arrêtée après interruption (annulation=%d, update=0x%08X).\n",
			playbackCancelled ? 1 : 0, (unsigned int)result);
#endif
	}
	else
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: buffer consommé (état=%d, échantillons lus=%d, séquence=%d, drops=%d).\n",
			(int)waveBuffer.state,
			(int)audrvVoiceGetPlayedSampleCount(&driver, voiceId),
			(int)audrvVoiceGetWaveBufSeq(&driver, voiceId),
			(int)audrvVoiceGetVoiceDropsCount(&driver, voiceId));
#endif
	}

	if (!releasePlaybackMemoryPool())
	{
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Play: buffer conservé car le mempool n'a pas pu être détaché.\n");
#endif
	}
}

void AudioPlayer::Stop()
{
	std::lock_guard<std::mutex> lock(audioMutex);
#if SCREENREADER_DEBUG
debug_log_write("AudioPlayer::Stop: demande d'arrêt (initialisé=%d).\n", init ? 1 : 0);
#endif

	if (init)
	{
		audrvVoiceStop(&driver, voiceId);
#if SCREENREADER_DEBUG
		const Result result = audrvUpdate(&driver);
debug_log_write("AudioPlayer::Stop: voix arrêtée, audrvUpdate = 0x%08X.\n", (unsigned int)result);
#else
		audrvUpdate(&driver);
#endif
	}
}
