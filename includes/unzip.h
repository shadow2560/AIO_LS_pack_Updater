#ifndef _UNZIP_H_
#define _UNZIP_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

void fnc_clean_logo(char *atmo_logo_folder, char *hekate_nologo_file_path);
void fnc_clean_theme();
void clean_sd(bool clean_theme);
int unzip(const char *output, char *subfolder_in_zip);

#ifdef __cplusplus
}
#endif

#endif