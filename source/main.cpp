#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>
#include <string.h>
#include <stdlib.h>

#include "main_util.h"
#include "90dns_setter.hpp"
#include "download.hpp"
#include "unzip.hpp"
#include "reboot_to_payload.hpp"
#include "firmwares_install/daybreak-cli.hpp"
#include "translate.hpp"
#include "main_config.h"

extern u32 __nx_applet_exit_mode;
const u64 hbmenu_title_id = 0x0104444444441001;

translation_map language_vars;
#define ROOT					"/"
#define APP_PATH				"/switch/AIO_LS_pack_Updater/"
#define APP_OUTPUT			  "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"

#define APP_VERSION			 "4.3.4"
#define CURSOR_LIST_MAX		 5
#define UP_APP		  0
#define UP_CFW		  1
#define UP_FW		  2
#define UP_HBMENU_INSTALL		  3
#define UP_90DNS		  4
#define UP_ATMO_PROTECT_CONFIGS		  5
const char *OPTION_LIST[CURSOR_LIST_MAX+1];
bool debug_enabled = true;

char CFW_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/Switch_AIO_LS_pack.zip";
char CFW_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
char pack_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/pack_version.txt";
char pack_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/pack_version.txt";
char pack_version_local_filepath[FS_MAX_PATH] = "/pack_version.txt";
char pack_version_local_filepath_beta[FS_MAX_PATH] = "/pack_version.txt";
char subfolder_in_zip[FS_MAX_PATH] = "";
char subfolder_in_zip_beta[FS_MAX_PATH] = "switch_AIO_LS_pack-main/";
s64 pack_size = 1000000000;
s64 pack_size_beta = 1000000000;
char APP_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/AIO_LS_pack_Updater.nro";
char APP_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro";
char firmware_path[FS_MAX_PATH] = "/dernier_firmware_compatible";
char firmware_path_beta[FS_MAX_PATH] = "/dernier_firmware_compatible";
char atmo_logo_dir[FS_MAX_PATH] = "logo";
char atmo_logo_dir_beta[FS_MAX_PATH] = "logo";
char hekate_nologo_file_path[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
char hekate_nologo_file_path_beta[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
int exit_mode_param = 0;
int exit_mode_param_beta = 0;

char pack_version[15];
char last_pack_version[15];
char firmware_version[50];
char atmosphere_version[50];
char emummc_value[10];
char emummc_type[10];
char fusee_gelee_patch[15];
char console_model[50];
u64 console_id = 0;
SetSysSerialNumber console_serial;
// bool sd_is_exfat;
bool console_is_erista = false;
bool beta_mode = false;
PadState pad;

PrintConsole menu_console;
PrintConsole logs_console;
// PrintConsole infos_console;
/* ConsoleFont custom_font;
custom_font->gfx=;
custom_font->asciiOffset=;
custom_font->numChars=;
custom_font->tileWidth=;
custom_font->tileHeight=;
*/

void refreshScreen(int cursor)
{
	consoleClear();
	if (!beta_mode) {
		printf("\x1B[36m");
		printf(language_vars["lng_title"], APP_VERSION);
	} else {
		printf("\x1B[31m");
		printf(language_vars["lng_title_beta"], APP_VERSION);
	}
	if (debug_enabled) {
		printf(" - Debug");
	}
	printf("\x1B[37m\n\n");
	printf(language_vars["lng_a_menu"]);
	printf("\n\n");
	printf(language_vars["lng_x_menu"]);
	printf("\n");
	printf(language_vars["lng_y_menu"]);
	printf("\n");
	if (!beta_mode) {
		printf(language_vars["lng_minus_menu"]);
	} else {
		printf(language_vars["lng_minus_menu_beta"]);
	}
	printf("\n\n");
	printf(language_vars["lng_plus_menu"]);
	printf("\n\n");

	for (int i = 0; i < CURSOR_LIST_MAX + 1; i++) {
		if (cursor == i) {
			printf("\x1B[31m%s\x1B[37m\n\n", OPTION_LIST[i]);
		} else {
			printf("%s\n\n", OPTION_LIST[i]);
		}
	}
	consoleUpdate(&menu_console);
}

void printDisplay(const char *text, ...)
{
	va_list v;
	va_start(v, text);
	vfprintf(stdout, text, v);
	va_end(v);
	consoleUpdate(&logs_console);
}

int appInit() {
	// menu_console = consoleGetDefault();
	consoleInit(&menu_console);
	consoleSetWindow(&menu_console, 0, 0, 80, 26);
	/*
	logs_console.font = menu_console->font;
	logs_console.renderer = NULL;
	logs_console.cursorX = 0;
	logs_console.cursorY = 0;
	logs_console.prevCursorX = 0;
	logs_console.prevCursorY = 0;
	*/
	logs_console.consoleWidth = 80;
	logs_console.consoleHeight = 18;
	logs_console.windowX = 0;
	logs_console.windowY = 27;
	logs_console.windowWidth = 80;
	logs_console.windowHeight = 18;
	logs_console.bg = 6;
	/*
	logs_console.tabSize = 3;
	logs_console.fg = 7;
	logs_console.flags = 0;
	*/
	consoleInit(&logs_console);
	consoleSetWindow(&logs_console, 0, 27, 80, 18);
	consoleSelect(&menu_console);
	// menu_console->font = default_font_bin;
	// consoleSetFont(menu_console, custom_font);
	socketInitializeDefault();
	// nxlinkStdio();
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	romfsInit();	//Init of romfs
// hiddbgInitialize();
appletBeginBlockingHomeButton(0);
appletSetAutoSleepDisabled(true);
// hiddbgDeactivateHomeButton();
	return 0;
}

void logs_console_clear() {
	consoleSelect(&logs_console);
	consoleClear();
	consoleUpdate(&logs_console);
	consoleSelect(&menu_console);
	
}

void appExit() {
	if (debug_enabled) {
		debug_log_write("Fermeture de l'application.\n\n-----------------------------\n\n");
	}
	appletEndBlockingHomeButton();
	appletSetAutoSleepDisabled(false);
	// hiddbgExit();
	socketExit();
	romfsExit();
	consoleExit(&logs_console);
	consoleExit(&menu_console);
}

void get_version_pack() {
	FILE *pack_version_file;
	pack_version_file = fopen(pack_version_local_filepath, "r");
	if (pack_version_file == NULL) {
		return;
	}
		char * buffer = (char *) malloc( 15 );
		fgets(buffer, 15, pack_version_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				pack_version[i] = buffer[i];
			}
			i++;
		}
		pack_version[i] = '\0';
		free( buffer );
	fclose(pack_version_file);
}

void get_last_version_pack() {
	FILE *pack_version_file;
	bool res;
	consoleSelect(&logs_console);
	if (!beta_mode) {
		res = downloadFile(pack_version_url, TEMP_FILE, OFF, false);
	} else {
		res = downloadFile(pack_version_url_beta, TEMP_FILE, OFF, false);
	}
	if (res) {
		logs_console_clear();
		pack_version_file = fopen(TEMP_FILE, "r");
		if (pack_version_file == NULL) {
			consoleSelect(&menu_console);
			return;
		}
		char * buffer = (char *) malloc( 15 );
		fgets(buffer, 15, pack_version_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				last_pack_version[i] = buffer[i];
			}
			i++;
		}
		last_pack_version[i] = '\0';
		free(buffer);
		fclose(pack_version_file);
	}
	consoleSelect(&menu_console);
}

void get_fw_version() {
	setsysInitialize();
	Result ret = 0;
	SetSysFirmwareVersion ver;
	if (R_FAILED(ret = setsysGetFirmwareVersion(&ver))) {
		setsysExit();
		return;
	}

	char sysVersionBuffer[50];
	snprintf(sysVersionBuffer, 50, "%u.%u.%u", ver.major, ver.minor, ver.micro);
	snprintf(firmware_version, sizeof(firmware_version), "%s", sysVersionBuffer);
	setsysExit();
}

void get_ams_version() {
	splInitialize();
	u32 ExosphereApiVersionConfigItem = 65000;
	u64 version;
	Result rc = 0;
	if (R_FAILED(rc = splGetConfig((SplConfigItem)(ExosphereApiVersionConfigItem), &version))) {
		splExit();
		return;
	}
	const u32 version_micro = (version >> 40) & 0xff;
	const u32 version_minor = (version >> 48) & 0xff;
	const u32 version_major = (version >> 56) & 0xff;
	char sysVersionBuffer[50];
	snprintf(sysVersionBuffer, 50, "%u.%u.%u", version_major, version_minor, version_micro);
	snprintf(atmosphere_version, sizeof(atmosphere_version), "%s", sysVersionBuffer);
	splExit();
	if (is_emummc()) {
		strcpy(emummc_value, "Emunand");
	} else {
		strcpy(emummc_value,"Sysnand");
	}
}

void get_fusee_gelee_exploit() {
	splInitialize();
	u32 ExosphereHasRcmBugPatch	   = 65004;
	u64 hardware_type;
	u64 has_rcm_bug_patch;
	Result rc = 0;
	if (R_FAILED(rc = splGetConfig(SplConfigItem_HardwareType, &hardware_type))) {
		splExit();
		return;
	}
	if (R_FAILED(rc = splGetConfig((SplConfigItem)(ExosphereHasRcmBugPatch), &has_rcm_bug_patch))) {
		splExit();
		return;
	}
	console_is_erista = hardware_type == 0 || hardware_type == 1;
	if (console_is_erista && !has_rcm_bug_patch) {
		strcpy(fusee_gelee_patch, language_vars["lng_usable"]);
	} else {
		strcpy(fusee_gelee_patch, language_vars["lng_not_usable"]);
	}
	if (hardware_type == 0) {
		strcpy(console_model, "Switch Erista Icosa");
	}
	if (hardware_type == 0) {
		strcpy(console_model, "Switch Erista Copper");
	}
	if (hardware_type == 2) {
		strcpy(console_model, "Switch Mariko Hoag");
	}
	if (hardware_type == 3) {
		strcpy(console_model, "Switch Mariko Iowa");
	}
	if (hardware_type == 4) {
		strcpy(console_model, "Switch Lite");
	}
		if (hardware_type == 5) {
		strcpy(console_model, "Switch OLED");
	}
	splExit();
}

void get_device_id() {
	splInitialize();
	Result rc = 0;
	if (R_FAILED(rc = splGetConfig(SplConfigItem_DeviceId, &console_id))) {
		splExit();
		return;
	}
	splExit();
}

void get_serial_number() {
	setsysInitialize();
	Result rc = 0;
	if (R_FAILED(rc = setsysGetSerialNumber(&console_serial))) {
		setsysExit();
		return;
	}
	setsysExit();
}

bool ask_question(char *question_text) {
	bool rc;
	consoleSelect(&logs_console);
	printf("%s\n", question_text);
	printf("	[A]: ");
	printf(language_vars["lng_yes"]);
	printf("		  [B]: ");
	printf(language_vars["lng_no"]);
	printf("\n");
	consoleUpdate(&logs_console);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_A) {
			rc = true;
			break;
		} else if (kDown & HidNpadButton_B) {
			rc = false;
			break;
		}
	}
	logs_console_clear();
	consoleSelect(&logs_console);
	return rc;
}

void display_infos() {
	// consoleInit(&infos_console);
	// consoleSetWindow(&infos_console, 1, 0, 80, 43);
	consoleSelect(&logs_console);
	if (debug_enabled) {
		debug_log_write("Affichage des informations.\n");
	}
	printf(language_vars["lng_infos_begin"]);
	printf("\n\n");
	if (isApplet()) {
		printf(language_vars["lng_infos_is_applet"]);
	} else {
		printf(language_vars["lng_infos_is_not_applet"]);
	}
	printf("\n");
	printf(language_vars["lng_infos_pack_version"], pack_version);
	printf("\n");
	printf(language_vars["lng_infos_last_pack_version"], last_pack_version);
	printf("\n");
	printf(language_vars["lng_infos_console_id"], console_id);
	printf("\n");
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		printf(language_vars["lng_infos_serial_incognito"]);
	} else {
		printf(language_vars["lng_infos_serial"], console_serial.number);
	}
	printf("\n");
	if (!is_emummc()) {
		printf(language_vars["lng_infos_sysnand"], emummc_value);
	} else {
		printf(language_vars["lng_infos_emunand"], emummc_value, emummc_type);
	}
	printf("\n");
	printf(language_vars["lng_infos_console_model"], console_model);
	printf("\n");
	printf(language_vars["lng_infos_fusee_gelee_patch"], fusee_gelee_patch);
	printf("\n");
	printf(language_vars["lng_infos_actual_firmware_version"], firmware_version);
	printf("\n");
	printf(language_vars["lng_infos_actual_atmosphere_version"], atmosphere_version);
	printf("\n");
	if (GetChargerType() == 0) {
		printf(language_vars["lng_infos_official_charge"], get_battery_charge());
	} else if (GetChargerType() == 1) {
		printf(language_vars["lng_infos_usb_charge"], get_battery_charge());
	} else {
		printf(language_vars["lng_infos_no_charge"], get_battery_charge());
	}
	printf("\n");
	consoleUpdate(&logs_console);
	consoleSelect(&menu_console);
}

void record_infos() {
	if (debug_enabled) {
		debug_log_write("Ecriture des informations dans le fichier d'informations.\n");
	}
	consoleSelect(&logs_console);
	FILE *log_infos;
	log_infos = fopen("switch/AIO_LS_pack_Updater/console_infos.log", "w");
	if ( log_infos == NULL ) {
		printf(language_vars["lng_record_infos_log_not_open_error"]);
		printf("\n");
		consoleUpdate(&logs_console);
		return;
	}
	fprintf(log_infos, language_vars["lng_infos_begin"]);
	fprintf(log_infos, "\n\n");
	if (isApplet()) {
		fprintf(log_infos, language_vars["lng_infos_is_applet"]);
	} else {
		fprintf(log_infos, language_vars["lng_infos_is_not_applet"]);
	}
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_pack_version"], pack_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_last_pack_version"], last_pack_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_console_id"], console_id);
	fprintf(log_infos, "\n");
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		fprintf(log_infos, language_vars["lng_infos_serial_incognito"]);
	} else {
		fprintf(log_infos, language_vars["lng_infos_serial"], console_serial.number);
	}
	fprintf(log_infos, "\n");
	if (!is_emummc()) {
		fprintf(log_infos, language_vars["lng_infos_sysnand"], emummc_value);
	} else {
		fprintf(log_infos, language_vars["lng_infos_emunand"], emummc_value, emummc_type);
	}
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_console_model"], console_model);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_fusee_gelee_patch"], fusee_gelee_patch);
	fprintf(log_infos, "\n");
		// struct statvfs s;
		// statvfs("payload.bin", &s);
		// fprintf(log_infos, "%i\n", s.f_basetype);
		// Result res;
		// fprintf(log_infos, "%i\n", system("stat -f -c '%t' ./payload.bin > sdmc:/t.txt"));
		// res = fsIsExFatSupported(&sd_is_exfat);
		// fprintf(log_infos, "Exfat : %i - %d\n", sd_is_exfat, res);
	// if (sd_is_exfat) {
		// fprintf(log_infos, "Formatage de la SD: EXFAT\n");
	// } else {
		// fprintf(log_infos, "Formatage de la SD: FAT32\n");
	// }
	fprintf(log_infos, language_vars["lng_infos_actual_firmware_version"], firmware_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars["lng_infos_actual_atmosphere_version"], atmosphere_version);
	fprintf(log_infos, "\n");
	if (GetChargerType() == 0) {
		fprintf(log_infos, language_vars["lng_infos_official_charge"], get_battery_charge());
	} else if (GetChargerType() == 1) {
		fprintf(log_infos, language_vars["lng_infos_usb_charge"], get_battery_charge());
	} else {
		fprintf(log_infos, language_vars["lng_infos_no_charge"], get_battery_charge());
	}
	fprintf(log_infos, "\n");
	// fprintf(log_infos, "Infos batterie et chargeur : %i, %d%% %i", IsChargingEnabled(), get_battery_charge(), GetChargerType());
	// fprintf(log_infos, "%s", language_vars["lng_title"]);
	printf(language_vars["lng_record_infos_success"]);
	consoleUpdate(&logs_console);
	fclose(log_infos);
}

void force_reboot() {
		FILE *payload_file;
	payload_file = fopen("/payload.bin.temp", "rb");
	if (payload_file) {
		fclose(payload_file);
	} else {
		rename("/payload.bin", "/payload.bin.temp");
		custom_cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "/payload.bin");
	}
	spsmInitialize();
	if (R_FAILED(appletRequestToReboot())) {
		spsmShutdown(true);
	}
	spsmExit();
	appExit();
}

void simple_reboot() {
	appExit();
	if (R_FAILED(appletRequestToReboot())) {
		spsmInitialize();
		spsmShutdown(true);
	}
	spsmExit();
}

void aply_reboot() {
	if (console_is_erista) {
		rebootAms_rcm();
	} else {
		force_reboot();
	}
}

void switch_app_mode() {
	if (!beta_mode) {
		if (debug_enabled) {
			debug_log_write("Switch de l'application en mode Beta.\n");
		}
		beta_mode = true;
	} else {
		if (debug_enabled) {
			debug_log_write("Switch de l'application en mode normal.\n");
		}
		beta_mode = false;
	}
}

#include <filesystem>
#include <vector>
#include "contents_install/sdInstall.hpp"
#include "contents_install/hos/hos_Titles.hpp"
#include "contents_install/util/util.hpp"

bool install_hbmenu() {
	if (debug_enabled) {
		debug_log_write("Installation du HBMenu.\n");
	}
	printDisplay(language_vars["lng_hbmenu_install_begin"]);
	printDisplay("\n\n");
	std::vector<std::filesystem::path> nsp_list;
	std::filesystem::path nsp_path;
	inst::util::initInstallServices();
	std::vector<std::pair<u64, u32>> installedTitles;
	installedTitles = inst::util::listInstalledTitles();
	for (long unsigned int i = 0; i < installedTitles.size(); i++) {
		if (installedTitles[i].first == hbmenu_title_id) {
			// if (!titleid_curently_launched(hbmenu_title_id)) {
				if (debug_enabled) {
					debug_log_write("Désinstallation du HBMenu.\n");
				}
				printDisplay(language_vars["lng_hbmenu_install_uninstall_begin"]);
				printDisplay("\n");
				if (R_FAILED(hos::RemoveTitle(hos::Locate(hbmenu_title_id)))) {
					if (debug_enabled) {
						debug_log_write("Erreur durant la désinstallation du HBMenu.\n\n");
					}
					printDisplay("\033[0;31m");
					printDisplay(language_vars["lng_hbmenu_install_uninstall_error"]);
					printDisplay("\033[0;37m\n\n");
					inst::util::deinitInstallServices();
					return false;
				} else {
		if (debug_enabled) {
			debug_log_write("Désinstallation du HBMenu OK.\n");
		}
					printDisplay("\033[0;32m");
					printDisplay(language_vars["lng_hbmenu_install_uninstall_success"]);
					printDisplay("\033[0;37m\n\n");
				}
			// } else {
				// printDisplay("\033[0;31mErreur: Vous avez choisi d'installer l'icone du Homebrew Menu mais vous avez lance celui-ci, ceci est impossible, veuillez quitter cette version du Homebrew Menu lancee en mode application (non lancee en mode applet).\nPour corriger ce probleme vous pouvez lancer ce homebrew via l'album (mode applet a n'utiliser eventuellement que pour cette action specifique) ou maintenir \"R\" durant le lancement d'un autre contenu installe sur la console.\nAnulation de l'installation.\033[0;37m\n\n");
				// inst::util::deinitInstallServices();
				// return false;
			// }
			break;
		}
	}
		inst::util::deinitInstallServices();
	if (strcmp(firmware_version, "inconnue") == 0 || strcmp(firmware_version, "") == 0) {
		nsp_path = "romfs:/nsp/hbmenu.nsp";
	} else {
		char *c1 = substr(firmware_version, 1, 1);
		if (strcmp(c1, ".") == 0) {
			char *c2 = substr(firmware_version, 0, 1);
			if (atoi(c2) < 5) {
				nsp_path = "romfs:/nsp/hbmenu.nsp";
			} else {
				nsp_path = "romfs:/nsp/hbmenu_firmware_5.x.x+_only.nsp";
			}
			free(c2);
		} else {
			nsp_path = "romfs:/nsp/hbmenu_firmware_5.x.x+_only.nsp";
		}
		free(c1);
	}
	if (debug_enabled) {
		debug_log_write("NSP du HBMenu choisi: %s\n", nsp_path.filename().c_str());
	}
	nsp_list.push_back(nsp_path);
	if (nspInstStuff::installNspFromFile(nsp_list, 1)) {
		if (debug_enabled) {
			debug_log_write("Installation du HBMenu OK.\n\n");
		}
		printDisplay("\033[0;32m");
		printDisplay(language_vars["lng_hbmenu_install_success"]);
		printDisplay("\033[0;37m\n");
		return true;
	} else {
		if (debug_enabled) {
			debug_log_write("Erreur d'installation du HBMenu.\n\n");
		}
		printDisplay("\033[0;31m");
		printDisplay(language_vars["lng_hbmenu_install_error"]);
		printDisplay("\033[0;37m\n");
		return false;
	}
}

void menu_init() {
OPTION_LIST[0] = language_vars["lng_update_app_menu"];
	OPTION_LIST[1] = language_vars["lng_update_pack_menu"];
	OPTION_LIST[2] = language_vars["lng_update_firmware_menu"];
	OPTION_LIST[3] = language_vars["lng_update_hbmenu_menu"];
	OPTION_LIST[4] = language_vars["lng_set_90dns_menu"];
	OPTION_LIST[5] = language_vars["lng_protect_console_menu"];
}

void set_emummc_values() {
	int emummc_value = get_emunand_type();
	if (emummc_value == 0) {
		strcpy(emummc_type, "");
	} else if (emummc_value == 1) {
		strcpy(emummc_type, language_vars["lng_files"]);
	} else if (emummc_value == 2) {
		strcpy(emummc_type, language_vars["lng_partition"]);
	}
}

bool fnc_install_firmware() {
	if (debug_enabled) {
		debug_log_write("Début de l'installation du firmware.\n");
	}
	DIR *dir;
	if (!beta_mode) {
		dir = opendir(firmware_path);
	} else {
		dir = opendir(firmware_path_beta);
	}
	if (dir != NULL) {
		closedir(dir);
		/*
		custom_cp((char*) "romfs:/nro/Daybreak-cli.nro", (char*) "/switch/AIO_LS_pack_Updater/Daybreak-cli.nro");
		char temp_setting[FS_MAX_PATH+100]= "";
		if (!beta_mode) {
			strcat(strcat(strcat(temp_setting, "\"/switch/AIO_LS_pack_Updater/Daybreak-cli.nro\" \""), firmware_path), "\" \"false\" \"true\" \"false\"");
		} else {
			strcat(strcat(strcat(temp_setting, "\"/switch/AIO_LS_pack_Updater/Daybreak-cli.nro\" \""), firmware_path_beta), "\" \"false\" \"true\" \"false\"");
		}
		printf("\033[0;32m\nFinis!\nApplication de la mise a jour dans 5 secondes :)\033[0;37m\n");
		consoleUpdate(&logs_console);
		sleep(5);
		appExit();
		envSetNextLoad("/switch/AIO_LS_pack_Updater/Daybreak-cli.nro", temp_setting);
		return 0;
		*/
		if (!beta_mode) {
			if (daybreak_main(firmware_path, 2, 1, 2)) {
				if (debug_enabled) {
					debug_log_write("Installation du firmware OK.\n\n");
				}
				printDisplay("\033[0;32m\n");
				printDisplay(language_vars["lng_install_firmware_end_success"]);
				printDisplay("\033[0;37m\n");
				return true;
			} else {
				if (debug_enabled) {
					debug_log_write("Erreur d'installation du firmware.\n\n");
				}
				printDisplay("\033[0;31m\n");
				printDisplay(language_vars["lng_install_firmware_end_error"]);
				printDisplay("\033[0;37m\n");
				return false;
			}
		} else {
			if (daybreak_main(firmware_path_beta, 2, 1, 2)) {
				if (debug_enabled) {
					debug_log_write("Installation du firmware OK.\n\n");
				}
				printDisplay("\033[0;32m\n");
				printDisplay(language_vars["lng_install_firmware_end_success"]);
				printDisplay("\033[0;37m\n");
				return true;
			} else {
				if (debug_enabled) {
					debug_log_write("Erreur d'installation du firmware.\n\n");
				}
				printDisplay("\033[0;31m\n");
				printDisplay(language_vars["lng_install_firmware_end_error"]);
				printDisplay("\033[0;37m\n");
				return false;
			}
		}
	} else {
		if (debug_enabled) {
			debug_log_write("Erreur, répertoire du firmware \"%s\" non trouvé.\n\n", firmware_path);
		}
		printDisplay("\033[0;31m");
		printDisplay(language_vars["lng_install_firmware_error_folder_not_found"]);
		printDisplay("\033[0;37m\n");
		return false;
	}
	return false;
}

int main(int argc, char **argv)
{
	// init stuff
	appInit();
	if (argc == 1 && debug_enabled) {
		debug_log_start();
	}
	language_vars = set_translation_strings();
	menu_init();
	strcpy(pack_version, language_vars["lng_unknown_1"]);
	strcpy(last_pack_version, language_vars["lng_unknown_1"]);
	strcpy(firmware_version, language_vars["lng_unknown_1"]);
	strcpy(atmosphere_version, language_vars["lng_unknown_1"]);
	strcpy(emummc_value, language_vars["lng_unknown_1"]);
	strcpy(emummc_type, language_vars["lng_unknown_0"]);
	strcpy(fusee_gelee_patch, language_vars["lng_unknown_0"]);
	strcpy(console_model, language_vars["lng_unknown_0"]);
	configs_init();
	padInitializeDefault(&pad);

	// change directory to root (defaults to /switch/)
	chdir(ROOT);

	// set the cursor position to 0
	short cursor = 0;

	get_version_pack();
	get_last_version_pack();
	get_fw_version();
	get_ams_version();
	get_fusee_gelee_exploit();
	get_device_id();
	get_serial_number();
	set_emummc_values();
	remove(TEMP_FILE);

debug_log_write("Version du homebrew: %s\n", APP_VERSION);
	if (isApplet()) {
		if (debug_enabled) {
			debug_log_write("Homebrew en mode applet.\n");
		}
		printf("\x1B[44m");
	} else {
		if (debug_enabled) {
			debug_log_write("Homebrew hors mode applet.\n");
		}
	}

	if (debug_enabled) {
		debug_log_write("\nInformations sur la console:\n");
		debug_log_write("Version du pack: %s\n", pack_version);
		debug_log_write("Dernière version du pack: %s\n", last_pack_version);
		debug_log_write("Version du firmware: %s\n", firmware_version);
		debug_log_write("Version d'Atmosphere: %s\n", atmosphere_version);
		if (!is_emummc()) {
		debug_log_write("Console en sysnand.\n");
		} else {
			debug_log_write("Console en emunand de type %s.\n", emummc_type);
		}
		debug_log_write("Etat de l'exploit Fusee Gelee: %s\n", fusee_gelee_patch);
		debug_log_write("Modèle de la console: %s\n\n", console_model);
	}

	// set auto-update of the console off if it's not already done (FW 2.0.0+)
	if (firmware_version[0] == '1' && firmware_version[1] == '.') {
		if (debug_enabled) {
			debug_log_write("Firmware inférieur au firmware 2.0.0 détecté, impossible de vérifier ou de désactiver la mise à jour automatique.\n\n");
		}
	} else {
		setsysInitialize();
		bool res;
		setsysGetAutoUpdateEnableFlag(&res);
		if (!res) {
			if (debug_enabled) {
				debug_log_write("désactivation de la mise à jour automatique de la console déjà effectuée.\n\n");
			}
		} else {
			setsysSetAutoUpdateEnableFlag(false);
			if (debug_enabled) {
				debug_log_write("désactivation de la mise à jour automatique de la console effectuée.\n\n");
			}
		}
		setsysExit();
	}

	// main menu
	refreshScreen(cursor);

	// Loop for the menu
	while(appletMainLoop())
	{
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		// u64 kHeld = padGetButtons(&pad);

		// move cursor down...
		if (kDown & HidNpadButton_StickLDown || kDown & HidNpadButton_StickRDown || kDown & HidNpadButton_Down) // Could be replaced by HidNpadButton_AnyDown
		{
			if (cursor == CURSOR_LIST_MAX) cursor = 0;
			else cursor++;
			logs_console_clear();
			refreshScreen(cursor);
		}

		// move cursor up...
		else if (kDown & HidNpadButton_StickLUp || kDown & HidNpadButton_StickRUp || kDown & HidNpadButton_Up) // Could be replaced by HidNpadButton_AnyUp
		{
			if (cursor == 0) cursor = CURSOR_LIST_MAX;
			else cursor--;
			logs_console_clear();
			refreshScreen(cursor);
		}

		else if (kDown & HidNpadButton_A)
		{
			logs_console_clear();
			switch (cursor)
			{
			case UP_FW:
			{
				if (debug_enabled) {
					debug_log_write("Installation d'un firmware.\n");
				}
				consoleSelect(&logs_console);
				if (GetChargerType() == 0 && get_battery_charge() < 10) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_10"]);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 20) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_20"]);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				}
				bool update_firmware2 = false;
				update_firmware2 = ask_question((char*) language_vars["lng_ask_update_firmware"]);
				if (update_firmware2) {
					fnc_clean_theme();
					if (fnc_install_firmware()) {
						sleep(5);
						simple_reboot();
					}
				} else {
					if (debug_enabled) {
						debug_log_write("Annulation de l'installation du firmware.\n\n");
					}
				}
				consoleSelect(&menu_console);
				break;
			}
			case UP_CFW:
			{
				if (debug_enabled) {
					debug_log_write("Installation du pack.\n");
				}
				consoleSelect(&logs_console);
				if (GetChargerType() == 0 && get_battery_charge() < 20) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_20"]);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					printDisplay(language_vars["lng_battery_error_30"]);
					consoleSelect(&menu_console);
					break;
				}
				mkdir(APP_PATH, 0777);
				bool update_firmware = false;
				bool clean_theme = false;
				if (!beta_mode) {
					if (strcmp(firmware_path, "") != 0) {
						update_firmware = ask_question((char*) language_vars["lng_ask_update_firmware"]);
					}
				} else {
					if (strcmp(firmware_path_beta, "") != 0) {
						update_firmware = ask_question((char*) language_vars["lng_ask_update_firmware"]);
					}
				}
				if (update_firmware) {
					clean_theme = true;
				} else {
					clean_theme = ask_question((char*) language_vars["lng_ask_clean_theme"]);
				}
				bool clean_logos = ask_question((char*) language_vars["lng_ask_clean_logos"]);
				bool install_hbmenu_choice = ask_question((char*) language_vars["lng_ask_hbmenu_install"]);
				/*
				if (titleid_curently_launched(hbmenu_title_id) && install_hbmenu_choice) {
					install_hbmenu_choice = ask_question((char *) "Attention: Vous avez choisi d'installer l'icone du Homebrew Menu mais vous avez lance celui-ci, ceci n'est pas possible, veuillez quitter cette version du Homebrew Menu lancee en mode application (non lancee en mode applet).\nPour pouvoir faire ceci vous devez lancer ce homebrew en maintenant \"R\" en lancant un jeu ou lancer ce homebrew via l'album (en mode applet mais ceci n'est pas recommande pour installer le pack car cela diminu grandement les performances).\nVous pourez egalement effectuer l'installation de l'icone du Homebrew Menu via l'option appropriee du menu principal de ce homebrew apres l'installation du pack et le redemarrage de la console, dans ce cas le mode applet poura etre utilise.\n\nSouhaitez-vous continuer l'installation du pack en desactivant l'installation de l'icone du Homebrew Menu?");
					if (install_hbmenu_choice) {
						install_hbmenu_choice = false;
					} else {
						printDisplay("Installation du pack annulee.");
						break;
					}
				}
				*/
				printDisplay(language_vars["lng_install_pack_recap_begin"]);
				printDisplay("\n\n");
				if (update_firmware) {
					printDisplay(language_vars["lng_install_pack_recap_firmware_install"]);
					printDisplay("\n");
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_install_firmware"]);
					printDisplay("\n");
					if (clean_theme) {
						printDisplay(language_vars["lng_install_pack_recap_clean_theme"]);
						printDisplay("\n");
					} else {
						printDisplay(language_vars["lng_install_pack_recap_not_clean_theme"]);
						printDisplay("\n");
					}
				}
				if (clean_logos) {
					printDisplay(language_vars["lng_install_pack_recap_clean_logos"]);
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_clean_logos"]);
				}
				printDisplay("\n");
				if (install_hbmenu_choice) {
					printDisplay(language_vars["lng_install_pack_recap_install_hbmenu"]);
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_install_hbmenu"]);
				}
				printDisplay("\n");
				bool validate_choice = ask_question((char*) language_vars["lng_ask_validate_choices"]);
				if (validate_choice) {
						if (debug_enabled) {
							debug_log_write("\nRécapitulatif des paramètres d'installation du pack:.\n");
							if (update_firmware) {
								debug_log_write("Installation du firmware.\n");
							} else {
								debug_log_write("Non installation du firmware.\n");
								if (clean_theme) {
									debug_log_write("Nettoyage du thème.\n");
								} else {
									debug_log_write("Non nettoyage du thème.\n");
								}
							}
							if (clean_logos) {
								debug_log_write("Nettoyage des logos.\n");
							} else {
								debug_log_write("Non nettoyage des logos.\n");
							}
							if (install_hbmenu_choice) {
								debug_log_write("Installation du HBMenu.\n\n");
							} else {
								debug_log_write("Non installation du HBMenu.\n\n");
							}
						}
					bool not_has_enough_space_on_sd;
					if (!beta_mode) {
						not_has_enough_space_on_sd = get_sd_size_left() <= pack_size;
					} else {
						not_has_enough_space_on_sd = get_sd_size_left() <= pack_size_beta;
					}
					if (not_has_enough_space_on_sd) {
						if (debug_enabled) {
							debug_log_write("Pas assez d'espace sur la SD.\n\n");
						}
						printDisplay("\033[0;31m");
						printDisplay(language_vars["lng_error_not_enough_space_on_sd"]);
						printDisplay("\033[0;37m\n");
					} else {
						bool dl_pack_res;
						if (!beta_mode) {
							dl_pack_res = downloadFile(CFW_URL, TEMP_FILE, OFF, true);
						} else {
							dl_pack_res = downloadFile(CFW_URL_beta, TEMP_FILE, OFF, true);
						}
						if (dl_pack_res) {
							set_90dns();
							if (clean_theme) {
								clean_sd(true);
							} else {
								clean_sd(false);
							}
							int unzip_res;
							if (!beta_mode) {
								unzip_res = unzip(TEMP_FILE, subfolder_in_zip);
							} else {
								unzip_res = unzip(TEMP_FILE, subfolder_in_zip_beta);
							}
							if (unzip_res == 0) {
								remove(TEMP_FILE);
								if (clean_logos) {
										if (!beta_mode) {
											fnc_clean_logo(atmo_logo_dir, hekate_nologo_file_path);
										} else {
											fnc_clean_logo(atmo_logo_dir_beta, hekate_nologo_file_path_beta);
										}
								}
								if (install_hbmenu_choice) {
									install_hbmenu();
								}
								if (update_firmware) {
									fnc_install_firmware();
								}
								if (debug_enabled) {
									debug_log_write("Installation du pack OK.\n\n");
								}
								printDisplay("\033[0;32m\n");
								printDisplay(language_vars["lng_success_reboot_in_five_seconds"]);
								printDisplay("\033[0;37m\n");
								sleep(5);
								aply_reboot();
							} else {
								if (debug_enabled) {
									debug_log_write("Erreur durant la décompression du pack.\n\n");
								}
								remove(TEMP_FILE);
							}
						} else {
							if (debug_enabled) {
								debug_log_write("Erreur de téléchargement du pack.\n\n");
							}
							printDisplay("\033[0;31m");
							printDisplay(language_vars["lng_install_pack_download_pack_error"]);
							printDisplay("\033[0;37m\n");
							remove(TEMP_FILE);
						}
					}
				} else {
					if (debug_enabled) {
						debug_log_write("Annulation de l'installation du pack.\n\n");
					}
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_APP:
			{
				if (debug_enabled) {
					debug_log_write("Mise à jour de l'application.\n");
				}
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
				if (get_sd_size_left() <= 4000000) {
					if (debug_enabled) {
						debug_log_write("Pas assez d'espace sur la SD.\n\n");
					}
					printDisplay("\033[0;31m");
					printDisplay(language_vars["lng_error_not_enough_space_on_sd"]);
					printDisplay("\033[0;37m\n");
				} else {
					bool dl_app_res;
					if (!beta_mode) {
						dl_app_res = downloadFile(APP_URL, TEMP_FILE, OFF, true);
					} else {
						dl_app_res = downloadFile(APP_URL_beta, TEMP_FILE, OFF, true);
					}
					if (dl_app_res) {
						if (!custom_cp((char*) "romfs:/nro/aiosu-forwarder.nro", (char*) "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro")) {
							if (debug_enabled) {
								debug_log_write("Erreur de copie de Aiosu-forwarder.\n\n");
							}
							printDisplay("\033[0;31m");;
							printDisplay(language_vars["lng_error_copy_file"]);
							printDisplay("\033[0;37m");;
						} else {
							if (debug_enabled) {
								debug_log_write("Mise à jour de l'application OK.\n\n");
							}
							printDisplay("\033[0;32m\n");
							printDisplay(language_vars["lng_success_reboot_in_five_seconds"]);
							printDisplay("\033[0;37m\n");
							sleep(5);
							appExit();
							envSetNextLoad("/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro", "\"/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro\"");
							return 0;
						}
					} else {
						if (debug_enabled) {
							debug_log_write("Erreur de téléchargement de l'application.\n\n");
						}
						printDisplay("\033[0;31m");
						printDisplay(language_vars["lng_install_app_download_app_error"]);
						printDisplay("\033[0;37m\n");
						remove(TEMP_FILE);
					}
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_90DNS:
			{
				if (debug_enabled) {
					debug_log_write("Mise en place de 90DNS.\n");
				}
				consoleSelect(&logs_console);
				if (set_90dns()) {
						if (debug_enabled) {
							debug_log_write("Application de 90DNS OK.\n\n");
						}
					printDisplay("\033[0;32m\n");
					printDisplay(language_vars["lng_success_reboot_in_five_seconds"]);
					printDisplay("\033[0;37m\n");
					sleep(5);
					simple_reboot();
				} else {
						if (debug_enabled) {
							debug_log_write("Erreur durant l'application de 90DNS.\n\n");
						}
						printDisplay("\033[0;31m");
						printDisplay(language_vars["lng_dns_end_install_error"]);
						printDisplay("\033[0;37m\n");
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_ATMO_PROTECT_CONFIGS:
			{
				if (debug_enabled) {
					debug_log_write("Mise en place des protections d'Atmosphere.\n");
				}
				consoleSelect(&logs_console);
				printDisplay(language_vars["lng_protect_console_begin"]);
				printDisplay("\n");
				if (get_sd_size_left() <= 100000) {
					if (debug_enabled) {
						debug_log_write("Pas assez d'espace sur la SD.\n\n");
					}
					printDisplay("\033[0;31m");
					printDisplay(language_vars["lng_error_not_enough_space_on_sd"]);
					printDisplay("\033[0;37m\n");
				} else {
					mkdir((char*) "/atmosphere", 0777);
					mkdir((char*) "/atmosphere/config", 0777);
					mkdir((char*) "/atmosphere/hosts", 0777);
					mkdir((char*) "/bootloader", 0777);
					bool test_cp = true;
					if (!custom_cp((char*) "romfs:/config_files/exosphere.ini", (char*) "/exosphere.ini")) test_cp = false;
					if (!custom_cp((char*) "romfs:/config_files/system_settings.ini", (char*) "/atmosphere/config/system_settings.ini")) test_cp = false;
					if (!custom_cp((char*) "romfs:/config_files/default.txt", (char*) "/atmosphere/hosts/default.txt")) test_cp = false;
					if (!custom_cp((char*) "romfs:/config_files/hekate_ipl.ini", (char*) "/bootloader/hekate_ipl.ini")) test_cp = false;
					if (!set_90dns()) test_cp = false;
					if (test_cp) {
						if (debug_enabled) {
							debug_log_write("Application des protections OK.\n\n");
						}
						printDisplay("\033[0;32m\n");
						printDisplay(language_vars["lng_success_reboot_in_five_seconds"]);
						printDisplay("\033[0;37m\n");
						sleep(5);
						simple_reboot();
					} else {
						if (debug_enabled) {
							debug_log_write("Erreur durant l'application des protections.\n\n");
						}
						printDisplay("\033[0;31m\n");
						printDisplay(language_vars["lng_protect_console_error"]);
						printDisplay("\033[0;37m\n");
					}
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_HBMENU_INSTALL:
			{
				consoleSelect(&logs_console);
				install_hbmenu();
				consoleSelect(&menu_console);
				break;
			}
			}

		} else if (kDown & HidNpadButton_X) {
			logs_console_clear();
			display_infos();

		} else if (kDown & HidNpadButton_Y) {
			logs_console_clear();
			record_infos();

		} else if (kDown & HidNpadButton_Minus) {
			logs_console_clear();
			switch_app_mode();
			get_last_version_pack();
			debug_log_write("Dernière version du pack: %s\n\n", last_pack_version);
			remove(TEMP_FILE);
			cursor = 0;
			refreshScreen(cursor);

		// exit...
		} else if (kDown & HidNpadButton_Plus) {
			break;
		}
	}

	// cleanup then exit
	appExit();
	if (!beta_mode) {
		if (exit_mode_param != 0) {
			__nx_applet_exit_mode = 1;
		}
	} else {
		if (exit_mode_param_beta != 0) {
			__nx_applet_exit_mode = 1;
		}
	}
	return 0;
}
