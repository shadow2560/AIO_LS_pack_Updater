#ifndef _UNZIP_H_
#define _UNZIP_H_

bool cp(char *filein, char *fileout);
void fnc_clean_logo(char *atmo_logo_folder);
void fnc_clean_theme();
void clean_sd(bool clean_theme);
int unzip(const char *output, char *subfolder_in_zip);

#endif