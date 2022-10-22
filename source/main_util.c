#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> // chdir
#include <dirent.h> // mkdir
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <switch.h>

#include "main_util.h"

extern bool debug_enabled;

void debug_log_start() {
		FILE *debug_log_file;
	debug_log_file = fopen("/switch/AIO_LS_pack_Updater/debug.log", "w");
	fclose(debug_log_file);
}

void debug_log_write(const char *text, ...) {
	FILE *debug_log_file;
	debug_log_file = fopen("/switch/AIO_LS_pack_Updater/debug.log", "a");
	va_list v;
	va_start(v, text);
	vfprintf(debug_log_file, text, v);
	va_end(v);
	fclose(debug_log_file);
}

bool custom_cp(char *filein, char *fileout) {
	FILE *exein, *exeout;
	if (debug_enabled) {
		debug_log_write("Copie du fichier \"%s\" vers la destination \"%s\"\n", filein, fileout);
	}
	exein = fopen(filein, "rb");
	if (exein == NULL) {
		/* handle error */
		perror("file open for reading");
		if (debug_enabled) {
			debug_log_write("Erreur d'ouverture du fichier source.\n");
		}
		return false;
	}
	exeout = fopen(fileout, "wb");
	if (exeout == NULL) {
		/* handle error */
		perror("file open for writing");
		if (debug_enabled) {
			debug_log_write("Erreur d'ouverture du fichier cible.\n");
		}
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
		if (debug_enabled) {
			debug_log_write("Erreur de copie.\n");
		}
		return false;
	}
	if (fclose(exeout)) {
		perror("close output file");
		if (debug_enabled) {
			debug_log_write("Erreur de fermeture du fichier source.\n");
		}
		return false;
	}
	if (fclose(exein)) {
		perror("close input file");
		if (debug_enabled) {
			debug_log_write("Erreur de fermeture du fichier cible.\n");
		}
		return false;
	}
	return true;
}

int remove_directory(const char *path) {
	if (debug_enabled) {
		debug_log_write("Suppression du dossier \"%s\".\n", path);
	}
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
		splExit();
		return false;
	} else {
		splExit();
		return true;
	}
}