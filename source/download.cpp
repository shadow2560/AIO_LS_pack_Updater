#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <curl/curl.h>
#include <switch.h>

#include "main_util.h"
#include "download.hpp"
#include "translate.hpp"

#define API_AGENT "PoloNX"
// #define _1MiB   0x100000
#define _1MO   1000000

extern translation_map language_vars;
extern PrintConsole logs_console;

time_t prevtime;
time_t currtime;
double dif;
bool first = true;
bool dl_speed_displayed = false;
float dl_speed = 0;
double dlold;

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

static size_t WriteMemoryCallback2(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	MemoryStruct_t *mem = (MemoryStruct_t *)userp;

	char *ptr = (char *) realloc(mem->memory, mem->size + realsize + 1);
	if(!ptr) {
		/* out of memory! */
		debug_log_write("Pas assez de mémoire.\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

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
			if (dif >= 1.0f || dl_speed_displayed) {
				printf(language_vars["lng_dl_progress_0"].c_str(), dlnow / _1MO, dl_speed);
				printf("\033[0K\r");
				dl_speed_displayed = true;
			} else {
				printf(language_vars["lng_dl_progress_1"].c_str(), dlnow / _1MO);
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
			if (dif >= 1.0f || dl_speed_displayed) {
				printf("]   ");
				printf(language_vars["lng_dl_progress_with_bar_0"].c_str(), dlnow / _1MO, dltotal / _1MO, dl_speed);
				printf("\033[0K");
				dl_speed_displayed = true;
			} else {
				printf("]   ");
				printf(language_vars["lng_dl_progress_with_bar_1"].c_str(), dlnow / _1MO, dltotal / _1MO);
			}
		}
		consoleUpdate(&logs_console);
		if (dif >= 1.0f) {
			prevtime = currtime;
			dl_speed = ((dlnow - dlold) / dif) / _1MO;
			// debug_log_write("%.2f\n", dl_speed);
			dlold = dlnow;
		}
	}
	return 0;
}

bool downloadFile(const char *url, const char *output, int api, bool display_log) {
	if (display_log) {
		printf("\n\033[0;32m");
		printf(language_vars["lng_dl_begin"].c_str(), url);
		printf("\033[0;37m\n");
		consoleUpdate(&logs_console);
	}
	debug_log_write("Téléchargement de \"%s\".\n", url);
	if (!internet_is_connected()) {
		if (display_log) {
			printf("\n\033[0;31m");
			printf(language_vars["lng_error_no_internet_connection_for_function"].c_str());
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		}
		debug_log_write("Erreur: Aucune connexion à internet.\n");
		return false;
	}
	first = true;
	dl_speed_displayed = false;
	dl_speed = 0;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	if (curl) {
		FILE *fp = fopen(output, "wb");
		if (fp) {
			if (display_log) {
				printf("\n");
			}

			ntwrk_struct_t chunk = {0};
			chunk.data = (u_int8_t*) malloc(_1MO);
			chunk.data_size = _1MO;
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
						printf(language_vars["lng_dl_file_write_error"].c_str());
						printf("\033[0;37m\n");
						consoleUpdate(&logs_console);
						debug_log_write("Erreur durant l'écriture du fichier.\n\n");
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
					printf(language_vars["lng_dl_success"].c_str());
					printf("\033[0;37m\n\n");
					consoleUpdate(&logs_console);
				}
				debug_log_write("Téléchargement OK.\n\n");
				return true;
			} else {
				if (display_log) {
					printf("\n\n\033[0;31m");
					printf(language_vars["lng_dl_dl_error"].c_str());
					printf("\033[0;37m\n\n");
					consoleUpdate(&logs_console);
				}
				debug_log_write("Erreur de Téléchargement.\n\n");
				return false;
			}
		} else {
			curl_easy_cleanup(curl);
			if (display_log) {
				printf("\n\n\033[0;31m");
				printf(language_vars["lng_dl_open_temp_file_error"].c_str());
				printf("\033[0;37m\n\n");
				consoleUpdate(&logs_console);
			}
			curl_global_cleanup();
			debug_log_write("Erreur d'ouverture du fichier temporaire.\n\n");
			return false;
		}
	} else {
		if (display_log) {
			printf("\n\n\033[0;31m");
			printf(language_vars["lng_dl_curl_init_error"].c_str());
			printf("\033[0;37m\n\n");
			consoleUpdate(&logs_console);
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		debug_log_write("Erreur d'initialisation de Curl.\n\n");
		return false;
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	debug_log_write("Erreur inconnue.\n\n");
	return false;
}

bool downloadInMemory(char *url, MemoryStruct_t *chunk, int api, bool display_log) {
	if (display_log) {
		printf("\n\033[0;32m");
		printf(language_vars["lng_dl_begin"].c_str(), url);
		printf("\033[0;37m\n");
		consoleUpdate(&logs_console);
	}
	debug_log_write("Téléchargement de \"%s\".\n", url);
	if (!internet_is_connected()) {
		if (display_log) {
			printf("\n\033[0;31m");
			printf(language_vars["lng_error_no_internet_connection_for_function"].c_str());
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		}
		debug_log_write("Erreur: Aucune connexion à internet.\n");
		return false;
	}
	first = true;
	dl_speed_displayed = false;
	dl_speed = 0;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	if (curl) {
		if (display_log) {
			printf("\n");
		}

		chunk->memory = (char *) malloc(1);
		chunk->size = 0;

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, API_AGENT);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		// write calls
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback2);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

		if (api == OFF) {
			if (display_log) {
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
			}
		}

		// execute curl, save result
		CURLcode res = curl_easy_perform(curl);

		// Verify if something has been downloaded
		if (chunk->size <= 0) {
			if (display_log) {
				printf("\033[0;31m");
				printf(language_vars["lng_dl_file_write_error"].c_str());
				printf("\033[0;37m\n");
				consoleUpdate(&logs_console);
			}
			debug_log_write("Erreur durant l'écriture du fichier ou durant le téléchargement.\n\n");
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			free(chunk->memory);
			return false;
		}

		if (res == CURLE_OK) {
			if (display_log) {
				printf("\n\n\033[0;32m");
				printf(language_vars["lng_dl_success"].c_str());
				printf("\033[0;37m\n\n");
				consoleUpdate(&logs_console);
			}
			debug_log_write("Téléchargement OK.\n\n");
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return true;
		} else {
			if (display_log) {
				printf("\n\n\033[0;31m");
				printf(language_vars["lng_dl_dl_error"].c_str());
				printf("\033[0;37m\n\n");
				consoleUpdate(&logs_console);
			}
			debug_log_write("Erreur de Téléchargement.\n\n");
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			free(chunk->memory);
			return false;
		}
	} else {
		if (display_log) {
			printf("\n\n\033[0;31m");
			printf(language_vars["lng_dl_curl_init_error"].c_str());
			printf("\033[0;37m\n\n");
			consoleUpdate(&logs_console);
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		debug_log_write("Erreur d'initialisation de Curl.\n\n");
		return false;
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	debug_log_write("Erreur inconnue.\n\n");
	return false;
}

bool downloadDirectoryRecursive(const char *remote_url, const char *local_path, bool display_log) {
	if (display_log) {
		printf("\n\033[0;32m");
		printf(language_vars["lng_dl_directory_begin"].c_str(), remote_url);
		printf("\033[0;37m\n");
		consoleUpdate(&logs_console);
	}

	CURL *curl = curl_easy_init();
	if (!curl) {
		debug_log_write("Erreur d'initialisation de Curl pour le dossier.\n");
		return false;
	}

	char *file_list = (char*) calloc(1, 1);
	if (!file_list) {
		fprintf(stderr, "Échec de l'allocation mémoire.\n");
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, remote_url);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, API_AGENT);
	curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback2);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&file_list);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		if (display_log) {
			printf("\033[0;31mErreur lors de la récupération du dossier %s.\033[0;37m\n", remote_url);
		}
		curl_easy_cleanup(curl);
		free(file_list);
		return false;
	}

	// Création du répertoire local
	mkdir(local_path, 0777);

	// Parcours des fichiers et sous-dossiers
	char *line = strtok(file_list, "\n");
	while (line) {
		char remote_item_url[1024];
		snprintf(remote_item_url, sizeof(remote_item_url), "%s/%s", remote_url, line);

		char local_item_path[1024];
		snprintf(local_item_path, sizeof(local_item_path), "%s/%s", local_path, line);

		if (line[strlen(line) - 1] == '/') {
			downloadDirectoryRecursive(remote_item_url, local_item_path, display_log);
		} else {
			downloadFile(remote_item_url, local_item_path, OFF, display_log);
		}

		line = strtok(NULL, "\n");
	}

	free(file_list);
	curl_easy_cleanup(curl);
	return true;
}