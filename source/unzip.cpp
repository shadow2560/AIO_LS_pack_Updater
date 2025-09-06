#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <sys/types.h>
#include <sys/stat.h>

// #include <iostream>
// #include <fstream>

#include "download.hpp"
#include "main_util.h"
#include "unzip.hpp"
#include "translate.hpp"
// #include "json.hpp"
// #include "zip.h"

// size_t WRITEBUFFERSIZE = 0x100000;

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
			"atmosphere/config/",
			"bootloader/hekate_ipl.ini",
			"bootloader/nyx.ini",
			"exosphere.ini",
			"config/",
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

bool module_in_modules_to_keep(char *file_to_test) {
	FILE *files_list_file = fopen("/switch/AIO_LS_pack_Updater/modules_to_keep.txt", "r");
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
			"420000000007E51A",
			"420000000000000B"
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

void fnc_clean_modules() {
	debug_log_write("Suppression des modules.\n");
	printf(language_vars["lng_clean_modules_begin"].c_str());
	printf("\n");
	remove_directory("/atmosphere/kips");
	mkdir("/atmosphere/kips", 0777);
	DIR *contents_dir = opendir("atmosphere/contents");
	if (contents_dir != NULL) {
		// auto contents_json = nlohmann::ordered_json::array();
		struct dirent *ent;
		while ((ent = readdir(contents_dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}
			char temp_exefs_path[50] = "";
			strcat(strcat(strcat(temp_exefs_path, "atmosphere/contents/"), ent->d_name), "/exefs.nsp");
			char temp_module_path[40] = "";
			strcat(strcat(temp_module_path, "atmosphere/contents/"), ent->d_name);
			u64 module_id = 0;
			FILE* f=fopen(temp_exefs_path, "r");
			if (f != NULL) {
				fclose(f);
				if (module_in_modules_to_keep(ent->d_name)) {
					debug_log_write("Suppression du module \"%s\" non effectué car il est défini dans la liste des modules à ne pas supprimer.\n", ent->d_name);
					continue;
				}
				module_id = strtoul(ent->d_name, NULL, 16);
				close_module(module_id);
				remove_directory(temp_module_path);
				if (strcmp(ent->d_name, "010000000000bd00") == 0) {
					remove_directory("atmosphere/exefs_patches/bluetooth_patches");
				}
				continue;
			}
			/*
			std::ifstream sysconfig("atmosphere/contents/" + std::string(ent->d_name) + "/toolbox.json");
			if (!sysconfig.fail()) {
				auto data = nlohmann::ordered_json::parse(sysconfig);
				contents_json.push_back(data);
			}
			*/
		}
		closedir(contents_dir);
		/*
		if (contents_json.size()) {
			for (auto i : contents_json) {
				std::string path = "atmosphere/contents/" + i["tid"].get<std::string>();
				remove_directory(path.c_str());
			}
		}
		*/
	}
}

void fnc_clean_logo(char *atmo_logo_folder, char *hekate_nologo_file_path) {
	debug_log_write("Suppression des logos.\n");
	printf(language_vars["lng_clean_logos_begin"].c_str());
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
	debug_log_write("Suppression du thème.\n");
	// Full theme deletion, helped  with code from nx-theme-installer
	printf(language_vars["lng_clean_theme_begin"].c_str());
	printf("\n");
	consoleUpdate(&logs_console);
	remove_directory("atmosphere/contents/0100000000001000");
	remove_directory("atmosphere/contents/0100000000001013");
	remove_directory("atmosphere/contents/0100000000001007"); //Player select
	remove_directory("atmosphere/contents/0100000000000811"); //Custom font
	remove_directory("atmosphere/contents/0100000000000039"); //needed to enable custom font
	// End full theme deletion
}

void fnc_agressive_clean(bool fw_install_only) {
	debug_log_write("Nettoyage agressif.\n");
	printf(language_vars["lng_agressive_clean_begin"].c_str());
	printf("\n");
	consoleUpdate(&logs_console);
	FILE *folders_list_file;
	FILE *files_list_file;
	if (!fw_install_only) {
		folders_list_file = fopen("/switch/AIO_LS_pack_Updater/pack_install_folders_to_delete.txt", "r");
		files_list_file = fopen("/switch/AIO_LS_pack_Updater/pack_install_files_to_delete.txt", "r");
	} else {
		folders_list_file = fopen("/switch/AIO_LS_pack_Updater/fw_install_folders_to_delete.txt", "r");
		files_list_file = fopen("/switch/AIO_LS_pack_Updater/fw_install_files_to_delete.txt", "r");
	}
	if (folders_list_file != NULL) {
		debug_log_write("Suppression des répertoires  via un fichier personnalisé.\n");
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
		debug_log_write("Suppression des fichiers  via un fichier personnalisé.\n");
		char chaine[FS_MAX_PATH+1] = "";
		while (fgets(chaine, FS_MAX_PATH+1, files_list_file) != NULL) {
			if (strcmp(chaine, "") != 0 && strcmp(chaine, "\n") != 0) {
				char *c1 = substr(chaine, 0, strlen(chaine)-2);
				remove(c1);
				debug_log_write("Suppression du fichier  \"%s\".\n", c1);
				free(c1);
			}
		}
		fclose(files_list_file);
	}
	if (folders_list_file == NULL && files_list_file == NULL) {
		debug_log_write("Nettoyage agressif intégré.\n");
		if (!fw_install_only) {
			sleep(1);
		} else {
			sleep(1);
		}
	}
}

void clean_sd(bool clean_theme, bool agressive_clean) {
	debug_log_write("Nettoyage de la SD.\n");
	printf("\033[0;32m");
	printf(language_vars["lng_clean_sd_begin"].c_str());
	printf("\033[0;37m\n");
	printf(language_vars["lng_clean_sd_clean_unusful_files"].c_str());
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
	// remove("config/sys-patch/config.ini");
	if (!beta_mode) {
		remove_directory(firmware_path);
	} else {
		remove_directory(firmware_path_beta);
	}
	if (clean_theme) {
		fnc_clean_theme();
	}
	if (agressive_clean) {
		fnc_agressive_clean(false);
	}
	printf("\033[0;32m");
	printf(language_vars["lng_clean_sd_finish"].c_str());
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
		size_t WRITEBUFFERSIZE = zip_entry_size(zip);
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
	unzFile zfile;
	zfile = unzOpen64(output);
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
	// char dirname_on_sd[FS_MAX_PATH] = {0};
	char filename_on_sd[FS_MAX_PATH] = {0};
	unz_file_info file_info = {0};
	DIR *dir;
	FILE *outfile;
	// void *buf;
	// void* temp_file;
	char sha256_in_test[65];
	char sha256_out_test[65];
	char *c1;
	char full_path_on_sd[FS_MAX_PATH+2];
	FsFileSystem *fs_sd = fsdevGetDeviceFileSystem("sdmc");
	char* atmo_bootlogo_dir;
	if (!beta_mode) {
		atmo_bootlogo_dir = (char*) malloc((strlen(atmo_logo_dir) * sizeof(char)) + (40 * sizeof(char)));
		strcpy(atmo_bootlogo_dir, "");
		if ((atmo_logo_dir[strlen(atmo_logo_dir)-1]) != '/') {
			strcat(strcat(strcat(atmo_bootlogo_dir, "atmosphere/exefs_patches/"), atmo_logo_dir), "/");
		} else {
			strcat(strcat(atmo_bootlogo_dir, "atmosphere/exefs_patches/"), atmo_logo_dir);
		}
	} else {
		atmo_bootlogo_dir = (char*) malloc((strlen(atmo_logo_dir_beta) * sizeof(char)) + (40 * sizeof(char)));
		strcpy(atmo_bootlogo_dir, "");
		if ((atmo_logo_dir_beta[strlen(atmo_logo_dir_beta)-1]) != '/') {
			strcat(strcat(strcat(atmo_bootlogo_dir, "atmosphere/exefs_patches/"), atmo_logo_dir_beta), "/");
		} else {
			strcat(strcat(atmo_bootlogo_dir, "atmosphere/exefs_patches/"), atmo_logo_dir_beta);
		}
	}
	short copy_retry=3;

	for (uLong i = 0; i < gi.number_entry; i++) {
		unzOpenCurrentFile(zfile);
		unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		size_t WRITEBUFFERSIZE = file_info.uncompressed_size;

		// debug_log_write("%s\n", filename_inzip);
		c1 = substr(filename_inzip,subfolder_in_zip_length, strlen(filename_inzip));
		strcpy(filename_on_sd, c1);
		free(c1);
		/*
		uLong l = strlen(filename_on_sd)-1;
		while(1) {
			if (l < 0) {
				break;
			}
			if (filename_on_sd[l] == '/') {
				break;
			}
			l--;
		}
		if (l < 0) {
			memset(dirname_on_sd, '\0', sizeof(dirname_on_sd));
		} else if (filename_on_sd[0] != '/') {
			c1 = substr(filename_on_sd, 0, l);
			strcpy(dirname_on_sd, c1);
			free(c1);
		} else {
			c1 = substr(filename_on_sd, 1, l);
			strcpy(dirname_on_sd, c1);
			free(c1);
		}
		// debug_log_write("%s\n", dirname_on_sd);
		*/
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
			strcpy(full_path_on_sd, "/");
			strcat(full_path_on_sd, filename_on_sd);
			fsFsGetEntryType(fs_sd, full_path_on_sd, &test_file_or_dir);
			if (test_file_or_dir == 0 || test_file_or_dir == 1) {
				test_if_file_exist = true;
			}
			if (file_in_files_to_keep(filename_on_sd) && test_if_file_exist) {
				debug_log_write("fichier ou dossier \"%s\" non copié car il est défini comme devant être conservé.\n", filename_on_sd);
				printf("\033[0;35m");
				printf(language_vars["lng_install_pack_file_skip"].c_str(), filename_on_sd);
				printf("\033[0;37m\n");
				unzCloseCurrentFile(zfile);
				unzGoToNextFile(zfile);
				continue;
			}
		}
		if (strcmp(filename_on_sd, "") == 0) {
			debug_log_write("Problème de copie, ce dossier ou fichier n'a pas de nom et ne doit donc pas être copié.\n");
			unzCloseCurrentFile(zfile);
			unzGoToNextFile(zfile);
			continue;
		}
		// check if the string ends with a /, if so, then its a directory.
		if ((filename_on_sd[strlen(filename_on_sd) - 1]) == '/') {
			// check if directory exists
			dir = opendir(filename_on_sd);
			if (dir) {
				closedir(dir);
				if (strcmp(filename_on_sd, atmo_bootlogo_dir) == 0) {
					printf("\033[0;34m");
					printf(language_vars["lng_install_pack_folder_create"].c_str(), filename_on_sd);
					printf("\033[0;37m\n");
					remove_directory(filename_on_sd);
					mkdir(filename_on_sd, 0777);
					debug_log_write("remplacement  du répertoire du bootlogo d'Atmmosphere \"%s\".\n", filename_on_sd);
					unzCloseCurrentFile(zfile);
					unzGoToNextFile(zfile);
					continue;
				}
			} else {
				outfile = fopen(filename_on_sd, "rb");
				if (outfile != NULL) {
					fclose(outfile);
					remove(filename_on_sd);
				}
				printf("\033[0;34m");
				printf(language_vars["lng_install_pack_folder_create"].c_str(), filename_on_sd);
				printf("\033[0;37m\n");
				mkdir(filename_on_sd, 0777);
				debug_log_write("Création du répertoire \"%s\".\n", filename_on_sd);
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
			if ((!beta_mode && pack_files_in_zip_sha256_verify_before_copy_param == 1) || (beta_mode && pack_files_in_zip_sha256_verify_before_copy_param_beta == 1)) {
				memset(sha256_in_test, '\0', sizeof(sha256_in_test));
				memset(sha256_out_test, '\0', sizeof(sha256_out_test));
				outfile = fopen(filename_on_sd, "rb");
				if (outfile != NULL) {
					fclose(outfile);
					// temp_file = malloc(file_info.uncompressed_size + 1);
					// unzReadCurrentFile(zfile, temp_file, file_info.uncompressed_size);
					// get_sha256_data(temp_file, file_info.uncompressed_size, sha256_in_test);
					// free(temp_file);
					get_sha256_data_for_minizip_opened_file(&zfile, WRITEBUFFERSIZE, sha256_in_test);
					get_sha256_file(filename_on_sd, sha256_out_test);
					// debug_log_write("%s - %s\n", sha256_in_test, sha256_out_test);
					if (strcmp(sha256_in_test, sha256_out_test) == 0) {
						debug_log_write("Le fichier \"%s\" est le même dans le zip et sur la SD, la copie n'est pas nécessaire.\n", filename_on_sd);
						printf("\033[0;32m");
						printf(language_vars["lng_install_pack_same_files"].c_str(), filename_on_sd);
						printf("\033[0;37m\n");
						consoleUpdate(&logs_console);
						unzCloseCurrentFile(zfile);
						unzGoToNextFile(zfile);
						continue;
					} else {
						unzCloseCurrentFile(zfile);
						unzOpenCurrentFile(zfile);
					}
				}
			}
		}

		if (strcmp(filename_on_sd, "payload.bin") == 0){
			detected_payload_bin = true;
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33m");
			printf(language_vars["lng_install_pack_extract_file_protected"].c_str(), filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		} else if (strcmp(filename_on_sd, "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro") == 0 || strcmp(filename_on_sd, "atmosphere/package3") == 0 || strcmp(filename_on_sd, "atmosphere/stratosphere.romfs") == 0){
			outfile = fopen(strcat(filename_on_sd, ".temp"), "wb");

			printf("\033[0;33m");
			printf(language_vars["lng_install_pack_extract_file_protected"].c_str(), filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		} else {
			outfile = fopen(filename_on_sd, "wb");

			printf("\033[0;36m");
			printf(language_vars["lng_install_pack_extract_file"].c_str(), filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
		}

		const size_t CHUNK = 512*1024;
		void *buf = malloc(CHUNK);
		if (!buf) {
			debug_log_write("Echec Malloc chunk\n");
			printf("\033[0;31m");
			printf(language_vars["lng_install_pack_file_write_error"].c_str(), filename_on_sd);
			printf("\033[0;37m\n");
			fclose(outfile);
			unzCloseCurrentFile(zfile);
			unzClose(zfile);
			return 1;
		}
		bool retry_copy = false;
		int r;
		while ((r = unzReadCurrentFile(zfile, buf, CHUNK)) > 0) {
			if ((size_t)r != fwrite(buf, 1, r, outfile)) {
				// debug_log_write("Erreur durant fwrite pour le fichier %s\n", filename_on_sd);
				free(buf);
				fclose(outfile);
				unzCloseCurrentFile(zfile);
				retry_copy = true;
			}
		}
		if (r < 0) {
			debug_log_write("Erreur durant unzReadCurrentFile (%d) pour le fichier %s\n", r, filename_on_sd);
			free(buf);
			fclose(outfile);
			unzCloseCurrentFile(zfile);
			retry_copy = true;
		}
		if (retry_copy) {
			copy_retry--;
			if (copy_retry > 0) {
				debug_log_write("Tentative de réécriture du fichier...\n");
				printf("\033[0;31m");
				printf(language_vars["lng_install_pack_file_write__retrying_error"].c_str(), filename_on_sd);
				printf("\033[0;37m\n");
				consoleUpdate(&logs_console);
				continue;
			}
			printf("\033[0;31m");
			printf(language_vars["lng_install_pack_file_write_error"].c_str(), filename_on_sd);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
			unzClose(zfile);
			return 1;
		}
		/*
		buf = malloc(WRITEBUFFERSIZE);
		// for (size_t j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE)) {
			// if (j != fwrite(buf, 1, j, outfile)) {
				debug_log_write("Erreur durant l'ecriture du fichier \"%s\".\n", filename_on_sd);
				fclose(outfile);
				unzCloseCurrentFile(zfile);
				free(buf);
				copy_retry--;
				if (copy_retry > 0) {
					debug_log_write("Tentative de réécriture du fichier...\n");
					printf("\033[0;31m");
					printf(language_vars["lng_install_pack_file_write__retrying_error"].c_str(), filename_on_sd);
					printf("\033[0;37m\n");
					consoleUpdate(&logs_console);
					continue;
				}
				printf("\033[0;31m");
				printf(language_vars["lng_install_pack_file_write_error"].c_str(), filename_on_sd);
				printf("\033[0;37m\n");
				consoleUpdate(&logs_console);
				unzClose(zfile);
				return 1;
			}
		}
		*/

		fclose(outfile);
		free(buf);
		if ((!beta_mode && pack_files_in_zip_sha256_verify_before_copy_param == 1) || (beta_mode && pack_files_in_zip_sha256_verify_before_copy_param_beta == 1)) {
			memset(sha256_out_test, '\0', sizeof(sha256_out_test));
			get_sha256_file(filename_on_sd, sha256_out_test);
			if (strcmp(sha256_in_test, "") == 0) {
				unzCloseCurrentFile(zfile);
				unzOpenCurrentFile(zfile);
				// temp_file = malloc(file_info.uncompressed_size + 1);
				// unzReadCurrentFile(zfile, temp_file, file_info.uncompressed_size);
				// get_sha256_data(temp_file, file_info.uncompressed_size, sha256_in_test);
				// free(temp_file);
				get_sha256_data_for_minizip_opened_file(&zfile, WRITEBUFFERSIZE, sha256_in_test);
			}
			if (strcmp(sha256_in_test, sha256_out_test) != 0) {
				debug_log_write("Erreur durant l'écriture du fichier \"%s\", erreur de vérification du SHA256 après copie.\n", filename_on_sd);
				unzCloseCurrentFile(zfile);
				copy_retry--;
				if (copy_retry > 0) {
					debug_log_write("Tentative de réécriture du fichier...\n");
					printf("\033[0;31m");
					printf(language_vars["lng_install_pack_file_write__retrying_error"].c_str(), filename_on_sd);
					printf("\033[0;37m\n");
					consoleUpdate(&logs_console);
					continue;
				}
				printf("\033[0;31m");
				printf(language_vars["lng_install_pack_file_write_error"].c_str(), filename_on_sd);
				printf("\033[0;37m\n");
				consoleUpdate(&logs_console);
				unzClose(zfile);
				return 1;
			}
		}
		unzCloseCurrentFile(zfile);
		unzGoToNextFile(zfile);
		debug_log_write("Ecriture du fichier \"%s\" OK.\n", filename_on_sd);
		copy_retry = 3;
	}

	unzClose(zfile);
	free((char*) atmo_bootlogo_dir);
	if (detected_payload_bin == false) rename("payload.bin", "payload.bin.temp");
	// remove(output);
	remove("payload.bin");
	custom_cp((char*) "romfs:/payload/ams_rcm.bin", (char*) "payload.bin");

	return 0;
}
