#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>
#include <switch.h>

#include "download.hpp"
#include "translate.hpp"

#define API_AGENT "PoloNX"
#define _1MiB   0x100000

extern translation_map language_vars;
extern PrintConsole logs_console;

static time_t prevtime;
time_t currtime;
double dif;
static bool first = true;
double dlold;

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

static size_t WriteMemoryCallback(void *contents, size_t size, size_t num_files, void *userp) {
	ntwrk_struct_t *data_struct = (ntwrk_struct_t *)userp;
	size_t realsize = size * num_files;

	if (realsize + data_struct->offset >= data_struct->data_size) {
		fwrite(data_struct->data, data_struct->offset, 1, data_struct->out);
		data_struct->offset = 0;
	}

	memcpy(&data_struct->data[data_struct->offset], contents, realsize);
	data_struct->offset += realsize;
	data_struct->data[data_struct->offset] = 0;
	return realsize;
}

int download_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (first) {
		time(&prevtime);
		first = false;
	}
	time(&currtime);
	dif = difftime(currtime, prevtime);
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	int counter = round(tv.tv_usec / 100000);

	if (counter == 0 || counter == 2 || counter == 4 || counter == 6 || counter == 8) {
		if (dltotal <= 0.0) {
			if (dif > 1.2f) {
				printf(language_vars["lng_dl_progress_0"], dlnow / _1MiB, (dlnow - dlold) / dif);
				printf("\r");
			} else {
				printf(language_vars["lng_dl_progress_1"], dlnow / _1MiB);
				printf("\r");
			}
		} else {
			int numberOfEqual = (dlnow * 100) / dltotal / 10;
			printf("\r* [");
			for (int i = 0; i < numberOfEqual; i++) {
				printf("=");
			}
			for (int i = 0; i < 10 - numberOfEqual; i++) {
				printf(" ");
			}
			if (dif > 1.2f) {
				printf("]   ");
				printf(language_vars["lng_dl_progress_with_bar_0"], dlnow / _1MiB, dltotal / _1MiB, (dlnow - dlold) / dif);
			} else {
				printf("]   ");
				printf(language_vars["lng_dl_progress_with_bar_1"], dlnow / _1MiB, dltotal / _1MiB);
			}
		}
		consoleUpdate(&logs_console);
	}

	dlold = dlnow;
	prevtime = currtime;
	return 0;
}

bool downloadFile(const char *url, const char *output, int api, bool display_log) {
	if (display_log) {
		printf("\n\033[0;32m");
		printf(language_vars["lng_dl_begin"], url);
		printf("\033[0;37m\n");
		consoleUpdate(&logs_console);
	}
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	if (curl) {
		FILE *fp = fopen(output, "wb");
		if (fp) {
			if (display_log) {
				printf("\n");
			}

			ntwrk_struct_t chunk = {0};
			chunk.data = (u_int8_t*) malloc(_1MiB);
			chunk.data_size = _1MiB;
			chunk.out = fp;

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, API_AGENT);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

			// write calls
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

			if (api == OFF) {
				if (display_log) {
					curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
					curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
					
				}
			}

			// execute curl, save result
			CURLcode res = curl_easy_perform(curl);

			// write from mem to file
			if (chunk.offset) {
				if (chunk.offset != fwrite(chunk.data, 1, chunk.offset, fp)) {
					if (display_log) {
						printf("\033[0;31m");
						printf(language_vars["lng_dl_file_write_error"]);
						printf("\033[0;37m\n");
						consoleUpdate(&logs_console);
					}
					curl_easy_cleanup(curl);
					curl_global_cleanup();
					free(chunk.data);
					fclose(chunk.out);
					fclose(fp);
					return false;
				}
			}

			curl_easy_cleanup(curl);
			curl_global_cleanup();
			free(chunk.data);
			fclose(chunk.out);
			fclose(fp);
			if (res == CURLE_OK) {
				if (display_log) {
					printf("\n\n\033[0;32m");
					printf(language_vars["lng_dl_success"]);
					printf("\033[0;37m\n\n");
					consoleUpdate(&logs_console);
				}
				return true;
			} else {
				if (display_log) {
					printf("\n\n\033[0;31m");
					printf(language_vars["lng_dl_dl_error"]);
					printf("\033[0;37m\n\n");
					consoleUpdate(&logs_console);
				}
				return false;
			}
		} else {
			curl_easy_cleanup(curl);
			if (display_log) {
				printf("\n\n\033[0;31m");
				printf(language_vars["lng_dl_open_temp_file_error"]);
				printf("\033[0;37m\n\n");
				consoleUpdate(&logs_console);
			}
			curl_global_cleanup();
			return false;
		}
	} else {
		if (display_log) {
			printf("\n\n\033[0;31m");
			printf(language_vars["lng_dl_curl_init_error"]);
			printf("\033[0;37m\n\n");
			consoleUpdate(&logs_console);
		}
		curl_global_cleanup();
		return false;
	}

	curl_global_cleanup();
	return false;
}
