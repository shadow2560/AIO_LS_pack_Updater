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
	const char* dl_pack_version;
	const char* pack_version_local_filepath;
	const char* subfolder_in_zip_pack;
	s64 pack_size;
	const char* dl_app;
	const char* app_sha256;
	const char *firmware_path;
	const char *atmo_logo_dir;
	const char *hekate_nologo_file_path;
	int exit_method;
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

void configs_init();
int get_emunand_type();

#ifdef __cplusplus
}
#endif

#endif