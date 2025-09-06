#ifndef _MAIN_UTIL_H_
#define _MAIN_UTIL_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <minizip/unzip.h>

typedef struct {
    char path[0x80];
    char nintendo[0x80];
} EmummcPaths;

extern bool beta_mode;
extern bool debug_enabled;
extern PrintConsole menu_console;
extern PrintConsole logs_console;

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

void debug_log_start();
void debug_log_write(const char *text, ...);
void to_lowercase(char *str);
void to_uppercase(char *str);
bool strcmp_ignore_case(const char *s1, const char *s2);
bool custom_cp(char *filein, char *fileout);
bool copy_directory_recursive(const char *source, const char *destination, bool include_source);
int remove_directory(const char *path);
char * substr(char *s, int x, int y);
u32 ParseHexInteger(const char *s);
bool isApplet();
u64 get_app_titleid();
u64 GetCurrentApplicationId();
bool titleid_curently_launched(u64 titleid);
u32 get_battery_charge();
int GetChargerType();
s64 get_sd_size_left();
bool is_emummc();
void smcAmsGetEmunandConfig(EmummcPaths* out_paths);
void get_sha256_file(const char *filepath, char *ret);
void get_sha256_data(void* datas, size_t size, char* ret);
void get_sha256_data_for_minizip_opened_file(unzFile* zfile, char* ret);
bool module_is_running(u64 module);
void close_module(u64 module);
SetSysFirmwareVersion int_get_fw_version();
bool internet_is_connected();

#ifdef __cplusplus
}
#endif

#endif