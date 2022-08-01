#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#define TEMP_FILE              "/switch/AIO_LS_pack_Updater/temp.zip"

#define ON              1
#define OFF             0


#include <stdbool.h>

//
bool downloadFile(const char *url, const char *output, int api, bool display_log);

#endif
