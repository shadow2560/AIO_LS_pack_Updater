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

void debug_log_start();
void debug_log_write(const char *text, ...);
bool custom_cp(char *filein, char *fileout);
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
void get_sha256_data_for_minizip_opened_file(unzFile* zfile, size_t buf_size, char* ret);
bool module_is_running(u64 module);
void close_module(u64 module);

#ifdef __cplusplus
}
#endif

#endif