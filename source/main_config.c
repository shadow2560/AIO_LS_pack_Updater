#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "main_util.h"
#include "ini.h"
#include "main_config.h"

extern char CFW_URL[1003];
extern char CFW_URL_beta[1003];
extern char pack_version_url[1003];
extern char pack_version_url_beta[1003];
extern char pack_version_local_filepath[FS_MAX_PATH];
extern char pack_version_local_filepath_beta[FS_MAX_PATH];
extern char subfolder_in_zip[FS_MAX_PATH];
extern char subfolder_in_zip_beta[FS_MAX_PATH];
extern s64 pack_size;
extern s64 pack_size_beta;
extern char APP_URL[1003];
extern char APP_URL_beta[1003];
extern char firmware_path[FS_MAX_PATH];
extern char firmware_path_beta[FS_MAX_PATH];
extern char atmo_logo_dir[FS_MAX_PATH];
extern char atmo_logo_dir_beta[FS_MAX_PATH];
extern char hekate_nologo_file_path[FS_MAX_PATH];
extern char hekate_nologo_file_path_beta[FS_MAX_PATH];
extern int exit_mode_param;
extern int exit_mode_param_beta;

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

int get_emunand_type() {
	if (!is_emummc()) {
		return 0;
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
					free((void*)emummc_config.e1.path);
					free((void*)emummc_config.e1.nintendo_path);
					return 1;
				}
			} else {
				if (strcmp(emummc_config.e1.path, "") != 0) {
					free((void*)emummc_config.e1.path);
				}
				free((void*)emummc_config.e1.nintendo_path);
				return 2;
			}
		}
	}
	return 0;
}