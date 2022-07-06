#ifndef _UNZIP_H_
#define _UNZIP_H_

// first sub-folder path in the zip, set to "" if you don't have sub-folder to ignore in your zip. The sub-folders must only contain one folder in each else this won't work properly, this is only intended to localize the real root of the project. The last "/" separator is optional.
// #define subfolder_in_zip          "test-main/"
// #define subfolder_in_zip          "switch_AIO_LS_pack-main/"

#define UP_APP          0
#define UP_CFW          1
#define UP_90dns          2

void cp(char *filein, char *fileout);
void clean_sd();
int unzip(const char *output);

#endif