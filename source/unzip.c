#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "unzip.h"

#define WRITEBUFFERSIZE 0x100000
#define MAXFILENAME	 0x301

extern PrintConsole logs_console;

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

void clean_sd() {
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
	// Full theme deletion, helped  with code from nx-theme-installer
	printf("Suppression d'un eventuel theme...\n");
	consoleUpdate(&logs_console);
	remove_directory("atmosphere/contents/0100000000001000");
	remove_directory("atmosphere/contents/0100000000001013");
	remove_directory("atmosphere/contents/0100000000001007"); //Player select
	remove_directory("atmosphere/contents/0100000000000811"); //Custom font
	remove_directory("atmosphere/contents/0100000000000039"); //needed to enable custom font
	// End full theme deletion
	printf("\033[0;32mNettoyage de la SD termine.\033[0;37m\n\n");
		consoleUpdate(&logs_console);
}

int unzip(const char *output)
{
	// FILE *logfile = fopen("log.txt", "w");
	extern char subfolder_in_zip[1003];
	char project_subfolder_in_zip[strlen(subfolder_in_zip) + 2];
	strcpy(project_subfolder_in_zip, subfolder_in_zip);
	unzFile zfile = unzOpen(output);
	unz_global_info gi = {0};
	unzGetGlobalInfo(zfile, &gi);
	uLong first_subfolder_passed = 0;
	if (strcmp(project_subfolder_in_zip, "") != 0) {
		for(int i = 0; project_subfolder_in_zip[i] != '\0'; i++) {
			if(project_subfolder_in_zip[i] == '/') {
				first_subfolder_passed++;
			}
		}
	if ((project_subfolder_in_zip[strlen(project_subfolder_in_zip) - 1]) != '/') {
		strcat(project_subfolder_in_zip, "/");
		first_subfolder_passed++;
	}
	}
	// fputs(strcat(project_subfolder_in_zip, "\n"), logfile);
bool detected_payload_bin = false;

	for (uLong i = 0; i < gi.number_entry; i++)
	{
		char filename_inzip[MAXFILENAME] = {0};
		unz_file_info file_info = {0};
		unzOpenCurrentFile(zfile);
		unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

		FILE *outfile;
		void *buf;

		char filename_on_sd[MAXFILENAME];
		int k = 0;
		for (uLong j = strlen(project_subfolder_in_zip); j < strlen(filename_inzip); j++)
		 {
			filename_on_sd[k] = filename_inzip[j];
			k++;
		}
		filename_on_sd[k] = '\0';
		// fputs (filename_on_sd, logfile);
		// fputs ("\n", logfile);
		if (first_subfolder_passed > i){
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;
		}
		// check if the string ends with a /, if so, then its a directory.
		else if ((filename_inzip[strlen(filename_inzip) - 1]) == '/')
		{
			// check if directory exists
			DIR *dir = opendir(filename_on_sd);
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
		}	

		else if (strcmp(filename_on_sd, "payload.bin") == 0){
			detected_payload_bin = true;
			outfile = fopen("payload.bin.temp", "wb");

			printf ("\033[0;33mDANS payload.bin! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
			consoleUpdate(&logs_console);
			sleep(2);
		} else if (strcmp(filename_on_sd, "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") == 0){
			outfile = fopen("switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro.temp", "wb");

			printf ("\033[0;33mDANS AIO_LS_pack_Updater.nro! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
			consoleUpdate(&logs_console);
			sleep(2);
		} else if (strcmp(filename_on_sd, "atmosphere/package3") == 0){
			outfile = fopen("atmosphere/package3.temp", "wb");

			printf ("\033[0;33mDANS PACKAGE3! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
			consoleUpdate(&logs_console);
			sleep(2);
		} else if (strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
			outfile = fopen("atmosphere/stratosphere.romfs.temp", "wb");

			printf ("\033[0;33mDANS STRATOSPHERE.ROMFS! NE PAS ETEINDRE LA CONSOLE!\033[0;37m\n");
			consoleUpdate(&logs_console);
			sleep(2);
		} else {
			outfile = fopen(filename_on_sd, "wb");

			printf("\033[0;36mExtraction de: %s\033[0;37m\n", filename_on_sd);
			consoleUpdate(&logs_console);
		}
		buf = malloc(WRITEBUFFERSIZE);
		for (size_t j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE)) {
			if (j != fwrite(buf, 1, j, outfile)) {
				printf("\033[0;31mErreur durant l'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD.\033[0;37m\n", filename_on_sd);
				consoleUpdate(&logs_console);
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
	printf("\033[0;32m\nFinis!\n\nRedemarage automatique dans 5 secondes :)\033[0;37m\n");
	remove(output);
	remove("payload.bin");
	cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");
	consoleUpdate(&logs_console);

	sleep(5);
	// fclose(logfile);
	return 0;
}
