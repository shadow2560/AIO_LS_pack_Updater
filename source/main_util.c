#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <minizip/unzip.h>
#include <switch.h>

#include "main_util.h"

bool debug_enabled = true;
bool beta_mode = false;
PrintConsole menu_console;
PrintConsole logs_console;

char CFW_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/Switch_AIO_LS_pack.zip";
char pack_sha256_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_pack.txt";
char pack_changelog_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/changelog.html";
char CFW_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/archive/refs/heads/main.zip";
char pack_sha256_url_beta[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_pack_beta.txt";
char pack_changelog_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/blob/main/changelog.md";
char pack_custom_files_url[1003] = "";
char pack_custom_files_url_beta[1003] = "";
char pack_custom_files_sha256_url[1003] = "";
char pack_custom_files_sha256_url_beta[1003] = "";
char pack_custom_files_subfolder_in_zip[FS_MAX_PATH] = "";
char pack_custom_files_subfolder_in_zip_beta[FS_MAX_PATH] = "";
s64 pack_custom_files_size = 10000000;
s64 pack_custom_files_size_beta = 10000000;
char pack_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/pack_version.txt";
char pack_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/pack_version.txt";
char pack_version_local_filepath[FS_MAX_PATH] = "/pack_version.txt";
char pack_version_local_filepath_beta[FS_MAX_PATH] = "/pack_version.txt";
char subfolder_in_zip[FS_MAX_PATH] = "";
char subfolder_in_zip_beta[FS_MAX_PATH] = "switch_AIO_LS_pack-main/";
s64 pack_size = 1000000000;
s64 pack_size_beta = 1000000000;
char APP_URL[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/AIO_LS_pack_Updater.nro";
char app_sha256_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/sha256_AIO_LS_pack_Updater.txt";
char app_version_url[1003] = "https://ls-atelier-tutos.fr/files/Switch_AIO_LS_pack/app_version.txt";
char APP_URL_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro";
char app_sha256_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/sha256_AIO_LS_pack_Updater.txt";
char app_version_url_beta[1003] = "https://github.com/shadow2560/switch_AIO_LS_pack/raw/main/app_version.txt";
char firmware_path[FS_MAX_PATH] = "/dernier_firmware_compatible";
char firmware_path_beta[FS_MAX_PATH] = "/dernier_firmware_compatible";
char atmo_logo_dir[FS_MAX_PATH] = "logo";
char atmo_logo_dir_beta[FS_MAX_PATH] = "logo";
char hekate_nologo_file_path[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
char hekate_nologo_file_path_beta[FS_MAX_PATH] = "romfs:/nologo/hekate_ipl.ini";
int pack_files_in_zip_sha256_verify_before_copy_param = 1;
int pack_files_in_zip_sha256_verify_before_copy_param_beta = 1;
int exit_mode_param = 0;
int exit_mode_param_beta = 0;
int install_pack_hekate_autoboot_choice_time = 0;
int install_pack_hekate_autoboot_choice_time_beta = 0;
int debug_enabled_param = 0;
int debug_enabled_param_beta = 0;

int hekate_autoboot = 0;
int hekate_autoboot_lineno = -1;
int hekate_autoboot_config = 0;
int hekate_autoboot_config_lineno = -1;
bool autoconfig_enabled = false;

void debug_log_start() {
	if (!debug_enabled) return;
	remove("/switch/AIO_LS_pack_Updater/debug.log");
	FILE *debug_log_file;
	debug_log_file = fopen("/switch/AIO_LS_pack_Updater/debug.log", "w");
	fclose(debug_log_file);
}

void debug_log_write(const char *text, ...) {
	if (!debug_enabled) return;
	FILE *debug_log_file;
	debug_log_file = fopen("/switch/AIO_LS_pack_Updater/debug.log", "a");
	va_list v;
	va_start(v, text);
	vfprintf(debug_log_file, text, v);
	va_end(v);
	fclose(debug_log_file);
}

void to_lowercase(char *str) {
	if (!str) return;
	while (*str) {
		*str = tolower((unsigned char)*str);
		str++;
	}
}

void to_uppercase(char *str) {
	if (!str) return;
	while (*str) {
		*str = tolower((unsigned char)*str);
		str++;
	}
}

bool strcmp_ignore_case(const char *s1, const char *s2) {
	if (!s1 || !s2) return false;
	while (*s1 && *s2) {
		if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
			return false;
		}
		s1++;
		s2++;
	}
	return true;
}

bool custom_cp(char *filein, char *fileout) {
	FILE *exein, *exeout;
	debug_log_write("Copie du fichier \"%s\" vers la destination \"%s\"\n", filein, fileout);
	exein = fopen(filein, "rb");
	if (exein == NULL) {
		/* handle error */
		perror("file open for reading");
		debug_log_write("Erreur d'ouverture du fichier source.\n");
		return false;
	}
	exeout = fopen(fileout, "wb");
	if (exeout == NULL) {
		/* handle error */
		perror("file open for writing");
		debug_log_write("Erreur d'ouverture du fichier cible.\n");
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
		debug_log_write("Erreur de copie.\n");
		return false;
	}
	if (fclose(exeout)) {
		perror("close output file");
		debug_log_write("Erreur de fermeture du fichier source.\n");
		return false;
	}
	if (fclose(exein)) {
		perror("close input file");
		debug_log_write("Erreur de fermeture du fichier cible.\n");
		return false;
	}
	return true;
}

bool copy_directory_recursive(const char *source, const char *destination, bool include_source) {
	debug_log_write("Début de la copie récursive du répertoire \"%s\" vers \"%s\" (include_source: %d).\n", source, destination, include_source);

	struct stat statbuf;
	if (stat(source, &statbuf) != 0) {
		perror("stat source directory");
		debug_log_write("Erreur d'accès au répertoire source.\n");
		return false;
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		fprintf(stderr, "Source is not a directory: %s\n", source);
		return false;
	}

	// Buffer pour le chemin racine
	char destination_root[FS_MAX_PATH];
	if (include_source) {
		const char *source_basename = strrchr(source, '/');
		if (!source_basename) source_basename = source;
		else source_basename++;

		if (snprintf(destination_root, FS_MAX_PATH, "%s/%s", destination, source_basename) >= FS_MAX_PATH) {
			fprintf(stderr, "Chemin destination trop long.\n");
			return false;
		}

		mkdir(destination_root, statbuf.st_mode);
	} else {
		strncpy(destination_root, destination, FS_MAX_PATH - 1);
		destination_root[FS_MAX_PATH - 1] = '\0'; // Toujours null-terminer
	}

	// Parcourir le contenu du répertoire source
	DIR *dir = opendir(source);
	if (!dir) {
		perror("opendir source");
		debug_log_write("Erreur d'ouverture du répertoire source.\n");
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		// Ignorer les entrées spéciales "." et ".."
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
			continue;
		}

		char source_path[FS_MAX_PATH];
		char destination_path[FS_MAX_PATH];

		// Construire les chemins source et destination
		if (snprintf(source_path, FS_MAX_PATH, "%s/%s", source, entry->d_name) >= FS_MAX_PATH ||
			snprintf(destination_path, FS_MAX_PATH, "%s/%s", destination_root, entry->d_name) >= FS_MAX_PATH) {
			fprintf(stderr, "Chemin trop long pour %s ou %s.\n", source, entry->d_name);
			closedir(dir);
			return false;
		}

		// Vérifier si l'entrée est un répertoire ou un fichier
		if (stat(source_path, &statbuf) == 0) {
			if (S_ISDIR(statbuf.st_mode)) {
				// Créer le sous-dossier dans la destination
				mkdir(destination_path, statbuf.st_mode);

				// Appel récursif pour copier le contenu du sous-dossier
				if (!copy_directory_recursive(source_path, destination_path, true)) {
					closedir(dir);
					return false;
				}
			} else if (S_ISREG(statbuf.st_mode)) {
				// Copier le fichier
				if (!custom_cp(source_path, destination_path)) {
					closedir(dir);
					return false;
				}
			}
		} else {
			perror("stat entry");
			debug_log_write("Erreur lors de l'analyse d'une entrée du répertoire source.\n");
		}
	}

	closedir(dir);
	return true;
}

int remove_directory(const char *path) {
	debug_log_write("Suppression du dossier \"%s\".\n", path);
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

char * substr(char *s, int x, int y) {
	char * ret = (char*) malloc(strlen(s) + 1);
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

// function directly taken from Atmosphere
u32 ParseHexInteger(const char *s) {
	u32 x = 0;
	if (s[0] == '0' && s[1] == 'x') {
		s += 2;
	}

	while (true) {
		const char c = *(s++);

		if (c == '\x00') {
			return x;
		} else {
			x <<= 4;

			if ('0' <= c && c <= '9') {
				x |= (c - '0');
			} else if ('a' <= c && c <= 'f') {
				x |= (c - 'a') + 10;
			} else if ('A' <= c && c <= 'F') {
				x |= (c - 'A') + 10;
			}
		}
	}
}

bool isApplet() {
	AppletType at = appletGetAppletType();
	return at != AppletType_Application && at != AppletType_SystemApplication;
}

u64 get_app_titleid() {
	pmdmntInitialize();
	pminfoInitialize();
	u64 m_pid = 0;
	u64 m_tid = 0;
	pmdmntGetApplicationProcessId(&m_pid);
	pminfoGetProgramId(&m_tid, m_pid);
	pminfoExit();
	pmdmntExit();
	return m_tid;
}

u64 GetCurrentApplicationId() {
	u64 app_id = 0;
	svcGetInfo(&app_id, InfoType_ProgramId, CUR_PROCESS_HANDLE, 0);
	return app_id;
}

bool titleid_curently_launched(u64 titleid) {
	if (get_app_titleid() == titleid) {
		return true;
	} else {
		return false;
	}
}

u32 get_battery_charge() {
		u32 charge;
	Result rc = psmInitialize();
	if (!R_FAILED(rc)) {
		rc = psmGetBatteryChargePercentage(&charge);
		if (!R_FAILED(rc)) {
			psmExit();
			return(charge);
		} else {
			psmExit();
			return -1;
		}
	}
	return -1;
}

int GetChargerType() {
	Result rc = psmInitialize();
	if (!R_FAILED(rc)) {
		PsmChargerType charger_type;
		if (R_FAILED(rc = psmGetChargerType(&charger_type))) {
			psmExit();;
			return -1;
		}
		if (charger_type == PsmChargerType_EnoughPower) {
			psmExit();;
			return 0; // "Official charger or dock";
		} else if (charger_type == PsmChargerType_LowPower) {
			psmExit();;
			return 1; // "USB-C charger";
		} else if (charger_type == PsmChargerType_Unconnected) {
			psmExit();;
			return 2; // "Charger no connected";
		} else {
			psmExit();;
			return 3; // "unknown result";
		}
	}
	return -1;
}

s64 get_sd_size_left() {
	s64 fs_sd_size;
	FsFileSystem *fs_sd;
	fs_sd = fsdevGetDeviceFileSystem("sdmc");
	fsFsGetFreeSpace(fs_sd, "/", &fs_sd_size);
	// fsFsClose(fs_sd);
	// nsInitialize();
	// nsGetFreeSpaceSize(NcmStorageId_SdCard, fs_sd_size);
	// nsExit();
		// printf("%ld\n", fs_sd_size);
		return fs_sd_size;
}

bool is_emummc() {
	u32 ExosphereEmummcType		   = 65007;
	Result rc = 0;
	u64 is_emummc;
	splInitialize();
	if (R_FAILED(rc = splGetConfig((SplConfigItem)(ExosphereEmummcType), &is_emummc))) {
		debug_log_write("Erreur de détection du status de l'emummc.\n");
	}
	splExit();
	if (!is_emummc) {
		return false;
	} else {
		return true;
	}
}

void smcAmsGetEmunandConfig(EmummcPaths* out_paths) {
	SecmonArgs args = {};
	args.X[0] = 0xF0000404; /* smcAmsGetEmunandConfig */
	args.X[1] = 0; /* EXO_EMUMMC_MMC_NAND*/
	args.X[2] = (u64)out_paths; /* out path */
	svcCallSecureMonitor(&args);
}

void get_sha256_file(const char* filepath, char* ret) {
	// ret = "";
	FILE *file = fopen(filepath, "rb");
	if (file == NULL) {
		return;
	}
	unsigned char sha256_hash[32];
	const size_t CHUNK = 512 * 1024;
	char *buf = (char *) malloc(CHUNK);
	if (!buf) {
		debug_log_write("Erreur malloc dans get_sha256_file\n");
		strcpy(ret, "");
		return;
	}
	Sha256Context ctx;
	sha256ContextCreate(&ctx);
	size_t i;
	while((i = fread(buf, 1, sizeof(buf), file)) > 0) {
		sha256ContextUpdate(&ctx, buf, i);
	}
	sha256ContextGetHash(&ctx, sha256_hash);
	free(buf);
	fclose(file);
	char tmp[3];
	ret[0] = '\0';
	for (int j = 0; j < 32; j++) {
		sprintf(tmp, "%02x", sha256_hash[j]);
		strcat(ret, tmp);
	}
}

void get_sha256_data(void* datas, size_t size, char* ret) {
	// ret = "";
	// FILE* file=fmemopen(datas, size, "r");
	unsigned char sha256_hash[32];
	const size_t CHUNK = 512 * 1024;
	char *buf = (char *) malloc(CHUNK);
	if (!buf) {
		debug_log_write("Erreur malloc dans get_sha256_data\n");
		strcpy(ret, "");
		return;
	}
	// char * buf = (char *) malloc(512);
	Sha256Context ctx;
	sha256ContextCreate(&ctx);
	// size_t i;
	// while((i = fread(buf, 1, sizeof(buf), file)) > 0) {
		// sha256ContextUpdate(&ctx, buf, i);
	// }
	sha256ContextUpdate(&ctx, datas, size);
	sha256ContextGetHash(&ctx, sha256_hash);
	// free(buf);
	// fclose(file);
	char tmp[3];
	ret[0] = '\0';
	for (int j = 0; j < 32; j++) {
		sprintf(tmp, "%02x", sha256_hash[j]);
		strcat(ret, tmp);
	}
}

void get_sha256_data_for_minizip_opened_file(unzFile* zfile, char* ret) {
	unsigned char sha256_hash[32];
	const size_t CHUNK = 512 * 1024;
	char *buf = (char *) malloc(CHUNK);
	if (!buf) {
		debug_log_write("Erreur malloc dans get_sha256_data_for_minizip_opened_file\n");
		strcpy(ret, "");
		return;
	}
	Sha256Context ctx;
	sha256ContextCreate(&ctx);
	int r;
	while ((r = unzReadCurrentFile(*zfile, buf, CHUNK)) > 0) {
		sha256ContextUpdate(&ctx, buf, r);
	}
	sha256ContextGetHash(&ctx, sha256_hash);
	free(buf);
	char tmp[3];
	ret[0] = '\0';
	for (int j = 0; j < 32; j++) {
		sprintf(tmp, "%02x", sha256_hash[j]);
		strcat(ret, tmp);
	}
}

bool module_is_running(u64 module) {
	pmdmntInitialize();
	u64 pid = 0;
	if (R_FAILED(pmdmntGetProcessId(&pid, module))) {
		pmdmntExit();
		return false;
	}
	pmdmntExit();
	return pid > 0;
}

void close_module(u64 module) {
	if (module_is_running(module)) {
		pmshellInitialize();
		pmshellTerminateProgram(module);
		pmshellExit();
	}
}

SetSysFirmwareVersion int_get_fw_version() {
	setsysInitialize();
	Result ret = 0;
	SetSysFirmwareVersion ver;
	if (R_FAILED(ret = setsysGetFirmwareVersion(&ver))) {
		setsysExit();
		return ver;
	}
	return ver;
	setsysExit();
}

bool internet_is_connected() {
	nifmInitialize(NifmServiceType_User);
	NifmInternetConnectionStatus status;
	nifmGetInternetConnectionStatus(NULL, NULL, &status);
	if (status == NifmInternetConnectionStatus_Connected) {
		nifmExit();
		return true;
	}
	nifmExit();
	return false;
}