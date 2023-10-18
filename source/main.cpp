#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <filesystem>
#include <vector>
#include "contents_install/sdInstall.hpp"
#include "contents_install/hos/hos_Titles.hpp"
#include "contents_install/util/util.hpp"

#include "main_util.h"
#include "ini_hekate_configs_parse.h"
#include "90dns_setter.hpp"
#include "90dns_tester.hpp"
#include "download.hpp"
#include "unzip.hpp"
#include "reboot_to_payload.hpp"
#include "firmwares_install/daybreak-cli.hpp"
#include "translate.hpp"
#include "main_config.h"

extern u32 __nx_applet_exit_mode;
const u64 hbmenu_title_id = 0x0104444444441001;
const u64 app_title_id = 0x0157ba2eaeab0000;

translation_map language_vars;
#define ROOT					"/"
#define APP_PATH				"/switch/AIO_LS_pack_Updater/"
#define APP_OUTPUT			  "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"

#define APP_VERSION			 "6.20.02"
#define CURSOR_LIST_MAX		 7
#define UP_APP		  0
#define UP_CFW		  1
#define UP_FW		  2
#define UP_HBMENU_INSTALL		  3
#define UP_90DNS		  4
#define UP_ATMO_PROTECT_CONFIGS		  5
#define UP_APP_INSTALL		  6
#define UP_RESET		  7
const char *OPTION_LIST[CURSOR_LIST_MAX+1];
bool debug_enabled = true;

char CFW_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/Switch_AIO_LS_pack.zip";
char pack_sha256_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_pack.txt";
char CFW_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
char pack_sha256_url_beta[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_pack_beta.txt";
char pack_custom_files_url[1003] = "";
char pack_custom_files_url_beta[1003] = "";
char pack_custom_files_sha256_url[1003] = "";
char pack_custom_files_sha256_url_beta[1003] = "";
char pack_custom_files_subfolder_in_zip[FS_MAX_PATH] = "";
char pack_custom_files_subfolder_in_zip_beta[FS_MAX_PATH] = "";
s64 pack_custom_files_size = 10000000;
s64 pack_custom_files_size_beta = 10000000;
char pack_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/pack_version.txt";
char pack_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/pack_version.txt";
char pack_version_local_filepath[FS_MAX_PATH] = "/pack_version.txt";
char pack_version_local_filepath_beta[FS_MAX_PATH] = "/pack_version.txt";
char subfolder_in_zip[FS_MAX_PATH] = "";
char subfolder_in_zip_beta[FS_MAX_PATH] = "switch_AIO_LS_pack-main/";
s64 pack_size = 1000000000;
s64 pack_size_beta = 1000000000;
char APP_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/AIO_LS_pack_Updater.nro";
char app_sha256_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_AIO_LS_pack_Updater.txt";
char app_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/app_version.txt";
char APP_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro";
char app_sha256_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/sha256_AIO_LS_pack_Updater.txt";
char app_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/app_version.txt";
char firmware_path[FS_MAX_PATH] = "/dernier_firmware_compatible";
char firmware_path_beta[FS_MAX_PATH] = "/dernier_firmware_compatible";
char atmo_logo_dir[FS_MAX_PATH] = "logo";
char atmo_logo_dir_beta[FS_MAX_PATH] = "logo";
char hekate_nologo_file_path[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
char hekate_nologo_file_path_beta[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
int pack_files_in_zip_sha256_verify_before_copy_param = 1;
int pack_files_in_zip_sha256_verify_before_copy_param_beta = 1;
int exit_mode_param = 0;
int exit_mode_param_beta = 0;
int install_pack_hekate_autoboot_choice_time = 0;
int install_pack_hekate_autoboot_choice_time_beta = 0;
int debug_enabled_param = 0;
int debug_enabled_param_beta = 0;

char pack_version[30];
char last_pack_version[30];
char pack_sha256[65];
char custom_files_pack_sha256[65];
char app_sha256[65];
char last_app_version[30];
char firmware_version[50];
char atmosphere_version[50];
char emummc_value[50];
char emummc_type[50];
EmummcPaths emummc_paths{};
char fusee_gelee_patch[50];
char console_model[50];
u64 console_id = 0;
SetSysSerialNumber console_serial;
// bool sd_is_exfat;
bool console_is_erista = false;
int hekate_autoboot = 0;
int hekate_autoboot_lineno = -1;
int hekate_autoboot_config = 0;
int hekate_autoboot_config_lineno = -1;

autoconfig_configuration autoconfig_config;
bool autoconfig_enabled = false;

bool beta_mode = false;



PadState pad;
MemoryStruct_t file_dl_in_memory;

PrintConsole menu_console;
PrintConsole logs_console;
/* ConsoleFont custom_font;
custom_font->gfx=;
custom_font->asciiOffset=;
custom_font->numChars=;
custom_font->tileWidth=;
custom_font->tileHeight=;
*/

void refreshScreen(int cursor) {
	consoleSelect(&menu_console);	consoleClear();
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
	if (!beta_mode) {
		printf(language_vars["lng_minus_menu"]);
	} else {
		printf(language_vars["lng_minus_menu_beta"]);
	}
	printf("\n\n");
	printf(language_vars["lng_ls_or_rs_menu"]);
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

void set_consoles_default_size() {
	PrintConsole* temp;
	consoleSetWindow(&menu_console, 0, 0, 80, 26);
	consoleSetWindow(&logs_console, 0, 27, 80, 18);
	temp = consoleSelect(&menu_console);
	consoleClear();
	consoleSelect(&logs_console);
	consoleClear();
	consoleSelect(temp);
	consoleUpdate(&menu_console);
	consoleUpdate(&logs_console);
		//  logs_console.windowWidth = 80;
	// logs_console.windowHeight = 18;
		// logs_console.windowX = 0;
	// logs_console.windowY = 27;
}

void fullscreen_menu() {
	consoleSetWindow(&logs_console, 0, 0, 0, 0);
	//  logs_console.windowWidth = 0;
	// logs_console.windowHeight = 0;
	// logs_console.windowX = 0;
	// logs_console.windowY = 0;
	consoleSelect(&menu_console);
	consoleSetWindow(&menu_console, 0, 0, 80, 43);
	consoleClear();
	consoleUpdate(&menu_console);
	consoleUpdate(&logs_console);
}

void help_menu() {
	fullscreen_menu();
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
	printf(language_vars["lng_move_menu"]);
	printf("\n");
	printf(language_vars["lng_a_menu"]);
	printf("\n");
	printf(language_vars["lng_minus_switch_menu"]);
	printf("\n\n");
	printf(language_vars["lng_x_menu"]);
	printf("\n");
	printf(language_vars["lng_y_menu"]);
	printf("\n");
	printf(language_vars["lng_l_r_menu"]);
	printf("\n");
	printf(language_vars["lng_zl_zr_menu"]);
	printf("\n");
	printf(language_vars["lng_l_zl_menu"]);
	printf("\n");
	printf(language_vars["lng_r_zr_menu"]);
	printf("\n");
	printf(language_vars["lng_l_zr_menu"]);
	printf("\n");
	printf(language_vars["lng_r_zl_menu"]);
	printf("\n\n");
	printf(language_vars["lng_plus_menu"]);
	printf("\n\n");
	printf(language_vars["lng_press_b_to_go_back"]);
	consoleUpdate(&menu_console);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_B) {
			break;
		}
	}
	consoleClear();
	consoleUpdate(&menu_console);
	set_consoles_default_size();
}

int appInit() {
	// menu_console = consoleGetDefault();
	consoleInit(&menu_console);
	if (isApplet()) {
		menu_console.bg = 4;
	}
	/*
	logs_console.font = menu_console->font;
	logs_console.renderer = NULL;
	logs_console.cursorX = 0;
	logs_console.cursorY = 0;
	logs_console.prevCursorX = 0;
	logs_console.prevCursorY = 0;
	//
	logs_console.consoleWidth = 80;
	logs_console.consoleHeight = 18;
	logs_console.windowX = 0;
	logs_console.windowY = 27;
	logs_console.windowWidth = 80;
	logs_console.windowHeight = 18;
	logs_console.bg = 0;
	//
	logs_console.tabSize = 3;
	logs_console.fg = 7;
	logs_console.flags = 0;
	*/
	consoleInit(&logs_console);
	set_consoles_default_size();
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
	chdir(ROOT);
	mkdir(APP_PATH, 0777);
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
		char * buffer = (char *) malloc( 30 );
		fgets(buffer, 30, pack_version_file);
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
		res = downloadInMemory(pack_version_url, &file_dl_in_memory, OFF, false);
	} else {
		res = downloadInMemory(pack_version_url_beta, &file_dl_in_memory, OFF, false);
	}
	if (res) {
		logs_console_clear();
		pack_version_file = fmemopen(file_dl_in_memory.memory, file_dl_in_memory.size, "r");
		if (pack_version_file == NULL) {
			consoleSelect(&menu_console);
			free(file_dl_in_memory.memory);
			file_dl_in_memory.size = 0;
			return;
		}
		char * buffer = (char *) malloc( 30 );
		fgets(buffer, 30, pack_version_file);
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
		free(file_dl_in_memory.memory);
		file_dl_in_memory.size = 0;
	}
	consoleSelect(&menu_console);
}

void get_last_sha256_custom_files_pack() {
	FILE *pack_sha256_file;
	bool res;
	if (!beta_mode) {
		if (strcmp(pack_custom_files_sha256_url, "") == 0) {
			return;
		}
		res = downloadInMemory(pack_custom_files_sha256_url, &file_dl_in_memory, OFF, false);
	} else {
		if (strcmp(pack_custom_files_sha256_url_beta, "") == 0) {
			return;
		}
		res = downloadInMemory(pack_custom_files_sha256_url_beta, &file_dl_in_memory, OFF, false);
	}
	if (res) {
		pack_sha256_file = fmemopen(file_dl_in_memory.memory, file_dl_in_memory.size, "r");
		if (pack_sha256_file == NULL) {
			free(file_dl_in_memory.memory);
			file_dl_in_memory.size = 0;
			return;
		}
		char * buffer = (char *) malloc( 65 );
		fgets(buffer, 65, pack_sha256_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				custom_files_pack_sha256[i] = buffer[i];
			}
			i++;
		}
		custom_files_pack_sha256[i] = '\0';
		free(buffer);
		fclose(pack_sha256_file);
		free(file_dl_in_memory.memory);
		file_dl_in_memory.size = 0;
	}
}

void get_last_sha256_pack() {
	FILE *pack_sha256_file;
	bool res;
	if (!beta_mode) {
		if (strcmp(pack_sha256_url, "") == 0) {
			return;
		}
		res = downloadInMemory(pack_sha256_url, &file_dl_in_memory, OFF, false);
	} else {
		if (strcmp(pack_sha256_url_beta, "") == 0) {
			return;
		}
		res = downloadInMemory(pack_sha256_url_beta, &file_dl_in_memory, OFF, false);
	}
	if (res) {
		logs_console_clear();
		consoleSelect(&logs_console);
		pack_sha256_file = fmemopen(file_dl_in_memory.memory, file_dl_in_memory.size, "r");
		if (pack_sha256_file == NULL) {
			free(file_dl_in_memory.memory);
			file_dl_in_memory.size = 0;
			return;
		}
		char * buffer = (char *) malloc( 65 );
		fgets(buffer, 65, pack_sha256_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				pack_sha256[i] = buffer[i];
			}
			i++;
		}
		pack_sha256[i] = '\0';
		free(buffer);
		fclose(pack_sha256_file);
		free(file_dl_in_memory.memory);
		file_dl_in_memory.size = 0;
	}
}

void get_last_sha256_app() {
	FILE *app_sha256_file;
	bool res;
	if (!beta_mode) {
		if (strcmp(app_sha256_url, "") == 0) {
			return;
		}
		res = downloadInMemory(app_sha256_url, &file_dl_in_memory, OFF, false);
	} else {
		if (strcmp(app_sha256_url_beta, "") == 0) {
			return;
		}
		res = downloadInMemory(app_sha256_url_beta, &file_dl_in_memory, OFF, false);
	}
	if (res) {
		logs_console_clear();
		consoleSelect(&logs_console);
		app_sha256_file = fmemopen(file_dl_in_memory.memory, file_dl_in_memory.size, "r");
		if (app_sha256_file == NULL) {
			free(file_dl_in_memory.memory);
			file_dl_in_memory.size = 0;
			return;
		}
		char * buffer = (char *) malloc( 65 );
		fgets(buffer, 65, app_sha256_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				app_sha256[i] = buffer[i];
			}
			i++;
		}
		app_sha256[i] = '\0';
		free(buffer);
		fclose(app_sha256_file);
		free(file_dl_in_memory.memory);
		file_dl_in_memory.size = 0;
	}
}

void get_last_version_app() {
	strcpy(last_app_version, APP_VERSION);
	FILE *pack_version_file;
	bool res;
	consoleSelect(&logs_console);
	if (!beta_mode) {
		res = downloadInMemory(app_version_url, &file_dl_in_memory, OFF, false);
	} else {
		res = downloadInMemory(app_version_url_beta, &file_dl_in_memory, OFF, false);
	}
	if (res) {
		logs_console_clear();
		pack_version_file = fmemopen(file_dl_in_memory.memory, file_dl_in_memory.size, "r");
		if (pack_version_file == NULL) {
			consoleSelect(&menu_console);
			free(file_dl_in_memory.memory);
			file_dl_in_memory.size = 0;
			return;
		}
		char * buffer = (char *) malloc( 30 );
		fgets(buffer, 30, pack_version_file);
		int i = 0;
		while (1) {
			if (buffer[i] == '\n' || buffer[i] == '\0') {
				break;
			} else {
				last_app_version[i] = buffer[i];
			}
			i++;
		}
		last_app_version[i] = '\0';
		free(buffer);
		fclose(pack_version_file);
		free(file_dl_in_memory.memory);
		file_dl_in_memory.size = 0;
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
		smcAmsGetEmunandConfig(&emummc_paths);
		if (emummc_paths.path[0] != '\0' && emummc_paths.nintendo[0] != '\0') {
			strcpy(emummc_type, language_vars["lng_files"]);
		} else if (emummc_paths.path[0] == '\0' && emummc_paths.nintendo[0] != '\0') {
			strcpy(emummc_type, language_vars["lng_partition"]);
		} else {
			strcpy(emummc_type, language_vars["lng_unknown_0"]);
		}
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

void pause_homebrew() {
	consoleSelect(&logs_console);
	printf("%s\n", language_vars["lng_press_any_key_to_continue"]);
	consoleUpdate(&logs_console);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown != 0) {
			break;
		}
	}
	logs_console_clear();
	consoleSelect(&logs_console);
}

bool ask_question(char *question_text, ...) {
	bool rc;
	consoleSelect(&logs_console);
	va_list v;
	va_start(v, question_text);
	vfprintf(stdout, question_text, v);
	va_end(v);
	printf("\n");
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

u64 ask_question_2(char *question_text, ...) {
	consoleSelect(&logs_console);
	va_list v;
	va_start(v, question_text);
	vfprintf(stdout, question_text, v);
	va_end(v);
	printf("\n");
	consoleUpdate(&logs_console);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown!= 0) {
			logs_console_clear();
			consoleSelect(&logs_console);
			return kDown;
		}
	}
}

void display_infos(int cursor) {
	fullscreen_menu();
	if (debug_enabled) {
		debug_log_write("Affichage des informations.\n");
	}
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
		// printf(language_vars["lng_infos_emunand"], emummc_value);
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
	int test_90dns=checkHostnames();
	if (test_90dns == 0) {
		printf(language_vars["lng_infos_90dns_status_ok"]);
	} else if (test_90dns == 1) {
		printf(language_vars["lng_infos_90dns_status_no_conection"]);
	} else if (test_90dns == 2) {
		printf(language_vars["lng_infos_90dns_status_not_protected"]);
	} else {
		printf(language_vars["lng_infos_90dns_status_error"]);
	}
	printf("\n");
	if (GetChargerType() == 0) {
		printf(language_vars["lng_infos_official_charge"], get_battery_charge());
	} else if (GetChargerType() == 1) {
		printf(language_vars["lng_infos_usb_charge"], get_battery_charge());
	} else {
		printf(language_vars["lng_infos_no_charge"], get_battery_charge());
	}
	printf("\n");
	if (hekate_autoboot == 0) {
		printf(language_vars["lng_infos_hekate_autoboot_enabled"]);
	} else {
		printf(language_vars["lng_infos_hekate_autoboot_disabled"]);
	}
	printf("\n\n");
	printf(language_vars["lng_press_b_to_go_back"]);
	consoleUpdate(&menu_console);
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_B) {
			break;
		}
	}
	consoleClear();
	consoleUpdate(&menu_console);
	set_consoles_default_size();
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
		// fprintf(log_infos, language_vars["lng_infos_emunand"], emummc_value);
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
	int test_90dns=checkHostnames();
	if (test_90dns == 0) {
		fprintf(log_infos, language_vars["lng_infos_90dns_status_ok"]);
	} else if (test_90dns == 1) {
		fprintf(log_infos, language_vars["lng_infos_90dns_status_no_conection"]);
	} else if (test_90dns == 2) {
		fprintf(log_infos, language_vars["lng_infos_90dns_status_not_protected"]);
	} else {
		fprintf(log_infos, language_vars["lng_infos_90dns_status_error"]);
	}
	fprintf(log_infos, "\n");
	if (GetChargerType() == 0) {
		fprintf(log_infos, language_vars["lng_infos_official_charge"], get_battery_charge());
	} else if (GetChargerType() == 1) {
		fprintf(log_infos, language_vars["lng_infos_usb_charge"], get_battery_charge());
	} else {
		fprintf(log_infos, language_vars["lng_infos_no_charge"], get_battery_charge());
	}
	fprintf(log_infos, "\n");
	if (hekate_autoboot == 0) {
		fprintf(log_infos, language_vars["lng_infos_hekate_autoboot_enabled"]);
	} else {
		fprintf(log_infos, language_vars["lng_infos_hekate_autoboot_disabled"]);
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

bool install_app_fwd() {
	if (debug_enabled) {
		debug_log_write("Installation du forwarder du homebrew.\n");
	}
	printDisplay(language_vars["lng_app_fwd_install_begin"]);
	printDisplay("\n\n");
	std::vector<std::filesystem::path> nsp_list;
	std::filesystem::path nsp_path;
	inst::util::initInstallServices();
	std::vector<std::pair<u64, u32>> installedTitles;
	installedTitles = inst::util::listInstalledTitles();
	for (long unsigned int i = 0; i < installedTitles.size(); i++) {
		if (installedTitles[i].first == app_title_id) {
			if (debug_enabled) {
				debug_log_write("Désinstallation du forwarder du homebrew.\n");
			}
			printDisplay(language_vars["lng_hbmenu_install_uninstall_begin"]);
			printDisplay("\n");
			if (R_FAILED(hos::RemoveTitle(hos::Locate(hbmenu_title_id)))) {
				if (debug_enabled) {
					debug_log_write("Erreur durant la désinstallation du forwarder du homebrew.\n\n");
				}
				printDisplay("\033[0;31m");
				printDisplay(language_vars["lng_hbmenu_install_uninstall_error"]);
				printDisplay("\033[0;37m\n\n");
				inst::util::deinitInstallServices();
				return false;
			} else {
	if (debug_enabled) {
		debug_log_write("Désinstallation du forwarder du homebrew OK.\n");
	}
				printDisplay("\033[0;32m");
				printDisplay(language_vars["lng_hbmenu_install_uninstall_success"]);
				printDisplay("\033[0;37m\n\n");
			}
			break;
		}
	}
		inst::util::deinitInstallServices();
	nsp_path = "romfs:/nsp/AIO_LS_pack_Updater_0157ba2eaeab0000.nsp";
	nsp_list.push_back(nsp_path);
	if (nspInstStuff::installNspFromFile(nsp_list, 1)) {
		if (debug_enabled) {
			debug_log_write("Installation du forwarder du homebrew OK.\n\n");
		}
		printDisplay("\033[0;32m");
		printDisplay(language_vars["lng_app_fwd_install_success"]);
		printDisplay("\033[0;37m\n");
		return true;
	} else {
		if (debug_enabled) {
			debug_log_write("Erreur d'installation du forwarder du homebrew.\n\n");
		}
		printDisplay("\033[0;31m");
		printDisplay(language_vars["lng_app_fwd_install_error"]);
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
	OPTION_LIST[6] = language_vars["lng_install_app_fwd_menu"];
	OPTION_LIST[7] = language_vars["lng_reset_menu"];
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

void debug_write_config_infos() {
	if (debug_enabled) {
	debug_log_write("Configurations:\n");
	debug_log_write("URL du pack: %s\n", CFW_URL);
	debug_log_write("URL du sha256 du pack: %s\n", pack_sha256_url);
	debug_log_write("URL du pack beta: %s\n", CFW_URL_beta);
	debug_log_write("URL du sha256 du pack beta: %s\n", pack_sha256_url_beta);
	debug_log_write("URL du zip complémentaire au pack: %s\n", pack_custom_files_url);
	debug_log_write("URL du zip complémentaire au pack beta: %s\n", pack_custom_files_url_beta);
	debug_log_write("URL du sha256 du zip complémentaire au pack: %s\n", pack_custom_files_url);
	debug_log_write("URL du sha256 du zip complémentaire au pack beta: %s\n", pack_custom_files_url_beta);
	debug_log_write("Début du chemin du pack dans le zip complémentaire au pack: %s\n", pack_custom_files_subfolder_in_zip);
	debug_log_write("Début du chemin du pack dans le zip complémentaire au pack beta: %s\n", pack_custom_files_subfolder_in_zip_beta);
	debug_log_write("Taille du zip complémentaire au pack: %lli\n", pack_custom_files_size);
	debug_log_write("Taille du zip complémentaire au pack beta: %lli\n", pack_custom_files_size_beta);
	debug_log_write("URL de la version du pack: %s\n", pack_version_url);
	debug_log_write("URL de la version du pack beta: %s\n", pack_version_url_beta);
	debug_log_write("Chemin du fichier local de la version du pack: %s\n", pack_version_local_filepath);
	debug_log_write("Chemin du fichier local de la version du pack beta: %s\n", pack_version_local_filepath_beta);
	debug_log_write("Début du chemin du pack dans le zip: %s\n", subfolder_in_zip);
	debug_log_write("Début du chemin du pack dans le zip beta: %s\n", subfolder_in_zip_beta);
	debug_log_write("Taille du pack: %lli\n", pack_size);
	debug_log_write("Taille du pack beta: %lli\n", pack_size_beta);
	debug_log_write("URL de l'application: %s\n", APP_URL);
	debug_log_write("URL du sha256 de l'application: %s\n", app_sha256_url);
	debug_log_write("URL de la dernière version de l'application: %s\n", app_version_url);
	debug_log_write("URL de l'application beta: %s\n", APP_URL_beta);
	debug_log_write("URL du sha256 de l'application beta: %s\n", app_sha256_url_beta);
	debug_log_write("URL de la dernière version de l'application beta: %s\n", app_version_url_beta);
	debug_log_write("Chemin local du firmware: %s\n", firmware_path);
	debug_log_write("Chemin local du firmware beta: %s\n", firmware_path_beta);
	debug_log_write("Chemin du logo d'Atmosphere: %s\n", atmo_logo_dir);
	debug_log_write("Chemin du logo d'Atmosphere beta: %s\n", atmo_logo_dir_beta);
	debug_log_write("Chemin de la config sans logo de Hekate: %s\n", hekate_nologo_file_path);
	debug_log_write("Chemin de la config sans logo de Hekate beta: %s\n", hekate_nologo_file_path_beta);
	debug_log_write("Vérifications SHA256 durant l'installation du pack: %i\n", pack_files_in_zip_sha256_verify_before_copy_param);
	debug_log_write("Vérifications SHA256 durant l'installation du pack beta: %i\n", pack_files_in_zip_sha256_verify_before_copy_param_beta);
	debug_log_write("Méthode de fermeture de l'application: %i\n", exit_mode_param);
	debug_log_write("Méthode de fermeture de l'application beta: %i\n", exit_mode_param_beta);
	debug_log_write("Temps du choix automatique du l'autoboot de Hekate: %i\n", install_pack_hekate_autoboot_choice_time);
	debug_log_write("Temps du choix automatique du l'autoboot de Hekate beta: %i\n", install_pack_hekate_autoboot_choice_time_beta);
	debug_log_write("Mode debug: %i\n", debug_enabled_param);
	debug_log_write("Mode debug beta: %i\n\n", debug_enabled_param_beta);
	}
}

void debug_write_console_infos() {
	if (debug_enabled) {
		debug_log_write("Informations sur la console:\n");
		debug_log_write("Version du pack: %s\n", pack_version);
		debug_log_write("Dernière version du pack: %s\n", last_pack_version);
		debug_log_write("Version du firmware: %s\n", firmware_version);
		debug_log_write("Version d'Atmosphere: %s\n", atmosphere_version);
		if (!is_emummc()) {
			debug_log_write("Console en sysnand.\n");
		} else {
			debug_log_write("Console en emunand de type %s.\n", emummc_type);
			if (emummc_paths.path[0] != '\0') {
				debug_log_write("Dossier associé à l'emunand: %s\n", emummc_paths.path);
			}
			debug_log_write("Dossier nintendo associé à l'emunand: %s\n", emummc_paths.nintendo);
			// debug_log_write("Console en emunand.\n");
		}
		debug_log_write("Etat de l'exploit Fusee Gelee: %s\n", fusee_gelee_patch);
		debug_log_write("Modèle de la console: %s\n", console_model);
		debug_log_write("Paramètre autoboot de Hekate: %i\n", hekate_autoboot);
		debug_log_write("Ligne du paramètre autoboot de Hekate dans le fichier de config: %i\n", hekate_autoboot_lineno);
				debug_log_write("Paramètre autoboot_list de Hekate: %i\n", hekate_autoboot_config);
		debug_log_write("Ligne du paramètre autoboot_list de Hekate dans le fichier de config: %i\n", hekate_autoboot_config_lineno);
		debug_log_write("\n");
	}
}

bool verify_update(char* local_version, char* remote_version) {
	if (strcmp(local_version, remote_version) == 0 || strcmp(remote_version, language_vars["lng_unknown_1"]) == 0 || strcmp(remote_version, "") == 0) {
		return false;
	}
	if ((strcmp(local_version, language_vars["lng_unknown_1"]) == 0 || strcmp(local_version, "") == 0) && (strcmp(remote_version, language_vars["lng_unknown_1"]) != 0 || strcmp(remote_version, "") != 0)) {
		return true;
	}
	char local_test[strlen(local_version)+1];
	char remote_test[strlen(remote_version)+1];
	size_t i = 0;
	size_t j = 0;
	size_t k;
	size_t l;
	bool break_local_while;
	bool break_remote_while;
	int local_version_test;
	int remote_version_test;
	remake_test:
	strcpy(local_test, "");
	strcpy(remote_test, "");
	k = 0;
	l = 0;
	break_local_while = false;
	break_remote_while = false;
	local_version_test = -1;
	remote_version_test = -2;
	while(i < strlen(local_version)) {
		if (local_version[i] == '0' || local_version[i] == '1' || local_version[i] == '2' || local_version[i] == '3' || local_version[i] == '4' || local_version[i] == '5' || local_version[i] == '6' || local_version[i] == '7' || local_version[i] == '8' || local_version[i] == '9') {
			local_test[k] = local_version[i];
			k++;
		}
		if (local_version[i] == '.') {
			break_local_while = true;
		}
		i++;
		if (break_local_while) {
			break;
		}
	}
		while(j < strlen(remote_version)) {
	if (remote_version[j] == '0' || remote_version[j] == '1' || remote_version[j] == '2' || remote_version[j] == '3' || remote_version[j] == '4' || remote_version[j] == '5' || remote_version[j] == '6' || remote_version[j] == '7' || remote_version[j] == '8' || remote_version[j] == '9') {
			remote_test[l] = remote_version[j];
			l++;
		}
		if (remote_version[j] == '.') {
			break_remote_while = true;
		}
		j++;
		if (break_remote_while) {
			break;
		}
	}
	local_version_test = atoi(local_test);
	remote_version_test = atoi(remote_test);
	if (remote_version_test > local_version_test) {
		return true;
	} else if (remote_version_test < local_version_test) {
		return false;
	} else {
		if (i == strlen(local_version) && j == strlen(remote_version)) {
			return false;
		} else if (i == strlen(local_version) && j != strlen(remote_version)) {
			return true;
		} else if (i != strlen(local_version) && j == strlen(remote_version)) {
			return false;
		}
		goto remake_test;
	}
}

bool auto_update_app(bool update_app) {
	if (verify_update((char* )APP_VERSION, last_app_version) && !update_app) {
		debug_log_write("Nouvelle version %s de l'app trouvée, la version actuelle de l'app est %s.\n\n", last_app_version, APP_VERSION);
		update_app = ask_question((char*) language_vars["lng_ask_update_app"]);
	}
	if (update_app) {
		if (debug_enabled) {
			debug_log_write("Mise à jour de l'application.\n");
		}
		consoleSelect(&logs_console);
		mkdir(APP_PATH, 0777);
		if (get_sd_size_left() <= 10000000) {
			if (debug_enabled) {
				debug_log_write("Pas assez d'espace sur la SD.\n\n");
				}
			printDisplay("\033[0;31m");
			printDisplay(language_vars["lng_error_not_enough_space_on_sd"]);
			printDisplay("\033[0;37m\n");
		} else {
			get_last_sha256_app();
			bool dl_app_res;
			if (!beta_mode) {
				dl_app_res = downloadFile(APP_URL, TEMP_FILE, OFF, true);
			} else {
				dl_app_res = downloadFile(APP_URL_beta, TEMP_FILE, OFF, true);
			}
			if (dl_app_res) {
				if (strcmp(app_sha256, "") != 0) {
					printDisplay(language_vars["lng_calculate_sha256_of_downloaded_file"]);
					printDisplay("\n");
					char dl_app_sha256[65] = "";
					get_sha256_file(TEMP_FILE, dl_app_sha256);
					if (debug_enabled) {
						debug_log_write("SHA256 de l'app à télécharger: ");
						debug_log_write("%s", app_sha256);
						debug_log_write("\n");
						debug_log_write("SHA256 de l'app téléchargée: ");
						debug_log_write("%s", dl_app_sha256);
						debug_log_write("\n\n");
					}
					if (strcmp(app_sha256, dl_app_sha256) != 0) {
						if (debug_enabled) {
							debug_log_write("Erreur de téléchargement de l'application, installation annulée.\n\n");
						}
						printDisplay("\033[0;31m");
						printDisplay(language_vars["lng_install_app_download_app_error"]);
						printDisplay("\033[0;37m\n");
						remove(TEMP_FILE);
						consoleSelect(&menu_console);
					}
				}
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
					return true;
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
	}
	return false;
}

void hekate_config_menu_refreshScreen(int hekate_config_choice_cursor, const IniData* iniData) {
	consoleSelect(&menu_console);
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
	printf(language_vars["lng_hekate_config_choice_menu_title"]);
	printf("\n\n");
	int limit_display_config = 15;
	int  min_display_config  = 0;
	int  max_display_config  = 0;
	if (hekate_config_choice_cursor  - limit_display_config > 0) {
		min_display_config = hekate_config_choice_cursor  - limit_display_config;
	}
	if (min_display_config + limit_display_config >= (int)iniData->numSections) {
		max_display_config = (int)iniData->numSections;
	} else {
		max_display_config = min_display_config + limit_display_config;
	}
	if (iniData->numSections <= 0) {
		printf(language_vars["lng_hekate_config_choice_no_configs_found"]);
	}
	for (int i = min_display_config; i < max_display_config; i++) {
		Section* section = &(iniData->sections[i]);
		if (hekate_config_choice_cursor == i) {
			printf("\x1B[31m%s\x1B[37m\n", section->section);
		} else {
			printf("%s\n", section->section);
		}
	}
	printf("\n\n");
	printf(language_vars["lng_press_b_to_go_back"]);
	printf("n\n");
	consoleUpdate(&menu_console);
}

int* hekate_config_choice_menu(bool enable_calculate_time) {
	IniData iniData = {NULL, 0};
	// fullscreen_menu();
	static int hekate_selected_config_index[2];
	hekate_selected_config_index[0] = 0;
	hekate_selected_config_index[1] = 0;
	parseIniFile("/bootloader/hekate_ipl.ini", &iniData, true);
	for (size_t i = 0; i < iniData.numSections; i++) {
		Section* section = &(iniData.sections[i]);
		if (strcmp(section->section, "config") == 0) {
			removeSection(&iniData, i);
		}
	}
	int number_of__main_configs = iniData.numSections;
	//Process any file-based boot entries from ini folder
	DIR *dr = opendir("/bootloader/ini");
	struct dirent *de;
	std::vector<std::string> iniFiles;
while ((de = readdir(dr)) != nullptr)
		iniFiles.push_back(de->d_name);
	closedir(dr);
	std::sort(iniFiles.begin(), iniFiles.end());
	if (!iniFiles.empty()) {
		// get boot entries from ini folder files
		char file[FS_MAX_PATH] = "/bootloader/ini/";
		for (auto const &iniFile : iniFiles) {
			strcat(file, iniFile.c_str());
			parseIniFile(file, &iniData,  false);
		}
	}
	
	for (size_t i = 0; i < iniData.numSections; i++) {
		Section* section = &(iniData.sections[i]);
		if (strcmp(section->section, "config") == 0) {
			removeSection(&iniData, i);
		}
		if (section->numKeyValues == 0) {
			removeSection(&iniData, i);
		}
	}
	hekate_config_menu_refreshScreen(hekate_selected_config_index[0], &iniData);
	bool time_calculate = false;
	int install_pack_hekate_autoboot_choice_time_local = 0;
	if (!beta_mode) {
		if (install_pack_hekate_autoboot_choice_time > 0 && enable_calculate_time) {
			time_calculate = true;
			install_pack_hekate_autoboot_choice_time_local = install_pack_hekate_autoboot_choice_time;
		}
	} else {
		if (install_pack_hekate_autoboot_choice_time_beta > 0 && enable_calculate_time) {
			time_calculate = true;
			install_pack_hekate_autoboot_choice_time_local = install_pack_hekate_autoboot_choice_time_beta;
		}
	}
	time_t start_time = time( NULL );
	time_t end_time = time( NULL );
	int passed_time = 0;
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);

		if (iniData.numSections > 0) {
			// move cursor down...
			if (kDown & HidNpadButton_StickLDown || kDown & HidNpadButton_StickRDown || kDown & HidNpadButton_Down) // Could be replaced by HidNpadButton_AnyDown
			{
				time_calculate = false;
				if (hekate_selected_config_index[0] == (int)iniData.numSections - 1) hekate_selected_config_index[0] = 0;
				else hekate_selected_config_index[0]++;
				hekate_config_menu_refreshScreen(hekate_selected_config_index[0], &iniData);
			}

			// move cursor up...
			else if (kDown & HidNpadButton_StickLUp || kDown & HidNpadButton_StickRUp || kDown & HidNpadButton_Up) // Could be replaced by HidNpadButton_AnyUp
			{
				time_calculate = false;
				if (hekate_selected_config_index[0] == 0) hekate_selected_config_index[0] = (int)iniData.numSections - 1;
				else hekate_selected_config_index[0]--;
				hekate_config_menu_refreshScreen(hekate_selected_config_index[0], &iniData);
			}

			else if (kDown & HidNpadButton_A || (time_calculate && passed_time > install_pack_hekate_autoboot_choice_time_local)) {
				time_calculate = false;
				if (iniData.sections[hekate_selected_config_index[0]].is_main_config) {
					hekate_selected_config_index[0]++;
					hekate_selected_config_index[1] = 0;
				} else {
					hekate_selected_config_index[0] = hekate_selected_config_index[0] + 1 - number_of__main_configs;
					hekate_selected_config_index[1] = 1;
				}
				break;;
			}

			else if (kDown & HidNpadButton_B) {
				time_calculate = false;
				hekate_selected_config_index[0] = -1;
				hekate_selected_config_index[1] = -1;
				break;
			}
		}  else {
			if (kDown & HidNpadButton_B || (time_calculate && passed_time > install_pack_hekate_autoboot_choice_time_local)) {
				time_calculate = false;
				hekate_selected_config_index[0] = -2;
				hekate_selected_config_index[1] = -2;
				break;
			}
		}
		if (time_calculate) {
			end_time = time( NULL );
			passed_time = (int) difftime(end_time, start_time);
			printf("Dans %i secondes le choix par defaut sera choisi. ", passed_time);
			printf("\r");
			consoleUpdate(&menu_console);
		}
	}

	freeIniData(&iniData);
	consoleClear();
	consoleUpdate(&menu_console);
	// set_consoles_default_size();
	return hekate_selected_config_index;
}

void update_hekate_autoboot_param(bool enable, int bootentry[2]) {
	consoleSelect(&logs_console);
	if (hekate_autoboot_lineno == -1  ||  hekate_autoboot_config_lineno == -1) {
		if (debug_enabled) {
			debug_log_write("Paramètre autoboot de Hekate inexistant.\n\n");
		}
		printDisplay(language_vars["lng_no_hekate_autoboot_config_set_error"]);
		return;
	}
	char autoboot_param[21] = "autoboot=";
	char autoboot_index_str[11];
	char autoboot_config_param[26] = "autoboot_list=";
	int* autoboot_config_index;
	char autoboot_config_index_str[11];
	if (enable) {
		if (bootentry[0] <= 0) {
			autoboot_config_index = hekate_config_choice_menu(false);
			consoleSelect(&logs_console);
		} else {
			autoboot_config_index  = bootentry;
		}
		if (autoboot_config_index[0] < 0) {
			if (debug_enabled) {
				debug_log_write("Configuration de l'autoboot de Hekate interrompu par l'utilisateur.\n\n");
			}
			printDisplay(language_vars["lng_hekate_autoboot_config_canceled"]);
			return;
		} else {
			sprintf(autoboot_index_str, "%d", autoboot_config_index[0]);
			strcat(strcat(autoboot_param, autoboot_index_str), "\n");
			sprintf(autoboot_config_index_str, "%d", autoboot_config_index[1]);
			strcat(strcat(autoboot_config_param, autoboot_config_index_str), "\n");
		}
	}
	/*
	if (hekate_autoboot == 0 && enable) {
		if (debug_enabled) {
			debug_log_write("Paramètre autoboot de Hekate déjà configuré.\n\n");
		}
		printDisplay(language_vars["lng_hekate_param_already_set"]);
		return;
	}
	*/
	if (hekate_autoboot != 0 && !enable) {
		if (debug_enabled) {
			debug_log_write("Paramètre autoboot de Hekate déjà configuré.\n\n");
		}
		printDisplay(language_vars["lng_hekate_param_already_set"]);
		return;
	}
	FILE* hekate_in=fopen("/bootloader/hekate_ipl.ini", "r");
	if (hekate_in == NULL) {
		if (debug_enabled) {
			debug_log_write("Erreur de lecture du fichier de config de Hekate.\n\n");
		}
		printDisplay(language_vars["lng_hekate_error_config_file_read"]);
		return;
	}
	FILE* hekate_out=fopen("/switch/AIO_LS_pack_Updater/hekate_ipl_temp.ini", "w");
	if (hekate_out == NULL) {
		if (debug_enabled) {
			debug_log_write("Erreur d'écriture du fichier temporaire de la config de Hekate.\n\n");
		}
		printDisplay(language_vars["lng_hekate_error_temp_file_write"]);
		return;
	}
	char buffer[1000];
	int i=1;
	while (fgets( buffer, 1000, hekate_in ) != NULL) {
		if (buffer[strlen(buffer)-1] != '\n') {
			fputs(buffer, hekate_out);
			continue;
		}
		if (i == hekate_autoboot_lineno) {
			if (enable) {
				fputs(autoboot_param, hekate_out);
			} else {
				fputs("autoboot=0\n", hekate_out);
			}
		} else if (i == hekate_autoboot_config_lineno) {
			if (enable) {
				fputs(autoboot_config_param, hekate_out);
			} else {
				fputs(buffer, hekate_out);
			}
		} else {
			fputs(buffer, hekate_out);
		}
		i++;
	}
	fclose(hekate_in);
	fclose(hekate_out);
	if (!custom_cp((char*) "/switch/AIO_LS_pack_Updater/hekate_ipl_temp.ini", (char*) "/bootloader/hekate_ipl.ini")) {
		if (debug_enabled) {
			debug_log_write("Erreur de copie de la nouvelle config de Hekate.\n\n");
		}
		printDisplay(language_vars["lng_hekate_replace_config_error"]);
		return;
	} else {
		remove("/switch/AIO_LS_pack_Updater/hekate_ipl_temp.ini");
	}
	if (enable) {
		if (debug_enabled) {
			debug_log_write("Paramètre autoboot de Hekate activé.\n\n");
		}
		printDisplay(language_vars["lng_hekate_autoboot_enabled_success"]);
	} else {
		if (debug_enabled) {
			debug_log_write("Paramètre autoboot de Hekate désactivé.\n\n");
		}
		printDisplay(language_vars["lng_hekate_autoboot_disabled_success"]);
	}
	get_hekate_autoboot_status();
}

void update_app_autoconfig_params() {
	printDisplay(language_vars["lng_configuring_app_autoconfig"]);
	printDisplay("\n");
	bool full_app_control = false;
	bool beta_launch = false;
	bool update_firmware = false;
	bool clean_theme = false;
	bool agressive_clean = false;
	bool clean_modules = false;
	bool keep_files=false;
	bool hekate_autoboot_enable = false;
	bool clean_logos = false;
	bool install_hbmenu_choice = false;
	bool install_app_fwd_choice = false;
	full_app_control = ask_question((char*) language_vars["lng_ask_autoconfig_full_control"]);
	beta_launch = ask_question((char*) language_vars["lng_ask_autoconfig_beta_launch"]);
	update_firmware = ask_question((char*) language_vars["lng_ask_update_firmware"]);
	if (update_firmware) {
		clean_theme = true;
	} else {
		clean_theme = ask_question((char*) language_vars["lng_ask_clean_theme"]);
	}
	agressive_clean = ask_question((char*) language_vars["lng_ask_agressive_clean"]);
	clean_modules = ask_question((char*) language_vars["lng_ask_clean_modules"]);
	keep_files = ask_question((char*) language_vars["lng_ask_keep_files"]);
	hekate_autoboot_enable = ask_question((char*) language_vars["lng_ask_hekate_autoboot"]);;
	clean_logos = ask_question((char*) language_vars["lng_ask_clean_logos"]);
	install_hbmenu_choice = ask_question((char*) language_vars["lng_ask_hbmenu_install"]);
	install_app_fwd_choice = ask_question((char*) language_vars["lng_ask_app_fwd_install"]);
	printDisplay(language_vars["lng_autoconfig_recap_begin"]);
	printDisplay("\n\n");
	if (full_app_control) {
		printDisplay(language_vars["lng_autoconfig_recap_full_app_control"]);
	} else {
		printDisplay(language_vars["lng_autoconfig_recap_not_full_app_control"]);
	}
	printDisplay("\n");
	if (beta_launch) {
		printDisplay(language_vars["lng_autoconfig_recap_beta_launch"]);
	} else {
		printDisplay(language_vars["lng_autoconfig_recap_not_beta_launch"]);
	}
	printDisplay("\n");
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
	if (agressive_clean) {
		printDisplay(language_vars["lng_install_pack_recap_agressive_clean"]);
	} else {
		printDisplay(language_vars["lng_install_pack_recap_not_agressive_clean"]);
	}
	printDisplay("\n");
	if (clean_modules) {
		printDisplay(language_vars["lng_install_pack_recap_clean_modules"]);
	} else {
		printDisplay(language_vars["lng_install_pack_recap_not_clean_modules"]);
	}
	printDisplay("\n");
	if (keep_files) {
		printDisplay(language_vars["lng_install_pack_recap_keep_files"]);
	} else {
		printDisplay(language_vars["lng_install_pack_recap_not_keep_files"]);
	}
	printDisplay("\n");
	if (hekate_autoboot_enable) {
		printDisplay(language_vars["lng_install_pack_recap_enable_hekate_autoboot"]);
	} else  {
		printDisplay(language_vars["lng_install_pack_recap_not_enable_hekate_autoboot"]);
	}
	printDisplay("\n");
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
	if (install_app_fwd_choice) {
		printDisplay(language_vars["lng_install_pack_recap_install_app_fwd"]);
	} else {
		printDisplay(language_vars["lng_install_pack_recap_not_install_app_fwd"]);
	}
	printDisplay("\n");
	bool validate_choice = ask_question((char*) language_vars["lng_ask_validate_choices"]);
	if (validate_choice) {
		FILE* config = fopen("/switch/AIO_LS_pack_Updater/autoconfig.ini", "w");
		if (config == NULL) {
			printDisplay(language_vars["lng_autoconfig_set_error"]);
			return;
		}
		fputs("[autoconfig]\n", config);
		if (full_app_control) {
			fputs("use_all_app_functions = 1\n", config);
		} else {
			fputs("use_all_app_functions = 0\n", config);
		}
		if (beta_launch) {
			fputs("pack_beta_enable = 1\n", config);
		} else {
			fputs("pack_beta_enable = 0\n", config);
		}
		if (update_firmware) {
			fputs("install_firmware = 1\n", config);
		} else {
			fputs("install_firmware = 0\n", config);
		}
		if (clean_theme) {
			fputs("delete_theme = 1\n", config);
		} else {
			fputs("delete_theme = 0\n", config);
		}
		if (agressive_clean) {
			fputs("agressive_clean = 1\n", config);
		} else {
			fputs("agressive_clean = 0\n", config);
		}
		if (clean_modules) {
			fputs("module_clean = 1\n", config);
		} else {
			fputs("module_clean = 0\n", config);
		}
		if (keep_files) {
			fputs("delete_some_files_protection = 1\n", config);
		} else {
			fputs("delete_some_files_protection = 0\n", config);
		}
		if (hekate_autoboot_enable) {
			fputs("hekate_autoboot_enable = 1\n", config);
		} else {
			fputs("hekate_autoboot_enable = 0\n", config);
		}
		if (clean_logos) {
			fputs("delete_logos = 1\n", config);
		} else {
			fputs("delete_logos = 0\n", config);
		}
		if (install_hbmenu_choice) {
			fputs("hbmenu_install = 1\n", config);
		} else {
			fputs("hbmenu_install = 0\n", config);
		}
		if (install_app_fwd_choice) {
			fputs("app_fwd_install = 1\n", config);
		} else {
			fputs("app_fwd_install = 0\n", config);
		}
		fclose(config);
		printDisplay(language_vars["lng_autoconfig_set_success"]);
	}
}

int main(int argc, char **argv) {
	// init stuff
	appInit();
	bool debug_already_started = false;
	if (argc == 1 && debug_enabled) {
		debug_log_start();
		debug_already_started = true;
	}
	configs_init();
	if (argc == 1 && debug_enabled && !debug_already_started) {
		debug_log_start();
		debug_already_started = true;
	}
	int first_char_slash_argv_0 = 0;
	for (size_t i = 0; i < strlen(argv[0]); i++) {
		if (argv[0][i] == '/') {
			break;
		}
		first_char_slash_argv_0++;
	}
	char* arg0_substring = substr(argv[0], first_char_slash_argv_0, strlen(argv[0]));;
	if (strcmp(arg0_substring, "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") != 0) {
		free(arg0_substring);
		if (!custom_cp((char*) "romfs:/nro/aiosu-forwarder.nro", (char*) "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro")) {
			if (debug_enabled) {
				debug_log_write("Erreur de copie de Aiosu-forwarder.\n\n");
				appExit();
				return 0;
			}
		} else {
			if (debug_enabled) {
				debug_log_write("Reconfiguration de l'application OK.\n\n");
			}
			char temp_app_nro_path[2000] = "\"/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro\"";
			strcat(strcat(strcat(temp_app_nro_path, " \""), argv[0]), "\"");
			appExit();
			envSetNextLoad("/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro", temp_app_nro_path);
			return 0;
		}
	}
	free(arg0_substring);
	language_vars = set_translation_strings();
	menu_init();

	strcpy(pack_version, language_vars["lng_unknown_1"]);
	strcpy(last_pack_version, language_vars["lng_unknown_1"]);
	strcpy(last_app_version, language_vars["lng_unknown_1"]);
	strcpy(firmware_version, language_vars["lng_unknown_1"]);
	strcpy(atmosphere_version, language_vars["lng_unknown_1"]);
	strcpy(emummc_value, language_vars["lng_unknown_1"]);
	strcpy(emummc_type, language_vars["lng_unknown_0"]);
	strcpy(fusee_gelee_patch, language_vars["lng_unknown_0"]);
	strcpy(console_model, language_vars["lng_unknown_0"]);
	padInitializeDefault(&pad);

	// change directory to root (defaults to /switch/)
	chdir(ROOT);

	// set the cursor position to 0
	short cursor = 0;

	if (debug_enabled) {
		debug_log_write("Version du homebrew: %s\n", APP_VERSION);
	}
	if (isApplet()) {
		if (debug_enabled) {
			debug_log_write("Homebrew en mode applet.\n");
		}
		// Display background of the menu console in blue
		// printf("\x1B[44m");
	} else {
		if (debug_enabled) {
			debug_log_write("Homebrew hors mode applet.\n\n");
		}
	}
	debug_write_config_infos();

	u64 kDown = 0;
	u64 kHeld = 0;

	//  verify  autoconfig
	get_autoconfig();
	if (autoconfig_enabled) {
		if (autoconfig_config.c1.use_all_app_functions != 1) {
			logs_console_clear();
			consoleSelect(&logs_console);
			time_t start_time;
			time_t cur_time;
			double dif = 0;
			double wait_time = 3;
			printDisplay(language_vars["lng_ask_disable_autoconfig"], wait_time);
			time(&start_time);
			while(dif <= wait_time) {
				padUpdate(&pad);
				kDown = padGetButtonsDown(&pad);
				if (kDown & HidNpadButton_A) {
					if (debug_enabled) {
						debug_log_write("Désactivation de l'auto-configuration du homebrew.\n");
					}
					remove("/switch/AIO_LS_pack_Updater/autoconfig.ini");
					get_autoconfig();
					printDisplay(language_vars["lng_autoconfig_disabled"]);
					printDisplay("\n");
					pause_homebrew();
					logs_console_clear();
					kDown = 0;
					consoleSelect(&menu_console);
					break;
				}
				time(&cur_time);
				dif = difftime(cur_time, start_time);
			}
			consoleSelect(&menu_console);
			kDown = 0;
		}
	}
	if (autoconfig_enabled) {
		if (autoconfig_config.c1.use_all_app_functions != 1) {
			if (debug_enabled) {
				debug_log_write("Homebrew en mode Autoconfig uniquement.\n");
			}
			cursor = UP_CFW;
			kDown = HidNpadButton_A;
			__nx_applet_exit_mode = 1;
		} else {
			if (debug_enabled) {
				debug_log_write("Homebrew en mode Autoconfig avec accès aux autres fonctions du homebrew.\n");
			}
		}
		if (autoconfig_config.c1.pack_beta_enable == 1) {
			switch_app_mode();
		}
	} else {
		if (debug_enabled) {
			debug_log_write("Autoconfig du homebrew désactivée.\n");
	}
	}

	get_version_pack();
	get_last_version_pack();
	get_last_version_app();
	get_fw_version();
	get_ams_version();
	get_fusee_gelee_exploit();
	get_device_id();
	get_serial_number();
	set_emummc_values();
	get_hekate_autoboot_status();
	debug_write_console_infos();
	remove(TEMP_FILE);

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

	if (debug_enabled) {
		checkHostnames();
	}
	// main menu
	refreshScreen(cursor);

	if (auto_update_app(false)) {
		return 0;
	}

	bool pack_update_found = verify_update(pack_version, last_pack_version);
	bool pack_update_found_install = false;
	if (pack_update_found) {
		if (!autoconfig_enabled || autoconfig_config.c1.use_all_app_functions == 1) {
			logs_console_clear();
			consoleSelect(&logs_console);
			printDisplay(language_vars["lng_pack_have_update"], pack_version, last_pack_version);
			printDisplay("\n\n");
			pack_update_found_install=ask_question((char*) language_vars["lng_ask_pack_have_update"]);
			logs_console_clear();
			consoleSelect(&menu_console);
			if (pack_update_found_install) {
				cursor = UP_CFW;
				kDown = HidNpadButton_A;
			}
		}
	}

	// Loop for the menu
	bool hekate_autoboot_enable_combot_disable = false;
	bool hekate_autoboot_disable_combot_disable = false;
	int hekate_autoboot_chosen[2];
	hekate_autoboot_chosen[0] = -1;
	hekate_autoboot_chosen[1] = -1;
	while(appletMainLoop()) {
		if ((!autoconfig_enabled || autoconfig_config.c1.use_all_app_functions == 1) && !pack_update_found_install) {
			padUpdate(&pad);
			kDown = padGetButtonsDown(&pad);
			kHeld = padGetButtons(&pad);
		}

		if (kHeld & HidNpadButton_L && kHeld & HidNpadButton_R) {
			if (debug_enabled) {
				debug_log_write("Appel du redémarrage de la console.");
			}
			simple_reboot();
		}
		if (kHeld & HidNpadButton_ZL && kHeld & HidNpadButton_ZR) {
			if (debug_enabled) {
				debug_log_write("Appel du redémarrage avec payload de nettoyage de la console.");
			}
			aply_reboot();
		}
		if (kHeld & HidNpadButton_R && kHeld & HidNpadButton_ZR && !hekate_autoboot_enable_combot_disable) {
			if (debug_enabled) {
				debug_log_write("Activation de l'auto-boot de Hekate.\n");
			}
			logs_console_clear();
			consoleSelect(&logs_console);
			printDisplay(language_vars["lng_install_pack_configuring_hekate_autoboot"]);
			printDisplay("\n");
			update_hekate_autoboot_param(true, hekate_autoboot_chosen);
			refreshScreen(cursor);
			//  hekate_autoboot_enable_combot_disable = true;
			hekate_autoboot_disable_combot_disable = false;
		}
		if (kHeld & HidNpadButton_L && kHeld & HidNpadButton_ZL && !hekate_autoboot_disable_combot_disable) {
			if (debug_enabled) {
				debug_log_write("Désactivation de l'auto-boot de Hekate.\n");
			}
			logs_console_clear();
			consoleSelect(&logs_console);
			printDisplay(language_vars["lng_install_pack_configuring_hekate_autoboot"]);
			printDisplay("\n");
			update_hekate_autoboot_param(false, hekate_autoboot_chosen);
			consoleSelect(&menu_console);
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = true;
		}
		if (kHeld & HidNpadButton_R && kHeld & HidNpadButton_ZL) {
			if (debug_enabled) {
				debug_log_write("Activation de l'auto-configuration du homebrew.\n");
			}
			logs_console_clear();
			consoleSelect(&logs_console);
			update_app_autoconfig_params();
			get_autoconfig();
				if (autoconfig_enabled) {
					sleep(5);
					if (!beta_mode) {
						if (exit_mode_param != 0) {
							__nx_applet_exit_mode = 1;
						}
					} else {
						if (exit_mode_param_beta != 0) {
							__nx_applet_exit_mode = 1;
						}
					}
					appExit();
					return 0;
				}
			consoleSelect(&menu_console);
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
		}
		if (autoconfig_enabled) {
			if (kHeld & HidNpadButton_L && kHeld & HidNpadButton_ZR) {
				if (debug_enabled) {
					debug_log_write("Désactivation de l'auto-configuration du homebrew.\n");
				}
				logs_console_clear();
				consoleSelect(&logs_console);
				remove("/switch/AIO_LS_pack_Updater/autoconfig.ini");
				get_autoconfig();
				printDisplay(language_vars["lng_autoconfig_disabled"]);
				consoleSelect(&menu_console);
				hekate_autoboot_enable_combot_disable = false;
				hekate_autoboot_disable_combot_disable = false;
			}
		}

		// move cursor down...
		if (kDown & HidNpadButton_StickLDown || kDown & HidNpadButton_StickRDown || kDown & HidNpadButton_Down) // Could be replaced by HidNpadButton_AnyDown
		{
			if (cursor == CURSOR_LIST_MAX) cursor = 0;
			else cursor++;
			logs_console_clear();
			refreshScreen(cursor);
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
		}

		// move cursor up...
		else if (kDown & HidNpadButton_StickLUp || kDown & HidNpadButton_StickRUp || kDown & HidNpadButton_Up) // Could be replaced by HidNpadButton_AnyUp
		{
			if (cursor == 0) cursor = CURSOR_LIST_MAX;
			else cursor--;
			logs_console_clear();
			refreshScreen(cursor);
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
		}

		else if (kDown & HidNpadButton_StickL || kDown & HidNpadButton_StickR) {
			logs_console_clear();
			help_menu();
			refreshScreen(cursor);
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
		}

		else if (kDown & HidNpadButton_A)
		{
			logs_console_clear();
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
			switch (cursor)
			{
			case UP_FW:
			{
				force_firmware_install:
				if (debug_enabled) {
					debug_log_write("Installation d'un firmware.\n");
				}
				consoleSelect(&logs_console);
				if (verify_update((char* )APP_VERSION, last_app_version)) {
					bool need_update_app = false;
					if (debug_enabled) {
						debug_log_write("Le homebrew doit être mis à jour.\n");
					}
					need_update_app = ask_question((char*) language_vars["lng_ask_app_need_update"]);
					if (need_update_app) {
						if (auto_update_app(true)) {
							return 0;
						}
					} else {
						if (debug_enabled) {
							debug_log_write("Installation annulée.\n\n");
						}
						if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
							goto exit_homebrew;
						}
						break;
					}
				}
				if (GetChargerType() == 0 && get_battery_charge() < 10) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_10"]);
					printDisplay("\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 20) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_20"]);
					printDisplay("\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					printDisplay("\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					break;
				}
				bool update_firmware2 = false;
				bool agressive_clean2 = false;
				bool clean_modules_2 = false;
				if (autoconfig_enabled) {
					if (autoconfig_config.c1.agressive_clean == 1) {
						agressive_clean2 = true;
					}
					if (autoconfig_config.c1.module_clean == 1) {
						clean_modules_2 = true;
					}
					if (autoconfig_config.c1.use_all_app_functions != 1) {
						update_firmware2 = true;
					} else {
						printDisplay(language_vars["lng_install_pack_recap_begin"]);
						printDisplay("\n\n");
						if (agressive_clean2) {
							printDisplay(language_vars["lng_install_pack_recap_agressive_clean"]);
							printDisplay("\n");
						} else {
							printDisplay(language_vars["lng_install_pack_recap_not_agressive_clean"]);
							printDisplay("\n");
						}
						if (clean_modules_2) {
							printDisplay(language_vars["lng_install_pack_recap_clean_modules"]);
							printDisplay("\n");
						} else {
							printDisplay(language_vars["lng_install_pack_recap_not_clean_modules"]);
							printDisplay("\n");
						}
						printDisplay("\n");
						update_firmware2 = ask_question((char*) language_vars["lng_ask_update_firmware"]);
					}
				} else {
					agressive_clean2 = ask_question((char*) language_vars["lng_ask_agressive_clean"]);
					clean_modules_2 = ask_question((char*) language_vars["lng_ask_clean_modules"]);
					printDisplay(language_vars["lng_install_pack_recap_begin"]);
					printDisplay("\n\n");
					if (agressive_clean2) {
						printDisplay(language_vars["lng_install_pack_recap_agressive_clean"]);
						printDisplay("\n");
					} else {
						printDisplay(language_vars["lng_install_pack_recap_not_agressive_clean"]);
						printDisplay("\n");
					}
					if (clean_modules_2) {
						printDisplay(language_vars["lng_install_pack_recap_clean_modules"]);
						printDisplay("\n");
					} else {
						printDisplay(language_vars["lng_install_pack_recap_not_clean_modules"]);
						printDisplay("\n");
					}
					printDisplay("\n");
					update_firmware2 = ask_question((char*) language_vars["lng_ask_update_firmware"]);
				}
				if (update_firmware2) {
					fnc_clean_theme();
					if (agressive_clean2) {
						fnc_agressive_clean(true);
					}
					if (clean_modules_2) {
						fnc_clean_modules();
					}
					if (fnc_install_firmware()) {
						sleep(5);
						simple_reboot();
					}
				} else {
					if (debug_enabled) {
						debug_log_write("Annulation de l'installation du firmware.\n\n");
					}
				}
				if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
					goto exit_homebrew;
				}
				consoleSelect(&menu_console);
				break;
			}
			case UP_CFW:
			{
				if (debug_enabled) {
					if (!autoconfig_enabled) {
						debug_log_write("Installation du pack.\n");
					} else {
						debug_log_write("Installation du pack avec les parammètres de configurations automatiques.\n");
					}
				}
				consoleSelect(&logs_console);
				if (verify_update((char* )APP_VERSION, last_app_version)) {
					bool need_update_app = false;
					if (debug_enabled) {
						debug_log_write("Le homebrew doit être mis à jour.\n");
					}
					need_update_app = ask_question((char*) language_vars["lng_ask_app_need_update"]);
					if (need_update_app) {
						if (auto_update_app(true)) {
							return 0;
						}
					} else {
						if (debug_enabled) {
							debug_log_write("Installation annulée.\n\n");
						}
						if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
							goto exit_homebrew;
						}
						pack_update_found_install = false;
						break;
					}
				}
				if (GetChargerType() == 0 && get_battery_charge() < 20) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_20"]);
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					pack_update_found_install = false;
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					pack_update_found_install = false;
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					pack_update_found_install = false;
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					pack_update_found_install = false;
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					if (debug_enabled) {
						debug_log_write("Erreur, batterie pas assez chargée.\n\n");
					}
					printDisplay(language_vars["lng_battery_error_30"]);
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						goto exit_homebrew;
					}
					consoleSelect(&menu_console);
					pack_update_found_install = false;
					break;
				}
				mkdir(APP_PATH, 0777);
				bool update_firmware = false;
				bool clean_theme = false;
				bool agressive_clean = false;
				bool clean_modules = false;
				bool keep_files=false;
				bool hekate_autoboot_enable = false;
				bool clean_logos = false;
				bool install_hbmenu_choice = false;
				bool install_app_fwd_choice = false;
				if (!autoconfig_enabled) {
					if (!beta_mode) {
						if (strcmp(firmware_path, "") != 0) {
							update_firmware = ask_question((char*) language_vars["lng_ask_update_firmware"]);
						}
					} else {
						if (strcmp(firmware_path_beta, "") != 0) {
							update_firmware = ask_question((char*) language_vars["lng_ask_update_firmware"]);
						}
					}
				} else {
					if (!beta_mode) {
						if (strcmp(firmware_path, "") != 0) {
							if (autoconfig_config.c1.install_firmware == 1) {
								update_firmware = true;
							}
						}
					} else {
						if (strcmp(firmware_path_beta, "") != 0) {
							if (autoconfig_config.c1.install_firmware == 1) {
								update_firmware = true;
							}
						}
					}
				}
				if (update_firmware) {
					clean_theme = true;
				} else {
					if (!autoconfig_enabled) {
						clean_theme = ask_question((char*) language_vars["lng_ask_clean_theme"]);
					} else {
						if (autoconfig_config.c1.delete_theme == 1) {
							clean_theme = true;
						}
					}
				}
				if (!autoconfig_enabled) {
					agressive_clean = ask_question((char*) language_vars["lng_ask_agressive_clean"]);
				} else {
					if (autoconfig_config.c1.agressive_clean == 1) {
						agressive_clean = true;
					}
				}
				if (!autoconfig_enabled) {
					clean_modules = ask_question((char*) language_vars["lng_ask_clean_modules"]);
				} else {
					if (autoconfig_config.c1.module_clean == 1) {
						clean_modules = true;
					}
				}
				if (!autoconfig_enabled) {
					keep_files = ask_question((char*) language_vars["lng_ask_keep_files"]);
				} else {
					if (autoconfig_config.c1.delete_some_files_protection == 1) {
						keep_files = true;
					}
				}
					if (!autoconfig_enabled) {
						hekate_autoboot_enable = ask_question((char*) language_vars["lng_ask_hekate_autoboot"]);;
					} else {
						if (autoconfig_config.c1.hekate_autoboot_enable == 1) {
							hekate_autoboot_enable = true;
						}
					}
				if (!autoconfig_enabled) {
					clean_logos = ask_question((char*) language_vars["lng_ask_clean_logos"]);
				} else {
					if (autoconfig_config.c1.delete_logos == 1) {
						clean_logos = true;
					}
				}
				if (!autoconfig_enabled) {
					install_hbmenu_choice = ask_question((char*) language_vars["lng_ask_hbmenu_install"]);
				} else {
					if (autoconfig_config.c1.hbmenu_install == 1) {
						install_hbmenu_choice = true;
					}
				}
				if (!autoconfig_enabled) {
					install_app_fwd_choice = ask_question((char*) language_vars["lng_ask_app_fwd_install"]);
				} else {
					if (autoconfig_config.c1.app_fwd_install == 1) {
						install_app_fwd_choice = true;
					}
				}
				/*
				if (titleid_curently_launched(hbmenu_title_id) && install_hbmenu_choice) {
					install_hbmenu_choice = ask_question((char *) "Attention: Vous avez choisi d'installer l'icone du Homebrew Menu mais vous avez lance celui-ci, ceci n'est pas possible, veuillez quitter cette version du Homebrew Menu lancee en mode application (non lancee en mode applet).\nPour pouvoir faire ceci vous devez lancer ce homebrew en maintenant \"R\" en lancant un jeu ou lancer ce homebrew via l'album (en mode applet mais ceci n'est pas recommande pour installer le pack car cela diminu grandement les performances).\nVous pourez egalement effectuer l'installation de l'icone du Homebrew Menu via l'option appropriee du menu principal de ce homebrew apres l'installation du pack et le redemarrage de la console, dans ce cas le mode applet poura etre utilise.\n\nSouhaitez-vous continuer l'installation du pack en desactivant l'installation de l'icone du Homebrew Menu?");
					if (install_hbmenu_choice) {
						install_hbmenu_choice = false;
					} else {
						printDisplay("Installation du pack annulee.");
						pack_update_found_install = false;
						break;
					}
				}
				*/
				printDisplay(language_vars["lng_install_pack_recap_begin"]);
				printDisplay("\n\n");
				if (pack_update_found) {
					printDisplay(language_vars["lng_pack_have_update"], pack_version, last_pack_version);
					printDisplay("\n\n");
				} else {
					printDisplay(language_vars["lng_pack_have_no_update"], pack_version, last_pack_version);
					printDisplay("\n\n");
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						if (agressive_clean) {
							printDisplay(language_vars["lng_install_pack_recap_agressive_clean"]);
							printDisplay("\n");
						} else {
							printDisplay(language_vars["lng_install_pack_recap_not_agressive_clean"]);
							printDisplay("\n");
						}
						if (clean_modules) {
							printDisplay(language_vars["lng_install_pack_recap_clean_modules"]);
							printDisplay("\n");
						} else {
							printDisplay(language_vars["lng_install_pack_recap_not_clean_modules"]);
							printDisplay("\n");
						}
						printDisplay("\n");
						bool autoconfig_ask_firmware_update=ask_question((char*) language_vars["lng_ask_update_firmware_only"]);
						if (autoconfig_ask_firmware_update) {
							goto force_firmware_install;
						} else {
							printDisplay(language_vars["lng_install_pack_recap_begin"]);
							printDisplay("\n\n");
							if (pack_update_found) {
								printDisplay(language_vars["lng_pack_have_update"], pack_version, last_pack_version);
								printDisplay("\n\n");
							} else {
								printDisplay(language_vars["lng_pack_have_no_update"], pack_version, last_pack_version);
								printDisplay("\n\n");
							}
						}
					}
				}
				printDisplay("\n");
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
				if (agressive_clean) {
					printDisplay(language_vars["lng_install_pack_recap_agressive_clean"]);
					printDisplay("\n");
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_agressive_clean"]);
					printDisplay("\n");
				}
				if (clean_modules) {
					printDisplay(language_vars["lng_install_pack_recap_clean_modules"]);
					printDisplay("\n");
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_clean_modules"]);
					printDisplay("\n");
				}
				if (keep_files) {
					printDisplay(language_vars["lng_install_pack_recap_keep_files"]);
					printDisplay("\n");
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_keep_files"]);
					printDisplay("\n");
				}
				if (hekate_autoboot_enable) {
					printDisplay(language_vars["lng_install_pack_recap_enable_hekate_autoboot"]);
				} else  {
					printDisplay(language_vars["lng_install_pack_recap_not_enable_hekate_autoboot"]);
				}
				printDisplay("\n");
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
				if (install_app_fwd_choice) {
					printDisplay(language_vars["lng_install_pack_recap_install_app_fwd"]);
				} else {
					printDisplay(language_vars["lng_install_pack_recap_not_install_app_fwd"]);
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
							if (agressive_clean) {
								debug_log_write("Nettoyage agressif.\n");
							} else {
								debug_log_write("Non nettoyage agressif.\n");
							}
							if (clean_modules) {
								debug_log_write("Nettoyage des modules.\n");
							} else {
								debug_log_write("Non nettoyage des modules.\n");
							}
							if (keep_files) {
								debug_log_write("Fichiers non écrasés.\n");
							} else {
								debug_log_write("Fichiers écrasés.\n");
							}
							if (hekate_autoboot_enable) {
								debug_log_write("Autoboot  de Hekate à activer.\n");
							} else {
								debug_log_write("Autoboot  de Hekate à désactiver.\n");
							}
							if (clean_logos) {
								debug_log_write("Nettoyage des logos.\n");
							} else {
								debug_log_write("Non nettoyage des logos.\n");
							}
							if (install_hbmenu_choice) {
								debug_log_write("Installation du HBMenu.\n");
							} else {
								debug_log_write("Non installation du HBMenu.\n");
							}
							if (install_app_fwd_choice) {
								debug_log_write("Installation du forwarder de cet homebrew.\n\n");
							} else {
								debug_log_write("Non installation du forwarder de cet homebrew.\n\n");
							}
						}
					bool not_has_enough_space_on_sd;
					if (!beta_mode) {
						if (strcmp(pack_custom_files_url, "") == 0) {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size;
						} else {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size + pack_custom_files_size;
						}
					} else {
						if (strcmp(pack_custom_files_url_beta, "") == 0) {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size_beta;
						} else {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size_beta + pack_custom_files_size_beta;
						}
					}
					if (not_has_enough_space_on_sd) {
						if (debug_enabled) {
							debug_log_write("Pas assez d'espace sur la SD.\n\n");
						}
						printDisplay("\033[0;31m");
						printDisplay(language_vars["lng_error_not_enough_space_on_sd"]);
						printDisplay("\033[0;37m\n");
					} else {
						get_last_sha256_pack();
						bool dl_pack_res;
						if (!beta_mode) {
							dl_pack_res = downloadFile(CFW_URL, TEMP_FILE, OFF, true);
							// dl_pack_res = true;
						} else {
							dl_pack_res = downloadFile(CFW_URL_beta, TEMP_FILE, OFF, true);
							// dl_pack_res = true;
						}
						if (dl_pack_res) {
							if (strcmp(pack_sha256, "") != 0) {
								printDisplay(language_vars["lng_calculate_sha256_of_downloaded_file"]);
								printDisplay("\n");
								char dl_pack_sha256[65] = "";
								get_sha256_file(TEMP_FILE, dl_pack_sha256);
								if (debug_enabled) {
									debug_log_write("SHA256 du pack à télécharger: ");
									debug_log_write("%s", pack_sha256);
									debug_log_write("\n");
									debug_log_write("SHA256 du pack téléchargé: ");
									debug_log_write("%s", dl_pack_sha256);
									debug_log_write("\n\n");
								}
								if (strcmp(pack_sha256, dl_pack_sha256) != 0) {
									if (debug_enabled) {
										debug_log_write("Erreur de téléchargement du pack, installation annulée\n\n.");
									}
									printDisplay("\033[0;31m");
									printDisplay(language_vars["lng_install_pack_download_pack_error"]);
									printDisplay("\033[0;37m\n");
									remove(TEMP_FILE);
									consoleSelect(&menu_console);
									if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
										goto exit_homebrew;
									}
									pack_update_found_install = false;
									break;
								}
							}
							set_90dns();
							if (clean_theme && agressive_clean) {
								clean_sd(true, true);
							} else if (clean_theme) {
								clean_sd(true, false);
							} else if (agressive_clean) {
								clean_sd(false, true);
							} else {
								clean_sd(false, false);
							}
							if (clean_modules) {
								fnc_clean_modules();
							}
							int unzip_res;
							if (!beta_mode) {
								unzip_res = unzip(TEMP_FILE, subfolder_in_zip, keep_files);
							} else {
								unzip_res = unzip(TEMP_FILE, subfolder_in_zip_beta, keep_files);
							}
							if (unzip_res == 0) {
								remove(TEMP_FILE);
								if (!beta_mode) {
									if (strcmp(pack_custom_files_url, "") != 0) {
										printDisplay(language_vars["lng_installing_pack_custom_files"]);
										printDisplay("\n");
										get_last_sha256_custom_files_pack();
										dl_pack_res = false;
										dl_pack_res = downloadFile(pack_custom_files_url, TEMP_FILE, OFF, true);
										if (dl_pack_res) {
											if (strcmp(custom_files_pack_sha256, "") != 0) {
												printDisplay(language_vars["lng_calculate_sha256_of_downloaded_file"]);
												printDisplay("\n");
												char dl_custom_files_pack_sha256[65] = "";
												get_sha256_file(TEMP_FILE, dl_custom_files_pack_sha256);
												if (debug_enabled) {
													debug_log_write("SHA256 du fichier zip complémentaire au pack à télécharger: ");
													debug_log_write("%s", custom_files_pack_sha256);
													debug_log_write("\n");
													debug_log_write("SHA256 du fichier zip complémentaire au pack téléchargé: ");
													debug_log_write("%s", dl_custom_files_pack_sha256);
													debug_log_write("\n");
												}
												if (strcmp(custom_files_pack_sha256, dl_custom_files_pack_sha256) != 0) {
													printDisplay("\033[0;31m");
													printDisplay(language_vars["lng_install_custom_files_pack_download_error"]);
													printDisplay("\033[0;37m\n");
													remove(TEMP_FILE);
												}
											}
											unzip_res = -1;
											unzip_res = unzip(TEMP_FILE, pack_custom_files_subfolder_in_zip, keep_files);
											if (unzip_res == 0) {
												remove(TEMP_FILE);
											}
										}
									}
								} else {
									if (strcmp(pack_custom_files_url_beta, "") != 0) {
										printDisplay(language_vars["lng_installing_pack_custom_files"]);
										printDisplay("\n");
										dl_pack_res = false;
										dl_pack_res = downloadFile(pack_custom_files_url_beta, TEMP_FILE, OFF, true);
										if (dl_pack_res) {
											unzip_res = -1;
											unzip_res = unzip(TEMP_FILE, pack_custom_files_subfolder_in_zip_beta, keep_files);
											if (unzip_res == 0) {
												remove(TEMP_FILE);
											}
										}
									}
								}
								if (clean_logos) {
										if (!beta_mode) {
											fnc_clean_logo(atmo_logo_dir, hekate_nologo_file_path);
										} else {
											fnc_clean_logo(atmo_logo_dir_beta, hekate_nologo_file_path_beta);
										}
								}
								printDisplay(language_vars["lng_install_pack_configuring_hekate_autoboot"]);
								printDisplay("\n");
								get_hekate_autoboot_status();
								if (hekate_autoboot_enable) {
									hiddbgInitialize();
									hiddbgDeactivateHomeButton();
									hiddbgExit();
									if (debug_enabled) {
										debug_log_write("Activation de l'auto-boot de Hekate.\n");
									}
									int* temp_hekate_autoboot_chosen;
									while(1) {
										temp_hekate_autoboot_chosen = hekate_config_choice_menu(true);
										refreshScreen(cursor);
										consoleSelect(&logs_console);
										if (temp_hekate_autoboot_chosen[0] == -2) {
											if (debug_enabled) {
												debug_log_write("Aucune configuration de Hekate trouvée, désactivation de l'autoboot de Hekate forcée.");
											}
											update_hekate_autoboot_param(false, hekate_autoboot_chosen);
											break;
										} else if (temp_hekate_autoboot_chosen[0] > -1) {
											hekate_autoboot_chosen[0] = temp_hekate_autoboot_chosen[0];
											hekate_autoboot_chosen[1] = temp_hekate_autoboot_chosen[1];
											update_hekate_autoboot_param(true, hekate_autoboot_chosen);
											break;
										}
										
									}
								} else  {
									if (debug_enabled) {
										debug_log_write("Désactivation de l'auto-boot de Hekate.\n");
									}
									update_hekate_autoboot_param(false, hekate_autoboot_chosen);
								}
								if (install_hbmenu_choice) {
									install_hbmenu();
								}
								if (install_app_fwd_choice) {
									install_app_fwd();
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
					if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
						appExit();
						return 0;
					}
				}
			if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
				goto exit_homebrew;
			}
				consoleSelect(&menu_console);
				pack_update_found_install = false;
				break;
			}

			case UP_APP:
			{
				if (auto_update_app(true)) {
					return 0;
				}
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

			case UP_APP_INSTALL:
			{
				consoleSelect(&logs_console);
				install_app_fwd();
				consoleSelect(&menu_console);
				break;
			}

			case UP_RESET:
			{
				consoleSelect(&logs_console);
				if (ask_question((char*) language_vars["lng_ask_validate_choices_for_reset"])) {
					if (debug_enabled) {
						debug_log_write("Réinitialisation du système.\n\n");
					}
					if (is_emummc()) {
						remove_directory(emummc_paths.nintendo);
					} else {
						remove_directory("nintendo");
					}
					nsResetToFactorySettingsForRefurbishment();
					printDisplay("\033[0;32m\n");
					printDisplay(language_vars["lng_success_reboot_in_five_seconds"]);
					printDisplay("\033[0;37m\n");
					sleep(5);
					simple_reboot();
				}
				consoleSelect(&menu_console);
				break;
			}
			}

		} else if (kDown & HidNpadButton_X) {
			logs_console_clear();
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
			display_infos(cursor);
			refreshScreen(cursor);

		} else if (kDown & HidNpadButton_Y) {
			logs_console_clear();
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
			record_infos();

		} else if (kDown & HidNpadButton_Minus) {
			logs_console_clear();
			hekate_autoboot_enable_combot_disable = false;
			hekate_autoboot_disable_combot_disable = false;
			switch_app_mode();
			get_last_version_pack();
			get_last_version_app();
			if (debug_enabled) {
				debug_log_write("Dernière version du pack: %s\n\n", last_pack_version);
			}
			remove(TEMP_FILE);
			cursor = 0;
			refreshScreen(cursor);
			if (auto_update_app(false)) {
				return 0;
			}
			pack_update_found = verify_update(pack_version, last_pack_version);
			pack_update_found_install = false;
			if (pack_update_found) {
				if (!autoconfig_enabled || autoconfig_config.c1.use_all_app_functions == 1) {
					logs_console_clear();
					consoleSelect(&logs_console);
					printDisplay(language_vars["lng_pack_have_update"], pack_version, last_pack_version);
					printDisplay("\n\n");
					pack_update_found_install=ask_question((char*) language_vars["lng_ask_pack_have_update"]);
					logs_console_clear();
					consoleSelect(&menu_console);
					if (pack_update_found_install) {
						cursor = UP_CFW;
						kDown = HidNpadButton_A;
					}
				}
			}

		// exit...
		} else if (kDown & HidNpadButton_Plus) {
			break;
		}
	}

	if (!beta_mode) {
		if (exit_mode_param != 0) {
			__nx_applet_exit_mode = 1;
		}
	} else {
		if (exit_mode_param_beta != 0) {
			__nx_applet_exit_mode = 1;
		}
	}
	exit_homebrew:
	// cleanup then exit
	if (autoconfig_enabled && autoconfig_config.c1.use_all_app_functions != 1) {
		pause_homebrew();
	}
	appExit();
	return 0;
}
