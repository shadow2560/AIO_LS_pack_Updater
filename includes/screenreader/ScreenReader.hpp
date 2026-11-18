#pragma once

#include <switch.h>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>

class ScreenReader
{
public:
    static bool Initialize();
    static void Shutdown();

    static void Enable(bool enable);
    static void DisableWithMessage(const std::string& text);
    static bool IsEnabled();

    static void Speak(const std::string& text, bool remember = true);
    // Remembered deferred messages are appended to the current utterance's replay buffer.
    static void SpeakAfterCurrent(const std::string& text, bool remember = true);
    static void SpeakAndWait(const std::string& text, bool remember = true);
    static void SpeakContextAfterCurrent(const std::string& context_window, const std::string& text,
        const std::string& nav_help, int selected_element = -1, int total_elements = -1);
    static void SpeakContext(const std::string& context_window, const std::string& text,
        const std::string& nav_help, int selected_element = -1, int total_elements = -1,
        bool waitForSpeech = false);
    // includeHistory makes line indices span retained history followed by the visible window.
    static bool ReadConsole(PrintConsole* console, int startLine = -1, int endLine = -1,
        const std::string& text_to_say_before = "", const std::string& text_to_say_after = "",
        bool waitForSpeech = false, bool includeHistory = false);
    static void RegisterConsole(PrintConsole* console, const std::string& label);
    static void ClearRegisteredConsoles();
    static void ToggleVirtualReview();
    static bool IsVirtualReviewActive();
    static void RepeatLast();
    static void Stop();
    static void Interrupt();

    static void SetRate(int rate);
    static int GetRate();
    static void SetLanguage(const std::string& lang);
    static void SetCustomLanguageSelectionEnabled(bool enabled);
    static bool HandleInput(PadState* pad, u64& buttonsDown);
    static void PollInput(PadState* pad);

private:
    static void Worker();
    static void CompleteBlockingSpeech(u64 speechId);
    static void LoadConfiguration();
    static void SaveConfiguration();
    static void ResetSpeechParameters();

    static std::atomic<bool> enabled;
    static std::atomic<bool> running;
    static std::mutex mutex;
    static std::condition_variable condition;
    static std::thread workerThread;

    static std::string pendingText;
    static std::vector<std::string> deferredSpeechQueue;
    static std::string lastSpokenText;
    static bool hasPending;
    static bool pendingIsFinalMessage;
    static bool pendingIsBlocking;
    static bool blockingSpeechInProgress;
    static bool blockingSpeechCompleted;
    static u64 blockingSpeechGeneration;
    static u64 pendingBlockingSpeechGeneration;
    static bool finalMessageInProgress;
    static int rate;
    static int pitch;
    static int pitchRange;
    static int wordGap;
    static int punctuation;
    static int intonation;
    static int voiceVariant;
    static std::string language;
    static std::string systemLanguage;
    static bool customLanguageSelectionEnabled;
    static bool autoLanguage;
    static int languageSelection;
    static bool announceNavigationHelp;
    static bool announceElementPosition;
    static bool virtualReviewActive;
    static PrintConsole* virtualReviewConsole;
    static int virtualReviewLine;
    static u64 resetConfirmationDeadlineTick;
    static std::atomic<bool> stopRequested;
};
