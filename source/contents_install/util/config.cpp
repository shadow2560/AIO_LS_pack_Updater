#include <fstream>
#include <iomanip>
#include "util/config.hpp"

namespace inst::config {
    std::string gAuthKey;
    bool deletePrompt;
    bool ignoreReqVers;
    bool overClock;
    bool validateNCAs;

    void parseConfig() {
        gAuthKey = {0x41,0x49,0x7a,0x61,0x53,0x79,0x42,0x4d,0x71,0x76,0x34,0x64,0x58,0x6e,0x54,0x4a,0x4f,0x47,0x51,0x74,0x5a,0x5a,0x53,0x33,0x43,0x42,0x6a,0x76,0x66,0x37,0x34,0x38,0x51,0x76,0x78,0x53,0x7a,0x46,0x30};
        deletePrompt = true;
        ignoreReqVers = true;
        overClock = false;
        validateNCAs = false;
    }
}