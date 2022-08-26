#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <regex>
#include <arpa/inet.h>
#include <unistd.h>
#include "switch.h"
#include "util/util.hpp"
#include "nx/ipc/tin_ipc.h"
#include "util/config.hpp"
#include "util/json.hpp"
#include "util/error.hpp"

namespace inst::util {

    void initInstallServices() {
        inst::config::parseConfig();
        ncmInitialize();
        nsextInitialize();
        esInitialize();
        splCryptoInitialize();
        splInitialize();
    }

    void deinitInstallServices() {
        ncmExit();
        nsextExit();
        esExit();
        splCryptoExit();
        splExit();
    }

    //struct caseInsensitiveLess : public std::binary_function< char,char,bool > {
        //bool operator () (char x, char y) const {
            //return toupper(static_cast< unsigned char >(x)) < toupper(static_cast< unsigned char >(y));
        //}
    //};

    auto caseInsensitiveLess = [](auto& x, auto& y)->bool { 
        return toupper(static_cast<unsigned char>(x)) < toupper(static_cast<unsigned char>(y));
    };

    bool ignoreCaseCompare(const std::string &a, const std::string &b) {
        return std::lexicographical_compare(a.begin(), a.end() , b.begin() ,b.end() , caseInsensitiveLess);
    }

    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions) {
        std::vector<std::filesystem::path> files;
        for(auto & p: std::filesystem::directory_iterator(dir))
        {
            try {
            if (std::filesystem::is_regular_file(p))
            {
                std::string ourExtension = p.path().extension().string();
                std::transform(ourExtension.begin(), ourExtension.end(), ourExtension.begin(), ::tolower);
                if (extensions.empty() || std::find(extensions.begin(), extensions.end(), ourExtension) != extensions.end())
                {
                    files.push_back(p.path());
                }
            }
         } catch (std::filesystem::filesystem_error & e) {}
        }
        std::sort(files.begin(), files.end(), ignoreCaseCompare);
        return files;
    }

    std::vector<std::filesystem::path> getDirsAtPath(const std::string & dir) {
        std::vector<std::filesystem::path> files;
        for(auto & p: std::filesystem::directory_iterator(dir))
        {
         try {
            if (std::filesystem::is_directory(p))
            {
                    files.push_back(p.path());
            }
         } catch (std::filesystem::filesystem_error & e) {}
        }
        std::sort(files.begin(), files.end(), ignoreCaseCompare);
        return files;
    }

    bool removeDirectory(std::string dir) {
        try {
            for(auto & p: std::filesystem::recursive_directory_iterator(dir))
            {
                if (std::filesystem::is_regular_file(p))
                {
                    std::filesystem::remove(p);
                }
            }
            rmdir(dir.c_str());
            return true;
        }
        catch (std::filesystem::filesystem_error & e) {
            return false;
        }
    }

    bool copyFile(std::string inFile, std::string outFile) {
       char ch;
       std::ifstream f1(inFile);
       std::ofstream f2(outFile);

       if(!f1 || !f2) return false;
       
       while(f1 && f1.get(ch)) f2.put(ch);
       return true;
    }

    std::string shortenString(std::string ourString, int ourLength, bool isFile) {
        std::filesystem::path ourStringAsAPath = ourString;
        std::string ourExtension = ourStringAsAPath.extension().string();
        if (ourString.size() - ourExtension.size() > (unsigned long)ourLength) {
            if(isFile) return (std::string)ourString.substr(0,ourLength) + "(...)" + ourExtension;
            else return (std::string)ourString.substr(0,ourLength) + "...";
        } else return ourString;
    }

    std::string readTextFromFile(std::string ourFile) {
        if (std::filesystem::exists(ourFile)) {
            FILE * file = fopen(ourFile.c_str(), "r");
            char line[1024];
            fgets(line, 1024, file);
            std::string url = line;
            fflush(file);
            fclose(file);
            return url;
        }
        return "";
    }

    std::string softwareKeyboard(std::string guideText, std::string initialText, int LenMax) {
        Result rc=0;
        SwkbdConfig kbd;
        char tmpoutstr[LenMax + 1] = {0};
        rc = swkbdCreate(&kbd, 0);
        if (R_SUCCEEDED(rc)) {
            swkbdConfigMakePresetDefault(&kbd);
            swkbdConfigSetGuideText(&kbd, guideText.c_str());
            swkbdConfigSetInitialText(&kbd, initialText.c_str());
            swkbdConfigSetStringLenMax(&kbd, LenMax);
            rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
            swkbdClose(&kbd);
            if (R_SUCCEEDED(rc) && tmpoutstr[0] != 0) return(((std::string)(tmpoutstr)));
        }
        return "";
    }

    std::vector<uint32_t> setClockSpeed(int deviceToClock, uint32_t clockSpeed) {
        uint32_t hz = 0;
        uint32_t previousHz = 0;

        if (deviceToClock > 2 || deviceToClock < 0) return {0,0};

        if(hosversionAtLeast(8,0,0)) {
            ClkrstSession session = {0};
            PcvModuleId pcvModuleId;
            pcvInitialize();
            clkrstInitialize();

            switch (deviceToClock) {
                case 0:
                    pcvGetModuleId(&pcvModuleId, PcvModule_CpuBus);
                    break;
                case 1:
                    pcvGetModuleId(&pcvModuleId, PcvModule_GPU);
                    break;
                case 2:
                    pcvGetModuleId(&pcvModuleId, PcvModule_EMC);
                    break;
            }

            clkrstOpenSession(&session, pcvModuleId, 3);
            clkrstGetClockRate(&session, &previousHz);
            clkrstSetClockRate(&session, clockSpeed);
            clkrstGetClockRate(&session, &hz);

            pcvExit();
            clkrstCloseSession(&session);
            clkrstExit();

            return {previousHz, hz};
        } else {
            PcvModule pcvModule;
            pcvInitialize();

            switch (deviceToClock) {
                case 0:
                    pcvModule = PcvModule_CpuBus;
                    break;
                case 1:
                    pcvModule = PcvModule_GPU;
                    break;
                case 2:
                    pcvModule = PcvModule_EMC;
                    break;
            }

            pcvGetClockRate(pcvModule, &previousHz);
            pcvSetClockRate(pcvModule, clockSpeed);
            pcvGetClockRate(pcvModule, &hz);
            
            pcvExit();

            return {previousHz, hz};
        }
    }

    std::string getIPAddress() {
        struct in_addr addr = {(in_addr_t) gethostid()};
        return inet_ntoa(addr);
    }
    
    std::string* getBatteryCharge() {
        std::string batColBlue = "#0000FFFF";
        std::string batColGreen = "#00FF00FF";
        std::string batColYellow = "#FFFF00FF";
        std::string batColOrange = "#FF8000FF";
        std::string batColRed = "#FF0000FF";
        std::string* batValue = new std::string[2];
        batValue[0] = "???";
        batValue[1] = batColBlue;
        u32 charge;

        Result rc = psmInitialize();
        if (!R_FAILED(rc)) {
            rc = psmGetBatteryChargePercentage(&charge);
            if (!R_FAILED(rc)) {
            if (charge < 15.0) {
                batValue[1] = batColRed;
            } else if (charge < 30.0) {
                batValue[1] = batColOrange;
            } else if (charge < 50.0) {
                batValue[1] = batColYellow;
            } else {
                batValue[1] = batColGreen;
            }
                batValue[0] = std::to_string(charge) + "%";
            }
        }
        psmExit();
        return batValue;
    }

    std::vector<std::pair<u64, u32>> listInstalledTitles() {
        std::vector<std::pair<u64, u32>> installedTitles = {};
        const NcmStorageId storageIDs[]{NcmStorageId_SdCard, NcmStorageId_BuiltInUser};
        for (const auto storageID : storageIDs) {
            NcmContentMetaDatabase metaDatabase = {};
            if(R_SUCCEEDED(ncmOpenContentMetaDatabase(&metaDatabase, storageID))) {
                auto metaKeys = new NcmContentMetaKey[64000]();
                s32 written = 0;
                s32 total = 0;
                if(R_SUCCEEDED(ncmContentMetaDatabaseList(&metaDatabase, &total, &written, metaKeys, 64000, NcmContentMetaType_Unknown, 0, 0, UINT64_MAX, NcmContentInstallType_Full)) && (written > 0))
                    for(s32 i = 0; i < written; i++) {
                        const auto &metaKey = metaKeys[i];
                        installedTitles.push_back({metaKey.id, metaKey.version});
                    }
                delete[] metaKeys;
                ncmContentMetaDatabaseClose(&metaDatabase);
            }
        }
        return installedTitles;
    }
    
    bool isTitleInstalled(std::string filename, const std::vector<std::pair<u64, u32>> &installedTitles) {
        static const std::regex idRegex(".*\\[([0-9a-fA-F]+)]\\[v(\\d+)].*");
        std::smatch match;
        if (std::regex_match(filename, match, idRegex)) {
            u64 id = stol(match[1], nullptr, 16);
            u32 version = stoi(match[2]);
            for (const auto &title: installedTitles)
                if (id == title.first and version <= title.second)
                    return true;
        }
        return false;
    }

}