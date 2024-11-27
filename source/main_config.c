#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "main_util.h"
#include "ini.h"
#include "main_config.h"

extern bool debug_enabled;

extern char CFW_URL[1003];
extern char pack_sha256_url[1003];
extern char pack_changelog_url[1003];
extern char CFW_URL_beta[1003];
extern char pack_sha256_url_beta[1003];
extern char pack_changelog_url_beta[1003];
extern char pack_custom_files_url[1003];
extern char pack_custom_files_url_beta[1003];
extern char pack_custom_files_sha256_url[1003];
extern char pack_custom_files_sha256_url_beta[1003];
extern char pack_custom_files_subfolder_in_zip[FS_MAX_PATH];
extern char pack_custom_files_subfolder_in_zip_beta[FS_MAX_PATH];
extern s64 pack_custom_files_size;
extern s64 pack_custom_files_size_beta;
extern char pack_version_url[1003];
extern char pack_version_url_beta[1003];
extern char pack_version_local_filepath[FS_MAX_PATH];
extern char pack_version_local_filepath_beta[FS_MAX_PATH];
extern char subfolder_in_zip[FS_MAX_PATH];
extern char subfolder_in_zip_beta[FS_MAX_PATH];
extern s64 pack_size;
extern s64 pack_size_beta;
extern char APP_URL[1003];
extern char app_sha256_url[1003];
extern char app_version_url[1003];
extern char APP_URL_beta[1003];
extern char app_sha256_url_beta[1003];
extern char app_version_url_beta[1003];
extern char firmware_path[FS_MAX_PATH];
extern char firmware_path_beta[FS_MAX_PATH];
extern char atmo_logo_dir[FS_MAX_PATH];
extern char atmo_logo_dir_beta[FS_MAX_PATH];
extern char hekate_nologo_file_path[FS_MAX_PATH];
extern char hekate_nologo_file_path_beta[FS_MAX_PATH];
extern int pack_files_in_zip_sha256_verify_before_copy_param;
extern int pack_files_in_zip_sha256_verify_before_copy_param_beta;
extern int exit_mode_param;
extern int exit_mode_param_beta;
extern int install_pack_hekate_autoboot_choice_time;
extern int install_pack_hekate_autoboot_choice_time_beta;
extern int debug_enabled_param;
extern int debug_enabled_param_beta;

extern int hekate_autoboot;
extern int hekate_autoboot_lineno;
extern int hekate_autoboot_config;
extern int hekate_autoboot_config_lineno;

extern bool autoconfig_enabled;

static int config_handler(void* config, const char* section, const char* name, const char* value, int lineno)
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
	} else if(MATCH("config", "pack_sha256")){
		if (value != 0) {
			pconfig->s1.pack_sha256 = strdup(value);
		} else {
			pconfig->s1.pack_sha256 = "";
		}
	} else if(MATCH("config", "pack_changelog")){
		if (value != 0) {
			pconfig->s1.pack_changelog = strdup(value);
		} else {
			pconfig->s1.pack_changelog = "";
		}
	} else if(MATCH("config", "pack_custom_files_adress")){
		if (value != 0) {
			pconfig->s1.pack_custom_files = strdup(value);
		} else {
			pconfig->s1.pack_custom_files = "";
		}
	} else if(MATCH("config", "pack_custom_files_sha256_adress")){
		if (value != 0) {
			pconfig->s1.pack_custom_files_sha256 = strdup(value);
		} else {
			pconfig->s1.pack_custom_files_sha256 = "";
		}
	} else if(MATCH("config", "custom_files_subfolder_in_zip_pack")){
		if (value != 0) {
			pconfig->s1.pack_custom_files_subfolder_in_zip = strdup(value);
		} else {
			pconfig->s1.pack_custom_files_subfolder_in_zip = "";
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
	}else if(MATCH("config", "app_sha256")){
		if (value != 0) {
			pconfig->s1.app_sha256 = strdup(value);
		} else {
			pconfig->s1.app_sha256 = "";
		}
	}else if(MATCH("config", "app_version_adress")){
		if (value != 0) {
			pconfig->s1.app_version = strdup(value);
		} else {
			pconfig->s1.app_version = "";
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
	}else if(MATCH("config", "pack_custom_files_size")){
		if (value != 0) {
			pconfig->s1.pack_custom_files_size = atoll(value);
		} else {
			pconfig->s1.pack_custom_files_size = 0;
		}
	}else if(MATCH("config", "pack_files_in_zip_sha256_verify_before_copy")){
		if (value != 0) {
			pconfig->s1.pack_files_in_zip_sha256_verify_before_copy = atoll(value);
		} else {
			pconfig->s1.pack_files_in_zip_sha256_verify_before_copy = 1;
		}
	}else if(MATCH("config", "exit_method")){
		if (value != 0) {
			pconfig->s1.exit_method = atoll(value);
		} else {
			pconfig->s1.exit_method = 0;
		}
	}else if(MATCH("config", "hekate_autoboot_choice_time")){
		if (value != 0) {
			pconfig->s1.hekate_autoboot_choice_time = atoll(value);
		} else {
			pconfig->s1.hekate_autoboot_choice_time = 0;
		}
	}else if(MATCH("config", "debug_enabled")){
		if (value != 0) {
			pconfig->s1.debug_enabled = atoll(value);
		} else {
			pconfig->s1.debug_enabled = 0;
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
	config.s1.pack_sha256 = "";
	config.s1.pack_changelog = "";
	config.s1.pack_custom_files = "";
	config.s1.pack_custom_files_sha256 = "";
	config.s1.pack_custom_files_subfolder_in_zip = "";
	config.s1.dl_pack_version = "";
	config.s1.pack_version_local_filepath = "";
	config.s1.subfolder_in_zip_pack = "";
	config.s1.dl_app = "";
	config.s1.app_sha256 = "";
	config.s1.app_version = "";
	config.s1.firmware_path = "";
	config.s1.atmo_logo_dir = "";
	config.s1.hekate_nologo_file_path = "";
	config.s1.pack_size = 0;
	config.s1.pack_custom_files_size = 0;
	config.s1.pack_files_in_zip_sha256_verify_before_copy = 1;
	config.s1.exit_method = 0;
	config.s1.hekate_autoboot_choice_time = 0;
	config.s1.debug_enabled = 0;
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
			if (strcmp(config.s1.pack_sha256, "") != 0) {
				strcpy(pack_sha256_url, config.s1.pack_sha256);
				free((void*)config.s1.pack_sha256);
			}
			if (strcmp(config.s1.pack_changelog, "") != 0) {
				strcpy(pack_changelog_url, config.s1.pack_changelog);
				free((void*)config.s1.pack_changelog);
			}
			if (strcmp(config.s1.pack_custom_files, "") != 0) {
				strcpy(pack_custom_files_url, config.s1.pack_custom_files);
				free((void*)config.s1.pack_custom_files);
			}
			if (strcmp(config.s1.pack_custom_files_sha256, "") != 0) {
				strcpy(pack_custom_files_sha256_url, config.s1.pack_custom_files_sha256);
				free((void*)config.s1.pack_custom_files_sha256);
			}
			if (strcmp(config.s1.pack_custom_files_subfolder_in_zip, "") != 0) {
				strcpy(pack_custom_files_subfolder_in_zip, config.s1.pack_custom_files_subfolder_in_zip);
				free((void*)config.s1.pack_custom_files_subfolder_in_zip);
			}
			if (strcmp(config.s1.dl_pack_version, "") != 0) {
				strcpy(pack_version_url, config.s1.dl_pack_version);
				free((void*)config.s1.dl_pack_version);
			}
			if (strcmp(config.s1.pack_version_local_filepath, "") != 0) {
				strcpy(pack_version_local_filepath, config.s1.pack_version_local_filepath);
				free((void*)config.s1.pack_version_local_filepath);
			}
			if (strcmp(config.s1.subfolder_in_zip_pack, "") != 0) {
				strcpy(subfolder_in_zip, config.s1.subfolder_in_zip_pack);
				free((void*)config.s1.subfolder_in_zip_pack);
			}
			if (strcmp(config.s1.dl_app, "") != 0) {
				strcpy(APP_URL, config.s1.dl_app);
				free((void*)config.s1.dl_app);
			}
			if (strcmp(config.s1.app_sha256, "") != 0) {
				strcpy(app_sha256_url, config.s1.app_sha256);
				free((void*)config.s1.app_sha256);
			}
			if (strcmp(config.s1.app_version, "") != 0) {
				strcpy(app_version_url, config.s1.app_version);
				free((void*)config.s1.app_version);
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
			if (config.s1.pack_custom_files_size != 0) {
				pack_custom_files_size = config.s1.pack_custom_files_size;
			}
			if (config.s1.pack_files_in_zip_sha256_verify_before_copy != 0) {
				pack_files_in_zip_sha256_verify_before_copy_param = 1;
			}
			if (config.s1.exit_method != 0) {
				exit_mode_param = 1;
			}
			if (config.s1.hekate_autoboot_choice_time != 0) {
				install_pack_hekate_autoboot_choice_time = config.s1.hekate_autoboot_choice_time;
			}
			if (config.s1.debug_enabled != 0) {
				debug_enabled_param = 1;
			}
		}
	}
	configuration config_beta;
	config_beta.s1.dl_pack = "";
	config_beta.s1.pack_sha256 = "";
	config_beta.s1.pack_changelog = "";
	config_beta.s1.pack_custom_files = "";
	config_beta.s1.pack_custom_files_sha256 = "";
	config_beta.s1.pack_custom_files_subfolder_in_zip = "";
	config_beta.s1.dl_pack_version = "";
	config_beta.s1.pack_version_local_filepath = "";
	config_beta.s1.subfolder_in_zip_pack = "";
	config_beta.s1.dl_app = "";
	config_beta.s1.app_sha256 = "";
	config_beta.s1.app_version = "";
	config_beta.s1.firmware_path = "";
	config_beta.s1.atmo_logo_dir = "";
	config_beta.s1.hekate_nologo_file_path = "";
	config_beta.s1.pack_size = 0;
	config_beta.s1.pack_custom_files_size = 0;
	config_beta.s1.pack_files_in_zip_sha256_verify_before_copy = 1;
	config_beta.s1.exit_method = 0;
	config_beta.s1.hekate_autoboot_choice_time = 0;
	config_beta.s1.debug_enabled = 0;
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
			if (strcmp(config_beta.s1.pack_sha256, "") != 0) {
				strcpy(pack_sha256_url_beta, config_beta.s1.pack_sha256);
				free((void*)config_beta.s1.pack_sha256);
			}
			if (strcmp(config_beta.s1.pack_changelog, "") != 0) {
				strcpy(pack_changelog_url_beta, config_beta.s1.pack_changelog);
				free((void*)config_beta.s1.pack_changelog);
			}
			if (strcmp(config_beta.s1.pack_custom_files, "") != 0) {
				strcpy(pack_custom_files_url_beta, config_beta.s1.pack_custom_files);
				free((void*)config_beta.s1.pack_custom_files);
			}
			if (strcmp(config_beta.s1.pack_custom_files_sha256, "") != 0) {
				strcpy(pack_custom_files_sha256_url_beta, config_beta.s1.pack_custom_files_sha256);
				free((void*)config_beta.s1.pack_custom_files_sha256);
			}
			if (strcmp(config_beta.s1.pack_custom_files_subfolder_in_zip, "") != 0) {
				strcpy(pack_custom_files_subfolder_in_zip_beta, config_beta.s1.pack_custom_files_subfolder_in_zip);
				free((void*)config_beta.s1.pack_custom_files_subfolder_in_zip);
			}
			if (strcmp(config_beta.s1.dl_pack_version, "") != 0) {
				strcpy(pack_version_url_beta, config_beta.s1.dl_pack_version);
				free((void*)config_beta.s1.dl_pack_version);
			}
			if (strcmp(config_beta.s1.pack_version_local_filepath, "") != 0) {
				strcpy(pack_version_local_filepath_beta, config_beta.s1.pack_version_local_filepath);
				free((void*)config_beta.s1.pack_version_local_filepath);
			}
			if (strcmp(config_beta.s1.subfolder_in_zip_pack, "") != 0) {
				strcpy(subfolder_in_zip_beta, config_beta.s1.subfolder_in_zip_pack);
				free((void*)config_beta.s1.subfolder_in_zip_pack);
			}
			if (strcmp(config_beta.s1.dl_app, "") != 0) {
				strcpy(APP_URL_beta, config_beta.s1.dl_app);
				free((void*)config_beta.s1.dl_app);
			}
			if (strcmp(config_beta.s1.app_sha256, "") != 0) {
				strcpy(app_sha256_url_beta, config_beta.s1.app_sha256);
				free((void*)config_beta.s1.app_sha256);
			}
			if (strcmp(config_beta.s1.app_version, "") != 0) {
				strcpy(app_version_url_beta, config_beta.s1.app_version);
				free((void*)config_beta.s1.app_version);
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
			if (config_beta.s1.pack_custom_files_size != 0) {
				pack_custom_files_size_beta = config_beta.s1.pack_custom_files_size;
			}
			if (config_beta.s1.pack_files_in_zip_sha256_verify_before_copy != 0) {
				pack_files_in_zip_sha256_verify_before_copy_param_beta = 1;
			}
			if (config_beta.s1.exit_method != 0) {
				exit_mode_param_beta = 1;
			}
			if (config_beta.s1.hekate_autoboot_choice_time != 0) {
				install_pack_hekate_autoboot_choice_time_beta = config_beta.s1.hekate_autoboot_choice_time;
			}
			if (config_beta.s1.debug_enabled != 0) {
				debug_enabled_param_beta = 1;
			}
		}
	}
	if (!debug_enabled) {
		if (debug_enabled_param != 0 || debug_enabled_param_beta != 0) {
			debug_enabled = true;
		}
	} else {
		debug_log_write("Debug mode forcé.\n");
	}
}

static int emummc_config_handler(void* config, const char* section, const char* name, const char* value, int lineno)
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
	int ret = 0;
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
				ret = 1;
			} else {
				ret = 2;
			}
			if (strcmp(emummc_config.e1.path, "") != 0) {
				free((void*)emummc_config.e1.path);
			}
			if (strcmp(emummc_config.e1.nintendo_path, "") != 0) {
				free((void*)emummc_config.e1.nintendo_path);
			}
		}
	}
	return ret;
}

static int hekate_config_handler(void* config, const char* section, const char* name, const char* value, int lineno)
{
	// config instance for filling in the values.
	hekate_configuration* pconfig = (hekate_configuration*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	// fill the values in config struct for config 1.
	if(MATCH("config", "autoboot")){
		if (value != 0) {
			pconfig->h1.autoboot = atoll(value);
			pconfig->h1.autoboot_lineno = lineno;
		} else {
			pconfig->h1.autoboot = 0;
			pconfig->h1.autoboot_lineno = -1;
		}
	} else if(MATCH("config", "autoboot_list")){
		if (value != 0) {
			pconfig->h1.autoboot_config = atoll(value);
			pconfig->h1.autoboot_config_lineno = lineno;
		} else {
			pconfig->h1.autoboot_config = 0;
			pconfig->h1.autoboot_config_lineno = -1;
		}
	// }else{
		// return 0;
	}
	return 1;
}

void get_hekate_autoboot_status() {
	// config for holding ini file values.
	hekate_configuration hekate_config;
	hekate_config.h1.autoboot = 0;
	hekate_config.h1.autoboot_lineno = -1;
	hekate_config.h1.autoboot_config = 0;
	hekate_config.h1.autoboot_config_lineno = -1;
	FILE *test_ini;
	test_ini = fopen("/bootloader/hekate_ipl.ini", "r");
	if (test_ini != NULL) {
		fclose(test_ini);
		// parse the .ini file
		if (ini_parse("/bootloader/hekate_ipl.ini", hekate_config_handler, &hekate_config) == 0) {
			if (hekate_config.h1.autoboot != 0) {
				hekate_autoboot = hekate_config.h1.autoboot_config;
				hekate_autoboot_lineno = hekate_config.h1.autoboot_lineno;
			} else {
				hekate_autoboot = 0;
				hekate_autoboot_lineno = hekate_config.h1.autoboot_lineno;
			}
			if (hekate_config.h1.autoboot_config != 0) {
				hekate_autoboot_config = hekate_config.h1.autoboot_config;
				hekate_autoboot_config_lineno = hekate_config.h1.autoboot_config_lineno;
			} else {
				hekate_autoboot_config = 0;
				hekate_autoboot_config_lineno = hekate_config.h1.autoboot_config_lineno;
			}
		}
	}
}

static int autoconfig_config_handler(void* config, const char* section, const char* name, const char* value, int lineno)
{
	// config instance for filling in the values.
	autoconfig_configuration* pconfig = (autoconfig_configuration*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	// fill the values in config struct for config 1.
if(MATCH("autoconfig", "use_all_app_functions")){
		if (value != NULL) {
			pconfig->c1.use_all_app_functions = atoll(value);
		} else {
			pconfig->c1.use_all_app_functions = 0;
		}
	} else if(MATCH("autoconfig", "pack_beta_enable")){
		if (value != NULL) {
			pconfig->c1.pack_beta_enable = atoll(value);
		} else {
			pconfig->c1.pack_beta_enable = 0;
		}
	} else if(MATCH("autoconfig", "install_firmware")){
		if (value != NULL) {
			pconfig->c1.install_firmware = atoll(value);
		} else {
			pconfig->c1.install_firmware = 0;
		}
	} else if(MATCH("autoconfig", "delete_theme")){
		if (value != NULL) {
			pconfig->c1.delete_theme = atoll(value);
		} else {
			pconfig->c1.delete_theme = 0;
		}
			} else if(MATCH("autoconfig", "agressive_clean")){
		if (value != NULL) {
			pconfig->c1.agressive_clean = atoll(value);
		} else {
			pconfig->c1.agressive_clean = 0;
		}
			} else if(MATCH("autoconfig", "module_clean")){
		if (value != NULL) {
			pconfig->c1.module_clean = atoll(value);
		} else {
			pconfig->c1.module_clean = 0;
		}
			} else if(MATCH("autoconfig", "delete_some_files_protection")){
		if (value != NULL) {
			pconfig->c1.delete_some_files_protection = atoll(value);
		} else {
			pconfig->c1.delete_some_files_protection = 0;
		}
			} else if(MATCH("autoconfig", "hekate_autoboot_enable")){
		if (value != NULL) {
			pconfig->c1.hekate_autoboot_enable = atoll(value);
		} else {
			pconfig->c1.hekate_autoboot_enable = 0;
		}
			} else if(MATCH("autoconfig", "delete_logos")){
		if (value != NULL) {
			pconfig->c1.delete_logos = atoll(value);
		} else {
			pconfig->c1.delete_logos = 0;
		}
			} else if(MATCH("autoconfig", "hbmenu_install")){
		if (value != NULL) {
			pconfig->c1.hbmenu_install = atoll(value);
		} else {
			pconfig->c1.hbmenu_install = 0;
		}
			} else if(MATCH("autoconfig", "app_fwd_install")){
		if (value != NULL) {
			pconfig->c1.app_fwd_install = atoll(value);
		} else {
			pconfig->c1.app_fwd_install = 0;
		}
	}else{
		return 0;
	}
	return 1;
}

extern autoconfig_configuration autoconfig_config;

void get_autoconfig() {
	// config for holding ini file values.
	autoconfig_config.c1.use_all_app_functions = 0;
	autoconfig_config.c1.pack_beta_enable = 0;
	autoconfig_config.c1.install_firmware = 0;
	autoconfig_config.c1.delete_theme = 0;
	autoconfig_config.c1.agressive_clean = 0;
	autoconfig_config.c1.module_clean = 0;
	autoconfig_config.c1.delete_some_files_protection = 0;
	autoconfig_config.c1.hekate_autoboot_enable = 0;
	autoconfig_config.c1.delete_logos = 0;
	autoconfig_config.c1.hbmenu_install = 0;
	autoconfig_config.c1.app_fwd_install = 0;
	FILE *test_ini;
	test_ini = fopen("/switch/AIO_LS_pack_Updater/autoconfig.ini", "r");
	if (test_ini != NULL) {
		fclose(test_ini);
		autoconfig_enabled = true;
		// parse the .ini file
		if (ini_parse("/switch/AIO_LS_pack_Updater/autoconfig.ini", autoconfig_config_handler, &autoconfig_config) != 0) {
			autoconfig_enabled = false;
			return;
		}
	} else {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.use_all_app_functions == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.pack_beta_enable == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.install_firmware == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.delete_theme == -1) {
		autoconfig_enabled = false;
		return;
	}
if (autoconfig_config.c1.agressive_clean == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.module_clean == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.delete_some_files_protection == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.hekate_autoboot_enable == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.delete_logos == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.hbmenu_install == -1) {
		autoconfig_enabled = false;
		return;
	}
	if (autoconfig_config.c1.app_fwd_install == -1) {
		autoconfig_enabled = false;
		return;
	}
		return;
}