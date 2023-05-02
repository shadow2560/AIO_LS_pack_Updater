#ifndef _MAIN_UTIL_H_
#define _MAIN_UTIL_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

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
void get_sha256_file(const char *filepath, char *ret);

#ifdef __cplusplus
}
#endif

#endif