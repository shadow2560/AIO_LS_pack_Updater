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
// #include <algorithm>
// #include <cstdarg>
#include <cstdio>
#include <cstring>
#include <limits>
#include <dirent.h>
// #include <optional>
#include <unistd.h>
#include <switch.h>
#include "ams_su.h"
#include "daybreak-cli.hpp"
#include "../translate.hpp"

extern translation_map language_vars;
extern PrintConsole logs_console;
extern bool debug_enabled;

const char app_version[] = "2.0.0";

PadState daybreak_pad;

		static constexpr u32 ExosphereApiVersionConfigItem = 65000;
		static constexpr u32 ExosphereHasRcmBugPatch	   = 65004;
		static constexpr u32 ExosphereEmummcType		   = 65007;
		static constexpr u32 ExosphereSupportedHosVersion  = 65011;

		u32 g_supported_version = std::numeric_limits<u32>::max();

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
float m_progress_percent = 0.0f;

void custom_pause() {
	printf(language_vars["lng_db_pause"]);
	printf("\n");
	consoleUpdate(&logs_console);
	while(1) {
		padUpdate(&daybreak_pad);
		u64 kDown = padGetButtonsDown(&daybreak_pad);
		if (kDown & HidNpadButton_A) {
			break;
		}
	}
}

u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep = 0) {
	return ((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((micro & 0xFF) << 8) | ((relstep & 0xFF) << 8);
}

extern "C" {

void DaybreakAppInit() {
	Result rc = 0;

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

}

void DaybreakAppExit() {
	hiddbgExit();
	nsExit();
	splExit();
	plExit();
	spsmExit();
	amssuExit();
}

}

bool DaybreakInitializeMenu() {
	Result rc = 0;
	/* Mark as initialized. */
	g_initialized = true;

	/* Attempt to get the exosphere version. */
	u64 version;
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereApiVersionConfigItem), &version))) {
		printf(language_vars["lng_db_atmosphere_not_found_error"], rc);
		printf("\n");
		custom_pause();
		return false;
	}

	const u32 version_micro = (version >> 40) & 0xff;
	const u32 version_minor = (version >> 48) & 0xff;
	const u32 version_major = (version >> 56) & 0xff;

	/* Validate the exosphere version. */
	const bool ams_supports_sysupdate_api = EncodeVersion(version_major, version_minor, version_micro) >= EncodeVersion(0, 14, 0);
	if (!ams_supports_sysupdate_api) {
		printf(language_vars["lng_db_atmosphere_outdated_error"], rc);
		printf("\n");
		custom_pause();
		return false;
	}

	/* Ensure DayBreak is ran as a NRO. */
	if (envIsNso()) {
		printf(language_vars["lng_db_nso_launch_error"], rc);
		printf("\n");
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

Result GetUpdateInformation() {
	Result rc = 0;
	/* Attempt to get the update information. */
	if (R_FAILED(rc = amssuGetUpdateInformation(&m_update_info, g_update_path))) {
		if (rc == 0x1a405) {
			printf(language_vars["lng_db_update_not_found_error"], rc);
			printf("\n");
		} else {
			printf(language_vars["lng_db_get_update_infos_error"], rc);
			printf("\n");
		}
		custom_pause();
		return rc;
	}
	/* Print update information. */
	printf(language_vars["lng_db_update_infos_fw_version"], (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf);
	printf("\n");
	if (m_update_info.exfat_supported) {
		printf(language_vars["lng_db_update_infos_exfat_support"]);
		printf("\n");
	} else {
		printf(language_vars["lng_db_update_infos_exfat_no_support"]);
		printf("\n");
	}
	printf(language_vars["lng_db_update_infos_fw_version_variations"], m_update_info.num_firmware_variations);
	printf("\n");
	consoleUpdate(&logs_console);
	/* Mark as having obtained update info. */
	m_has_info = true;
	return rc;
}

void ValidateUpdate() {
	Result rc = 0;

	/* Validate the update. */
	if (R_FAILED(rc = amssuValidateUpdate(&m_validation_info, g_update_path))) {
		printf(language_vars["lng_db_update_validation_error"], rc);
		printf("\n");
		custom_pause();
		return;
	}

	/* Check the result. */
	if (R_SUCCEEDED(m_validation_info.result)) {
		printf(language_vars["lng_db_update_validation_success"]);
		printf("\n");

		if (R_FAILED(m_validation_info.exfat_result)) {
		   const u32 version = m_validation_info.invalid_key.version;
			printf(language_vars["lng_db_update_validation_exfat_error"], m_validation_info.exfat_result);
			printf("\n");
			printf(language_vars["lng_db_update_validation_missing_content"], m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);
			printf("\n");

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
		printf(language_vars["lng_db_update_validation_content_error"], m_validation_info.result);
		printf("\n");
		printf(language_vars["lng_db_update_validation_missing_content"], m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);
		printf("\n");

		/* Log the missing content id. */
		printf("- Content id: ");
		for (size_t i = 0; i < sizeof(NcmContentId); i++) {
			printf("%02x", m_validation_info.invalid_content_id.c[i]);
		}
		printf("\n");
	}
consoleUpdate(&logs_console);
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
			printf(language_vars["lng_db_install_process_setup_error"], rc);
			printf("\n");
			MarkForReboot();
			return rc;
		}

		/* Log setup completion. */
		printf(language_vars["lng_db_install_process_setup_success"]);
		printf("\n");
		m_install_state = InstallState::NeedsPrepare;
	} else if (m_install_state == InstallState::NeedsPrepare) {
		/* Request update preparation. */
		if (R_FAILED(rc = amssuRequestPrepareUpdate(&m_prepare_result))) {
			printf(language_vars["lng_db_install_process_request_preparation_error"], rc);
			printf("\n");
			MarkForReboot();
			return rc;
		}

		/* Log awaiting prepare. */
		printf(language_vars["lng_db_install_process_preparing"]);
		printf("\n");
		m_install_state = InstallState::AwaitingPrepare;
	} else if (m_install_state == InstallState::AwaitingPrepare) {
		/* Check if preparation has a result. */
		if (R_FAILED(rc = asyncResultWait(&m_prepare_result, 0)) && rc != 0xea01) {
			printf(language_vars["lng_db_install_process_get_preparation_result_error"], rc);
			printf("\n");
			MarkForReboot();
			return rc;
		} else if (R_SUCCEEDED(rc)) {
			if (R_FAILED(rc = asyncResultGet(&m_prepare_result))) {
				printf(language_vars["lng_db_install_process_preparation_error"], rc);
				printf("\n");
				MarkForReboot();
				return rc;
			}
		}

		/* Check if the update has been prepared. */
		bool prepared;
		if (R_FAILED(rc = amssuHasPreparedUpdate(&prepared))) {
			printf(language_vars["lng_db_install_process_preparation_result_error"], rc);
			printf("\n");
			MarkForReboot();
			return rc;
		}

		/* Mark for application if preparation complete. */
		if (prepared) {
			printf("\n");
			printf(language_vars["lng_db_install_process_preparation_success"]);
			printf("\n");
			printf(language_vars["lng_db_install_process_applying_update"]);
			printf("\n");
			m_install_state = InstallState::NeedsApply;
			return rc;
		}

		/* Check update progress. */
		NsSystemUpdateProgress update_progress = {};
		if (R_FAILED(rc = amssuGetPrepareUpdateProgress(&update_progress))) {
			printf(language_vars["lng_db_install_process_get_progress_error"], rc);
			printf("\n");
			MarkForReboot();
			return rc;
		}

		// Update progress percent.
		if (static_cast<float>(update_progress.total_size) > 0.0f) {
			m_progress_percent = (static_cast<float>(update_progress.current_size) / static_cast<float>(update_progress.total_size)) * 100.0f;
			// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
			// consoleUpdate(&logs_console);
		} else {
			m_progress_percent = 0.0f;
			// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
			// consoleUpdate(&logs_console);
		}
		// printf("\r* %10.0f on %10.0f  *", static_cast<float>(update_progress.current_size), static_cast<float>(update_progress.total_size));
		// printf("\r* %i *", (static_cast<int>(update_progress.current_size) / static_cast<int>(update_progress.total_size)) * 100);
		// printf("\r* Update progress : %3.0f %s *", m_progress_percent, "%");
		// consoleUpdate(&logs_console);
	} else if (m_install_state == InstallState::NeedsApply) {
		/* Apply the prepared update. */
		if (R_FAILED(rc = amssuApplyPreparedUpdate())) {
			printf(language_vars["lng_db_install_process_applying_update_error"], rc);
			printf("\n");
		} else {
			/* Log success. */
			printf(language_vars["lng_db_install_process_success"]);
			printf("\n");

			if (g_reset_to_factory) {
				if (R_FAILED(rc = nsResetToFactorySettingsForRefurbishment())) {
					/* Fallback on ResetToFactorySettings. */
					if (rc == MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer)) {
						if (R_FAILED(rc = nsResetToFactorySettings())) {
							printf(language_vars["lng_db_install_process_reset_to_factory_error"], rc);
							printf("\n");
							MarkForReboot();
							return rc;
						}
					} else {
						printf(language_vars["lng_db_install_process_reset_to_factory_for_refurbishment_error"], rc);
						printf("\n");
						MarkForReboot();
						return rc;
					}
				}

				printf(language_vars["lng_db_install_process_reset_to_factory_success"], rc);
				printf("\n");
			}
		}

		MarkForReboot();
		return rc;
	}

	return rc;
}

bool daybreak_ask_question(char *question_text) {
	bool rc;
	printf("%s\n", question_text);
	printf("	[A]: ");
	printf(language_vars["lng_yes"]);
	printf("		  [B]: ");
	printf(language_vars["lng_no"]);
	printf("\n");
	consoleUpdate(&logs_console);
	while(1) {
		padUpdate(&daybreak_pad);
		u64 kDown = padGetButtonsDown(&daybreak_pad);
		if (kDown & HidNpadButton_A) {
			rc = true;
			break;
		} else if (kDown & HidNpadButton_B) {
			rc = false;
			break;
		}
	}
	return rc;
}

bool daybreak_main(char *current_path, int force_reset_to_factory, int force_exfat, int force_unsuported_firmware) {
	DaybreakAppInit();
	printf(language_vars["lng_db_title"], app_version);
	printf("\n");
	consoleUpdate(&logs_console);
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&daybreak_pad);
	if (!DaybreakInitializeMenu()) {
		printf(language_vars["lng_db_init_failed_error"]);
		printf("\n");
		custom_pause();
		return false;
	}
	Result rc = 0;
	u64 hardware_type;
	u64 has_rcm_bug_patch;
	u64 is_emummc;
	if (R_FAILED(rc = splGetConfig(SplConfigItem_HardwareType, &hardware_type))) {
		printf(language_vars["lng_db_get_hardware_type_error"], rc);
		printf("\n");
		custom_pause();
		return false;
	}
	
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereHasRcmBugPatch), &has_rcm_bug_patch))) {
		printf(language_vars["lng_db_get_rcm_bug_patch_error"], rc);
		printf("\n");
		custom_pause();
		return false;
	}
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereEmummcType), &is_emummc))) {
		printf(language_vars["lng_db_get_emummc_status_error"], rc);
		printf("\n");
		custom_pause();
		return false;
	}
	/* Warn if we're working with a patched unit. */
	const bool is_erista = hardware_type == 0 || hardware_type == 1;
	if (is_erista && has_rcm_bug_patch && !is_emummc) {
		printf(language_vars["lng_db_patched_model_warning"]);
		printf("\n");
		consoleUpdate(&logs_console);
	}
	snprintf(g_update_path, sizeof(g_update_path), "%s", current_path);
	m_has_drawn = true;
	/* Obtain update information. */
	if (!R_FAILED(GetUpdateInformation())) {
		/* Log this early so it is printed out before validation causes stalling. */
		printf(language_vars["lng_db_update_validation"]);
		printf("\n");
		consoleUpdate(&logs_console);
	}
	/* Perform validation if it hasn't been done already. */
	if (m_has_info && m_has_drawn && !m_has_validated) {
		ValidateUpdate();
	}
	/* Don't continue if validation hasn't been done or has failed. */
	if (!m_has_validated || R_FAILED(m_validation_info.result)) {
		printf(language_vars["lng_db_update_validation_failed_error"]);
		printf("\n");
		custom_pause();
		return false;
	}

	/* Check if exfat is supported. */
					g_exfat_supported = m_update_info.exfat_supported && R_SUCCEEDED(m_validation_info.exfat_result);
	if (!g_exfat_supported) {
		g_use_exfat = false;
	}
	if (g_exfat_supported && force_exfat == 1) {
		g_use_exfat = true;
	} else if (g_exfat_supported && force_exfat == 0) {
		g_use_exfat = daybreak_ask_question((char *) language_vars["lng_db_ask_exfat_driver_install"]);
	} else {
		g_use_exfat = false;
	}
	bool choose_fat32_forced = false;
	if (!g_exfat_supported && g_use_exfat) {
		choose_fat32_forced = daybreak_ask_question((char *) language_vars["lng_db_ask_fat32_if_exfat_not_supported"]);
		if (choose_fat32_forced) {
			g_use_exfat = false;
		} else {
			printf(language_vars["lng_db_update_canceled"]);
			printf("\n");
			custom_pause();
			return false;
		}
	}
	/* Warn the user if they're updating with exFAT supposed to be supported but not present/corrupted. */
	if (g_use_exfat == true && m_update_info.exfat_supported && R_FAILED(m_validation_info.exfat_result)) {
		printf(language_vars["lng_db_exfat_forced_but_not_supported_error"]);
		printf("\n");
		custom_pause();
		return false;
		
	}
	/* Warn the user if they're updating to a version higher than supported. */
	const u32 version = m_validation_info.invalid_key.version;
	bool force_update_not_supported = false;
	if (EncodeVersion((version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf) > g_supported_version) {
		if (force_unsuported_firmware == 1) {
			printf(language_vars["lng_db_force_unsupported_firmware_enabled"]);
			printf("\n");
			consoleUpdate(&logs_console);
			force_update_not_supported = true;
		} else if (force_unsuported_firmware == 0) {
			force_update_not_supported = daybreak_ask_question((char *) language_vars["lng_db_ask_force_unsupported_firmware_install"]);
		} else {
			force_update_not_supported = false;
		}
		if (!force_update_not_supported) {
			printf(language_vars["lng_db_firmware_not_supported_and_force_install_disabled"]);
			printf("\n");
			custom_pause();
			return false;
		}
	}
	/*
	if (g_exfat_supported) {
		bool exfat_supported = false;
		exfat_supported = fsIsExFatSupported(&exfat_supported);
		if (exfat_supported) {
			g_use_exfat = true;
		} else {
			g_use_exfat = false;
		}
	}
	*/
	if (force_reset_to_factory == 1) {
		printf(language_vars["lng_db_reset_to_factory_forced"]);
		printf("\n");
		g_reset_to_factory = true;
	} else if (force_reset_to_factory == 0) {
		g_reset_to_factory = daybreak_ask_question((char *) language_vars["lng_db_ask_reset_to_factory"]);
	} else {
		g_reset_to_factory = false;
	}
	// printf("Installing steps...\n");
	/* Prevent the home button from being pressed during installation. */
	hiddbgDeactivateHomeButton();
	m_install_state = InstallState::NeedsDraw;
	/* We have drawn now, allow setup to occur. */
	if (m_install_state == InstallState::NeedsDraw) {
		printf(language_vars["lng_db_install_update_begin"]);
		printf("\n");
		consoleUpdate(&logs_console);
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
			if (m_install_state == InstallState::NeedsApply || m_install_state == InstallState::AwaitingReboot) {
				break;
			}
			TransitionUpdateState();
			// printf("\r* Update progress : %3.0f%s *", m_progress_percent, "%");
			// consoleUpdate(&logs_console);
		}
	}
	if (m_install_state != InstallState::AwaitingPrepare && m_install_state != InstallState::AwaitingReboot) {
		TransitionUpdateState();
	}
	DaybreakAppExit();
	return true;
}