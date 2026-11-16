#include <switch.h>
#include "ScreenReaderDebug.hpp"
#include "EspeakEngine.hpp"
#include "../main_util.h"

#include <mutex>

static std::vector<int16_t>* g_buffer = nullptr;
static std::mutex g_espeakMutex;
static std::mutex g_synthesisGateMutex;
static bool g_initialized = false;
static bool g_synthesisBlocked = false;
static int g_sampleRate = 0;
#if SCREENREADER_DEBUG
static int g_callbackCount = 0;
static int g_callbackSamples = 0;
#endif

bool EspeakEngine::Initialize()
{
	std::lock_guard<std::mutex> lock(g_espeakMutex);
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Initialize: début.\n");
#endif

	if (g_initialized)
	{
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Initialize: moteur déjà initialisé (fréquence=%d).\n", g_sampleRate);
#endif
		return true;
	}

	g_sampleRate = espeak_Initialize(
		AUDIO_OUTPUT_RETRIEVAL,
		0,
		"romfs:/espeak-ng-data",
		0);

	if (g_sampleRate <= 0)
	{
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Initialize: espeak_Initialize a échoué (résultat=%d).\n", g_sampleRate);
#endif
		return false;
	}

	{
		std::lock_guard<std::mutex> gateLock(g_synthesisGateMutex);
		g_synthesisBlocked = false;
	}

	espeak_SetSynthCallback(Callback);
	g_initialized = true;
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Initialize: OK (fréquence=%d).\n", g_sampleRate);
#endif
	return true;
}

void EspeakEngine::Shutdown()
{
	std::lock_guard<std::mutex> lock(g_espeakMutex);
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Shutdown: début.\n");
#endif

	if (!g_initialized)
	{
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Shutdown: moteur déjà arrêté.\n");
#endif
		return;
	}

	g_buffer = nullptr;
	espeak_Terminate();
	g_initialized = false;
	g_sampleRate = 0;
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Shutdown: terminé.\n");
#endif
}

void EspeakEngine::Cancel()
{
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Cancel: demande d'annulation (initialisé=%d).\n", g_initialized ? 1 : 0);
#endif
	if (g_initialized)
	{
		SCREENREADER_DEBUG_RESULT(result, espeak_Cancel());
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Cancel: espeak_Cancel = %d.\n", (int)result);
#endif
	}
}

void EspeakEngine::BlockSynthesis()
{
	std::lock_guard<std::mutex> lock(g_synthesisGateMutex);
	g_synthesisBlocked = true;
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::BlockSynthesis: nouvelles synthèses bloquées.\n");
#endif
}

void EspeakEngine::AllowSynthesis()
{
	std::lock_guard<std::mutex> lock(g_synthesisGateMutex);
	if (g_initialized)
		g_synthesisBlocked = false;
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::AllowSynthesis: synthèses autorisées (initialisé=%d).\n", g_initialized ? 1 : 0);
#endif
}

int EspeakEngine::Callback(short* wav, int numsamples, espeak_EVENT*)
{
	if (!g_buffer || !wav || numsamples <= 0)
		return 0;

	g_buffer->insert(g_buffer->end(), wav, wav + numsamples);
#if SCREENREADER_DEBUG
	++g_callbackCount;
	g_callbackSamples += numsamples;
#endif
	return 0;
}

bool EspeakEngine::Synthesize(const std::string& text,
                              std::vector<int16_t>& outPcm,
                              int& outSampleRate,
                              bool allowWhenBlocked)
{
	std::lock_guard<std::mutex> lock(g_espeakMutex);
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: début (longueur=%d).\n", (int)text.size());
#endif

	outPcm.clear();
	outSampleRate = g_sampleRate;

	if (!g_initialized || text.empty())
	{
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: paramètres invalides (initialisé=%d, longueur=%d).\n",
			g_initialized ? 1 : 0, (int)text.size());
#endif
		return false;
	}

	std::unique_lock<std::mutex> gateLock(g_synthesisGateMutex);
	if (g_synthesisBlocked && !allowWhenBlocked) {
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: synthèse refusée par le verrou d'accessibilité.\n");
#endif
		return false;
	}

	g_buffer = &outPcm;
#if SCREENREADER_DEBUG
	g_callbackCount = 0;
	g_callbackSamples = 0;
#endif

	const espeak_ERROR synthResult = espeak_Synth(
		text.c_str(),
		text.size() + 1,
		0,
		POS_CHARACTER,
		0,
		espeakCHARS_UTF8,
		nullptr,
		nullptr);
	gateLock.unlock();
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: espeak_Synth = %d.\n", (int)synthResult);
#endif

	espeak_ERROR syncResult = EE_INTERNAL_ERROR;
	if (synthResult == EE_OK)
	{
		syncResult = espeak_Synchronize();
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: espeak_Synchronize = %d.\n", (int)syncResult);
#endif
	}

	g_buffer = nullptr;
	const bool success = synthResult == EE_OK && syncResult == EE_OK && !outPcm.empty();
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: fin (succès=%d, callbacks=%d, échantillons_callback=%d, PCM=%d, fréquence=%d).\n",
		success ? 1 : 0, g_callbackCount, g_callbackSamples, (int)outPcm.size(), outSampleRate);
#endif
	return success;
}
