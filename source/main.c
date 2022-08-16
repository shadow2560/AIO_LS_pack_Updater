#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <switch.h>
#include <string.h>
#include <stdlib.h>

#include "90dns_setter.h"
#include "download.h"
#include "unzip.h"
#include "reboot.h"
#include "ini.h"

#define ROOT					"/"
#define APP_PATH				"/switch/AIO_LS_pack_Updater/"
#define APP_OUTPUT			  "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"

#define APP_VERSION			 "3.2.3"
#define CURSOR_LIST_MAX		 4
#define UP_APP          0
#define UP_CFW          1
#define UP_FW          2
#define UP_90dns          3
#define UP_atmo_protect_configs          4

char CFW_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/Switch_AIO_LS_pack.zip";
char CFW_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
char pack_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/pack_version.txt";
char pack_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/version.txt";
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

char pack_version[15] = "inconnue";
char last_pack_version[15] = "inconnue";
char firmware_version[50] = "inconnue";
char atmosphere_version[50] = "inconnue";
char emummc_value[10] = "inconnue";
char emummc_type[10] = "inconnu";
char fusee_gelee_patch[15] = "inconnu";
char console_model[50] = "inconnu";
u64 console_id = 0;
SetSysSerialNumber console_serial;
bool sd_is_exfat = false;
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
	"= Mise a jour de l'application",
	"= Mise a jour du pack",
	"= Mise a jour du firmware",
	"= Application de la protection DNS sur tous les reseaux Wifi deja configures",
	"= Application de configurations pour  proteger au mieux la console lancee sous Atmosphere (Atmosphere 0.18.1 minimum requis)"
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

// function directly taken from Atmosphere
u32 ParseHexInteger(const char *s) {
            u32 x = 0;
            if (s[0] == '0' && s[1] == 'x') {
                s += 2;
            }

            while (true) {
                const char c = *(s++);

                if (c == '\x00') {
                    return x;
                } else {
                    x <<= 4;

                    if ('0' <= c && c <= '9') {
                        x |= (c - '0');
                    } else if ('a' <= c && c <= 'f') {
                        x |= (c - 'a') + 10;
                    } else if ('A' <= c && c <= 'F') {
                        x |= (c - 'A') + 10;
                    }
                }
            }
        }

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
	}else{
		return 0;
	}
	return 1;
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
					strcpy(emummc_type, "fichiers");
					free((void*)emummc_config.e1.path);
				}
			} else {
				strcpy(emummc_type, "partition");
			}
		}
	}
	free((void*)emummc_config.e1.nintendo_path);
}

void refreshScreen(int cursor)
{
	consoleClear();
	if (!beta_mode) {
		printf("\x1B[36mAIO_LS_pack_Updater: v%s.\x1B[37m\n\n\n", APP_VERSION);
	} else {
		printf("\x1B[31mAIO_LS_pack_Updater: v%s (BETA MODE)\x1B[37m\n\n", APP_VERSION);
	}
	printf("Appuyez sur (A) pour selectionner une option\n\n");
	printf("Appuyez sur (X) pour afficher diverses informations\n");
	printf("Appuyez sur (Y) pour enregistrer diverses informations dans un fichier\n");
	if (!beta_mode) {
		printf("Appuyez sur (-) pour passer en mode beta\n\n");
	} else {
		printf("Appuyez sur (-) pour passer en mode stable\n\n");
	}
	printf("Appuyez sur (+) pour quitter l'application\n\n");

	for (int i = 0; i < CURSOR_LIST_MAX + 1; i++) {
		if (cursor == i) {
			printf("\033[0;31m[X] %s\033[0;37m\n\n", OPTION_LIST[i]);
		} else {
			printf("[ ] %s\n\n", OPTION_LIST[i]);
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
	consoleSetWindow(&menu_console, 0, 0, 80, 24);
	/*
	logs_console.font = menu_console->font;
	logs_console.renderer = NULL;
	logs_console.cursorX = 0;
	logs_console.cursorY = 0;
	logs_console.prevCursorX = 0;
	logs_console.prevCursorY = 0;
	*/
	logs_console.consoleWidth = 80;
	logs_console.consoleHeight = 20;
	logs_console.windowX = 0;
	logs_console.windowY = 25;
	logs_console.windowWidth = 80;
	logs_console.windowHeight = 20;
	logs_console.bg = 6;
	/*
	logs_console.tabSize = 3;
	logs_console.fg = 7;
	logs_console.flags = 0;
	*/
	consoleInit(&logs_console);
	consoleSetWindow(&logs_console, 0, 25, 80, 20);
	consoleSelect(&menu_console);
	// menu_console->font = default_font_bin;
	// consoleSetFont(menu_console, custom_font);
	socketInitializeDefault();
	nxlinkStdio();
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
	socketExit();
	romfsExit();
	consoleExit(&logs_console);
	consoleExit(&menu_console);
}

s64 get_sd_size_left() {
	s64 fs_sd_size;
	fsFsGetFreeSpace(fs_sd, "/", &fs_sd_size);
	// nsInitialize();
	// nsGetFreeSpaceSize(NcmStorageId_SdCard, fs_sd_size);
	// nsExit();
		// printf("%ld\n", fs_sd_size);
		fsIsExFatSupported(&sd_is_exfat);
		return fs_sd_size;
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

u32 EncodeVersion(u32 major, u32 minor, u32 micro, u32 relstep) {
	return ((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((micro & 0xFF) << 8) | ((relstep & 0xFF) << 8);
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
		strcpy(console_model, "Switch Erista non patchée");
		strcpy(fusee_gelee_patch, "Utilisable");
	} else {
		strcpy(console_model, "Switch Erista patchée");
		strcpy(fusee_gelee_patch, "Non utilisable");
	}
	if (hardware_type == 0) {
		if (!has_rcm_bug_patch) {
			strcpy(console_model, "Switch Erista Icosa non patchée");
		} else {
			strcpy(console_model, "Switch Erista Icosa patchée");
		}
	}
	if (hardware_type == 0) {
		if (!has_rcm_bug_patch) {
			strcpy(console_model, "Switch Erista Copper non patchée");
		} else {
			strcpy(console_model, "Switch Erista Copper patchée");
		}
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
	printf("[A]: OUI          [B]: NON\n");
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
	printf("Informations:\n\n");
	printf("Version actuelle du pack : %s\n", pack_version);
	printf("Derniere version du pack : %s\n", last_pack_version);
	printf("ID de la console : %li\n", console_id);
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		printf("Numero de serie de la console  indetectable, Incognito est actif.\n");
	} else {
		printf("Numero de serie de la console : %s\n", console_serial.number);
	}
	if (strcmp(emummc_value, "Emunand") != 0) {
		printf("Type de systeme : %s\n", emummc_value);
	} else {
		printf("Type de systeme : %s via %s\n", emummc_value, emummc_type);
	}
	printf("Modele de la console : %s\n", console_model);
	printf("Etat de l'exploit Fusee Gelee : %s\n", fusee_gelee_patch);
	/*
	if (sd_is_exfat) {
		printf("Formatage de la SD: EXFAT\n");
	} else {
		printf("Formatage de la SD: FAT32\n");
	}
	*/
	printf("Version actuelle du firmware : %s\n", firmware_version);
	printf("Version actuelle d'Atmosphere : %s\n", atmosphere_version);
	// printf("Appuyez sur \"B\" pour revenir au menu principal.\n");
	consoleUpdate(&logs_console);
	/*
	while(1) {
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_B) {
			break;
		}
	}
	*/
	// consoleExit(&infos_console);
	consoleSelect(&menu_console);
}

void record_infos() {
	consoleSelect(&logs_console);
	FILE *log_infos;
	log_infos = fopen("switch/AIO_LS_pack_Updater/console_infos.log", "w");
	if ( log_infos == NULL ) {
		printf("Le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\" n'a pas pu être enregistré.");
		consoleUpdate(&logs_console);
		return;
	}
	fprintf(log_infos, "Informations:\n\n");
	fprintf(log_infos, "Version actuelle du pack : %s\n", pack_version);
	fprintf(log_infos, "Derniere version du pack : %s\n", last_pack_version);
	fprintf(log_infos, "ID de la console : %li\n", console_id);
	if ((console_serial.number[0] == 'x' && console_serial.number[1] == 'a' && console_serial.number[2] == 'w' && console_serial.number[3] == '0') || (console_serial.number[0] == 'X' && console_serial.number[1] == 'A' && console_serial.number[2] == 'W' && console_serial.number[3] == '0')) {
		fprintf(log_infos, "Numero de serie de la console  indetectable, Incognito est actif.\n");
	} else {
		fprintf(log_infos, "Numero de serie de la console : %s\n", console_serial.number);
		
	}
	if (strcmp(emummc_value, "Emunand") != 0) {
		fprintf(log_infos, "Type de systeme : %s\n", emummc_value);
	} else {
		fprintf(log_infos, "Type de systeme : %s via %s\n", emummc_value, emummc_type);
	}
	fprintf(log_infos, "Modele de la console : %s\n", console_model);
	fprintf(log_infos, "Etat de l'exploit Fusee Gelee : %s\n", fusee_gelee_patch);
	/*
	if (sd_is_exfat) {
		fprintf(log_infos, "Formatage de la SD: EXFAT\n");
	} else {
		fprintf(log_infos, "Formatage de la SD: FAT32\n");
	}
	*/
	fprintf(log_infos, "Version actuelle du firmware : %s\n", firmware_version);
	fprintf(log_infos, "Version actuelle d'Atmosphere : %s\n", atmosphere_version);
	printf("Le fichier contenant les informations de la console ont ete enregistrees dans le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\".");
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

int main(int argc, char **argv)
{
	// init stuff
	appInit();
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
	FILE *test_ini_beta;
	test_ini_beta = fopen("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater_beta.ini", "r");
	if (test_ini_beta != NULL) {
		fclose(test_ini_beta);
		// parse the .ini file
		if (ini_parse("/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater_beta.ini", config_handler, &config) == 0) {
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
		}
	}
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
				consoleSelect(&logs_console);
				bool update_firmware2 = false;
				DIR *dir2;
				update_firmware2 = ask_question("Souhaitez-vous vraiment mettre a jour le firmware (si oui les fichiers du theme seront aussi nettoyes)?");
				if (update_firmware2) {
					if (!beta_mode) {
						dir2 = opendir(firmware_path);
					} else {
						dir2 = opendir(firmware_path_beta);
					}
					if (dir2 != NULL) {
						closedir(dir2);
						fnc_clean_theme();
						cp((char*) "romfs:/nro/Daybreak-cli.nro", (char*) "/switch/AIO_LS_pack_Updater/Daybreak-cli.nro");
						char temp_setting[FS_MAX_PATH+100]= "";
						if (!beta_mode) {
							strcat(strcat(strcat(temp_setting, "\"/switch/AIO_LS_pack_Updater/Daybreak-cli.nro\" \""), firmware_path), "\" \"false\" \"true\" \"false\"");
						} else {
							strcat(strcat(strcat(temp_setting, "\"/switch/AIO_LS_pack_Updater/Daybreak-cli.nro\" \""), firmware_path_beta), "\" \"false\" \"true\" \"false\"");
						}
					appExit();
						envSetNextLoad("/switch/AIO_LS_pack_Updater/Daybreak-cli.nro", temp_setting);
						return 0;
					} else {
						printDisplay("\033[0;31mLe repertoire du firmware pour mettre a jour la console n'a pas pu etre ouvert, la mise a jour du firmware ne sera pas faite.\033[0;37m\nLe dossier contenant le firmware est le dossier \"%s\".\n", firmware_path);
					}
				}
				consoleSelect(&menu_console);
				break;
			case UP_CFW:
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
				bool update_firmware = false;
				bool clean_theme = false;
				DIR *dir;
				update_firmware = ask_question("Souhaitez-vous mettre a jour le firmware vers la derniere version compatible (si oui les fichiers du theme seront aussi nettoyes)?");
				if (update_firmware) {
					clean_theme = true;
				} else {
					clean_theme = ask_question("Souhaitez-vous nettoyer les fichiers du theme, utile si mise a jour du firmware par la suite?");
				}
				bool clean_logos = ask_question("Souhaitez-vous retirer les logos?");
				bool validate_choice = ask_question("Souhaitez-vous vraiment continuer?");
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
							printDisplay("\033[0;31mErreur, pas assez d'espace sur la SD.\033[0;37m\n");
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
								if (update_firmware) {
									if (!beta_mode) {
										dir = opendir(firmware_path);
									} else {
										dir = opendir(firmware_path_beta);
									}
									if (dir != NULL) {
										closedir(dir);
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
									} else {
										printDisplay("\033[0;31mLe repertoire du firmware pour mettre a jour la console n'a pas pu etre ouvert, la mise a jour du firmware ne sera pas faite.\033[0;37m\n");
									}
								}
								printDisplay("\033[0;32m\nFinis!\n\nRedemarage automatique dans 5 secondes :)\033[0;37m\n");
								sleep(5);
								aply_reboot();
							} else {
								remove(TEMP_FILE);
							}
						}
					} else {
						printDisplay("\033[0;31mUne erreure est survenue lors du telechargement du pack.\033[0;37m\n");
						remove(TEMP_FILE);
					}
				}
				consoleSelect(&menu_console);
				break;

			case UP_APP:
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
					bool dl_app_res;
					if (!beta_mode) {
						dl_app_res = downloadFile(APP_URL, TEMP_FILE, OFF, true);
					} else {
						dl_app_res = downloadFile(APP_URL_beta, TEMP_FILE, OFF, true);
					}
				if (dl_app_res) {
					if (get_sd_size_left() <= 2000000) {
						printDisplay("\033[0;31mErreur, pas assez d'espace sur la SD.\033[0;37m\n");
					} else {
						cp((char*) "romfs:/nro/aiosu-forwarder.nro", (char*) "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro");
						printDisplay("\033[0;32m\nFini!\n\nRedemarrage de l'application dans 5 secondes:)\033[0;37m\n");
						sleep(5);
						appExit();
						envSetNextLoad("/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro", "\"/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro\"");
						return 0;
					}
				}
				else
				{
					printDisplay("\033[0;31mUne erreure est survenue lors du telechargement de l'application\033[0;37m\n");
					remove(TEMP_FILE);
				}
				consoleSelect(&menu_console);
				break;

			case UP_90dns:
				consoleSelect(&logs_console);
				if (set_90dns()) {
					printDisplay("\033[0;32m\nFini!\n\nRedemarrage de la console dans 5 secondes:)\033[0;37m\n");
					sleep(5);
					simple_reboot();
				} else {
						printDisplay("\033[0;31mUne erreur s'est produite durant l'application des parametres DNS.\033[0;37m\n");
				}
				consoleSelect(&menu_console);
				break;

			case UP_atmo_protect_configs:
				consoleSelect(&logs_console);
				printDisplay("\033[0;32mApplication des configurations de protection...\033[0;37m\n");
				if (get_sd_size_left() <= 100000) {
					printDisplay("\033[0;31mErreur, pas assez d'espace sur la SD.\033[0;37m\n");
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
						printDisplay("\033[0;32m\nFini!\n\nRedemarrage de la console dans 5 secondes:)\033[0;37m\n");
						sleep(5);
						simple_reboot();
					} else {
						printDisplay("\033[0;31m\nUne erreur s'est produite durant l'application des parametres, verifiez l'espace restant sur votre SD.\033[0;37m\n");
					}
				}
				consoleSelect(&menu_console);
				break;

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
			cursor = 0;
			refreshScreen(cursor);

		// exit...
		} else if (kDown & HidNpadButton_Plus) {
			break;
		}
	}

	// cleanup then exit
	appExit();
	return 0;
}
