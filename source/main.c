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

#define APP_VERSION			 "2.0.0"
#define CURSOR_LIST_MAX		 2

	char CFW_URL[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
	char subfolder_in_zip[1003] = "switch_AIO_LS_pack-main/";
	char APP_URL[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro";

const char *OPTION_LIST[] =
{
	"= Mise a jour de l'application",
	"= Mise a jour du pack switch_AIO_LS_pack",
	"= Application de la protection DNS sur tous les reseaux Wifi deja configures"
};

// define a structure for holding the values in "config" section of the ini file.
typedef struct{
	const char* dl_pack;
	const char* subfolder_in_zip_pack;
	const char* dl_app;
} config_section;

// define a structure for holding all of the config.
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

	for (int i = 0; i < CURSOR_LIST_MAX + 1; i++)
		printf("[%c] %s\n\n", cursor == i ? 'X' : ' ', OPTION_LIST[i]);

	consoleUpdate(NULL);
}

void printDisplay(const char *text, ...)
{
	va_list v;
	va_start(v, text);
	vfprintf(stdout, text, v);
	va_end(v);
	consoleUpdate(NULL);
}

int appInit()
{
	consoleInit(NULL);
	socketInitializeDefault();
	nxlinkStdio();
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	romfsInit();	//Init of romfs

	return 0;
}

void appExit()
{
	socketExit();
	consoleExit(NULL);
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

	// muh loooooop
	while(appletMainLoop())
	{
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);

		// move cursor down...
		if (kDown & HidNpadButton_Down)
		{
			if (cursor == CURSOR_LIST_MAX) cursor = 0;
			else cursor++;
			refreshScreen(cursor);
		}

		// move cursor up...
		if (kDown & HidNpadButton_Up)
		{
			if (cursor == 0) cursor = CURSOR_LIST_MAX;
			else cursor--;
			refreshScreen(cursor);
		}

		if (kDown & HidNpadButton_A)
		{
			switch (cursor)
			{
			case UP_CFW:
				mkdir(APP_PATH, 0777);
				if (downloadFile(CFW_URL, TEMP_FILE, OFF)){
					set_90dns();
					clean_sd();
					unzip("/switch/AIO_LS_pack_Updater/temp.zip");
					remove(TEMP_FILE);
					rebootAms_rcm();
				}
				
				else
				{
					printDisplay("Une erreure est survenue lors du telechargement du cfw. etes vous connecte a internet ?\n");
					remove(TEMP_FILE);
				}
				break;

			case UP_APP:
				mkdir(APP_PATH, 0777);
				if (downloadFile(APP_URL, TEMP_FILE, OFF))
				{
					cp("romfs:/nro/aiosu-forwarder.nro", "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro");
					printDisplay("\033[0;32m\nFini!\n\nRedemarrage de l'app dans 5 secondes:)\n");
					sleep(5);
					envSetNextLoad("/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro", "\"/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro\"");
					appExit();
					return 0;
				}
				else
				{
					printDisplay("Une erreure est survenue lors du telechargement de l'application\n");
					remove(TEMP_FILE);
				}
				break;

			case UP_90dns:
				if (set_90dns()) {
					sleep(5);
					rebootSystem();
				} else {
						printDisplay("Une erreur s'est produite durant l'application des paramètres DNS.");
				}
				break;

			}
		}
		
		// exit...
		if (kDown & HidNpadButton_Plus) break;
	}

	// cleanup then exit
	appExit();
	return 0;
}
