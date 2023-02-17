#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main_util.h"
#include "unzip.hpp"
#include "translate.hpp"
// #include "zip.h"

size_t WRITEBUFFERSIZE = 0x100000;

extern translation_map language_vars;
extern PrintConsole logs_console;
extern char firmware_path[FS_MAX_PATH];
extern bool debug_enabled;

bool prefix(const char* pre, const char *str){
	return strncmp(pre, str, strlen(pre)) == 0;
}

int check(unsigned const char type) {
	if(type == DT_REG)
		return 1;
	if(type == DT_DIR)
		return 0;
	return -1;
}

bool file_in_files_to_keep(char *file_to_test) {
	FILE *files_list_file = fopen("/switch/AIO_LS_pack_Updater/files_to_keep.txt", "r");
	if (files_list_file != NULL) {
		char chaine[FS_MAX_PATH+1] = "";
		while (fgets(chaine, FS_MAX_PATH+1, files_list_file) != NULL) {
			if (strcmp(chaine, "") != 0 && strcmp(chaine, "\n") != 0) {
				char *c1 = substr(chaine, 0, strlen(chaine)-2);
				if (strlen(c1) > strlen(file_to_test)) {
					free(c1);
					continue;
				}
				char *c2 = substr(file_to_test, 0, strlen(c1));
				// debug_log_write("%s; %s\n", c1, c2);
				// debug_log_write("%i\n\n", strcmp(c1, c2));
				if (strcmp(c1, c2) == 0) {
					free(c2);
					free(c1);
					fclose(files_list_file);
					return true;
				}
				free(c2);
				free(c1);
			}
		}
		fclose(files_list_file);
		return false;
	} else {
		const char* default_files_protect_list[] =
		{
			"atmosphere/config",
			"bootloader/hekate_ipl.ini",
			"bootloader/nyx.ini",
			"exosphere.ini",
			"config",
			"boot.ini",
			"switch/DBI/dbi.config"
		};
		for (size_t i=0; i<sizeof(default_files_protect_list)/sizeof(default_files_protect_list[0]); i++) {
			if (strlen(default_files_protect_list[i]) > strlen(file_to_test)) {
				continue;
			}
			char *c3 = substr(file_to_test, 0, strlen(default_files_protect_list[i]));
			if (strcmp(default_files_protect_list[i], c3) == 0) {
				free(c3);
				return true;
			}
			free(c3);
		}
		return false;
	}
	return false;
}

void fnc_clean_logo(char *atmo_logo_folder, char *hekate_nologo_file_path) {
	if (debug_enabled) {
		debug_log_write("Suppression des logos.\n");
	}
	printf(language_vars["lng_clean_logos_begin"]);
	printf("\n");
	consoleUpdate(&logs_console);
	char base_atmo_logo_folder[FS_MAX_PATH] = "/atmosphere/exefs_patches/";
	if (strcmp(atmo_logo_folder, "") != 0 && strcmp(atmo_logo_folder, "/") != 0 && atmo_logo_folder[0] != '/') {
		remove_directory(strcat(base_atmo_logo_folder, atmo_logo_folder));
	}
	remove("bootloader/bootlogo.bmp");
	remove("bootloader/res/bootlogo.bmp");
	remove("bootloader/res/background.bmp");
	remove("config/nx-hbmenu/settings.cfg");
	if (strcmp(hekate_nologo_file_path, "") != 0 && strcmp(hekate_nologo_file_path, "/") != 0) {
		custom_cp(hekate_nologo_file_path, (char*) "/bootloader/hekate_ipl.ini");
	}
	mkdir("/config", 0777);
	mkdir("/config/nx-hbmenu", 0777);
	mkdir("/config/nx-hbmenu/themes", 0777);
	custom_cp((char*) "romfs:/nologo/hbmenu_default_theme.cfg", (char*) "/config/nx-hbmenu/themes/theme.cfg");
}

void fnc_clean_theme() {
	if (debug_enabled) {
		debug_log_write("Suppression du thème.\n");
	}
	// Full theme deletion, helped  with code from nx-theme-installer
	printf(language_vars["lng_clean_theme_begin"]);
	printf("\n");
	consoleUpdate(&logs_console);
	remove_directory("atmosphere/contents/0100000000001000");
	remove_directory("atmosphere/contents/0100000000001013");
	remove_directory("atmosphere/contents/0100000000001007"); //Player select
	remove_directory("atmosphere/contents/0100000000000811"); //Custom font
	remove_directory("atmosphere/contents/0100000000000039"); //needed to enable custom font
	// End full theme deletion
}

void fnc_agressive_clean() {
	if (debug_enabled) {
		debug_log_write("Nettoyage agressif.\n");
	}
	printf(language_vars["lng_agressive_clean_begin"]);
	printf("\n");
	consoleUpdate(&logs_console);
	FILE *folders_list_file = fopen("/switch/AIO_LS_pack_Updater/folders_to_delete.txt", "r");
	FILE *files_list_file = fopen("/switch/AIO_LS_pack_Updater/files_to_delete.txt", "r");
	if (folders_list_file != NULL) {
		if (debug_enabled) {
			debug_log_write("Suppression des répertoires  via un fichier personnalisé.\n");
		}
		char chaine[FS_MAX_PATH+1] = "";
		while (fgets(chaine, FS_MAX_PATH+1, folders_list_file) != NULL) {
			if (strcmp(chaine, "") != 0 && strcmp(chaine, "\n") != 0) {
				char *c1 = substr(chaine, 0, strlen(chaine)-2);
				remove_directory(c1);
				free(c1);
			}
		}
		fclose(folders_list_file);
	}
	if (files_list_file != NULL) {
		if (debug_enabled) {
			debug_log_write("Suppression des fichiers  via un fichier personnalisé.\n");
		}
		char chaine[FS_MAX_PATH+1] = "";
		while (fgets(chaine, FS_MAX_PATH+1, files_list_file) != NULL) {
			if (strcmp(chaine, "") != 0 && strcmp(chaine, "\n") != 0) {
				char *c1 = substr(chaine, 0, strlen(chaine)-2);
				remove(c1);
				if (debug_enabled) {
					debug_log_write("Suppression du fichier  \"%s\".\n", c1);
				}
				free(c1);
			}
		}
		fclose(files_list_file);
	}
	if (folders_list_file == NULL && files_list_file == NULL) {
		if (debug_enabled) {
			debug_log_write("Nettoyage agressif intégré.\n");
		}
		remove_directory("atmosphere/contents/00FF0000000002AA");
		remove_directory("atmosphere/contents/054e4f4558454000");
		remove_directory("atmosphere/contents/010000000000000D");
		remove_directory("atmosphere/contents/0100000000001013");
		remove_directory("atmosphere/contents/0100000000000352");
		remove_directory("atmosphere/contents/0100000000000F12");
		remove_directory("atmosphere/contents/010000000000CF12");
		remove_directory("atmosphere/contents/010000000000C235");
		remove_directory("atmosphere/contents/0100000000000faf");
		remove_directory("atmosphere/contents/4200000000000010");
		remove_directory("atmosphere/contents/010000000000bd00");
		remove_directory("atmosphere/exefs_patches/bluetooth_patches");
		remove_directory("atmosphere/contents/0100000000000081");
		remove_directory("atmosphere/contents/0100000000000901");
		remove_directory("atmosphere/contents/0000000000534C56");
		remove_directory("atmosphere/contents/420000000000000F");
		remove_directory("atmosphere/contents/0100000000000464");
		remove_directory("atmosphere/contents/430000000000000B");
		remove_directory("atmosphere/contents/00FF0000636C6BFF");
		remove_directory("atmosphere/contents/690000000000000D");
		remove_directory("atmosphere/contents/420000000000000E");
		remove_directory("atmosphere/contents/43000000000000ff");
		remove_directory("atmosphere/contents/4200000000000000");
		remove_directory("atmosphere/contents/4200000000000FFF");
		remove_directory("atmosphere/contents/0100000000001000");
	}
}

void clean_sd(bool clean_theme, bool agressive_clean) {
	if (debug_enabled) {
		debug_log_write("Nettoyage de la SD.\n");
	}
	printf("\033[0;32m");
	printf(language_vars["lng_clean_sd_begin"]);
	printf("\033[0;37m\n");
	printf(language_vars["lng_clean_sd_clean_unusful_files"]);
	printf("\n");
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
	if (agressive_clean) {
		fnc_agressive_clean();
	}
	printf("\033[0;32m");
	printf(language_vars["lng_clean_sd_finish"]);
	printf("\033[0;37m\n\n");
		consoleUpdate(&logs_console);
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
	custom_cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");

	fclose(logfile);
	return 0;
}
*/

int unzip(const char *output, char *subfolder_in_zip, bool keep_files) {
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
	// debug_log_write("%s\n", subfolder_in_zip);
	// debug_log_write("%ld\n\n", strlen(subfolder_in_zip));
	bool detected_payload_bin = false;
	char filename_inzip[FS_MAX_PATH] = {0};
	char filename_on_sd[FS_MAX_PATH] = {0};
	unz_file_info file_info = {0};
	DIR *dir;
	FILE *outfile;
	void *buf;
	FsFileSystem *fs_sd = fsdevGetDeviceFileSystem("sdmc");

	for (uLong i = 0; i < gi.number_entry; i++) {
		unzOpenCurrentFile(zfile);
		unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

		// debug_log_write("%s\n", filename_inzip);
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
		// debug_log_write("%s\n", filename_on_sd);
		// debug_log_write("%s\n\n", filename_inzip);
		if (first_subfolder_passed > i){
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;
		}
		if (keep_files) {
			bool test_if_file_exist = false;
			/*
			dir = opendir(filename_on_sd);
			if (dir) {
				closedir(dir);
				test_if_file_exist = true;
			}
			outfile = fopen(filename_on_sd, "rb");
			if (outfile != NULL) {
				fclose(outfile);
				test_if_file_exist = true;
			}
			*/
			FsDirEntryType test_file_or_dir = (FsDirEntryType) -1;
			char full_path_on_sd[FS_MAX_PATH+2] = "/";
			strcat(full_path_on_sd, filename_on_sd);
			fsFsGetEntryType(fs_sd, full_path_on_sd, &test_file_or_dir);
			if (test_file_or_dir == 0 || test_file_or_dir == 1) {
				test_if_file_exist = true;
			}
			if (file_in_files_to_keep(filename_on_sd) && test_if_file_exist) {
				if (debug_enabled) {
					debug_log_write("fichier ou dossier \"%s\" non copié car il est défini comme devant être conservé.\n", filename_on_sd);
				}
				printf("\033[0;35m");
				printf(language_vars["lng_install_pack_file_skip"], filename_on_sd);
				printf("\033[0;37m\n");
				unzCloseCurrentFile(zfile);
				unzGoToNextFile(zfile);
				continue;
			}
		}
		// check if the string ends with a /, if so, then its a directory.
		if ((filename_on_sd[strlen(filename_on_sd) - 1]) == '/') {
			// check if directory exists
			dir = opendir(filename_on_sd);
			if (dir) {
				closedir(dir);
			} else {
				outfile = fopen(filename_on_sd, "rb");
				if (outfile != NULL) {
					fclose(outfile);
					remove(filename_on_sd);
				}
				printf("\033[0;34m");
				printf(language_vars["lng_install_pack_folder_create"], filename_on_sd);
				printf("\033[0;37m\n");
				mkdir(filename_on_sd, 0777);
				if (debug_enabled) {
					debug_log_write("Création du répertoire \"%s\".\n", filename_on_sd);
				}
				consoleUpdate(&logs_console);
			}
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;
		} else {
			dir = opendir(filename_on_sd);
			if (dir) {
				closedir(dir);
				remove_directory(filename_on_sd);
			}
		}

		if (strcmp(filename_on_sd, "payload.bin") == 0){
			detected_payload_bin = true;
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33m");
			printf(language_vars["lng_install_pack_extract_file_protected"], filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		} else if (strcmp(filename_on_sd, "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") == 0 || strcmp(filename_on_sd, "atmosphere/package3") == 0 || strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33m");
			printf(language_vars["lng_install_pack_extract_file_protected"], filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		} else {
			outfile = fopen(filename_on_sd, "wb");

			printf("\033[0;36m");
			printf(language_vars["lng_install_pack_extract_file"], filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		}
		buf = malloc(WRITEBUFFERSIZE);
		for (size_t j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE)) {
			if (j != fwrite(buf, 1, j, outfile)) {
				if (debug_enabled) {
					debug_log_write("Erreur durant l'ecriture du fichier \"%s\".\n", filename_on_sd);
				}
				printf("\033[0;31m");
				printf(language_vars["lng_install_pack_file_write_error"], filename_on_sd);
				printf("\033[0;37m\n");
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
		if (debug_enabled) {
			debug_log_write("Ecriture du fichier \"%s\" OK.\n", filename_on_sd);
		}
	}

	unzClose(zfile);
	if (detected_payload_bin == false) rename("payload.bin", "payload.bin.temp");
	// remove(output);
	remove("payload.bin");
	custom_cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");

	return 0;
}
