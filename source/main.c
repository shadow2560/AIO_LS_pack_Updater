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

#define APP_VERSION			 "2.6.0"
#define CURSOR_LIST_MAX		 3
#define UP_APP          0
#define UP_CFW          1
#define UP_90dns          2
#define UP_atmo_protect_configs          3

	char CFW_URL[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
	char subfolder_in_zip[1003] = "switch_AIO_LS_pack-main/";
	s64 pack_size = 1000000000;
	char APP_URL[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro";

FsFileSystem *fs_sd;

PrintConsole menu_console;
PrintConsole logs_console;
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
	"= Mise a jour du pack switch_AIO_LS_pack",
	"= Application de la protection DNS sur tous les reseaux Wifi deja configures",
	"= Application de configurations pour  proteger au mieux la console lancee sous Atmosphere (Atmosphere 0.18.1 minimum requis)"
};

// define a structure for holding the values in "config" section of the ini file.
typedef struct{
	const char* dl_pack;
	const char* subfolder_in_zip_pack;
	s64 pack_size;
	const char* dl_app;
} config_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	config_section s1;
} configuration;

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

void refreshScreen(int cursor)
{
	consoleClear();

	printf("\x1B[36mAIO_LS_pack_Updater: v%s.\x1B[37m\n\n\n", APP_VERSION);
	printf("Appuyez sur (A) pour selectionner une option\n\n");
	printf("Appuyez sur (+) pour quitter l'application\n\n\n");

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

int appInit()
{
	// menu_console = consoleGetDefault();
	consoleInit(&menu_console);
	consoleSetWindow(&menu_console, 0, 0, 80, 20);
	/*
	logs_console.font = menu_console->font;
	logs_console.renderer = NULL;
	logs_console.cursorX = 0;
	logs_console.cursorY = 0;
	logs_console.prevCursorX = 0;
	logs_console.prevCursorY = 0;
	*/
	logs_console.consoleWidth = 80;
	logs_console.consoleHeight = 23;
	logs_console.windowX = 0;
	logs_console.windowY = 21;
	logs_console.windowWidth = 80;
	logs_console.windowHeight = 23;
	logs_console.bg = 6;
	/*
	logs_console.tabSize = 3;
	logs_console.fg = 7;
	logs_console.flags = 0;
	*/
	consoleInit(&logs_console);
	consoleSetWindow(&logs_console, 0, 21, 80, 23);
	consoleSelect(&menu_console);
	// menu_console->font = default_font_bin;
	// consoleSetFont(menu_console, custom_font);
	socketInitializeDefault();
	nxlinkStdio();
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	romfsInit();	//Init of romfs
	fs_sd = fsdevGetDeviceFileSystem("sdmc");

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
		return fs_sd_size;
}

int main(int argc, char **argv)
{
	// init stuff
	appInit();
	// config for holding ini file values.
	configuration config;
	config.s1.dl_pack = "";
	config.s1.subfolder_in_zip_pack = "";
	config.s1.dl_app = "";
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
			if (strcmp(config.s1.subfolder_in_zip_pack, "") != 0) {
				strcpy(subfolder_in_zip, config.s1.subfolder_in_zip_pack);
				free((void*)config.s1.subfolder_in_zip_pack);
			} else {
				strcpy(subfolder_in_zip, "");
			}
			if (strcmp(config.s1.dl_app, "") != 0) {
				strcpy(APP_URL, config.s1.dl_app);
				free((void*)config.s1.dl_app);
			}
			if (config.s1.pack_size != 0) {
				pack_size = config.s1.pack_size;
			}
		}
	}
	PadState pad;
	padInitializeDefault(&pad);

	// change directory to root (defaults to /switch/)
	chdir(ROOT);

	// set the cursor position to 0
	short cursor = 0;

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
			case UP_CFW:
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
				if (downloadFile(CFW_URL, TEMP_FILE, OFF)){
					if (get_sd_size_left() <= pack_size) {
						printDisplay("\033[0;31mErreur, pas assez d'espace sur la SD.\033[0;37m\n");
					} else {
						set_90dns();
						clean_sd();
						if (0 == unzip("/switch/AIO_LS_pack_Updater/temp.zip")) {
							remove(TEMP_FILE);
							rebootAms_rcm();
						} else {
							remove(TEMP_FILE);
						}
					}
				}
				
				else
				{
					printDisplay("\033[0;31mUne erreure est survenue lors du telechargement du cfw.\033[0;37m\n");
					remove(TEMP_FILE);
				}
				consoleSelect(&menu_console);
				break;

			case UP_APP:
				consoleSelect(&logs_console);
				mkdir(APP_PATH, 0777);
				if (downloadFile(APP_URL, TEMP_FILE, OFF))
				{
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
					appExit();
					rebootSystem();
				} else {
						printDisplay("\033[0;31mUne erreur s'est produite durant l'application des paramètres DNS.\033[0;37m\n");
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
						appExit();
						rebootSystem();
					} else {
						printDisplay("\033[0;31m\nUne erreur s'est produite durant l'application des parametres, verifiez l'espace restant sur votre SD.\033[0;37m\n");
					}
				}
				consoleSelect(&menu_console);
				break;

			}
		}
		
		// exit...
		else if (kDown & HidNpadButton_Plus) {
			break;
		}
	}

	// cleanup then exit
	appExit();
	return 0;
}
