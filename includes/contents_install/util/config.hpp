#pragma once

#include <vector>

namespace inst::config {
    static const std::string appDir = "sdmc:/switch/AIO_LS_pack_Updater";
    static const std::string appVersion = "1.7.1";

    extern std::string gAuthKey;
    extern bool ignoreReqVers;
    extern bool validateNCAs;
    extern bool overClock;
    extern bool deletePrompt;

    void parseConfig();
}