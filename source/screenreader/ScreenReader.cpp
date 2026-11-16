#include <switch.h>
#include "ScreenReaderDebug.hpp"
#include "ScreenReader.hpp"
#include "EspeakEngine.hpp"
#include "AudioPlayer.hpp"
#include "../main_util.h"
#include "../hb_translate.hpp"
#include "ini.h"

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <chrono>

extern PadState pad;

namespace {
	typedef HomebrewLanguage LanguageChoice;
	static const LanguageChoice* languageChoices = homebrew_languages;
	static const int languageChoiceCount = (int)homebrew_language_count;
	static const int defaultRate = 175;
	static const int minimumRate = 80;
	static const int maximumRate = 450;
	static const int defaultPitch = 35;
	static const int defaultPitchRange = 50;
	static const int defaultWordGap = 0;
	static const int defaultPunctuation = espeakPUNCT_NONE;
	static const int defaultIntonation = 0;
	static const int defaultVoiceVariant = 0;
	static const char* voiceVariants[] = {
		"", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8",
		"f1", "f2", "f3", "f4", "f5"
	};
	static const int voiceVariantCount = sizeof(voiceVariants) / sizeof(voiceVariants[0]);
	static const int intonationModes[] = { 0, 2, 3, 4 };
	static const int intonationModeCount = sizeof(intonationModes) / sizeof(intonationModes[0]);
	static const char* configurationPath = "/switch/AIO_LS_pack_Updater/screenreader.ini";
	static const char* temporaryConfigurationPath = "/switch/AIO_LS_pack_Updater/screenreader.ini.tmp";
	static const char* backupConfigurationPath = "/switch/AIO_LS_pack_Updater/screenreader.ini.bak";
	static std::mutex configurationFileMutex;
	struct ReviewConsoleEntry {
		PrintConsole* console;
		std::string label;
	};
	static std::vector<ReviewConsoleEntry> reviewConsoles;
	static PrintConsole* virtualReviewPreviousConsole = nullptr;

	static void restoreVirtualReviewConsole(PrintConsole* reviewConsole) {
		if (!virtualReviewPreviousConsole)
			return;
		if (reviewConsole && get_active_console() == reviewConsole) {
			select_console(virtualReviewPreviousConsole);
			consoleUpdate(virtualReviewPreviousConsole);
		}
		virtualReviewPreviousConsole = nullptr;
	}

	static std::string normalizeLanguage(const std::string& value)
	{
		std::string result = value;
		for (size_t i = 0; i < result.size(); i++) {
			if (result[i] == '_')
				result[i] = '-';
			else if (result[i] >= 'A' && result[i] <= 'Z')
				result[i] = (char)(result[i] - 'A' + 'a');
		}
		return result;
	}

	struct ScreenReaderConfiguration {
		std::string enabled;
		std::string rate;
		std::string language;
		std::string announceNavigationHelp;
		std::string announceElementPosition;
		std::string pitch;
		std::string pitchRange;
		std::string wordGap;
		std::string punctuation;
		std::string intonation;
		std::string voiceVariant;
	};

	static int configurationHandler(void* user, const char* section,
		const char* name, const char* value, int lineno)
	{
		(void)lineno;
		if (!user || !section || !name || strcmp(section, "screenreader") != 0)
			return 1;

		ScreenReaderConfiguration* config = (ScreenReaderConfiguration*)user;
		const std::string parsedValue = value ? value : "";
		if (strcmp(name, "enabled") == 0)
			config->enabled = parsedValue;
		else if (strcmp(name, "rate") == 0)
			config->rate = parsedValue;
		else if (strcmp(name, "language") == 0)
			config->language = parsedValue;
		else if (strcmp(name, "announce_navigation_help") == 0)
			config->announceNavigationHelp = parsedValue;
		else if (strcmp(name, "announce_element_position") == 0)
			config->announceElementPosition = parsedValue;
		else if (strcmp(name, "pitch") == 0)
			config->pitch = parsedValue;
		else if (strcmp(name, "pitch_range") == 0)
			config->pitchRange = parsedValue;
		else if (strcmp(name, "word_gap") == 0)
			config->wordGap = parsedValue;
		else if (strcmp(name, "punctuation") == 0)
			config->punctuation = parsedValue;
		else if (strcmp(name, "intonation") == 0)
			config->intonation = parsedValue;
		else if (strcmp(name, "voice_variant") == 0)
			config->voiceVariant = parsedValue;
		return 1;
	}

	static bool parseBoolean(const std::string& value, bool defaultValue)
	{
		if (value == "0")
			return false;
		if (value == "1")
			return true;
		return defaultValue;
	}

	static bool parseInteger(const std::string& value, long& result)
	{
		if (value.empty())
			return false;
		errno = 0;
		char* end = nullptr;
		const long parsed = strtol(value.c_str(), &end, 10);
		if (end == value.c_str())
			return false;
		while (*end && isspace((unsigned char)*end))
			end++;
		if (*end)
			return false;
		if (errno == ERANGE)
			result = value[0] == '-' ? LONG_MIN : LONG_MAX;
		else
			result = parsed;
		return true;
	}

	static int normalizeRate(long value)
	{
		if (value < minimumRate)
			value = minimumRate;
		else if (value > maximumRate)
			value = maximumRate;

		// eSpeak rates are persisted on 5-unit increments. This makes 82 become 80.
		value = minimumRate + ((value - minimumRate) / 5) * 5;
		return (int)value;
	}

	static int parseConfigurationInteger(const std::string& value, int fallback,
		int minimum, int maximum, int step = 1)
	{
		long parsed = 0;
		if (!parseInteger(value, parsed))
			return fallback;
		if (parsed < minimum)
			parsed = minimum;
		else if (parsed > maximum)
			parsed = maximum;
		if (step > 1)
			parsed = minimum + ((parsed - minimum) / step) * step;
		return (int)parsed;
	}

	static int parseIntonation(const std::string& value)
	{
		long parsed = 0;
		if (!parseInteger(value, parsed))
			return defaultIntonation;
		for (int i = 0; i < intonationModeCount; i++) {
			if (parsed == intonationModes[i])
				return (int)parsed;
		}
		return defaultIntonation;
	}

	static int findIntonationMode(int value)
	{
		for (int i = 0; i < intonationModeCount; i++) {
			if (intonationModes[i] == value)
				return i;
		}
		return 0;
	}

	static int findLanguageSelection(const std::string& value)
	{
		const std::string normalized = normalizeLanguage(value);
		if (normalized == "default")
			return 0;
		for (int i = 0; i < languageChoiceCount; i++) {
			if (normalized == languageChoices[i].code)
				return i + 1;
		}
		return -1;
	}
}

std::atomic<bool> ScreenReader::enabled{false};
std::atomic<bool> ScreenReader::running{false};
std::mutex ScreenReader::mutex;
std::condition_variable ScreenReader::condition;
std::thread ScreenReader::workerThread;

std::string ScreenReader::pendingText;
std::vector<std::string> ScreenReader::deferredSpeechQueue;
std::string ScreenReader::lastSpokenText;
bool ScreenReader::hasPending = false;
bool ScreenReader::pendingIsFinalMessage = false;
bool ScreenReader::pendingIsBlocking = false;
bool ScreenReader::blockingSpeechInProgress = false;
bool ScreenReader::blockingSpeechCompleted = false;
u64 ScreenReader::blockingSpeechGeneration = 0;
u64 ScreenReader::pendingBlockingSpeechGeneration = 0;
bool ScreenReader::finalMessageInProgress = false;
static bool speechBatchActive = false;
static bool speechBatchHasRememberedText = false;
int ScreenReader::rate = 175;
int ScreenReader::pitch = defaultPitch;
int ScreenReader::pitchRange = defaultPitchRange;
int ScreenReader::wordGap = defaultWordGap;
int ScreenReader::punctuation = defaultPunctuation;
int ScreenReader::intonation = defaultIntonation;
int ScreenReader::voiceVariant = defaultVoiceVariant;
std::string ScreenReader::language = "fr";
std::string ScreenReader::systemLanguage = "fr";
bool ScreenReader::customLanguageSelectionEnabled = false;
bool ScreenReader::autoLanguage = true;
int ScreenReader::languageSelection = 0;
bool ScreenReader::announceNavigationHelp = true;
bool ScreenReader::announceElementPosition = true;
bool ScreenReader::virtualReviewActive = false;
PrintConsole* ScreenReader::virtualReviewConsole = nullptr;
int ScreenReader::virtualReviewLine = 0;
u64 ScreenReader::resetConfirmationDeadlineTick = 0;
std::atomic<bool> ScreenReader::stopRequested{false};

static void speakVirtualReviewLine(PrintConsole* console, int line);

void ScreenReader::LoadConfiguration()
{
	FILE* configurationFile = fopen(configurationPath, "r");
	if (configurationFile) {
		fclose(configurationFile);
	} else {
		FILE* backupFile = fopen(backupConfigurationPath, "r");
		if (backupFile) {
			fclose(backupFile);
			if (rename(backupConfigurationPath, configurationPath) != 0) {
#if SCREENREADER_DEBUG
				const int error = errno;
				debug_log_write("ScreenReader: impossible de restaurer la configuration de secours (errno=%d).\n", error);
#endif
			}
		}
	}

	ScreenReaderConfiguration config;
	const int parseResult = ini_parse(configurationPath, configurationHandler, &config);
	if (parseResult == -1) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: configuration absente, création des valeurs par défaut.\n");
#endif
		enabled = false;
		rate = defaultRate;
		pitch = defaultPitch;
		pitchRange = defaultPitchRange;
		wordGap = defaultWordGap;
		punctuation = defaultPunctuation;
		intonation = defaultIntonation;
		voiceVariant = defaultVoiceVariant;
		autoLanguage = true;
		languageSelection = 0;
		announceNavigationHelp = true;
		announceElementPosition = true;
		SaveConfiguration();
		return;
	}
	if (parseResult != 0) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: erreur de syntaxe INI à la ligne %d, valeurs récupérées normalisées.\n", parseResult);
#endif
	}

	// For enabled, every value other than the exact string "0" means enabled.
	const bool enabledValueValid = config.enabled == "0" || config.enabled == "1";
	enabled = config.enabled != "0";

	long parsedRate = 0;
	const bool rateValueValid = parseInteger(config.rate, parsedRate);
	if (rateValueValid)
		rate = normalizeRate(parsedRate);
	else
		rate = defaultRate;
	pitch = parseConfigurationInteger(config.pitch, defaultPitch, 0, 99, 5);
	pitchRange = parseConfigurationInteger(config.pitchRange, defaultPitchRange, 0, 99, 5);
	wordGap = parseConfigurationInteger(config.wordGap, defaultWordGap, 0, 100);
	punctuation = parseConfigurationInteger(config.punctuation, defaultPunctuation,
		espeakPUNCT_NONE, espeakPUNCT_SOME);
	intonation = parseIntonation(config.intonation);
	voiceVariant = parseConfigurationInteger(config.voiceVariant, defaultVoiceVariant,
		0, voiceVariantCount - 1);

	const int selection = findLanguageSelection(config.language);
	if (selection < 0) {
		languageSelection = 0;
		autoLanguage = true;
	} else {
		languageSelection = selection;
		autoLanguage = selection == 0;
	}

	if (languageSelection > 0)
		language = languageChoices[languageSelection - 1].code;
	announceNavigationHelp = parseBoolean(config.announceNavigationHelp, true);
	announceElementPosition = parseBoolean(config.announceElementPosition, true);

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader: configuration chargée (enabled=%d, rate=%d, pitch=%d, range=%d, word_gap=%d, punctuation=%d, intonation=%d, voice_variant=%d, language=%s, nav_help=%d, element_position=%d).\n",
		enabled ? 1 : 0, rate, pitch, pitchRange, wordGap, punctuation, intonation, voiceVariant,
		autoLanguage ? "default" : language.c_str(),
		announceNavigationHelp ? 1 : 0, announceElementPosition ? 1 : 0);
#endif
	if (!enabledValueValid || !rateValueValid || (selection < 0) ||
		(config.announceNavigationHelp != "0" && config.announceNavigationHelp != "1") ||
		(config.announceElementPosition != "0" && config.announceElementPosition != "1") ||
		config.rate != std::to_string(rate) ||
		config.pitch != std::to_string(pitch) ||
		config.pitchRange != std::to_string(pitchRange) ||
		config.wordGap != std::to_string(wordGap) ||
		config.punctuation != std::to_string(punctuation) ||
		config.intonation != std::to_string(intonation) ||
		config.voiceVariant != std::to_string(voiceVariant)) {
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader: une ou plusieurs valeurs ont été normalisées.\n");
#endif
	}
	SaveConfiguration();
}

void ScreenReader::SaveConfiguration()
{
	std::lock_guard<std::mutex> fileLock(configurationFileMutex);

	bool currentEnabled;
	int currentRate;
	int currentPitch;
	int currentPitchRange;
	int currentWordGap;
	int currentPunctuation;
	int currentIntonation;
	int currentVoiceVariant;
	std::string currentLanguage;
	bool currentNavigationHelp;
	bool currentElementPosition;
	{
		std::lock_guard<std::mutex> lock(mutex);
		currentEnabled = enabled;
		currentRate = rate;
		currentPitch = pitch;
		currentPitchRange = pitchRange;
		currentWordGap = wordGap;
		currentPunctuation = punctuation;
		currentIntonation = intonation;
		currentVoiceVariant = voiceVariant;
		currentLanguage = autoLanguage ? "default" : language;
		currentNavigationHelp = announceNavigationHelp;
		currentElementPosition = announceElementPosition;
	}

	FILE* file = fopen(temporaryConfigurationPath, "w");
	if (!file) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: impossible d'ouvrir le fichier temporaire de configuration.\n");
#endif
		return;
	}
	const int writeResult = fprintf(file,
		"[screenreader]\n"
		"enabled=%d\n"
		"rate=%d\n"
		"language=%s\n"
		"announce_navigation_help=%d\n"
		"announce_element_position=%d\n"
		"pitch=%d\n"
		"pitch_range=%d\n"
		"word_gap=%d\n"
		"punctuation=%d\n"
		"intonation=%d\n"
		"voice_variant=%d\n",
		currentEnabled ? 1 : 0, currentRate, currentLanguage.c_str(),
		currentNavigationHelp ? 1 : 0, currentElementPosition ? 1 : 0,
		currentPitch, currentPitchRange, currentWordGap, currentPunctuation, currentIntonation,
		currentVoiceVariant);
	const bool writeOk = writeResult > 0 && fflush(file) == 0;
	const int closeResult = fclose(file);
	if (!writeOk || closeResult != 0) {
		remove(temporaryConfigurationPath);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: échec d'écriture de la configuration.\n");
#endif
		return;
	}
	/* FAT-backed libnx filesystems may reject rename() when the destination
	 * already exists. Move the current file aside, publish the new one, then
	 * remove the backup. If publication fails, restore the previous file. */
	bool backupCreated = false;
	FILE* currentFile = fopen(configurationPath, "r");
	if (!currentFile) {
		FILE* backupFile = fopen(backupConfigurationPath, "r");
		if (backupFile) {
			fclose(backupFile);
			if (rename(backupConfigurationPath, configurationPath) != 0) {
#if SCREENREADER_DEBUG
				const int error = errno;
#endif
				remove(temporaryConfigurationPath);
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader: impossible de restaurer la sauvegarde précédente (errno=%d).\n", error);
#endif
				return;
			}
			currentFile = fopen(configurationPath, "r");
		}
	}
	if (currentFile) {
		fclose(currentFile);
		if (remove(backupConfigurationPath) != 0 && errno != ENOENT) {
#if SCREENREADER_DEBUG
			const int error = errno;
#endif
			remove(temporaryConfigurationPath);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader: impossible de supprimer l'ancienne sauvegarde (errno=%d).\n", error);
#endif
			return;
		}
		if (rename(configurationPath, backupConfigurationPath) != 0) {
#if SCREENREADER_DEBUG
			const int error = errno;
#endif
			remove(temporaryConfigurationPath);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader: impossible de préserver la configuration actuelle (errno=%d).\n", error);
#endif
			return;
		}
		backupCreated = true;
	}

	if (rename(temporaryConfigurationPath, configurationPath) != 0) {
#if SCREENREADER_DEBUG
		const int error = errno;
#endif
		remove(temporaryConfigurationPath);
		if (backupCreated) {
			if (rename(backupConfigurationPath, configurationPath) != 0) {
#if SCREENREADER_DEBUG
				const int restoreError = errno;
#endif
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader: échec de publication (errno=%d), restauration impossible (errno=%d).\n",
					error, restoreError);
#endif
				return;
			}
		}
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: échec de publication de la configuration (errno=%d).\n", error);
#endif
		return;
	}

	if (backupCreated && remove(backupConfigurationPath) != 0) {
#if SCREENREADER_DEBUG
		const int error = errno;
#endif
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: configuration publiée, mais sauvegarde temporaire impossible à supprimer (errno=%d).\n", error);
#endif
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader: configuration sauvegardée (enabled=%d, rate=%d, pitch=%d, range=%d, word_gap=%d, punctuation=%d, intonation=%d, language=%s).\n",
		currentEnabled ? 1 : 0, currentRate, currentPitch, currentPitchRange,
		currentWordGap, currentPunctuation, currentIntonation, currentLanguage.c_str());
#endif
}

bool ScreenReader::Initialize()
{
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Initialize: début.\n");
#endif

	if (running) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Initialize: déjà actif.\n");
#endif
		return true;
	}

	if (!EspeakEngine::Initialize())
	{
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Initialize: échec initialisation eSpeak.\n");
#endif
		return false;
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Initialize: moteur eSpeak initialisé.\n");
#endif

	if (!AudioPlayer::Initialize())
	{
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Initialize: échec initialisation audio.\n");
#endif
		EspeakEngine::Shutdown();
		return false;
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Initialize: moteur audio initialisé.\n");
#endif

	LoadConfiguration();

	{
		std::lock_guard<std::mutex> lock(mutex);
		hasPending = false;
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = false;
		speechBatchHasRememberedText = false;
		blockingSpeechInProgress = false;
		blockingSpeechCompleted = false;
		finalMessageInProgress = false;
		stopRequested = false;
		running = true;
	}

	try
	{
		workerThread = std::thread(&ScreenReader::Worker);
	}
	catch (...)
	{
		running = false;
		AudioPlayer::Exit();
		EspeakEngine::Shutdown();
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Initialize: création du thread impossible.\n");
#endif
		return false;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Initialize: thread créé.\n");
#endif
	if (enabled)
		EspeakEngine::AllowSynthesis();
	else
		EspeakEngine::BlockSynthesis();
	return true;
}

void ScreenReader::Shutdown()
{
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Shutdown: début.\n");
#endif
	restoreVirtualReviewConsole(virtualReviewConsole);
	virtualReviewActive = false;
	virtualReviewConsole = nullptr;
	resetConfirmationDeadlineTick = 0;
	{
		std::lock_guard<std::mutex> lock(mutex);
		running = false;
		stopRequested = true;
		hasPending = false;
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = false;
		speechBatchHasRememberedText = false;
		blockingSpeechCompleted = true;
		finalMessageInProgress = false;
	}

	condition.notify_all();
	EspeakEngine::BlockSynthesis();
	EspeakEngine::Cancel();
	AudioPlayer::Stop();

	if (workerThread.joinable())
	{
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Shutdown: attente du worker.\n");
#endif
		workerThread.join();
	}

	AudioPlayer::Exit();
	EspeakEngine::Shutdown();
	enabled = false;
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Shutdown: terminé.\n");
#endif
}

void ScreenReader::Enable(bool enable)
{
	if (enable && !running) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::Enable: activation ignorée, moteur non initialisé.\n");
#endif
		return;
	}
	if (enable) {
		std::lock_guard<std::mutex> lock(mutex);
		if (finalMessageInProgress) {
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::Enable: activation refusée pendant le message final.\n");
#endif
			return;
		}
	}
	if (enable)
	{
		restoreVirtualReviewConsole(virtualReviewConsole);
		virtualReviewActive = false;
		virtualReviewConsole = nullptr;
		virtualReviewLine = 0;
		resetConfirmationDeadlineTick = 0;
		EspeakEngine::AllowSynthesis();
	}
	else {
		restoreVirtualReviewConsole(virtualReviewConsole);
		virtualReviewActive = false;
		virtualReviewConsole = nullptr;
		resetConfirmationDeadlineTick = 0;
		EspeakEngine::BlockSynthesis();
	}
	enabled = enable;
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Enable: %d.\n", enable ? 1 : 0);
#endif
	SaveConfiguration();
	if (!enable)
		Stop();
}

void ScreenReader::ResetSpeechParameters()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		rate = defaultRate;
		pitch = defaultPitch;
		pitchRange = defaultPitchRange;
		wordGap = defaultWordGap;
		punctuation = defaultPunctuation;
		intonation = defaultIntonation;
		voiceVariant = defaultVoiceVariant;
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::ResetSpeechParameters: rate=%d, pitch=%d, range=%d, word_gap=%d, punctuation=%d, intonation=%d, voice_variant=%d.\n",
		defaultRate, defaultPitch, defaultPitchRange, defaultWordGap, defaultPunctuation, defaultIntonation,
		defaultVoiceVariant);
#endif
	SaveConfiguration();
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader: paramètres vocaux réinitialisés et sauvegardés.\n");
#endif
}

void ScreenReader::DisableWithMessage(const std::string& text)
{
	if (text.empty())
	{
		Enable(false);
		return;
	}

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!running || !enabled)
			return;
		if (blockingSpeechInProgress) {
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::DisableWithMessage: demande ignorée pendant une annonce bloquante.\n");
#endif
			return;
		}

		// Reject every new public speech request before cancelling the current one.
		enabled = false;
		restoreVirtualReviewConsole(virtualReviewConsole);
		virtualReviewActive = false;
		virtualReviewConsole = nullptr;
		stopRequested = true;
		hasPending = false;
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = false;
		speechBatchHasRememberedText = false;
		finalMessageInProgress = true;
		pendingText.clear();
	}
	SaveConfiguration();

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::DisableWithMessage: revue désactivée, préparation du message final.\n");
#endif

	// Stop the current utterance without terminating eSpeak or the worker thread.
	EspeakEngine::BlockSynthesis();
	AudioPlayer::Stop();
	EspeakEngine::Cancel();

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!running)
			return;

		pendingText = text;
		hasPending = true;
		pendingIsFinalMessage = true;
		pendingIsBlocking = false;
		// The worker clears this flag only when it takes the final message.
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::DisableWithMessage: message final mis en attente (longueur=%d).\n", (int)text.size());
#endif
	condition.notify_one();
}

bool ScreenReader::IsEnabled()
{
	return enabled;
}

void ScreenReader::SetRate(int r)
{
	if (r < minimumRate) r = minimumRate;
	else if (r > maximumRate) r = maximumRate;
	{
		std::lock_guard<std::mutex> lock(mutex);
		rate = r;
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SetRate: %d.\n", r);
#endif
	SaveConfiguration();
}

int ScreenReader::GetRate()
{
	std::lock_guard<std::mutex> lock(mutex);
	return rate;
}

void ScreenReader::SetLanguage(const std::string& lang)
{
	std::lock_guard<std::mutex> lock(mutex);
	systemLanguage = normalizeLanguage(lang);
	if (autoLanguage)
		language = systemLanguage;
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::SetLanguage: système=%s, voix=%s, auto=%d.\n",
		systemLanguage.c_str(), language.c_str(), autoLanguage ? 1 : 0);
#endif
}

void ScreenReader::SetCustomLanguageSelectionEnabled(bool enable)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		customLanguageSelectionEnabled = enable;
	}
	// The L+Up/Down voice selector is always available. This flag only controls
	// the legacy ZL+Left/Right shortcut and must not overwrite a saved voice.
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SetCustomLanguageSelectionEnabled: %d.\n", enable ? 1 : 0);
#endif
}

void ScreenReader::Speak(const std::string& text, bool remember)
{
	if (text.empty())
		return;

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!enabled || !running || blockingSpeechInProgress)
		{
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::Speak: demande ignorée (enabled=%d, running=%d, blocking=%d).\n",
				enabled ? 1 : 0, running ? 1 : 0, blockingSpeechInProgress ? 1 : 0);
#endif
			return;
		}

		pendingText = text;
		hasPending = true;
		speechBatchActive = true;
		speechBatchHasRememberedText = remember;
		if (remember)
			lastSpokenText = text;
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		stopRequested = true;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Speak: texte mis en attente (longueur=%d).\n", (int)text.size());
#endif
	AudioPlayer::Stop();
	EspeakEngine::Cancel();
	condition.notify_one();
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Speak: worker notifié.\n");
#endif
}

void ScreenReader::SpeakAfterCurrent(const std::string& text, bool remember)
{
	if (text.empty())
		return;
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!enabled || !running || blockingSpeechInProgress || finalMessageInProgress)
			return;
		if (!speechBatchActive) {
			speechBatchActive = true;
			speechBatchHasRememberedText = false;
		}
		if (remember) {
			if (speechBatchHasRememberedText)
				lastSpokenText += " --- ";
			else
				lastSpokenText.clear();
			lastSpokenText += text;
			speechBatchHasRememberedText = true;
		}
		deferredSpeechQueue.push_back(text);
	}
	condition.notify_one();
}

void ScreenReader::SpeakContext(const std::string& context_window, const std::string& text,
	const std::string& nav_help, int selected_element, int total_elements, bool waitForSpeech)
{
	if (!enabled || !running)
		return;
	bool includeNavigationHelp;
	bool includeElementPosition;
	{
		std::lock_guard<std::mutex> lock(mutex);
		includeNavigationHelp = announceNavigationHelp;
		includeElementPosition = announceElementPosition;
	}

	std::string message;
	const size_t estimatedLength = context_window.size() + text.size() + nav_help.size() + 32;
	message.reserve(estimatedLength);

	const auto appendSegment = [&message](const std::string& segment) {
		if (segment.empty())
			return;
		if (!message.empty())
			message += " --- ";
		message += segment;
	};

	appendSegment(context_window);
	appendSegment(text);

	if (includeElementPosition && selected_element > 0 && total_elements > 0 &&
		selected_element <= total_elements) {
		const std::string position = std::to_string(selected_element) + " " + language_vars["lng_screenreader_position_of"] + " " +
			std::to_string(total_elements);
		appendSegment(position);
	}

	if (includeNavigationHelp)
		appendSegment(nav_help);

	if (message.empty()) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::SpeakContext: aucun segment à lire.\n");
#endif
		return;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SpeakContext: annonce construite (longueur=%d, aide=%d, position=%d).\n",
		(int)message.size(), includeNavigationHelp ? 1 : 0, includeElementPosition ? 1 : 0);
#endif
	if (waitForSpeech)
		SpeakAndWait(message);
	else
		Speak(message);
}

void ScreenReader::SpeakContextAfterCurrent(const std::string& context_window, const std::string& text,
	const std::string& nav_help, int selected_element, int total_elements)
{
	if (!enabled || !running)
		return;
	bool includeNavigationHelp;
	bool includeElementPosition;
	{
		std::lock_guard<std::mutex> lock(mutex);
		includeNavigationHelp = announceNavigationHelp;
		includeElementPosition = announceElementPosition;
	}

	std::string message;
	message.reserve(context_window.size() + text.size() + nav_help.size() + 32);
	const auto appendSegment = [&message](const std::string& segment) {
		if (segment.empty())
			return;
		if (!message.empty())
			message += " --- ";
		message += segment;
	};
	appendSegment(context_window);
	appendSegment(text);
	if (includeElementPosition && selected_element > 0 && total_elements > 0 &&
		selected_element <= total_elements) {
		appendSegment(std::to_string(selected_element) + " " + language_vars["lng_screenreader_position_of"] + " " + std::to_string(total_elements));
	}
	if (includeNavigationHelp)
		appendSegment(nav_help);
	SpeakAfterCurrent(message);
}

void ScreenReader::SpeakAndWait(const std::string& text, bool remember)
{
	if (text.empty())
		return;
	u64 speechId;

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!enabled || !running || blockingSpeechInProgress) {
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::SpeakAndWait: demande ignorée (enabled=%d, running=%d, blocking=%d).\n",
				enabled ? 1 : 0, running ? 1 : 0, blockingSpeechInProgress ? 1 : 0);
#endif
			return;
		}

		blockingSpeechInProgress = true;
		blockingSpeechCompleted = false;
		speechId = ++blockingSpeechGeneration;
		hasPending = false;
		pendingText.clear();
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = true;
		speechBatchHasRememberedText = remember;
		if (remember)
			lastSpokenText = text;
		stopRequested = true;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SpeakAndWait: préparation de l'annonce bloquante (longueur=%d).\n", (int)text.size());
#endif
	AudioPlayer::Stop();
	EspeakEngine::Cancel();

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!running || blockingSpeechCompleted || blockingSpeechGeneration != speechId) {
			if (blockingSpeechGeneration == speechId) {
				blockingSpeechInProgress = false;
				blockingSpeechCompleted = false;
			}
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::SpeakAndWait: annonce annulée avant sa mise en attente.\n");
#endif
			return;
		}

		pendingText = text;
		hasPending = true;
		pendingIsFinalMessage = false;
		pendingIsBlocking = true;
		pendingBlockingSpeechGeneration = speechId;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SpeakAndWait: annonce bloquante mise en attente.\n");
#endif
	condition.notify_one();

	std::unique_lock<std::mutex> lock(mutex);
	while (!blockingSpeechCompleted && blockingSpeechGeneration == speechId && running) {
		condition.wait_for(lock, std::chrono::milliseconds(16));
		if (blockingSpeechCompleted || blockingSpeechGeneration != speechId || !running)
			break;

		lock.unlock();
		padUpdate(&pad);
		const u64 buttonsHeld = padGetButtons(&pad);
		if ((buttonsHeld & (HidNpadButton_ZR | HidNpadButton_B)) == (HidNpadButton_ZR | HidNpadButton_B))
			Interrupt();
		lock.lock();
	}
	if (blockingSpeechGeneration == speechId) {
		blockingSpeechInProgress = false;
		blockingSpeechCompleted = false;
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::SpeakAndWait: annonce terminée, reprise de l'application.\n");
#endif
}

bool ScreenReader::ReadConsole(PrintConsole* console, int startLine, int endLine,
	const std::string& text_to_say_before, const std::string& text_to_say_after,
	bool waitForSpeech)
{
	if (!enabled || !running)
		return false;
	if (!console) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::ReadConsole: console invalide.\n");
#endif
		return false;
	}
	if (!consoleIsTextBufferEnabled(console)) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::ReadConsole: buffer textuel non activé.\n");
#endif
		return false;
	}
	if (!console->windowHeight || !console->windowWidth) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::ReadConsole: dimensions de console invalides.\n");
#endif
		return false;
	}

	const int lastLine = consoleGetLastTextLine(console);
	if (startLine == -1)
		startLine = 0;
	else if (startLine < 0)
		startLine = 0;
	else if (startLine > lastLine)
		startLine = lastLine;

	if (endLine == -1)
		endLine = lastLine;
	else if (endLine < 0)
		endLine = 0;
	else if (endLine > lastLine)
		endLine = lastLine;

	if (lastLine >= 0 && startLine > endLine) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::ReadConsole: plage invalide (%d > %d).\n", startLine, endLine);
#endif
		return false;
	}

	const size_t textLength = lastLine >= 0 && startLine <= endLine ?
		consoleGetText(console, startLine, endLine, NULL, 0) : 0;
	std::string consoleText;
	if (textLength) {
		consoleText.resize(textLength + 1);
		consoleGetText(console, startLine, endLine, &consoleText[0], textLength + 1);
		consoleText.resize(textLength);
	}

	std::string message;
	message.reserve(text_to_say_before.size() + textLength + text_to_say_after.size() + 2);
	if (!text_to_say_before.empty())
		message = text_to_say_before;
	if (!consoleText.empty()) {
		if (!message.empty())
			message += '\n';
		message += consoleText;
	}
	if (!text_to_say_after.empty()) {
		if (!message.empty())
			message += '\n';
		message += text_to_say_after;
	}

	if (message.empty()) {
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::ReadConsole: aucun texte à lire.\n");
#endif
		return false;
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::ReadConsole: lecture des lignes %d à %d (%d caractères UTF-8).\n",
		startLine, endLine, (int)message.size());
#endif
	if (waitForSpeech)
		SpeakAndWait(message);
	else
		Speak(message);
	return true;
}

void ScreenReader::RegisterConsole(PrintConsole* console, const std::string& label)
{
	if (!console)
		return;

	for (size_t i = 0; i < reviewConsoles.size(); i++) {
		if (reviewConsoles[i].console == console) {
			reviewConsoles[i].label = label.empty() ? language_vars["lng_screenreader_console_default"] : label;
			return;
		}
	}

	ReviewConsoleEntry entry;
	entry.console = console;
	entry.label = label.empty() ? language_vars["lng_screenreader_console_default"] : label;
	reviewConsoles.push_back(entry);
}

void ScreenReader::ClearRegisteredConsoles()
{
	reviewConsoles.clear();
}

void ScreenReader::RepeatLast()
{
	std::string text;
	{
		std::lock_guard<std::mutex> lock(mutex);
		text = lastSpokenText;
	}
	if (text.empty())
		Speak(language_vars["lng_screenreader_nothing_to_repeat"], false);
	else
		Speak(text, false);
}

void ScreenReader::ToggleVirtualReview()
{
	if (!enabled || !running)
		return;

	if (virtualReviewActive) {
		restoreVirtualReviewConsole(virtualReviewConsole);
		virtualReviewActive = false;
		virtualReviewConsole = nullptr;
		Speak(language_vars["lng_screenreader_virtual_review_disabled"], false);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: revue virtuelle désactivée.\n");
#endif
		return;
	}

	PrintConsole* console = get_active_console();
	if (!console || !consoleIsTextBufferEnabled(console) || console->windowHeight <= 0) {
		Speak(language_vars["lng_screenreader_console_unreadable"], false);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: impossible d'activer la revue virtuelle (console ou buffer indisponible).\n");
#endif
		return;
	}

	const int lastTextLine = consoleGetLastTextLine(console);
	if (lastTextLine < 0) {
		Speak(language_vars["lng_screenreader_console_empty"], false);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader: revue virtuelle non activée, console vide.\n");
#endif
		return;
	}

	virtualReviewPreviousConsole = get_active_console();
	virtualReviewConsole = console;
	// cursorY is the output cursor, not a meaningful reading position. Start
	// at the first visible row so that leading and intermediate empty rows remain navigable.
	virtualReviewLine = 0;
	virtualReviewActive = true;
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader: revue virtuelle activée (ligne=%d, dernière ligne=%d).\n",
		virtualReviewLine, lastTextLine);
#endif
	speakVirtualReviewLine(virtualReviewConsole, virtualReviewLine);
}

bool ScreenReader::IsVirtualReviewActive()
{
	return virtualReviewActive;
}

static void speakVirtualReviewLine(PrintConsole* console, int line)
{
	if (!console || line < 0 || line >= console->windowHeight)
		return;

	const size_t textSize = consoleGetText(console, line, line, nullptr, 0);
	if (textSize == 0) {
		ScreenReader::Speak(language_vars["lng_screenreader_empty_line"]);
		return;
	}

	ScreenReader::ReadConsole(console, line, line);
}

bool ScreenReader::HandleInput(PadState* pad, u64& buttonsDown)
{
	if (!pad || !running)
		return false;

	const u64 buttonsHeld = padGetButtons(pad);
	const u64 interruptButtons = HidNpadButton_ZR | HidNpadButton_B;
	if ((buttonsHeld & interruptButtons) == interruptButtons && (buttonsDown & interruptButtons)) {
		Interrupt();
		buttonsDown = 0;
		return true;
	}
	static int stickRepeatDelay = 0;
	if (stickRepeatDelay > 0)
		stickRepeatDelay--;
	if (resetConfirmationDeadlineTick != 0) {
		const u64 currentTick = armGetSystemTick();
		const bool confirmationPending =
			(s64)(resetConfirmationDeadlineTick - currentTick) >= 0;

		if (!confirmationPending) {
			resetConfirmationDeadlineTick = 0;
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::HandleInput: délai de confirmation expiré.\n");
#endif
		}
		else {
			if (enabled && (buttonsHeld & HidNpadButton_ZL) && (buttonsDown & HidNpadButton_B)) {
				Interrupt();
				resetConfirmationDeadlineTick = 0;
				buttonsDown = 0;
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader::HandleInput: seconde pression ZL+B reconnue, reset confirmé.\n");
#endif
				ResetSpeechParameters();
				Speak(language_vars["lng_screenreader_reset_done"], false);
				return true;
			}

			buttonsDown = 0;
			return true;
		}
	}

	if (enabled && (buttonsHeld & HidNpadButton_ZR) && (buttonsDown & HidNpadButton_Y)) {
		Interrupt();
		ToggleVirtualReview();
		buttonsDown = 0;
		return true;
	}

	if (enabled && (buttonsHeld & HidNpadButton_ZL) && (buttonsDown & HidNpadButton_B)) {
		Interrupt();
		buttonsDown = 0;
		resetConfirmationDeadlineTick = armGetSystemTick() + armNsToTicks(1000000000ULL);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::HandleInput: première pression ZL+B, confirmation ouverte (1 seconde réelle).\n");
#endif
		Speak(language_vars["lng_screenreader_reset_confirmation"], false);
		return true;
	}

	if (virtualReviewActive) {
		if (buttonsDown & (HidNpadButton_X | HidNpadButton_Y)) {
			Interrupt();
			const bool readToEnd = (buttonsDown & HidNpadButton_Y) != 0;
			buttonsDown = 0;
			if (!virtualReviewConsole || get_active_console() != virtualReviewConsole ||
				!consoleIsTextBufferEnabled(virtualReviewConsole)) {
				restoreVirtualReviewConsole(virtualReviewConsole);
				virtualReviewActive = false;
				virtualReviewConsole = nullptr;
				Speak(language_vars["lng_screenreader_console_changed_review_disabled"], false);
				return true;
			}

			const int lastTextLine = consoleGetLastTextLine(virtualReviewConsole);
			if (lastTextLine < 0) {
				restoreVirtualReviewConsole(virtualReviewConsole);
				virtualReviewActive = false;
				virtualReviewConsole = nullptr;
				Speak(language_vars["lng_screenreader_console_empty_review_disabled"], false);
				return true;
			}
			if (virtualReviewLine > lastTextLine)
				virtualReviewLine = lastTextLine;
			if (readToEnd)
				ReadConsole(virtualReviewConsole, virtualReviewLine, -1);
			else
				speakVirtualReviewLine(virtualReviewConsole, virtualReviewLine);
			return true;
		}

		const u64 horizontalButtons = buttonsDown & (HidNpadButton_Left | HidNpadButton_Right);
		HidAnalogStickState leftStick = padGetStickPos(pad, 0);
		HidAnalogStickState rightStick = padGetStickPos(pad, 1);
		int horizontalDirection = 0;
		if (horizontalButtons & HidNpadButton_Left)
			horizontalDirection = -1;
		else if (horizontalButtons & HidNpadButton_Right)
			horizontalDirection = 1;
		else if (stickRepeatDelay == 0) {
			if (leftStick.x < -16000 || rightStick.x < -16000)
				horizontalDirection = -1;
			else if (leftStick.x > 16000 || rightStick.x > 16000)
				horizontalDirection = 1;
		}

		if (horizontalDirection != 0) {
			Interrupt();
			buttonsDown = 0;
			stickRepeatDelay = 12;
			if (reviewConsoles.size() < 2) {
				Speak(language_vars["lng_screenreader_no_other_console"], false);
				return true;
			}

			int currentIndex = -1;
			for (size_t i = 0; i < reviewConsoles.size(); i++) {
				if (reviewConsoles[i].console == virtualReviewConsole) {
					currentIndex = (int)i;
					break;
				}
			}

			int nextIndex = -1;
			PrintConsole* nextConsole = nullptr;
			const int consoleCount = (int)reviewConsoles.size();
			for (int attempt = 0; attempt < consoleCount; attempt++) {
				if (currentIndex < 0)
					nextIndex = horizontalDirection > 0 ? attempt : consoleCount - 1 - attempt;
				else {
					nextIndex = (currentIndex + horizontalDirection * (attempt + 1)) % consoleCount;
					if (nextIndex < 0)
						nextIndex += consoleCount;
				}

				PrintConsole* candidate = reviewConsoles[nextIndex].console;
				if (candidate && candidate != virtualReviewConsole &&
					consoleIsTextBufferEnabled(candidate) && candidate->windowHeight > 0 &&
					candidate->windowWidth > 0) {
					nextConsole = candidate;
					break;
				}
			}

			if (!nextConsole) {
				Speak(language_vars["lng_screenreader_no_other_console"], false);
				return true;
			}

			virtualReviewConsole = nextConsole;
			virtualReviewLine = 0;
			select_console(nextConsole);
			consoleUpdate(nextConsole);

			const std::string& label = reviewConsoles[nextIndex].label;
			const int lastTextLine = consoleGetLastTextLine(nextConsole);
			if (lastTextLine < 0) {
				Speak(label + ". " + language_vars["lng_screenreader_console_empty"], false);
			} else if (!ReadConsole(nextConsole, 0, 0, label)) {
				Speak(label, false);
			}
			return true;
		}

		const u64 navigationButtons = buttonsDown & (HidNpadButton_Up | HidNpadButton_Down);
		int direction = 0;
		if (navigationButtons & HidNpadButton_Up)
			direction = -1;
		else if (navigationButtons & HidNpadButton_Down)
			direction = 1;
		else if (stickRepeatDelay == 0) {
			if (leftStick.y > 16000 || rightStick.y > 16000)
				direction = -1;
			else if (leftStick.y < -16000 || rightStick.y < -16000)
				direction = 1;
		}

		buttonsDown = 0;
		if (direction != 0) {
			stickRepeatDelay = 12;
			if (!virtualReviewConsole || get_active_console() != virtualReviewConsole ||
				!consoleIsTextBufferEnabled(virtualReviewConsole)) {
				restoreVirtualReviewConsole(virtualReviewConsole);
				virtualReviewActive = false;
				virtualReviewConsole = nullptr;
				Speak(language_vars["lng_screenreader_console_changed_review_disabled"], false);
				return true;
			}

			const int lastTextLine = consoleGetLastTextLine(virtualReviewConsole);
			if (lastTextLine < 0) {
				restoreVirtualReviewConsole(virtualReviewConsole);
				virtualReviewActive = false;
				virtualReviewConsole = nullptr;
				Speak(language_vars["lng_screenreader_console_empty_review_disabled"], false);
				return true;
			}

			if (virtualReviewLine > lastTextLine) {
				virtualReviewLine = lastTextLine;
				speakVirtualReviewLine(virtualReviewConsole, virtualReviewLine);
				return true;
			}

			const int nextLine = virtualReviewLine + direction;
			if (nextLine < 0) {
				Speak(language_vars["lng_screenreader_console_start"], false);
			} else if (nextLine > lastTextLine) {
				Speak(language_vars["lng_screenreader_console_end"], false);
			} else {
				virtualReviewLine = nextLine;
				speakVirtualReviewLine(virtualReviewConsole, virtualReviewLine);
			}
		}
		return true;
	}

	if (!buttonsDown)
		return false;

	const u64 contextShortcutButtons = buttonsDown & (HidNpadButton_X | HidNpadButton_Y);
	if (enabled && (buttonsHeld & HidNpadButton_L) && !(buttonsHeld & HidNpadButton_ZL) &&
		(contextShortcutButtons == HidNpadButton_X || contextShortcutButtons == HidNpadButton_Y)) {
		bool newValue;
		if (contextShortcutButtons == HidNpadButton_X) {
			{
				std::lock_guard<std::mutex> lock(mutex);
				announceElementPosition = !announceElementPosition;
				newValue = announceElementPosition;
			}
			SaveConfiguration();
			Speak(language_vars[newValue ? "lng_screenreader_position_enabled" : "lng_screenreader_position_disabled"], false);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::HandleInput: annonce de position=%d.\n", newValue ? 1 : 0);
#endif
		} else {
			{
				std::lock_guard<std::mutex> lock(mutex);
				announceNavigationHelp = !announceNavigationHelp;
				newValue = announceNavigationHelp;
			}
			SaveConfiguration();
			Speak(language_vars[newValue ? "lng_screenreader_navigation_help_enabled" : "lng_screenreader_navigation_help_disabled"], false);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader::HandleInput: aide à la navigation=%d.\n", newValue ? 1 : 0);
#endif
		}
		buttonsDown &= ~(HidNpadButton_L | contextShortcutButtons);
		return true;
	}

	const u64 voiceDirectionButtons = buttonsDown & (HidNpadButton_Up | HidNpadButton_Down);
	if (enabled && (buttonsHeld & HidNpadButton_L) &&
		!(buttonsHeld & (HidNpadButton_ZL | HidNpadButton_R | HidNpadButton_ZR)) &&
		voiceDirectionButtons && !(voiceDirectionButtons & (voiceDirectionButtons - 1))) {
		int nextSelection;
		int selectedVariant;
		{
			std::lock_guard<std::mutex> lock(mutex);
			nextSelection = voiceVariant +
				(voiceDirectionButtons == HidNpadButton_Down ? 1 : -1);
			if (nextSelection < 0)
				nextSelection = voiceVariantCount - 1;
			else if (nextSelection >= voiceVariantCount)
				nextSelection = 0;

			voiceVariant = nextSelection;
			selectedVariant = voiceVariant;
		}
		buttonsDown &= ~(HidNpadButton_L | voiceDirectionButtons);
		SaveConfiguration();

		char variantName[64];
		if (selectedVariant == 0)
			snprintf(variantName, sizeof(variantName), "%s", language_vars["lng_screenreader_voice_original"].c_str());
		else if (selectedVariant <= 8)
			snprintf(variantName, sizeof(variantName), language_vars["lng_screenreader_voice_masculine"].c_str(), selectedVariant);
		else
			snprintf(variantName, sizeof(variantName), language_vars["lng_screenreader_voice_feminine"].c_str(), selectedVariant - 8);
		char message[128];
		snprintf(message, sizeof(message), language_vars["lng_screenreader_voice_label"].c_str(), variantName);
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader::HandleInput: variante de voix=%s (%d), langue inchangée.\n",
			voiceVariants[selectedVariant][0] ? voiceVariants[selectedVariant] : "originale", selectedVariant);
#endif
		Speak(message, false);
		return true;
	}

	const u64 directionButtons = buttonsDown & (
		HidNpadButton_Up | HidNpadButton_Down |
		HidNpadButton_Left | HidNpadButton_Right);
	const bool oneDirection = directionButtons && !(directionButtons & (directionButtons - 1));
	if (enabled && oneDirection && !(buttonsHeld & HidNpadButton_ZL)) {
		char message[128];
		bool handled = false;
		if ((buttonsHeld & HidNpadButton_ZR) &&
			!(buttonsHeld & (HidNpadButton_L | HidNpadButton_R))) {
			int value;
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (directionButtons == HidNpadButton_Up)
					pitch += 5;
				else if (directionButtons == HidNpadButton_Down)
					pitch -= 5;
				else if (directionButtons == HidNpadButton_Right)
					pitchRange += 5;
				else
					pitchRange -= 5;
				if (pitch > 99) pitch = 99;
				if (pitch < 0) pitch = 0;
				if (pitchRange > 99) pitchRange = 99;
				if (pitchRange < 0) pitchRange = 0;
				value = (directionButtons == HidNpadButton_Up || directionButtons == HidNpadButton_Down) ? pitch : pitchRange;
			}
			if (directionButtons == HidNpadButton_Up || directionButtons == HidNpadButton_Down)
				snprintf(message, sizeof(message), language_vars["lng_screenreader_pitch"].c_str(), value);
			else
				snprintf(message, sizeof(message), language_vars["lng_screenreader_pitch_range"].c_str(), value);
			handled = true;
		} else if ((buttonsHeld & HidNpadButton_R) &&
			!(buttonsHeld & (HidNpadButton_L | HidNpadButton_ZR))) {
			if (directionButtons == HidNpadButton_Up || directionButtons == HidNpadButton_Down) {
				int value;
				{
					std::lock_guard<std::mutex> lock(mutex);
					wordGap += directionButtons == HidNpadButton_Up ? 1 : -1;
					if (wordGap < 0) wordGap = 0;
					if (wordGap > 100) wordGap = 100;
					value = wordGap;
				}
				snprintf(message, sizeof(message), language_vars["lng_screenreader_word_gap"].c_str(), value);
				handled = true;
			} else if (directionButtons == HidNpadButton_Left || directionButtons == HidNpadButton_Right) {
				int value;
				{
					std::lock_guard<std::mutex> lock(mutex);
					int modeIndex = findIntonationMode(intonation);
					modeIndex += directionButtons == HidNpadButton_Right ? 1 : -1;
					if (modeIndex < 0)
						modeIndex = intonationModeCount - 1;
					else if (modeIndex >= intonationModeCount)
						modeIndex = 0;
					intonation = intonationModes[modeIndex];
					value = intonation;
				}
				const char* modes[] = {
					language_vars["lng_screenreader_intonation_natural"].c_str(),
					language_vars["lng_screenreader_intonation_reduced"].c_str(),
					language_vars["lng_screenreader_intonation_reduced_no_comma_rise"].c_str(),
					language_vars["lng_screenreader_intonation_sentence_rise"].c_str()
				};
				const int modeIndex = findIntonationMode(value);
				snprintf(message, sizeof(message), language_vars["lng_screenreader_intonation_label"].c_str(), modes[modeIndex]);
				handled = true;
			}
		}
		if (handled) {
			buttonsDown &= ~(directionButtons | (buttonsHeld & (HidNpadButton_L | HidNpadButton_R | HidNpadButton_ZR)));
			SaveConfiguration();
			Speak(message, false);
			return true;
		}
	}

	if (enabled && (buttonsHeld & HidNpadButton_R) &&
		!(buttonsHeld & (HidNpadButton_L | HidNpadButton_ZL | HidNpadButton_ZR)) &&
		(buttonsDown & HidNpadButton_Y)) {
		int value;
		{
			std::lock_guard<std::mutex> lock(mutex);
			punctuation = (punctuation + 1) % 3;
			value = punctuation;
		}
		const char* modes[] = { language_vars["lng_screenreader_punctuation_none"].c_str(), language_vars["lng_screenreader_punctuation_all"].c_str(), language_vars["lng_screenreader_punctuation_some"].c_str() };
		char message[96];
		snprintf(message, sizeof(message), language_vars["lng_screenreader_punctuation_label"].c_str(), modes[value]);
		buttonsDown &= ~(HidNpadButton_R | HidNpadButton_Y);
		SaveConfiguration();
		Speak(message, false);
		return true;
	}

	if (!(buttonsHeld & HidNpadButton_ZL) ||
		(buttonsHeld & (HidNpadButton_L | HidNpadButton_R | HidNpadButton_ZR)))
		return false;

	const u64 shortcutButtons = buttonsDown & (
		HidNpadButton_Y | HidNpadButton_X |
		HidNpadButton_Up | HidNpadButton_Down |
		HidNpadButton_Left | HidNpadButton_Right);
	if (!shortcutButtons || (shortcutButtons & (shortcutButtons - 1)))
		return false;

	if (shortcutButtons == HidNpadButton_Y) {
		if (enabled) {
			DisableWithMessage(language_vars["lng_screenreader_disabled"]);
#if SCREENREADER_DEBUG
			debug_log_write("Revue d'écran désactivée par raccourci global.\n");
#endif
		} else {
			Enable(true);
			if (enabled) {
				Speak(language_vars["lng_screenreader_enabled"], false);
#if SCREENREADER_DEBUG
				debug_log_write("Revue d'écran activée par raccourci global.\n");
#endif
			}
		}
		buttonsDown &= ~(HidNpadButton_ZL | HidNpadButton_Y);
		return true;
	}

	if (!enabled)
		return false;

	if ((shortcutButtons == HidNpadButton_Left || shortcutButtons == HidNpadButton_Right) &&
		!customLanguageSelectionEnabled)
		return false;

	buttonsDown &= ~(HidNpadButton_ZL | shortcutButtons);

	if (shortcutButtons == HidNpadButton_X) {
		RepeatLast();
		return true;
	}

	if (shortcutButtons == HidNpadButton_Up || shortcutButtons == HidNpadButton_Down) {
		int newRate = GetRate() + (shortcutButtons == HidNpadButton_Up ? 10 : -10);
		SetRate(newRate);
		newRate = GetRate();
		char message[80];
		snprintf(message, sizeof(message), language_vars["lng_screenreader_rate"].c_str(), newRate);
		Speak(message, false);
		return true;
	}

	int nextSelection;
	std::string selectedLanguage;
	{
		std::lock_guard<std::mutex> lock(mutex);
		nextSelection = languageSelection + (shortcutButtons == HidNpadButton_Right ? 1 : -1);
		if (nextSelection < 0)
			nextSelection = languageChoiceCount;
		else if (nextSelection > languageChoiceCount)
			nextSelection = 0;

		languageSelection = nextSelection;
		if (nextSelection == 0) {
			autoLanguage = true;
			language = systemLanguage;
		} else {
			autoLanguage = false;
			language = languageChoices[nextSelection - 1].code;
		}
		selectedLanguage = language;
	}
	SaveConfiguration();

	char message[128];
	if (nextSelection == 0) {
		snprintf(message, sizeof(message), "%s", language_vars["lng_screenreader_language_auto"].c_str());
	} else {
		snprintf(message, sizeof(message), language_vars["lng_screenreader_language_selected"].c_str(), selectedLanguage.c_str());
	}
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::HandleInput: langue sélectionnée=%s, auto=%d.\n",
#endif

	Speak(message, false);
	return true;
}

void ScreenReader::PollInput(PadState* pad)
{
	if (!pad || !running)
		return;
	padUpdate(pad);
	u64 buttonsDown = padGetButtonsDown(pad);
	HandleInput(pad, buttonsDown);
}

void ScreenReader::Stop()
{
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader::Stop: demande d'arrêt.\n");
#endif
	{
		std::lock_guard<std::mutex> lock(mutex);
		stopRequested = true;
		hasPending = false;
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = false;
		speechBatchHasRememberedText = false;
		finalMessageInProgress = false;
		if (blockingSpeechInProgress)
			blockingSpeechCompleted = true;
	}

	EspeakEngine::BlockSynthesis();
	AudioPlayer::Stop();
	EspeakEngine::Cancel();
	condition.notify_all();
}

void ScreenReader::Interrupt()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		stopRequested = true;
		hasPending = false;
		pendingText.clear();
		pendingIsFinalMessage = false;
		pendingIsBlocking = false;
		deferredSpeechQueue.clear();
		speechBatchActive = false;
		speechBatchHasRememberedText = false;
		pendingBlockingSpeechGeneration = 0;
		finalMessageInProgress = false;
		++blockingSpeechGeneration;
		blockingSpeechInProgress = false;
		blockingSpeechCompleted = true;
	}

	AudioPlayer::Stop();
	EspeakEngine::Cancel();
	condition.notify_all();
}

void ScreenReader::Worker()
{
#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader worker loop started.\n");
#endif

	while (running)
	{
		std::string text;
		std::string voice;
		int currentRate;
		int currentPitch;
		int currentPitchRange;
		int currentWordGap;
		int currentPunctuation;
		int currentIntonation;
		int currentVoiceVariant;
		bool finalMessage = false;
		bool blockingSpeech = false;
		u64 blockingSpeechId = 0;

		{
			std::unique_lock<std::mutex> lock(mutex);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: attente d'un texte.\n");
#endif
			condition.wait(lock, [] { return !running || hasPending || !deferredSpeechQueue.empty(); });

			if (!running)
			{
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: arrêt demandé pendant l'attente.\n");
#endif
				break;
			}

			if (hasPending) {
				text.swap(pendingText);
				hasPending = false;
				finalMessage = pendingIsFinalMessage;
				pendingIsFinalMessage = false;
				blockingSpeech = pendingIsBlocking;
				pendingIsBlocking = false;
				blockingSpeechId = pendingBlockingSpeechGeneration;
				pendingBlockingSpeechGeneration = 0;
			} else {
				text.swap(deferredSpeechQueue.front());
				deferredSpeechQueue.erase(deferredSpeechQueue.begin());
			}
			stopRequested = false;
			voice = language;
			currentVoiceVariant = voiceVariant;
			if (currentVoiceVariant > 0) {
				voice += "+";
				voice += voiceVariants[currentVoiceVariant];
			}
			currentRate = rate;
			currentPitch = pitch;
			currentPitchRange = pitchRange;
			currentWordGap = wordGap;
			currentPunctuation = punctuation;
			currentIntonation = intonation;
		}

#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: texte récupéré (longueur=%d, voix=%s, variante=%d, débit=%d).\n",
			(int)text.size(), voice.c_str(), currentVoiceVariant, currentRate);
#endif

		espeak_ERROR voiceResult = espeak_SetVoiceByName(voice.c_str());
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: espeak_SetVoiceByName(%s) = %d.\n", voice.c_str(), (int)voiceResult);
#endif
		if (voiceResult != EE_OK) {
			const size_t variantSeparator = voice.find('+');
			std::string variantSuffix;
			if (variantSeparator != std::string::npos)
				variantSuffix = voice.substr(variantSeparator);
			const std::string requestedLanguage = variantSeparator == std::string::npos ? voice : voice.substr(0, variantSeparator);
			const size_t languageSeparator = requestedLanguage.find('-');

			// Some homebrew language choices are regional/script aliases. Try the
			// primary language with the requested voice variant before dropping it.
			if (languageSeparator != std::string::npos && !variantSuffix.empty()) {
				voice = requestedLanguage.substr(0, languageSeparator) + variantSuffix;
				voiceResult = espeak_SetVoiceByName(voice.c_str());
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: variante sur langue principale %s = %d.\n", voice.c_str(), (int)voiceResult);
#endif
			}
			if (voiceResult != EE_OK) {
				voice = requestedLanguage;
				voiceResult = espeak_SetVoiceByName(voice.c_str());
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: voix originale de repli %s = %d.\n", voice.c_str(), (int)voiceResult);
#endif
			}
			if (voiceResult != EE_OK && languageSeparator != std::string::npos) {
				voice = requestedLanguage.substr(0, languageSeparator);
				voiceResult = espeak_SetVoiceByName(voice.c_str());
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: langue de repli %s = %d.\n", voice.c_str(), (int)voiceResult);
#endif
			}
			if (voiceResult != EE_OK) {
				voice = "en";
				voiceResult = espeak_SetVoiceByName(voice.c_str());
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: voix de secours en = %d.\n", (int)voiceResult);
#endif
			}
		}
		// Set the rate after selecting the voice: SetVoiceByName may restore voice defaults.
		SCREENREADER_DEBUG_RESULT(rateResult, espeak_SetParameter(espeakRATE, currentRate, 0));
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: espeak_SetParameter(espeakRATE) = %d.\n", (int)rateResult);
#endif

		SCREENREADER_DEBUG_RESULT(volumeResult, espeak_SetParameter(espeakVOLUME, 100, 0));
		SCREENREADER_DEBUG_RESULT(pitchResult, espeak_SetParameter(espeakPITCH, currentPitch, 0));
		SCREENREADER_DEBUG_RESULT(rangeResult, espeak_SetParameter(espeakRANGE, currentPitchRange, 0));
		SCREENREADER_DEBUG_RESULT(wordGapResult, espeak_SetParameter(espeakWORDGAP, currentWordGap, 0));
		SCREENREADER_DEBUG_RESULT(punctuationResult, espeak_SetParameter(espeakPUNCTUATION, currentPunctuation, 0));
		SCREENREADER_DEBUG_RESULT(intonationResult, espeak_SetParameter(espeakINTONATION, currentIntonation, 0));
		SCREENREADER_DEBUG_RESULT(punctuationListResult, espeak_SetPunctuationList(L".,;:!?"));
		SCREENREADER_DEBUG_RESULT(capitalsResult, espeak_SetParameter(espeakCAPITALS, 3, 0));
#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: paramètres appliqués (rate=%d, volume=%d, pitch=%d, range=%d, wordgap=%d, punctuation=%d, intonation=%d, punctuation_list=%d, capitals=%d).\n",
			(int)rateResult, (int)volumeResult, (int)pitchResult, (int)rangeResult, (int)wordGapResult,
			(int)punctuationResult, (int)intonationResult, (int)punctuationListResult, (int)capitalsResult);
#endif

		bool skipSpeech = false;
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (stopRequested || !running)
			{
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: synthèse ignorée avant son démarrage (stop=%d, running=%d).\n",
					stopRequested ? 1 : 0, running ? 1 : 0);
#endif
				if (finalMessage)
					finalMessageInProgress = false;
				skipSpeech = true;
			}
		}
		if (skipSpeech) {
			if (blockingSpeech)
				CompleteBlockingSpeech(blockingSpeechId);
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (!hasPending && deferredSpeechQueue.empty()) {
					speechBatchActive = false;
					speechBatchHasRememberedText = false;
				}
			}
			continue;
		}

		std::vector<int16_t> pcm;
		int sampleRate = 0;

#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: début de la synthèse.\n");
#endif
		if (!EspeakEngine::Synthesize(text, pcm, sampleRate, finalMessage))
		{
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: échec synthèse (échantillons=%d, fréquence=%d).\n",
				(int)pcm.size(), sampleRate);
#endif
			if (finalMessage) {
				std::lock_guard<std::mutex> lock(mutex);
				finalMessageInProgress = false;
			}
			if (blockingSpeech)
				CompleteBlockingSpeech(blockingSpeechId);
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (!hasPending && deferredSpeechQueue.empty()) {
					speechBatchActive = false;
					speechBatchHasRememberedText = false;
				}
			}
			continue;
		}

#if SCREENREADER_DEBUG
		debug_log_write("ScreenReader worker: synthèse terminée (échantillons=%d, fréquence=%d).\n",
			(int)pcm.size(), sampleRate);
#endif

		skipSpeech = false;
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (stopRequested || !running)
			{
#if SCREENREADER_DEBUG
				debug_log_write("ScreenReader worker: lecture annulée avant AudioPlayer::Play (stop=%d, running=%d).\n",
					stopRequested ? 1 : 0, running ? 1 : 0);
#endif
				if (finalMessage)
					finalMessageInProgress = false;
				skipSpeech = true;
			}
		}
		if (skipSpeech) {
			if (blockingSpeech)
				CompleteBlockingSpeech(blockingSpeechId);
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (!hasPending && deferredSpeechQueue.empty()) {
					speechBatchActive = false;
					speechBatchHasRememberedText = false;
				}
			}
			continue;
		}

		if (!pcm.empty())
		{
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: appel AudioPlayer::Play.\n");
#endif
			AudioPlayer::Play(pcm.data(), pcm.size(), sampleRate, &stopRequested);
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: AudioPlayer::Play terminé.\n");
#endif
		}
		else {
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: aucun échantillon PCM à lire.\n");
#endif
		}

		if (finalMessage) {
			std::lock_guard<std::mutex> lock(mutex);
			finalMessageInProgress = false;
#if SCREENREADER_DEBUG
			debug_log_write("ScreenReader worker: message final terminé.\n");
#endif
		}
		if (blockingSpeech)
			CompleteBlockingSpeech(blockingSpeechId);
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (!hasPending && deferredSpeechQueue.empty()) {
				speechBatchActive = false;
				speechBatchHasRememberedText = false;
			}
		}
	}

#if SCREENREADER_DEBUG
	debug_log_write("ScreenReader worker stopped.\n");
#endif
}

void ScreenReader::CompleteBlockingSpeech(u64 speechId)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (speechId != 0 && blockingSpeechInProgress && blockingSpeechGeneration == speechId)
			blockingSpeechCompleted = true;
	}
	condition.notify_all();
}
