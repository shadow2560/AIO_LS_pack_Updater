/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstring>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <thread>
#include <memory>
#include "sdInstall.hpp"
#include "install/install_nsp.hpp"
#include "install/install_xci.hpp"
#include "install/sdmc_xci.hpp"
#include "install/sdmc_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"
#include "util/config.hpp"
#include "util/util.hpp"

namespace nspInstStuff {

    bool installNspFromFile(std::vector<std::filesystem::path> ourTitleList, int whereToInstall) {
        inst::util::initInstallServices();
        std::vector<std::pair<u64, u32>> installedTitles;
        installedTitles = inst::util::listInstalledTitles();
        for (long unsigned int i = 0; i < ourTitleList.size(); i++) {
            auto& file = ourTitleList[i];
            std::string itm = file.filename().string();
            if (inst::util::isTitleInstalled(itm, installedTitles)) {
                LOG_DEBUG("Already installed, reinstalling...\n");
				continue;
            }
        }
        
        bool nspInstalled = true;
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;

        if (whereToInstall) m_destStorageId = NcmStorageId_BuiltInUser;
        unsigned int titleItr;

        std::vector<int> previousClockValues;
        if (inst::config::overClock) {
            previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
        }

        try
        {
            unsigned int titleCount = ourTitleList.size();
            for (titleItr = 0; titleItr < titleCount; titleItr++) {
                std::unique_ptr<tin::install::Install> installTask;

                if (ourTitleList[titleItr].extension() == ".xci" || ourTitleList[titleItr].extension() == ".xcz") {
                    auto sdmcXCI = std::make_shared<tin::install::xci::SDMCXCI>(ourTitleList[titleItr]);
                    installTask = std::make_unique<tin::install::xci::XCIInstallTask>(m_destStorageId, inst::config::ignoreReqVers, sdmcXCI);
                } else {
                    auto sdmcNSP = std::make_shared<tin::install::nsp::SDMCNSP>(ourTitleList[titleItr]);
                    installTask = std::make_unique<tin::install::nsp::NSPInstall>(m_destStorageId, inst::config::ignoreReqVers, sdmcNSP);
                }

                LOG_DEBUG("%s\n", "Preparing installation");
                installTask->Prepare();
                installTask->Begin();
            }
        }
        catch (std::exception& e)
        {
            LOG_DEBUG("Failed to install");
            LOG_DEBUG("%s", e.what());
            // fprintf(stdout, "%s", e.what());
            nspInstalled = false;
        }

        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }

        if(nspInstalled) {
            LOG_DEBUG("Success");
        }
        LOG_DEBUG("Done");
        inst::util::deinitInstallServices();
        return nspInstalled;
    }
}
