#include <switch.h>
#include "ScreenReaderDebug.hpp"
#include "MemoryDiagnostics.h"
#include "EspeakEngine.hpp"
#include "../main_util.h"

#include <mutex>
#include <new>
#include <atomic>

static std::vector<int16_t>* g_buffer = nullptr;
static std::mutex g_espeakMutex;
static std::mutex g_synthesisGateMutex;
static bool g_initialized = false;
static bool g_synthesisBlocked = false;
static int g_sampleRate = 0;
static std::atomic<bool> g_callbackAllocationFailed(false);
struct EspeakBufferReset
{
	~EspeakBufferReset()
	{
		g_buffer = nullptr;
	}
};
#if MEMORY_DIAGNOSTICS
static size_t g_callbackCount = 0;
static size_t g_callbackSamples = 0;
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

#if MEMORY_DIAGNOSTICS
	++g_callbackCount;
	g_callbackSamples += (size_t)numsamples;
	const size_t previousSize = g_buffer->size();
	const size_t previousCapacity = g_buffer->capacity();
	if (previousSize > previousCapacity || (size_t)numsamples > previousCapacity - previousSize) {
		const size_t requiredSamples = previousSize + (size_t)numsamples;
		debug_log_write("EspeakEngine::Callback: croissance PCM requise (callback=%d, bloc=%d samples, total_requis=%llu samples/%llu octets, taille_avant=%llu, capacité_avant=%llu).\n",
			(int)g_callbackCount, numsamples, (unsigned long long)requiredSamples,
			(unsigned long long)(requiredSamples * sizeof(int16_t)),
			(unsigned long long)previousSize, (unsigned long long)previousCapacity);
	}
#endif
	try {
		g_buffer->insert(g_buffer->end(), wav, wav + numsamples);
	} catch (const std::bad_alloc&) {
		g_callbackAllocationFailed.store(true);
#if MEMORY_DIAGNOSTICS
		debug_log_write("EspeakEngine::Callback: std::bad_alloc pendant insert PCM (callback=%d, samples=%d, taille=%d, capacité=%d).\n",
			(int)g_callbackCount, numsamples, (int)g_buffer->size(), (int)g_buffer->capacity());
#endif
		return 1;
	} catch (...) {
		g_callbackAllocationFailed.store(true);
#if MEMORY_DIAGNOSTICS
		debug_log_write("EspeakEngine::Callback: exception inconnue pendant insert PCM (callback=%d, samples=%d, taille=%d, capacité=%d).\n",
			(int)g_callbackCount, numsamples, (int)g_buffer->size(), (int)g_buffer->capacity());
#endif
		return 1;
	}
#if MEMORY_DIAGNOSTICS
	if (g_buffer->capacity() != previousCapacity)
		debug_log_write("EspeakEngine::Callback: réallocation PCM terminée (callback=%d, taille=%d, capacité_avant=%d, capacité_après=%d).\n",
			(int)g_callbackCount, (int)g_buffer->size(), (int)previousCapacity, (int)g_buffer->capacity());
#endif
	return 0;
}

bool EspeakEngine::Synthesize(const std::string& text,
                              std::vector<int16_t>& outPcm,
                              int& outSampleRate,
                              bool allowWhenBlocked)
{
#if SCREENREADER_DEBUG
	debug_log_write("EspeakEngine::Synthesize: avant acquisition de g_espeakMutex.\n");
#endif
	std::lock_guard<std::mutex> lock(g_espeakMutex);
#if SCREENREADER_DEBUG
	debug_log_write("EspeakEngine::Synthesize: g_espeakMutex acquis (longueur=%d).\n", (int)text.size());
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
#if SCREENREADER_DEBUG
	debug_log_write("EspeakEngine::Synthesize: g_synthesisGateMutex acquis (bloqué=%d, autorisé=%d).\n",
		g_synthesisBlocked ? 1 : 0, allowWhenBlocked ? 1 : 0);
#endif
	if (g_synthesisBlocked && !allowWhenBlocked) {
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: synthèse refusée par le verrou d'accessibilité.\n");
#endif
		return false;
	}

	g_buffer = &outPcm;
	EspeakBufferReset bufferReset;
	g_callbackAllocationFailed.store(false);
#if SCREENREADER_DEBUG
	debug_log_write("EspeakEngine::Synthesize: buffer PCM associé (adresse=%p, taille=%d, capacité=%d).\n",
		(void*)g_buffer, (int)outPcm.size(), (int)outPcm.capacity());
#endif
#if MEMORY_DIAGNOSTICS
	g_callbackCount = 0;
	g_callbackSamples = 0;
	MemoryDiagnosticsLogAllocator("avant espeak_Synth / callback PCM");
#endif
#if SCREENREADER_DEBUG
	debug_log_write("EspeakEngine::Synthesize: lancement espeak_Synth (texte=%d octets, buffer_initial=%d, capacité_initiale=%d).\n",
		(int)text.size(), (int)outPcm.size(), (int)outPcm.capacity());
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
	const bool callbackFailed = g_callbackAllocationFailed.load();
	const bool success = synthResult == EE_OK && syncResult == EE_OK && !outPcm.empty() && !callbackFailed;
#if SCREENREADER_DEBUG
debug_log_write("EspeakEngine::Synthesize: fin (succès=%d, PCM=%d, fréquence=%d).\n",
		success ? 1 : 0, (int)outPcm.size(), outSampleRate);
#endif
#if MEMORY_DIAGNOSTICS
debug_log_write("MemoryDiagnostics: synthèse terminée (succès=%d, callback_allocation_failed=%d, callbacks=%d, échantillons_callback=%d, PCM=%d, capacité=%d, synth_rc=%d, sync_rc=%d).\n",
		success ? 1 : 0, callbackFailed ? 1 : 0, (int)g_callbackCount, (int)g_callbackSamples,
		(int)outPcm.size(), (int)outPcm.capacity(), (int)synthResult, (int)syncResult);
MemoryDiagnosticsLogAllocator("après espeak_Synchronize / callback PCM");
#endif
	return success;
}
