#ifndef _DOWNLOAD_HPP_
#define _DOWNLOAD_HPP_

typedef struct {
	char *memory;
	size_t size;
} MemoryStruct_t;

typedef struct {
	u_int8_t *data;
	size_t data_size;
	u_int64_t offset;
	FILE *out;
} ntwrk_struct_t;

#define TEMP_FILE              "/switch/AIO_LS_pack_Updater/temp.zip"

#define ON              1
#define OFF             0

#include <stdbool.h>

//
bool downloadFile(const char *url, const char *output, int api, bool display_log);
bool downloadInMemory(char *url, MemoryStruct_t *chunk, int api, bool display_log);

#endif