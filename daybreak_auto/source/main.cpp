/*
 * Copyright (c) Adubbz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <limits>
#include <dirent.h>
#include "assert.hpp"
#include <optional>
#include <unistd.h>
#include <switch.h>
#include "ams_su.h"
#include "reboot.hpp"

PadState pad;

		static constexpr u32 ExosphereApiVersionConfigItem = 65000;
		static constexpr u32 ExosphereHasRcmBugPatch	   = 65004;
		static constexpr u32 ExosphereEmummcType		   = 65007;
		static constexpr u32 ExosphereSupportedHosVersion  = 65011;

		constinit u32 g_supported_version = std::numeric_limits<u32>::max();

		bool g_initialized = false;

		/* Update install state. */
		char g_update_path[FS_MAX_PATH];
		bool g_reset_to_factory = false;
		bool g_exfat_supported = false;
		bool g_use_exfat = false;

/* variables to record some results */
AmsSuUpdateInformation m_update_info;

AmsSuUpdateValidationInfo m_validation_info;
bool m_has_drawn;
bool m_has_info;
bool m_has_validated;
enum class InstallState {
	NeedsDraw,
	NeedsSetup,
	NeedsPrepare,
	AwaitingPrepare,
	NeedsApply,
	AwaitingReboot,
};
static constexpr size_t UpdateTaskBufferSize = 0x100000;
InstallState m_install_state;
AsyncResult m_prepare_result;
float m_progress_percent;

void custom_pause() {
	printf("Press \"A\" key to continue...\n");
	consoleUpdate(NULL);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_A) {
			break;
		}
	}
}

void force_reboot() {
	printf("Console will reboot in 5 secondes.");
	consoleUpdate(NULL);
	sleep(5);
	rebootAms_rcm();
}

u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep = 0) {
	return ((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((micro & 0xFF) << 8) | ((relstep & 0xFF) << 8);
}

void userAppInit() {
	Result rc = 0;

	if (R_FAILED(rc = romfsInit())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = spsmInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = plInitialize(PlServiceType_User))) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = splInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = nsInitialize())) {
		fatalThrow(rc);
	}

	if (R_FAILED(rc = hiddbgInitialize())) {
		fatalThrow(rc);
	}

	consoleInit(NULL);
}

bool InitializeMenu() {
	Result rc = 0;
	/* Mark as initialized. */
	g_initialized = true;

	/* Attempt to get the exosphere version. */
	u64 version;
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereApiVersionConfigItem), &version))) {
		printf("Atmosphere not found. Daybreak requires Atmosphere to be installed. %d", rc);
		custom_pause();
		return false;
	}

	const u32 version_micro = (version >> 40) & 0xff;
	const u32 version_minor = (version >> 48) & 0xff;
	const u32 version_major = (version >> 56) & 0xff;

	/* Validate the exosphere version. */
	const bool ams_supports_sysupdate_api = EncodeVersion(version_major, version_minor, version_micro) >= EncodeVersion(0, 14, 0);
	if (!ams_supports_sysupdate_api) {
		printf("Outdated Atmosphere version. Daybreak requires Atmosphere 0.14.0 or later. %d", rc);
		custom_pause();
		return false;
	}

	/* Ensure DayBreak is ran as a NRO. */
	if (envIsNso()) {
		printf("Unsupported Environment. Please launch Daybreak via the Homebrew menu. %d", rc);
		custom_pause();
		return false;
	}

	/* Attempt to get the supported version. */
   if (R_SUCCEEDED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereSupportedHosVersion), &version))) {
		g_supported_version = static_cast<u32>(version);
	}

	/* Initialize ams:su. */
	if (R_FAILED(rc = amssuInitialize())) {
		fatalThrow(rc);
	}

	return true;
}

void userAppExit() {
	hiddbgExit();
	nsExit();
	splExit();
	plExit();
	spsmExit();
	romfsExit();
	amssuExit();
	consoleExit(NULL);
}

Result GetUpdateInformation() {
	Result rc = 0;
	/* Attempt to get the update information. */
	if (R_FAILED(rc = amssuGetUpdateInformation(&m_update_info, g_update_path))) {
		if (rc == 0x1a405) {
			printf("No update found in folder.\nEnsure your ncas are named correctly!\nResult: 0x%08x\n", rc);
		} else {
			printf("Failed to get update information.\nResult: 0x%08x\n", rc);
		}
		custom_pause();
		return rc;
	}
	/* Print update information. */
	printf("- Version: %d.%d.%d\n", (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf);
	if (m_update_info.exfat_supported) {
		printf("- exFAT: Supported\n");
	} else {
		printf("- exFAT: Unsupported\n");
	}
	printf("- Firmware variations: %d\n", m_update_info.num_firmware_variations);
	consoleUpdate(NULL);
	/* Mark as having obtained update info. */
	m_has_info = true;
	return rc;
}

void ValidateUpdate() {
	Result rc = 0;

	/* Validate the update. */
	if (R_FAILED(rc = amssuValidateUpdate(&m_validation_info, g_update_path))) {
		printf("Failed to validate update.\nResult: 0x%08x\n", rc);
		custom_pause();
		return;
	}

	/* Check the result. */
	if (R_SUCCEEDED(m_validation_info.result)) {
		printf("Update is valid!\n");

		if (R_FAILED(m_validation_info.exfat_result)) {
		   const u32 version = m_validation_info.invalid_key.version;
			printf("exFAT Validation failed with result: 0x%08x\n", m_validation_info.exfat_result);
			printf("Missing content:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

			/* Log the missing content id. */
			printf("- Content id: ");
			for (size_t i = 0; i < sizeof(NcmContentId); i++) {
				printf("%02x", m_validation_info.invalid_content_id.c[i]);
			}
			printf("\n");
		}
	} else {
		/* Log the missing content info. */
		const u32 version = m_validation_info.invalid_key.version;
		printf("Validation failed with result: 0x%08x\n", m_validation_info.result);
		printf("Missing content:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

		/* Log the missing content id. */
		printf("- Content id: ");
		for (size_t i = 0; i < sizeof(NcmContentId); i++) {
			printf("%02x", m_validation_info.invalid_content_id.c[i]);
		}
		printf("\n");
	}
consoleUpdate(NULL);
	/* Mark validation as being complete. */
	m_has_validated = true;
}

void MarkForReboot() {
	m_install_state = InstallState::AwaitingReboot;
}

    Result TransitionUpdateState() {
        Result rc = 0;
        if (m_install_state == InstallState::NeedsSetup) {
            /* Setup the update. */
            if (R_FAILED(rc = amssuSetupUpdate(nullptr, UpdateTaskBufferSize, g_update_path, g_use_exfat))) {
                printf("Failed to setup update.\nResult: 0x%08x\n", rc);
                MarkForReboot();
                return rc;
            }

            /* Log setup completion. */
            printf("Update setup complete.\n");
            m_install_state = InstallState::NeedsPrepare;
        } else if (m_install_state == InstallState::NeedsPrepare) {
            /* Request update preparation. */
            if (R_FAILED(rc = amssuRequestPrepareUpdate(&m_prepare_result))) {
                printf("Failed to request update preparation.\nResult: 0x%08x\n", rc);
                MarkForReboot();
                return rc;
            }

            /* Log awaiting prepare. */
            printf("Preparing update...\n");
            m_install_state = InstallState::AwaitingPrepare;
        } else if (m_install_state == InstallState::AwaitingPrepare) {
            /* Check if preparation has a result. */
            if (R_FAILED(rc = asyncResultWait(&m_prepare_result, 0)) && rc != 0xea01) {
                printf("Failed to check update preparation result.\nResult: 0x%08x\n", rc);
                MarkForReboot();
                return rc;
            } else if (R_SUCCEEDED(rc)) {
                if (R_FAILED(rc = asyncResultGet(&m_prepare_result))) {
                    printf("Failed to prepare update.\nResult: 0x%08x\n", rc);
                    MarkForReboot();
                    return rc;
                }
            }

            /* Check if the update has been prepared. */
            bool prepared;
            if (R_FAILED(rc = amssuHasPreparedUpdate(&prepared))) {
                printf("Failed to check if update has been prepared.\nResult: 0x%08x\n", rc);
                MarkForReboot();
                return rc;
            }

            /* Mark for application if preparation complete. */
            if (prepared) {
                printf("Update preparation complete.\nApplying update...\n");
                m_install_state = InstallState::NeedsApply;
                return rc;
            }

            /* Check update progress. */
            NsSystemUpdateProgress update_progress = {};
            if (R_FAILED(rc = amssuGetPrepareUpdateProgress(&update_progress))) {
                printf("Failed to check update progress.\nResult: 0x%08x\n", rc);
                MarkForReboot();
                return rc;
            }

            /* Update progress percent. */
            if (update_progress.total_size > 0.0f) {
                m_progress_percent = static_cast<float>(update_progress.current_size) / static_cast<float>(update_progress.total_size);
				// printf("* Update progress : %f%s *\r", m_progress_percent, "%");
				// consoleUpdate(NULL);
            } else {
                m_progress_percent = 0.0f;
				// printf("* Update progress : %f%s *\r", m_progress_percent, "%");
				// consoleUpdate(NULL);
            }
        } else if (m_install_state == InstallState::NeedsApply) {
            /* Apply the prepared update. */
            if (R_FAILED(rc = amssuApplyPreparedUpdate())) {
                printf("Failed to apply update.\nResult: 0x%08x\n", rc);
            } else {
                /* Log success. */
                printf("Update applied successfully.\n");

                if (g_reset_to_factory) {
                    if (R_FAILED(rc = nsResetToFactorySettingsForRefurbishment())) {
                        /* Fallback on ResetToFactorySettings. */
                        if (rc == MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer)) {
                            if (R_FAILED(rc = nsResetToFactorySettings())) {
                                printf("Failed to reset to factory settings.\nResult: 0x%08x\n", rc);
                                MarkForReboot();
                                return rc;
                            }
                        } else {
                            printf("Failed to reset to factory settings for refurbishment.\nResult: 0x%08x\n", rc);
                            MarkForReboot();
                            return rc;
                        }
                    }

                    printf("Successfully reset to factory settings.%d\n", rc);
                }
            }

            MarkForReboot();
            return rc;
        }

        return rc;
    }

int main(int argc, char **argv) {
	userAppInit();
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&pad);
	if (!InitializeMenu()) {
		printf("Error during init process.\n");
		custom_pause();
		force_reboot();
		return 1;
	}
	if (argc != 2) {
		printf("Bad program call.");
		custom_pause();
		force_reboot();
		return 1;
	}
	Result rc = 0;
	u64 hardware_type;
	u64 has_rcm_bug_patch;
	u64 is_emummc;
	if (R_FAILED(rc = splGetConfig(SplConfigItem_HardwareType, &hardware_type))) {
		printf("An error has occurred. Failed to get hardware type. %d\n", rc);
		custom_pause();
		force_reboot();
		return 1;
	}
	
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereHasRcmBugPatch), &has_rcm_bug_patch))) {
		printf("An error has occurred. Failed to check RCM bug status. %d\n", rc);
		custom_pause();
		force_reboot();
		return 1;
	}
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereEmummcType), &is_emummc))) {
		printf("An error has occurred. Failed to check emuMMC status. %d\n", rc);
		custom_pause();
		force_reboot();
		return 1;
	}
	/* Warn if we're working with a patched unit. */
	const bool is_erista = hardware_type == 0 || hardware_type == 1;
	if (is_erista && has_rcm_bug_patch && !is_emummc) {
		printf("Warning: Patched unit detected. You may burn fuses or render your switch inoperable.\n");
		consoleUpdate(NULL);
	}
	char current_path[FS_MAX_PATH];
	strncpy(current_path, argv[1], sizeof(current_path));
	snprintf(g_update_path, sizeof(g_update_path), "%s", current_path);
	m_has_drawn = true;
	/* Obtain update information. */
	if (!R_FAILED(GetUpdateInformation())) {
		/* Log this early so it is printed out before validation causes stalling. */
		printf("Validating update, this may take a moment...\n");
		consoleUpdate(NULL);
	}
	/* Perform validation if it hasn't been done already. */
	if (m_has_info && m_has_drawn && !m_has_validated) {
		ValidateUpdate();
	}
	/* Don't continue if validation hasn't been done or has failed. */
	if (!m_has_validated || R_FAILED(m_validation_info.result)) {
		printf("Unsuccesful validation of the update.\n");
		custom_pause();
		force_reboot();
		return 1;
	}

	/* Check if exfat is supported. */
					g_exfat_supported = m_update_info.exfat_supported && R_SUCCEEDED(m_validation_info.exfat_result);
	if (!g_exfat_supported) {
		g_use_exfat = false;
	}
	/* Warn the user if they're updating with exFAT supposed to be supported but not present/corrupted. */
	if (m_update_info.exfat_supported && R_FAILED(m_validation_info.exfat_result)) {
		printf("Error: exFAT firmware is missing or corrupt.\n");
		custom_pause();
		force_reboot();
		return 1;
		
	}
	/* Warn the user if they're updating to a version higher than supported. */
	const u32 version = m_validation_info.invalid_key.version;
	if (EncodeVersion((version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf) > g_supported_version) {
		printf("Error: firmware is too new and not known to be supported.\n");
		custom_pause();
		force_reboot();
		return 1;
	}
	if (g_exfat_supported) {
		bool exfat_supported = false;
		fsIsExFatSupported(&exfat_supported);
		if (exfat_supported) {
			g_use_exfat = true;
		} else {
			g_use_exfat = false;
		}
	}
	g_reset_to_factory = false;
	// printf("Installing steps...\n");
	/* Prevent the home button from being pressed during installation. */
	hiddbgDeactivateHomeButton();
	m_install_state = InstallState::NeedsDraw;
	/* We have drawn now, allow setup to occur. */
	if (m_install_state == InstallState::NeedsDraw) {
		printf("Beginning update setup...\n");
		consoleUpdate(NULL);
		m_install_state = InstallState::NeedsSetup;
	}
	/* Transition to the next update state. */
	if (m_install_state != InstallState::NeedsDraw && m_install_state != InstallState::AwaitingReboot) {
		TransitionUpdateState();
	}
	if (m_install_state != InstallState::NeedsSetup && m_install_state != InstallState::AwaitingReboot) {
		TransitionUpdateState();
	}
	if (m_install_state != InstallState::NeedsPrepare && m_install_state != InstallState::AwaitingReboot) {
		while (1) {
			if (m_install_state == InstallState::NeedsApply) {
				break;
			}
			TransitionUpdateState();
		}
	}
	if (m_install_state != InstallState::AwaitingPrepare && m_install_state != InstallState::AwaitingReboot) {
		TransitionUpdateState();
	}
	force_reboot();
	if (R_FAILED(appletRequestToReboot())) {
		spsmShutdown(true);
	}
	userAppExit();
	return 0;
}