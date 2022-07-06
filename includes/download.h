#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

// #define APP_URL         "https://github.com/shadow2560/test/raw/main/AIO_LS_pack_Updater.nro"
// #define APP_URL         "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"
// #define CFW_URL         "https://github.com/shadow2560/test/archive/refs/heads/main.zip"
// #define CFW_URL         "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip"
#define TEMP_FILE       "/switch/AIO_LS_pack_Updater/temp.zip"

#define ON              1
#define OFF             0


#include <stdbool.h>

//
bool downloadFile(const char *url, const char *output, int api);

#endif
