#ifndef _UNZIP_HPP_
#define _UNZIP_HPP_

void fnc_clean_logo(char *atmo_logo_folder, char *hekate_nologo_file_path);
void fnc_clean_theme();
void fnc_agressive_clean();
void clean_sd(bool clean_theme, bool agressive_clean);
int unzip(const char *output, char *subfolder_in_zip, bool keep_files);

#endif