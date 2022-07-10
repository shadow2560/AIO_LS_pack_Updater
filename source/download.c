#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>
#include <switch.h>

#include "download.h"

#define API_AGENT "PoloNX"
#define _1MiB   0x100000

extern PrintConsole logs_console;

typedef struct
{
	char *memory;
	size_t size;
} MemoryStruct_t;

typedef struct
{
	u_int8_t *data;
	size_t data_size;
	u_int64_t offset;
	FILE *out;
} ntwrk_struct_t;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t num_files, void *userp)
{
	ntwrk_struct_t *data_struct = (ntwrk_struct_t *)userp;
	size_t realsize = size * num_files;

	if (realsize + data_struct->offset >= data_struct->data_size)
	{
		fwrite(data_struct->data, data_struct->offset, 1, data_struct->out);
		data_struct->offset = 0;
	}

	memcpy(&data_struct->data[data_struct->offset], contents, realsize);
	data_struct->offset += realsize;
	data_struct->data[data_struct->offset] = 0;
	return realsize;
}

int download_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	int counter = round(tv.tv_usec / 100000);

	if (counter == 0 || counter == 2 || counter == 4 || counter == 6 || counter == 8)
	{
		if (dltotal <= 0.0) {
			printf("* Telechargement: %.2fMB *\r", dlnow / _1MiB);
		} else {
			printf("* Telechargement: %.2fMB sur %.2fMB *\r", dlnow / _1MiB, dltotal / _1MiB);
		}
		consoleUpdate(&logs_console);
	}

	return 0;
}

bool downloadFile(const char *url, const char *output, int api)
{
	printf("\n\033[0;32mTelechargement de\n%s\nVeuillez patienter...\033[0;37m\n", url);
	consoleUpdate(&logs_console);
	CURL *curl = curl_easy_init();
	if (curl)
	{
		FILE *fp = fopen(output, "wb");
		if (fp)
		{
			printf("\n");

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

			if (api == OFF)
			{
			  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
			}

			// execute curl, save result
			CURLcode res = curl_easy_perform(curl);

			// write from mem to file
			if (chunk.offset)
			  fwrite(chunk.data, 1, chunk.offset, fp);

			curl_easy_cleanup(curl);
			free(chunk.data);
			fclose(chunk.out);
			fclose(fp);
			if (res == CURLE_OK)
			{
				printf("\n\n\033[0;32mTelechargement complete\033[0;37m\n\n");
				consoleUpdate(&logs_console);
				return true;
			} else {
				printf("\n\n\033[0;31mErreur Durant le telechargement, verifiez votre connexion internet ainsi que l'espace restant sur votre SD puis tentez de relancer le telechargement.\033[0;37m\n\n");
				consoleUpdate(&logs_console);
				return false;
			}
		} else {
			curl_easy_cleanup(curl);
			printf("\n\n\033[0;31mErreur d'ouverture du fichier temporaire, tentez de relancer le telechargement.\033[0;37m\n\n");
			consoleUpdate(&logs_console);
			return false;
		}
	} else {
		printf("\n\n\033[0;31mErreur, l'initialisation de curl a echouee, tentez de relancer le telechargement.\033[0;37m\n\n");
		consoleUpdate(&logs_console);
		return false;
	}

	return false;
}
