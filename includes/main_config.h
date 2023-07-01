#ifndef _MAIN_CONFIG_H_
#define _MAIN_CONFIG_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

// define a structure for holding the values in "config" section of the ini file.
typedef struct{
	const char* dl_pack;
	const char* pack_sha256;
	const char* pack_custom_files;
	const char* pack_custom_files_sha256;
	const char* pack_custom_files_subfolder_in_zip;
	s64 pack_custom_files_size;
	const char* dl_pack_version;
	const char* pack_version_local_filepath;
	const char* subfolder_in_zip_pack;
	s64 pack_size;
	const char* dl_app;
	const char* app_sha256;
	const char* app_version;
	const char *firmware_path;
	const char *atmo_logo_dir;
	const char *hekate_nologo_file_path;
	int pack_files_in_zip_sha256_verify_before_copy;
	int exit_method;
	int hekate_autoboot_choice_time;
	int debug_enabled;
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

// define a structure for holding the values in "config" section of the hekate ini file.
typedef struct{
	int autoboot;
	int autoboot_lineno;
	int autoboot_config;
	int autoboot_config_lineno;
} hekate_config_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	hekate_config_section h1;
} hekate_configuration;

// define a structure for holding the values in "autoconfig" section of the autoconfig ini file.
typedef struct{
	short use_all_app_functions;
	short pack_beta_enable;
	short install_firmware;
	short delete_theme;
	short agressive_clean;
	short module_clean;
	short delete_some_files_protection;
	short hekate_autoboot_enable;
	short delete_logos;
	short hbmenu_install;
} autoconfig_config_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	autoconfig_config_section c1;
} autoconfig_configuration;

void configs_init();
int get_emunand_type();
void get_hekate_autoboot_status();
void get_autoconfig();

#ifdef __cplusplus
}
#endif

#endif