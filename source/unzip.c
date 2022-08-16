#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <switch.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "unzip.h"
// #include "zip.h"

size_t WRITEBUFFERSIZE = 0x100000;

extern PrintConsole logs_console;
extern char firmware_path[FS_MAX_PATH];

bool prefix(const char* pre, const char *str){
	return strncmp(pre, str, strlen(pre)) == 0;
}

bool cp(char *filein, char *fileout) {
	FILE *exein, *exeout;
	exein = fopen(filein, "rb");
	if (exein == NULL) {
		/* handle error */
		perror("file open for reading");
		return false;
	}
	exeout = fopen(fileout, "wb");
	if (exeout == NULL) {
		/* handle error */
		perror("file open for writing");
		return false;
	}
	size_t n, m;
	unsigned char buff[8192];
	do {
		n = fread(buff, 1, sizeof buff, exein);
		if (n) m = fwrite(buff, 1, n, exeout);
		else   m = 0;
	}
	while ((n > 0) && (n == m));
	if (m) {
		perror("copy");
		return false;
	}
	if (fclose(exeout)) {
		perror("close output file");
		return false;
	}
	if (fclose(exein)) {
		perror("close input file");
		return false;
	}
	return true;
}

int check(unsigned const char type) {
	if(type == DT_REG)
		return 1;
	if(type == DT_DIR)
		return 0;
	return -1;
}

int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
	  struct dirent *p;

	  r = 0;
	  while (!r && (p=readdir(d))) {
		  int r2 = -1;
		  char *buf;
		  size_t len;

		  /* Skip the names "." and ".." as we don't want to recurse on them. */
		  if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
			 continue;

		  len = path_len + strlen(p->d_name) + 2; 
		  buf = (char*) malloc(len);

		  if (buf) {
			 struct stat statbuf;

			 snprintf(buf, len, "%s/%s", path, p->d_name);
			 if (!stat(buf, &statbuf)) {
				if (S_ISDIR(statbuf.st_mode))
				   r2 = remove_directory(buf);
				else
				   r2 = unlink(buf);
			 }
			 free(buf);
		  }
		  r = r2;
	  }
	  closedir(d);
   }

   if (!r)
	  r = rmdir(path);

   return r;
}

void fnc_clean_logo(char *atmo_logo_folder, char *hekate_nologo_file_path) {
	printf("Suppression des logos...\n");
	consoleUpdate(&logs_console);
	char base_atmo_logo_folder[FS_MAX_PATH] = "/atmosphere/exefs_patches/";
	remove_directory(strcat(base_atmo_logo_folder, atmo_logo_folder));
	remove("bootloader/bootlogo.bmp");
	remove("bootloader/res/bootlogo.bmp");
	remove("bootloader/res/background.bmp");
	remove("config/nx-hbmenu/settings.cfg");
	cp(hekate_nologo_file_path, "/bootloader/hekate_ipl.ini");
	mkdir("/config", 0777);
	mkdir("/config/nx-hbmenu", 0777);
	mkdir("/config/nx-hbmenu/themes", 0777);
	cp("romfs:/nologo/hbmenu_default_theme.cfg", "/config/nx-hbmenu/themes/theme.cfg");
}

void fnc_clean_theme() {
		// Full theme deletion, helped  with code from nx-theme-installer
		printf("Suppression d'un eventuel theme...\n");
		consoleUpdate(&logs_console);
		remove_directory("atmosphere/contents/0100000000001000");
		remove_directory("atmosphere/contents/0100000000001013");
		remove_directory("atmosphere/contents/0100000000001007"); //Player select
		remove_directory("atmosphere/contents/0100000000000811"); //Custom font
		remove_directory("atmosphere/contents/0100000000000039"); //needed to enable custom font
		// End full theme deletion
}

void clean_sd(bool clean_theme) {
	printf("\033[0;32mNettoyage de la SD:\033[0;37m\n");
	printf("Nettoyage des fichiers inutiles...");
	consoleUpdate(&logs_console);
	DIR *dir = opendir("atmosphere/titles");
	if (dir) {
		closedir(dir);
		
		DIR *dir2 = opendir("atmosphere/contents");
		if (dir2) {
			closedir(dir2);
			remove_directory("atmosphere/titles");
		} else {
			rename("atmosphere/titles", "atmosphere/contents");
		}
	}
	remove_directory("atmosphere/contents/010000000000000D");
	remove_directory("atmosphere/contents/010000000000002B");
	remove_directory("atmosphere/contents/010000000000003C");
	remove_directory("atmosphere/contents/0100000000000008");
	remove_directory("atmosphere/contents/0100000000000032");
	remove_directory("atmosphere/contents/0100000000000034");
	remove_directory("atmosphere/contents/0100000000000036");
	remove_directory("atmosphere/contents/0100000000000037");
	remove_directory("atmosphere/contents/0100000000000042");
	remove("nettoyage_sd.bat");
	remove("atmosphere/BCT.ini");
	remove("atmosphere/loader.ini");
	remove("atmosphere/system_settings.ini");
	remove("atmosphere/hekate_kips/loader.kip");
	remove_directory("sept");
	remove_directory("switch/atmosphere-updater");
	remove_directory("switch/sigpatch-updater");
	remove_directory("switch/sigpatches-updater");
	remove_directory("switch/DeepSea-Toolbox");
	remove("switch/GagOrder.nro");
	remove_directory("atmosphere/exefs_patches/Signature_Patches_by_br4z0rf_and_Jakibaki");
	remove_directory("switch/appstore/res");
	remove("BCT.ini");
	remove("fusee-secondary.bin");
	remove("bootlogo.bmp");
	remove("hekate_ipl.ini");
	remove_directory("switch/CFWSettings");
	remove_directory("switch/CFW-Settings");
	remove("modules/atmosphere/fs_mitm.kip");
	remove_directory("atmosphere/titles/010000000000100D");
	remove("atmosphere/fusee-mtc.bin");
	remove_directory("atmosphere/kip_patches/default_nogc");
	remove("atmosphere/config/BCT.ini");
	remove("atmosphere/config_templates/BCT.ini");
	remove("atmosphere/fusee-secondary.bin");
	remove("atmosphere/flags/clean_stratosphere_for_0.19.0.flag");
	remove("Atmosphere_fusee-primary.bin");
	remove_directory("switch/KosmosToolbox");
	remove_directory("switch/KosmosUpdater");
	remove_directory("switch/HekateToolbox");
	remove("bootloader/hekate_ipl.ini.old");
	remove("switch/DeepSea-Updater/DeepSeaUpdater.nro");
	remove_directory("switch/ChoiDuJourNX");
	remove("switch/ChoiDuJourNX.nro");
	remove_directory("nsp_forwaders");
	remove("readme.html");
	remove("readme.md");
	remove("bootloader/bootlogo.bmp");
	remove("nsp_forwarders/Tinfoil V14.nsp");
	remove_directory("Firmware 14.1.2");
	remove_directory(firmware_path);
	if (clean_theme) {
		fnc_clean_theme();
	}
	printf("\033[0;32mNettoyage de la SD termine.\033[0;37m\n\n");
		consoleUpdate(&logs_console);
}

char * substr(char *s, int x, int y) {
	char * ret = malloc(strlen(s) + 1);
	char * p = ret;
	char * q = &s[x];

	assert(ret != NULL);

	while(x  < y) {
	*p++ = *q++;
		x ++; 
	}

	*p++ = '\0';

	return ret;
}

/*
int unzip2(const char *output, char *subfolder_in_zip) {
	FILE *logfile = fopen("log.txt", "w");
	uLong first_subfolder_passed = 0;
	uLong subfolder_in_zip_length;
	if (strcmp(subfolder_in_zip, "") != 0 && strcmp(subfolder_in_zip, "/") != 0) {
		for(int i = 0; subfolder_in_zip[i] != '\0'; i++) {
			if(subfolder_in_zip[i] == '/') {
				first_subfolder_passed++;
			}
		}
	if ((subfolder_in_zip[strlen(subfolder_in_zip) - 1]) != '/') {
		strcat(subfolder_in_zip, "/");
		first_subfolder_passed++;
	}
	subfolder_in_zip_length = strlen(subfolder_in_zip);
	} else {
		strcpy(subfolder_in_zip, "/");
		subfolder_in_zip_length = 0;
	}
	fprintf(logfile, "%s\n", subfolder_in_zip);
	fprintf(logfile, "%ld\n\n", strlen(subfolder_in_zip));
	bool detected_payload_bin = false;
	char filename_on_sd[FS_MAX_PATH] = {0};
	DIR *dir;
	unsigned char *buf;
	struct zip_t *zip = zip_open(output, 0, 'r');
	int i, n = zip_entries_total(zip);
	for (i = 0; i < n; ++i) {
		zip_entry_openbyindex(zip, i);
			const char *filename_inzip = zip_entry_name(zip);
			int isdir = zip_entry_isdir(zip);
			unsigned long long size = zip_entry_size(zip);
			unsigned int crc32 = zip_entry_crc32(zip);
		// char *c1 = substr(filename_inzip,subfolder_in_zip_length, strlen(filename_inzip));
		// strcpy(filename_on_sd, c1);
		// free(c1);
		int k = 0;
		for (uLong j = subfolder_in_zip_length; j < strlen(filename_inzip); j++) {
			filename_on_sd[k] = filename_inzip[j];
			k++;
		}
		filename_on_sd[k] = '\0';
		fprintf(logfile, "%s\n", filename_on_sd);
		fprintf(logfile, "%s\n", filename_inzip);
		if (first_subfolder_passed > i){
			zip_entry_close(zip);
			continue;
		}
		if (isdir) {
			if ((filename_on_sd[strlen(filename_on_sd) - 1]) == '/') {
				dir = opendir(filename_on_sd);
				if (dir) {
					closedir(dir);
					} else {
						printf("\033[0;34mCreation du repertoir: %s\033[0;37m\n", filename_on_sd);
						mkdir(filename_on_sd, 0777);
						consoleUpdate(&logs_console);
					}
					zip_entry_close(zip);
					continue;
			}
		}
		char outfile[FS_MAX_PATH] = {0};
		FILE *unziped_file;
		if (strcmp(filename_on_sd, "payload.bin") == 0){
			detected_payload_bin = true;
			strcat(strcat(outfile, filename_on_sd), ".temp");

			printf("\033[0;33mExtraction de: %-5s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		} else if (strcmp(filename_on_sd, "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") == 0 || strcmp(filename_on_sd, "atmosphere/package3") == 0 || strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
			strcat(strcat(outfile, filename_on_sd), ".temp");

			printf("\033[0;33mExtraction de: %-5s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		} else {
			strcat(outfile, filename_on_sd);

			printf("\033[0;36mExtraction de: %s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		}
		unziped_file = fopen(outfile, "wb");
		WRITEBUFFERSIZE = zip_entry_size(zip);
		buf = calloc(sizeof(unsigned char), WRITEBUFFERSIZE);
		size_t j = zip_entry_noallocread(zip, buf, WRITEBUFFERSIZE);
			if (j != fwrite(buf, 1, j, unziped_file)) {
				fprintf(logfile, "Erreur durant l'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD.\n", filename_on_sd);
				printf("\033[0;31mErreur durant l'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD.\033[0;37m\n", filename_on_sd);
				consoleUpdate(&logs_console);
				free(buf);
				fclose(unziped_file);
				zip_entry_close(zip);
				zip_close(zip);
				return 1;
			}
		// fprintf(logfile, "%d\n\n", zip_entry_fread(zip, outfile));
		free(buf);
		fclose(unziped_file);
		zip_entry_close(zip);
	}
	zip_close(zip);
	if (detected_payload_bin == false) rename("payload.bin", "payload.bin.temp");
	// remove(output);
	remove("payload.bin");
	cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");

	fclose(logfile);
	return 0;
}
*/

int unzip(const char *output, char *subfolder_in_zip) {
	// FILE *logfile = fopen("log.txt", "w");
	unzFile zfile = unzOpen(output);
	unz_global_info gi = {0};
	unzGetGlobalInfo(zfile, &gi);
	uLong first_subfolder_passed = 0;
	uLong subfolder_in_zip_length;
	if (strcmp(subfolder_in_zip, "") != 0 && strcmp(subfolder_in_zip, "/") != 0) {
		for(int i = 0; subfolder_in_zip[i] != '\0'; i++) {
			if(subfolder_in_zip[i] == '/') {
				first_subfolder_passed++;
			}
		}
	if ((subfolder_in_zip[strlen(subfolder_in_zip) - 1]) != '/') {
		strcat(subfolder_in_zip, "/");
		first_subfolder_passed++;
	}
	subfolder_in_zip_length = strlen(subfolder_in_zip);
	} else {
		strcpy(subfolder_in_zip, "/");
		subfolder_in_zip_length = 0;
	}
	// fprintf(logfile, "%s\n", subfolder_in_zip);
	// fprintf(logfile, "%ld\n\n", strlen(subfolder_in_zip));
	bool detected_payload_bin = false;
	char filename_inzip[FS_MAX_PATH] = {0};
	char filename_on_sd[FS_MAX_PATH] = {0};
	unz_file_info file_info = {0};
	DIR *dir;
	FILE *outfile;
	void *buf;

	for (uLong i = 0; i < gi.number_entry; i++) {
		unzOpenCurrentFile(zfile);
		unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

		// fprintf(logfile, "%s\n", filename_inzip);
		char *c1 = substr(filename_inzip,subfolder_in_zip_length, strlen(filename_inzip));
		strcpy(filename_on_sd, c1);
		free(c1);
		/*
		int k = 0;
		for (uLong j = subfolder_in_zip_length; j < strlen(filename_inzip); j++) {
			filename_on_sd[k] = filename_inzip[j];
			k++;
		}
		filename_on_sd[k] = '\0';
		*/
		// fprintf(logfile, "%s\n", filename_on_sd);
		// fprintf(logfile, "%s\n\n", filename_inzip);
		if (first_subfolder_passed > i){
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;
		// check if the string ends with a /, if so, then its a directory.
		} else if ((filename_on_sd[strlen(filename_on_sd) - 1]) == '/') {
			// check if directory exists
			dir = opendir(filename_on_sd);
			if (dir) {
				closedir(dir);
			} else {
				printf("\033[0;34mCreation du repertoir: %s\033[0;37m\n", filename_on_sd);
				mkdir(filename_on_sd, 0777);
				consoleUpdate(&logs_console);
			}
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;

		} else if (strcmp(filename_on_sd, "payload.bin") == 0){
			detected_payload_bin = true;
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33mExtraction de: %-5s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		} else if (strcmp(filename_on_sd, "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") == 0 || strcmp(filename_on_sd, "atmosphere/package3") == 0 || strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33mExtraction de: %-5s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		} else {
			outfile = fopen(filename_on_sd, "wb");

			printf("\033[0;36mExtraction de: %s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		}
		buf = malloc(WRITEBUFFERSIZE);
		for (size_t j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE)) {
			if (j != fwrite(buf, 1, j, outfile)) {
				// fprintf(logfile, "Erreur durant l'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD.\n", filename_on_sd);
				printf("\033[0;31mErreur durant l'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD.\033[0;37m\n", filename_on_sd);
				consoleUpdate(&logs_console);
				fclose(outfile);
				unzCloseCurrentFile(zfile);
				free(buf);
				unzClose(zfile);
				return 1;
			}
		}

		fclose(outfile);
		free(buf);
		unzCloseCurrentFile(zfile);
		unzGoToNextFile(zfile);
	}

	unzClose(zfile);
	if (detected_payload_bin == false) rename("payload.bin", "payload.bin.temp");
	// remove(output);
	remove("payload.bin");
	cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");

	// fclose(logfile);
	return 0;
}
