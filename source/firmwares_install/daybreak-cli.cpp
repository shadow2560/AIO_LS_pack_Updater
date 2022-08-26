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

extern PrintConsole logs_console;

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
	printf("Appuyez sur \"A\" pour continuer...\n");
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
		printf("Atmosphere non trouve, Daybreak requiere Atmosphere. %d", rc);
		custom_pause();
		return false;
	}

	const u32 version_micro = (version >> 40) & 0xff;
	const u32 version_minor = (version >> 48) & 0xff;
	const u32 version_major = (version >> 56) & 0xff;

	/* Validate the exosphere version. */
	const bool ams_supports_sysupdate_api = EncodeVersion(version_major, version_minor, version_micro) >= EncodeVersion(0, 14, 0);
	if (!ams_supports_sysupdate_api) {
		printf("Version d'Atmosphere depassee, Daybreak requiere Atmosphere 0.14.0 ou superieure. %d", rc);
		custom_pause();
		return false;
	}

	/* Ensure DayBreak is ran as a NRO. */
	if (envIsNso()) {
		printf("Environment non supporte, Lancez Daybreak via le Homebrew menu. %d", rc);
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
			printf("Aucune mise a jour trouvee dans le dossier.\nassurez-vous que vos ncas sont nommes correctement!\nResult: 0x%08x\n", rc);
		} else {
			printf("Echec de recuperation des informations de la mise a jour.\nResult: 0x%08x\n", rc);
		}
		custom_pause();
		return rc;
	}
	/* Print update information. */
	printf("- Version: %d.%d.%d\n", (m_update_info.version >> 26) & 0x1f, (m_update_info.version >> 20) & 0x1f, (m_update_info.version >> 16) & 0xf);
	if (m_update_info.exfat_supported) {
		printf("- exFAT: Supporte\n");
	} else {
		printf("- exFAT: Non supporte\n");
	}
	printf("- Firmware variations: %d\n", m_update_info.num_firmware_variations);
	consoleUpdate(&logs_console);
	/* Mark as having obtained update info. */
	m_has_info = true;
	return rc;
}

void ValidateUpdate() {
	Result rc = 0;

	/* Validate the update. */
	if (R_FAILED(rc = amssuValidateUpdate(&m_validation_info, g_update_path))) {
		printf("Echec de la validation de la mise a jour.\nResult: 0x%08x\n", rc);
		custom_pause();
		return;
	}

	/* Check the result. */
	if (R_SUCCEEDED(m_validation_info.result)) {
		printf("La mise a jour est valide!\n");

		if (R_FAILED(m_validation_info.exfat_result)) {
		   const u32 version = m_validation_info.invalid_key.version;
			printf("Echec de la validation de l'exFAT avec le resultat: 0x%08x\n", m_validation_info.exfat_result);
			printf("Contenu manquant:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

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
		printf("Echec de la validation avec le resultat: 0x%08x\n", m_validation_info.result);
		printf("Contenu manquant:\n- Program id: %016lx\n- Version: %d.%d.%d\n", m_validation_info.invalid_key.id, (version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf);

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
			printf("Echec durant la mise en place de la mise a jour.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Log setup completion. */
		printf("Mise a jour mise en place.\n");
		m_install_state = InstallState::NeedsPrepare;
	} else if (m_install_state == InstallState::NeedsPrepare) {
		/* Request update preparation. */
		if (R_FAILED(rc = amssuRequestPrepareUpdate(&m_prepare_result))) {
			printf("Echec de la requete de preparation de la mise a jour.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Log awaiting prepare. */
		printf("Preparation de la mise a jour...\n");
		m_install_state = InstallState::AwaitingPrepare;
	} else if (m_install_state == InstallState::AwaitingPrepare) {
		/* Check if preparation has a result. */
		if (R_FAILED(rc = asyncResultWait(&m_prepare_result, 0)) && rc != 0xea01) {
			printf("Echec de la recuperation du resultat de la preparation de la mise a jour.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		} else if (R_SUCCEEDED(rc)) {
			if (R_FAILED(rc = asyncResultGet(&m_prepare_result))) {
				printf("Echec de la preparation de la mise a jour.\nResult: 0x%08x\n", rc);
				MarkForReboot();
				return rc;
			}
		}

		/* Check if the update has been prepared. */
		bool prepared;
		if (R_FAILED(rc = amssuHasPreparedUpdate(&prepared))) {
			printf("Echec de la verification pour savoir si la mise a jour a bien ete preparee.\nResult: 0x%08x\n", rc);
			MarkForReboot();
			return rc;
		}

		/* Mark for application if preparation complete. */
		if (prepared) {
			printf("\nPreparation de la mise a jour effectuee.\nApplication de la mise a jour...\n");
			m_install_state = InstallState::NeedsApply;
			return rc;
		}

		/* Check update progress. */
		NsSystemUpdateProgress update_progress = {};
		if (R_FAILED(rc = amssuGetPrepareUpdateProgress(&update_progress))) {
			printf("Echec de la verification de la progression de la mise a jour.\nResult: 0x%08x\n", rc);
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
			printf("Echec de l'application de la mise a jour.\nResult: 0x%08x\n", rc);
		} else {
			/* Log success. */
			printf("Application de la mise a jour effectuee.\n");

			if (g_reset_to_factory) {
				if (R_FAILED(rc = nsResetToFactorySettingsForRefurbishment())) {
					/* Fallback on ResetToFactorySettings. */
					if (rc == MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer)) {
						if (R_FAILED(rc = nsResetToFactorySettings())) {
							printf("Echec de la reinitialisation d'usine de la console.\nResult: 0x%08x\n", rc);
							MarkForReboot();
							return rc;
						}
					} else {
						printf("Echec de la reinitialisation complete d'usine de la console.\nResult: 0x%08x\n", rc);
						MarkForReboot();
						return rc;
					}
				}

				printf("Reinitialisation d'usine effectuee.%d\n", rc);
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
	printf("[A]: Oui          [B]: Non\n");
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
	printf("Daybreak-cli %s par shadow256, largement basee sur Daybreak  d'Atmosphere.", app_version);
	consoleUpdate(&logs_console);
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&daybreak_pad);
	if (!DaybreakInitializeMenu()) {
		printf("Erreur durant le processus d'initialisation.\n");
		custom_pause();
		return false;
	}
	Result rc = 0;
	u64 hardware_type;
	u64 has_rcm_bug_patch;
	u64 is_emummc;
	if (R_FAILED(rc = splGetConfig(SplConfigItem_HardwareType, &hardware_type))) {
		printf("Echec de la recuperation du type de materiel. %d\n", rc);
		custom_pause();
		return false;
	}
	
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereHasRcmBugPatch), &has_rcm_bug_patch))) {
		printf("Echec de la recuperation du statut du bug RCM. %d\n", rc);
		custom_pause();
		return false;
	}
	if (R_FAILED(rc = splGetConfig(static_cast<SplConfigItem>(ExosphereEmummcType), &is_emummc))) {
		printf("Echec de la recuperation du status de l'emuMMC. %d\n", rc);
		custom_pause();
		return false;
	}
	/* Warn if we're working with a patched unit. */
	const bool is_erista = hardware_type == 0 || hardware_type == 1;
	if (is_erista && has_rcm_bug_patch && !is_emummc) {
		printf("Attention: Modele patche detecte, Vous pourriez bruler des Efuses ou rendre la console  inutilisable.\n");
		consoleUpdate(&logs_console);
	}
	snprintf(g_update_path, sizeof(g_update_path), "%s", current_path);
	m_has_drawn = true;
	/* Obtain update information. */
	if (!R_FAILED(GetUpdateInformation())) {
		/* Log this early so it is printed out before validation causes stalling. */
		printf("Validation de la mise a jour, cela peut prendre du temps...\n");
		consoleUpdate(&logs_console);
	}
	/* Perform validation if it hasn't been done already. */
	if (m_has_info && m_has_drawn && !m_has_validated) {
		ValidateUpdate();
	}
	/* Don't continue if validation hasn't been done or has failed. */
	if (!m_has_validated || R_FAILED(m_validation_info.result)) {
		printf("Echec de la validation de la mise a jour.\n");
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
		g_use_exfat = daybreak_ask_question((char *) "Souhaitez-vous installer le driver EXFAT (recommande)?");
	} else {
		g_use_exfat = false;
	}
	bool choose_fat32_forced = false;
	if (!g_exfat_supported && g_use_exfat) {
		choose_fat32_forced = daybreak_ask_question((char *) "Ce firmware  ne supporte pas l'EXFAT, souhaitez-vous n'utiliser que le FAT32?");
		if (choose_fat32_forced) {
			g_use_exfat = false;
		} else {
			printf("Mise a jour annulee.");
			custom_pause();
			return false;
		}
	}
	/* Warn the user if they're updating with exFAT supposed to be supported but not present/corrupted. */
	if (g_use_exfat == true && m_update_info.exfat_supported && R_FAILED(m_validation_info.exfat_result)) {
		printf("Erreur: Le firmware EXFAT est manquant ou corrompu.\n");
		custom_pause();
		return false;
		
	}
	/* Warn the user if they're updating to a version higher than supported. */
	const u32 version = m_validation_info.invalid_key.version;
	bool force_update_not_supported = false;
	if (EncodeVersion((version >> 26) & 0x1f, (version >> 20) & 0x1f, (version >> 16) & 0xf) > g_supported_version) {
		if (force_unsuported_firmware == 1) {
			printf("Forcage de la mise a jour sur un firmware non supportee active.");
			consoleUpdate(&logs_console);
			force_update_not_supported = true;
		} else if (force_unsuported_firmware == 0) {
			force_update_not_supported = daybreak_ask_question((char *) "Ce firmware est trop recent et n'est pas encore supporte.\nVoulez-vous l'installer malgre tout?");
		} else {
			force_update_not_supported = false;
		}
		if (!force_update_not_supported) {
			printf("Erreur: Ce firmware est trop recent et n'est pas force a l'installation, la mise a jour est annulee.\n");
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
		g_reset_to_factory = true;
	} else if (force_reset_to_factory == 0) {
		g_reset_to_factory = daybreak_ask_question((char *) "Souhaitez-vous reinitialiser la console d'usine?");
	} else {
		g_reset_to_factory = false;
	}
	// printf("Installing steps...\n");
	/* Prevent the home button from being pressed during installation. */
	hiddbgDeactivateHomeButton();
	m_install_state = InstallState::NeedsDraw;
	/* We have drawn now, allow setup to occur. */
	if (m_install_state == InstallState::NeedsDraw) {
		printf("Debut de la mise en place de la mise a jour...\n");
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