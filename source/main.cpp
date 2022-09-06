#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>
#include <string.h>
#include <stdlib.h>

#include "main_util.h"
#include "90dns_setter.h"
#include "download.h"
#include "unzip.h"
#include "reboot_to_payload.h"
#include "ini.h"
#include "firmwares_install/daybreak-cli.hpp"
#include "translate.h"

extern u32 __nx_applet_exit_mode;
const u64 hbmenu_title_id = 0x0104444444441001;

lng language_vars;
#define ROOT					"/"
#define APP_PATH				"/switch/AIO_LS_pack_Updater/"
#define APP_OUTPUT			  "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"

#define APP_VERSION			 "4.0.1"
#define CURSOR_LIST_MAX		 5
#define UP_APP		  0
#define UP_CFW		  1
#define UP_FW		  2
#define UP_HBMENU_INSTALL		  3
#define UP_90DNS		  4
#define UP_ATMO_PROTECT_CONFIGS		  5

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
bool sd_is_exfat;
bool console_is_erista = false;
bool beta_mode = false;
FsFileSystem *fs_sd;
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

const char *OPTION_LIST[] =
{
	language_vars.lng_update_app_menu,
	language_vars.lng_update_pack_menu,
	language_vars.lng_update_firmware_menu,
	language_vars.lng_update_hbmenu_menu,
	language_vars.lng_set_90dns_menu,
	language_vars.lng_protect_console_menu
};

// define a structure for holding the values in "config" section of the ini file.
typedef struct{
	const char* dl_pack;
	const char* dl_pack_version;
	const char* pack_version_local_filepath;
	const char* subfolder_in_zip_pack;
	s64 pack_size;
	const char* dl_app;
	const char *firmware_path;
	const char *atmo_logo_dir;
	const char *hekate_nologo_file_path;
	int exit_method;
} config_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	config_section s1;
} configuration;

// define a structure for holding the values in "emummc" section of the emummc ini file.
typedef struct{
	int enabled;
	u32 id;
	u32 sector;
	const char* path;
	const char* nintendo_path;
} emummc_config_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	emummc_config_section e1;
} emummc_configuration;

static int config_handler(void* config, const char* section, const char* name, const char* value)
{
	// config instance for filling in the values.
	configuration* pconfig = (configuration*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	// fill the values in config struct for Section 1.
	if(MATCH("config", "dl_pack_adress")){
		if (value != 0) {
			pconfig->s1.dl_pack = strdup(value);
		} else {
			pconfig->s1.dl_pack = "";
		}
	} else 	if(MATCH("config", "pack_version_adress")){
		if (value != 0) {
			pconfig->s1.dl_pack_version = strdup(value);
		} else {
			pconfig->s1.dl_pack_version = "";
		}
		} else 	if(MATCH("config", "pack_version_local_filepath")){
		if (value != 0) {
			pconfig->s1.pack_version_local_filepath = strdup(value);
		} else {
			pconfig->s1.pack_version_local_filepath = "";
		}
	}else if(MATCH("config", "subfolder_of_pack_in_zip")){
		if (value != 0) {
			pconfig->s1.subfolder_in_zip_pack = strdup(value);
		} else {
			pconfig->s1.subfolder_in_zip_pack = "";
		}
	}else if(MATCH("config", "dl_app_adress")){
		if (value != 0) {
			pconfig->s1.dl_app = strdup(value);
		} else {
			pconfig->s1.dl_app = "";
		}
	}else if(MATCH("config", "firmware_path")){
		if (value != 0) {
			pconfig->s1.firmware_path = strdup(value);
		} else {
			pconfig->s1.dl_app = "";
		}
	}else if(MATCH("config", "atmo_logo_dir")){
		if (value != 0) {
			pconfig->s1.atmo_logo_dir = strdup(value);
		} else {
			pconfig->s1.atmo_logo_dir = "";
		}
	}else if(MATCH("config", "hekate_nologo_file_path")){
		if (value != 0) {
			pconfig->s1.hekate_nologo_file_path = strdup(value);
		} else {
			pconfig->s1.hekate_nologo_file_path = "";
		}
	}else if(MATCH("config", "pack_size")){
		if (value != 0) {
			pconfig->s1.pack_size = atoll(value);
		} else {
			pconfig->s1.pack_size = 0;
		}
	}else if(MATCH("config", "exit_method")){
		if (value != 0) {
			pconfig->s1.exit_method = atoll(value);
		} else {
			pconfig->s1.exit_method = 0;
		}
	}else{
		return 0;
	}
	return 1;
}

void configs_init() {
	strcpy(pack_version, language_vars.lng_unknown_1);
	strcpy(last_pack_version, language_vars.lng_unknown_1);
	strcpy(firmware_version, language_vars.lng_unknown_1);
	strcpy(atmosphere_version, language_vars.lng_unknown_1);
	strcpy(emummc_value, language_vars.lng_unknown_1);
	strcpy(emummc_type, language_vars.lng_unknown_0);
	strcpy(fusee_gelee_patch, language_vars.lng_unknown_0);
	strcpy(console_model, language_vars.lng_unknown_0);
	// config for holding ini file values.
	configuration config;
	config.s1.dl_pack = "";
	config.s1.dl_pack_version = "";
	config.s1.pack_version_local_filepath = "";
	config.s1.subfolder_in_zip_pack = "";
	config.s1.dl_app = "";
	config.s1.firmware_path = "";
	config.s1.atmo_logo_dir = "";
	config.s1.hekate_nologo_file_path = "";
	config.s1.pack_size = 0;
	config.s1.exit_method = 0;
	FILE *test_ini;
	test_ini = fopen("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.ini", "r");
	if (test_ini != NULL) {
		fclose(test_ini);
		// parse the .ini file
		if (ini_parse("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.ini", config_handler, &config) == 0) {
			if (strcmp(config.s1.dl_pack, "") != 0) {
				strcpy(CFW_URL, config.s1.dl_pack);
				free((void*)config.s1.dl_pack);
			}
			if (strcmp(config.s1.dl_pack_version, "") != 0) {
				strcpy(pack_version_url, config.s1.dl_pack_version);
				free((void*)config.s1.dl_pack_version);
			}
			if (strcmp(config.s1.pack_version_local_filepath, "") != 0) {
				strcpy(pack_version_local_filepath, config.s1.pack_version_local_filepath);
				free((void*)config.s1.dl_pack_version);
			}
			if (strcmp(config.s1.subfolder_in_zip_pack, "") != 0) {
				strcpy(subfolder_in_zip, config.s1.subfolder_in_zip_pack);
				free((void*)config.s1.subfolder_in_zip_pack);
			}
			if (strcmp(config.s1.dl_app, "") != 0) {
				strcpy(APP_URL, config.s1.dl_app);
				free((void*)config.s1.dl_app);
			}
			if (strcmp(config.s1.firmware_path, "") != 0) {
				strcpy(firmware_path, config.s1.firmware_path);
				free((void*)config.s1.firmware_path);
			}
			if (strcmp(config.s1.atmo_logo_dir, "") != 0) {
				strcpy(atmo_logo_dir, config.s1.atmo_logo_dir);
				free((void*)config.s1.atmo_logo_dir);
			}
			if (strcmp(config.s1.hekate_nologo_file_path, "") != 0) {
				strcpy(hekate_nologo_file_path, config.s1.hekate_nologo_file_path);
				free((void*)config.s1.hekate_nologo_file_path);
			}
			if (config.s1.pack_size != 0) {
				pack_size = config.s1.pack_size;
			}
			if (config.s1.exit_method != 0) {
				exit_mode_param = 1;
			}
		}
	}
	configuration config_beta;
	config_beta.s1.dl_pack = "";
	config_beta.s1.dl_pack_version = "";
	config_beta.s1.pack_version_local_filepath = "";
	config_beta.s1.subfolder_in_zip_pack = "";
	config_beta.s1.dl_app = "";
	config_beta.s1.firmware_path = "";
	config_beta.s1.atmo_logo_dir = "";
	config_beta.s1.hekate_nologo_file_path = "";
	config_beta.s1.pack_size = 0;
	config_beta.s1.exit_method = 0;
	FILE *test_ini_beta;
	test_ini_beta = fopen("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater_beta.ini", "r");
	if (test_ini_beta != NULL) {
		fclose(test_ini_beta);
		// parse the .ini file
		if (ini_parse("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater_beta.ini", config_handler, &config_beta) == 0) {
			if (strcmp(config_beta.s1.dl_pack, "") != 0) {
				strcpy(CFW_URL_beta, config_beta.s1.dl_pack);
				free((void*)config_beta.s1.dl_pack);
			}
			if (strcmp(config_beta.s1.dl_pack_version, "") != 0) {
				strcpy(pack_version_url_beta, config_beta.s1.dl_pack_version);
				free((void*)config_beta.s1.dl_pack_version);
			}
			if (strcmp(config_beta.s1.pack_version_local_filepath, "") != 0) {
				strcpy(pack_version_local_filepath_beta, config_beta.s1.pack_version_local_filepath);
				free((void*)config_beta.s1.dl_pack_version);
			}
			if (strcmp(config_beta.s1.subfolder_in_zip_pack, "") != 0) {
				strcpy(subfolder_in_zip_beta, config_beta.s1.subfolder_in_zip_pack);
				free((void*)config_beta.s1.subfolder_in_zip_pack);
			}
			if (strcmp(config_beta.s1.dl_app, "") != 0) {
				strcpy(APP_URL_beta, config_beta.s1.dl_app);
				free((void*)config_beta.s1.dl_app);
			}
			if (strcmp(config_beta.s1.firmware_path, "") != 0) {
				strcpy(firmware_path_beta, config_beta.s1.firmware_path);
				free((void*)config_beta.s1.firmware_path);
			}
			if (strcmp(config_beta.s1.atmo_logo_dir, "") != 0) {
				strcpy(atmo_logo_dir_beta, config_beta.s1.atmo_logo_dir);
				free((void*)config_beta.s1.atmo_logo_dir);
			}
			if (strcmp(config_beta.s1.hekate_nologo_file_path, "") != 0) {
				strcpy(hekate_nologo_file_path_beta, config_beta.s1.hekate_nologo_file_path);
				free((void*)config_beta.s1.hekate_nologo_file_path);
			}
			if (config_beta.s1.pack_size != 0) {
				pack_size_beta = config_beta.s1.pack_size;
			}
			if (config_beta.s1.exit_method != 0) {
				exit_mode_param_beta = 1;
			}
		}
	}
}

static int emummc_config_handler(void* config, const char* section, const char* name, const char* value)
{
	// config instance for filling in the values.
	emummc_configuration* pconfig = (emummc_configuration*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	// fill the values in config struct for emummc 1.
	if(MATCH("emummc", "enabled")){
		if (value != 0) {
			pconfig->e1.enabled = atoll(value);
		} else {
			pconfig->e1.enabled = 0;
		}
	} else 	if(MATCH("emummc", "id")){
		if (value != 0) {
			pconfig->e1.id = ParseHexInteger(value);
		} else {
			pconfig->e1.id = 0;
		}
		} else 	if(MATCH("emummc", "sector")){
		if (value != 0) {
			pconfig->e1.sector = ParseHexInteger(value);;
		} else {
			pconfig->e1.sector = 0;
		}
	}else if(MATCH("emummc", "path")){
		if (value != 0) {
			pconfig->e1.path = strdup(value);
		} else {
			pconfig->e1.path = "";
		}
	}else if(MATCH("emummc", "nintendo_path")){
		if (value != 0) {
			pconfig->e1.nintendo_path = strdup(value);
		} else {
			pconfig->e1.nintendo_path = "";
		}
	}else{
		return 0;
	}
	return 1;
}

void get_emunand_type() {
	if (strcmp(emummc_value, "Emunand") != 0) {
		strcpy(emummc_type, "");
		return;
	}
	// config for holding ini file values.
	emummc_configuration emummc_config;
	emummc_config.e1.enabled = 0;
	emummc_config.e1.sector = 0;
	emummc_config.e1.id = 0;
	emummc_config.e1.path = "";
	emummc_config.e1.nintendo_path = "";
	FILE *test_ini;
	test_ini = fopen("/emuMMC/emummc.ini", "r");
	if (test_ini != NULL) {
		fclose(test_ini);
		// parse the .ini file
		if (ini_parse("/emuMMC/emummc.ini", emummc_config_handler, &emummc_config) == 0) {
			if (emummc_config.e1.sector == 0) {
				if (strcmp(emummc_config.e1.path, "") != 0) {
					strcpy(emummc_type, language_vars.lng_files);
					free((void*)emummc_config.e1.path);
				}
			} else {
				strcpy(emummc_type, language_vars.lng_partition);
			}
		}
	}
	free((void*)emummc_config.e1.nintendo_path);
}

s64 get_sd_size_left() {
	s64 fs_sd_size;
	fsFsGetFreeSpace(fs_sd, "/", &fs_sd_size);
	// nsInitialize();
	// nsGetFreeSpaceSize(NcmStorageId_SdCard, fs_sd_size);
	// nsExit();
		// printf("%ld\n", fs_sd_size);
		return fs_sd_size;
}

void refreshScreen(int cursor)
{
	consoleClear();
	if (!beta_mode) {
		printf("\x1B[36m");
		printf(language_vars.lng_title, APP_VERSION);
	} else {
		printf("\x1B[31m");
		printf(language_vars.lng_title_beta, APP_VERSION);
	}
	printf("\x1B[37m\n\n");
	printf(language_vars.lng_a_menu);
	printf("\n\n");
	printf(language_vars.lng_x_menu);
	printf("\n");
	printf(language_vars.lng_y_menu);
	printf("\n");
	if (!beta_mode) {
		printf(language_vars.lng_minus_menu);
	} else {
		printf(language_vars.lng_minus_menu_beta);
	}
	printf("\n\n");
	printf(language_vars.lng_plus_menu);
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
	fs_sd = fsdevGetDeviceFileSystem("sdmc");
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

void appExit()
{
	appletEndBlockingHomeButton();
	appletSetAutoSleepDisabled(false);
	// hiddbgExit();
	fsFsClose(fs_sd);
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
		while (i <= 15) {
			pack_version[i] = '\0';
			i++;
		}
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
		while (i <= 15) {
			last_pack_version[i] = '\0';
			i++;
		}
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

	char sysVersionBuffer[20];
	snprintf(sysVersionBuffer, 20, "%u.%u.%u", ver.major, ver.minor, ver.micro);
	snprintf(firmware_version, sizeof(firmware_version), "%s", sysVersionBuffer);
	setsysExit();
}

void get_ams_version() {
	splInitialize();
	u32 ExosphereApiVersionConfigItem = 65000;
	u32 ExosphereEmummcType		   = 65007;
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
	u64 is_emummc;
	if (R_FAILED(rc = splGetConfig((SplConfigItem)(ExosphereEmummcType), &is_emummc))) {
		splExit();
		return;
	}
	if (is_emummc) {
		strcpy(emummc_value, "Emunand");
	} else {
		strcpy(emummc_value,"Sysnand");
	}
	splExit();
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
		strcpy(fusee_gelee_patch, language_vars.lng_usable);
	} else {
		strcpy(fusee_gelee_patch, language_vars.lng_not_usable);
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
	printf(language_vars.lng_yes);
	printf("		  [B]: ");
	printf(language_vars.lng_no);
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
	printf(language_vars.lng_infos_begin);
	printf("\n\n");
	if (isApplet()) {
		printf(language_vars.lng_infos_is_applet);
	} else {
		printf(language_vars.lng_infos_is_not_applet);
	}
	printf("\n");
	printf(language_vars.lng_infos_pack_version, pack_version);
	printf("\n");
	printf(language_vars.lng_infos_last_pack_version, last_pack_version);
	printf("\n");
	printf(language_vars.lng_infos_console_id, console_id);
	printf("\n");
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		printf(language_vars.lng_infos_serial_incognito);
	} else {
		printf(language_vars.lng_infos_serial, console_serial.number);
	}
	printf("\n");
	if (strcmp(emummc_value, "Emunand") != 0) {
		printf(language_vars.lng_infos_sysnand, emummc_value);
	} else {
		printf(language_vars.lng_infos_emunand, emummc_value, emummc_type);
	}
	printf("\n");
	printf(language_vars.lng_infos_console_model, console_model);
	printf("\n");
	printf(language_vars.lng_infos_fusee_gelee_patch, fusee_gelee_patch);
	printf("\n");
	printf(language_vars.lng_infos_actual_firmware_version, firmware_version);
	printf("\n");
	printf(language_vars.lng_infos_actual_atmosphere_version, atmosphere_version);
	printf("\n");
	if (GetChargerType() == 0) {
		printf(language_vars.lng_infos_official_charge, get_battery_charge());
	} else if (GetChargerType() == 1) {
		printf(language_vars.lng_infos_usb_charge, get_battery_charge());
	} else {
		printf(language_vars.lng_infos_no_charge, get_battery_charge());
	}
	printf("\n");
	consoleUpdate(&logs_console);
	consoleSelect(&menu_console);
}

void record_infos() {
	consoleSelect(&logs_console);
	FILE *log_infos;
	log_infos = fopen("switch/AIO_LS_pack_Updater/console_infos.log", "w");
	if ( log_infos == NULL ) {
		printf(language_vars.lng_record_infos_log_not_open_error);
		printf("\n");
		consoleUpdate(&logs_console);
		return;
	}
	fprintf(log_infos, language_vars.lng_infos_begin);
	fprintf(log_infos, "\n\n");
	if (isApplet()) {
		fprintf(log_infos, language_vars.lng_infos_is_applet);
	} else {
		fprintf(log_infos, language_vars.lng_infos_is_not_applet);
	}
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_pack_version, pack_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_last_pack_version, last_pack_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_console_id, console_id);
	fprintf(log_infos, "\n");
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		fprintf(log_infos, language_vars.lng_infos_serial_incognito);
	} else {
		fprintf(log_infos, language_vars.lng_infos_serial, console_serial.number);
	}
	fprintf(log_infos, "\n");
	if (strcmp(emummc_value, "Emunand") != 0) {
		fprintf(log_infos, language_vars.lng_infos_sysnand, emummc_value);
	} else {
		fprintf(log_infos, language_vars.lng_infos_emunand, emummc_value, emummc_type);
	}
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_console_model, console_model);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_fusee_gelee_patch, fusee_gelee_patch);
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
	fprintf(log_infos, language_vars.lng_infos_actual_firmware_version, firmware_version);
	fprintf(log_infos, "\n");
	fprintf(log_infos, language_vars.lng_infos_actual_atmosphere_version, atmosphere_version);
	fprintf(log_infos, "\n");
	if (GetChargerType() == 0) {
		fprintf(log_infos, language_vars.lng_infos_official_charge, get_battery_charge());
	} else if (GetChargerType() == 1) {
		fprintf(log_infos, language_vars.lng_infos_usb_charge, get_battery_charge());
	} else {
		fprintf(log_infos, language_vars.lng_infos_no_charge, get_battery_charge());
	}
	fprintf(log_infos, "\n");
	// fprintf(log_infos, "Infos batterie et chargeur : %i, %d%% %i", IsChargingEnabled(), get_battery_charge(), GetChargerType());
	// fprintf(log_infos, "%s", language_vars.lng_title);
	printf(language_vars.lng_record_infos_success);
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
		cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "/payload.bin");
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
		beta_mode = true;
	} else {
		beta_mode = false;
	}
}

#include <filesystem>
#include <vector>
#include "contents_install/sdInstall.hpp"
#include "contents_install/hos/hos_Titles.hpp"
#include "contents_install/util/util.hpp"

bool install_hbmenu() {
	printDisplay(language_vars.lng_hbmenu_install_begin);
	printDisplay("\n\n");
	std::vector<std::filesystem::path> nsp_list;
	std::filesystem::path nsp_path;
	inst::util::initInstallServices();
	std::vector<std::pair<u64, u32>> installedTitles;
	installedTitles = inst::util::listInstalledTitles();
	for (long unsigned int i = 0; i < installedTitles.size(); i++) {
		if (installedTitles[i].first == hbmenu_title_id) {
			// if (!titleid_curently_launched(hbmenu_title_id)) {
				printDisplay(language_vars.lng_hbmenu_install_uninstall_begin);
				printDisplay("\n");
				if (R_FAILED(hos::RemoveTitle(hos::Locate(hbmenu_title_id)))) {
					printDisplay("\033[0;31m");
					printDisplay(language_vars.lng_hbmenu_install_uninstall_error);
					printDisplay("\033[0;37m\n\n");
					inst::util::deinitInstallServices();
					return false;
				} else {
					printDisplay("\033[0;32m");
					printDisplay(language_vars.lng_hbmenu_install_uninstall_success);
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
	// printDisplay("NSP du forwarder choisi: %s\n\n", nsp_path.filename().c_str());
	nsp_list.push_back(nsp_path);
	if (nspInstStuff::installNspFromFile(nsp_list, 1)) {
		printDisplay("\033[0;32m");
		printDisplay(language_vars.lng_hbmenu_install_success);
		printDisplay("\033[0;37m\n");
		return true;
	} else {
		printDisplay("\033[0;31m");
		printDisplay(language_vars.lng_hbmenu_install_error);
		printDisplay("\033[0;37m\n");
		return false;
	}
}

int main(int argc, char **argv)
{
	// init stuff
	appInit();
	language_vars = set_translation_strings();
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
	get_emunand_type();
	remove(TEMP_FILE);

	if (isApplet()) {
		printf("\x1B[44m");
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
				consoleSelect(&logs_console);
				if (GetChargerType() == 0 && get_battery_charge() < 10) {
					printDisplay(language_vars.lng_battery_error_10);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 20) {
					printDisplay(language_vars.lng_battery_error_20);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					printDisplay("\n");
					consoleSelect(&menu_console);
					break;
				}
				bool update_firmware2 = false;
				DIR *dir2;
				update_firmware2 = ask_question((char*) language_vars.lng_ask_update_firmware);
				if (update_firmware2) {
					if (!beta_mode) {
						dir2 = opendir(firmware_path);
					} else {
						dir2 = opendir(firmware_path_beta);
					}
					if (dir2 != NULL) {
						closedir(dir2);
						fnc_clean_theme();
						if (!beta_mode) {
							if (daybreak_main(firmware_path, 2, 1, 2)) {
								printDisplay("\033[0;32m\n");
								printDisplay(language_vars.lng_success_reboot_in_five_seconds);
								printDisplay("\033[0;37m\n");
								sleep(5);
								simple_reboot();
							} else {
								printDisplay("\033[0;31m\n");
								printDisplay(language_vars.lng_install_firmware_end_error);
								printDisplay("\033[0;37m\n");
							}
						} else {
							if (daybreak_main(firmware_path_beta, 2, 1, 2)) {
								printDisplay("\033[0;32m\n");
								printDisplay(language_vars.lng_success_reboot_in_five_seconds);
								printDisplay("\033[0;37m\n");
								sleep(5);
								simple_reboot();
							} else {
								printDisplay("\033[0;31m\n");
								printDisplay(language_vars.lng_install_firmware_end_error);
								printDisplay("\033[0;37m\n");
							}
						}
					} else {
						printDisplay("\033[0;31m");
						printDisplay(language_vars.lng_install_firmware_error_folder_not_found);
						printDisplay("\033[0;37m\n");
						printDisplay(language_vars.lng_install_firmware_folder_location, firmware_path);
						printDisplay("\n");
					}
				}
				consoleSelect(&menu_console);
				break;
			}
			case UP_CFW:
			{
				consoleSelect(&logs_console);
				if (GetChargerType() == 0 && get_battery_charge() < 20) {
					printDisplay(language_vars.lng_battery_error_20);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 1 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 2 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == 3 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					consoleSelect(&menu_console);
					break;
				} else if (GetChargerType() == -1 && get_battery_charge() < 30) {
					printDisplay(language_vars.lng_battery_error_30);
					consoleSelect(&menu_console);
					break;
				}
				mkdir(APP_PATH, 0777);
				bool update_firmware = false;
				bool clean_theme = false;
				DIR *dir;
				if (!beta_mode) {
					if (strcmp(firmware_path, "") != 0) {
						update_firmware = ask_question((char*) language_vars.lng_ask_update_firmware);
					}
				} else {
					if (strcmp(firmware_path_beta, "") != 0) {
						update_firmware = ask_question((char*) language_vars.lng_ask_update_firmware);
					}
				}
				if (update_firmware) {
					clean_theme = true;
				} else {
					clean_theme = ask_question((char*) language_vars.lng_ask_clean_theme);
				}
				bool clean_logos = ask_question((char*) language_vars.lng_ask_clean_logos);
				bool install_hbmenu_choice = ask_question((char*) language_vars.lng_ask_hbmenu_install);
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
				printDisplay(language_vars.lng_install_pack_recap_begin);
				printDisplay("\n\n");
				if (update_firmware) {
					printDisplay(language_vars.lng_install_pack_recap_firmware_install);
					printDisplay("\n");
				} else {
					printDisplay(language_vars.lng_install_pack_recap_not_install_firmware);
					printDisplay("\n");
					if (clean_theme) {
						printDisplay(language_vars.lng_install_pack_recap_clean_theme);
						printDisplay("\n");
					} else {
						printDisplay(language_vars.lng_install_pack_recap_not_clean_theme);
						printDisplay("\n");
					}
				}
				if (clean_logos) {
					printDisplay(language_vars.lng_install_pack_recap_clean_logos);
				} else {
					printDisplay(language_vars.lng_install_pack_recap_not_clean_logos);
				}
				printDisplay("\n");
				if (install_hbmenu_choice) {
					printDisplay(language_vars.lng_install_pack_recap_install_hbmenu);
				} else {
					printDisplay(language_vars.lng_install_pack_recap_not_install_hbmenu);
				}
				printDisplay("\n");
				bool validate_choice = ask_question((char*) language_vars.lng_ask_validate_choices);
				if (validate_choice) {
					bool dl_pack_res;
					if (!beta_mode) {
						dl_pack_res = downloadFile(CFW_URL, TEMP_FILE, OFF, true);
					} else {
						dl_pack_res = downloadFile(CFW_URL_beta, TEMP_FILE, OFF, true);
					}
					if (dl_pack_res) {
						bool not_has_enough_space_on_sd;
						if (!beta_mode) {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size;
						} else {
							not_has_enough_space_on_sd = get_sd_size_left() <= pack_size_beta;
						}
						if (not_has_enough_space_on_sd) {
							printDisplay("\033[0;31m");
							printDisplay(language_vars.lng_error_not_enough_space_on_sd);
							printDisplay("\033[0;37m\n");
						} else {
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
									if (!beta_mode) {
										dir = opendir(firmware_path);
									} else {
										dir = opendir(firmware_path_beta);
									}
									if (dir != NULL) {
										closedir(dir);
										/*
										cp((char*) "romfs:/nro/Daybreak-cli.nro", (char*) "/switch/AIO_LS_pack_Updater/Daybreak-cli.nro");
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
												printDisplay("\033[0;32m\n");
												printDisplay(language_vars.lng_install_firmware_end_success);
												printDisplay("\033[0;37m\n");
											} else {
												printDisplay("\033[0;31m\n");
												printDisplay(language_vars.lng_install_firmware_end_error);
												printDisplay("\033[0;37m\n");
											}
										} else {
											if (daybreak_main(firmware_path_beta, 2, 1, 2)) {
												printDisplay("\033[0;32m\n");
												printDisplay(language_vars.lng_install_firmware_end_success);
												printDisplay("\033[0;37m\n");
											} else {
												printDisplay("\033[0;31m\n");
												printDisplay(language_vars.lng_install_firmware_end_error);
												printDisplay("\033[0;37m\n");
											}
										}
									} else {
										printDisplay("\033[0;31m");
										printDisplay(language_vars.lng_install_firmware_error_folder_not_found);
										printDisplay("\033[0;37m\n");
									}
								}
								printDisplay("\033[0;32m\n");
								printDisplay(language_vars.lng_success_reboot_in_five_seconds);
								printDisplay("\033[0;37m\n");
								sleep(5);
								aply_reboot();
							} else {
								remove(TEMP_FILE);
							}
						}
					} else {
						printDisplay("\033[0;31m");
						printDisplay(language_vars.lng_install_pack_download_pack_error);
						printDisplay("\033[0;37m\n");
						remove(TEMP_FILE);
					}
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_APP:
			{
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
					bool dl_app_res;
					if (!beta_mode) {
						dl_app_res = downloadFile(APP_URL, TEMP_FILE, OFF, true);
					} else {
						dl_app_res = downloadFile(APP_URL_beta, TEMP_FILE, OFF, true);
					}
				if (dl_app_res) {
					if (get_sd_size_left() <= 4000000) {
						printDisplay("\033[0;31m");
						printDisplay(language_vars.lng_error_not_enough_space_on_sd);
						printDisplay("\033[0;37m\n");
					} else {
						cp((char*) "romfs:/nro/aiosu-forwarder.nro", (char*) "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro");
						printDisplay("\033[0;32m\n");
						printDisplay(language_vars.lng_success_reboot_in_five_seconds);
						printDisplay("\033[0;37m\n");
						sleep(5);
						appExit();
						envSetNextLoad("/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro", "\"/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro\"");
						return 0;
					}
				}
				else
				{
					printDisplay("\033[0;31m");
					printDisplay(language_vars.lng_install_app_download_app_error);
					printDisplay("\033[0;37m\n");
					remove(TEMP_FILE);
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_90DNS:
			{
				consoleSelect(&logs_console);
				if (set_90dns()) {
					printDisplay("\033[0;32m\n");
					printDisplay(language_vars.lng_success_reboot_in_five_seconds);
					printDisplay("\033[0;37m\n");
					sleep(5);
					simple_reboot();
				} else {
						printDisplay("\033[0;31m");
						printDisplay(language_vars.lng_dns_end_install_error);
						printDisplay("\033[0;37m\n");
				}
				consoleSelect(&menu_console);
				break;
			}

			case UP_ATMO_PROTECT_CONFIGS:
			{
				consoleSelect(&logs_console);
				printDisplay(language_vars.lng_protect_console_begin);
				printDisplay("\n");
				if (get_sd_size_left() <= 100000) {
					printDisplay("\033[0;31m");
					printDisplay(language_vars.lng_error_not_enough_space_on_sd);
					printDisplay("\033[0;37m\n");
				} else {
					mkdir((char*) "/atmosphere", 0777);
					mkdir((char*) "/atmosphere/config", 0777);
					mkdir((char*) "/atmosphere/hosts", 0777);
					mkdir((char*) "/bootloader", 0777);
					bool test_cp = true;
					if (!cp((char*) "romfs:/config_files/exosphere.ini", (char*) "/exosphere.ini")) test_cp = false;
					if (!cp((char*) "romfs:/config_files/system_settings.ini", (char*) "/atmosphere/config/system_settings.ini")) test_cp = false;
					if (!cp((char*) "romfs:/config_files/default.txt", (char*) "/atmosphere/hosts/default.txt")) test_cp = false;
					if (!cp((char*) "romfs:/config_files/hekate_ipl.ini", (char*) "/bootloader/hekate_ipl.ini")) test_cp = false;
					if (!set_90dns()) test_cp = false;
					if (test_cp) {
						printDisplay("\033[0;32m\n");
						printDisplay(language_vars.lng_success_reboot_in_five_seconds);
						printDisplay("\033[0;37m\n");
						sleep(5);
						simple_reboot();
					} else {
						printDisplay("\033[0;31m\n");
						printDisplay(language_vars.lng_protect_console_error);
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
