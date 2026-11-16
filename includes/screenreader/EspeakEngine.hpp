#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "../../espeak-ng/src/include/espeak-ng/speak_lib.h"

class EspeakEngine
{
public:
    static bool Initialize();
    static void Shutdown();
    static void Cancel();
    static void BlockSynthesis();
    static void AllowSynthesis();

    static bool Synthesize(const std::string& text,
                           std::vector<int16_t>& outPcm,
                           int& outSampleRate,
                           bool allowWhenBlocked = false);

private:
    static int Callback(short* wav, int numsamples, espeak_EVENT* events);
};
