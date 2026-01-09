#include "keys.h"
#include "keyfile.h"
#include <utils/types.h>
#include <libs/fatfs/ff.h>
#include <string.h>
#include <utils/ini.h>
#include <storage/sd.h>
#include "../gfx/gfx.h"

#include "../tools.h"

#define GetHexFromChar(c) ((c & 0x0F) + (c >= 'A' ? 9 : 0))

char *getKey(const char *search, link_t *inilist) {
	LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, inilist, link){
		if (ini_sec->type == INI_CHOICE){
			LIST_FOREACH_ENTRY(ini_kv_t, kv, &ini_sec->kvs, link) {
				if (!strcmp(search, kv->key)) {
					return kv->val;
				}
			}
		}   
	}
	return NULL;
}

bool AddKey(u8 *buff, char *in, u32 len){
	if (in == NULL || strlen(in) != len * 2)
		return false;

	for (int i = 0; i < len; i++) {
		// buff[i] = (u8)((GetHexFromChar(in[i * 2]) << 4) | GetHexFromChar(in[i * 2 + 1]));
		u8 hi = GetHexFromChar(in[i * 2]);
		u8 lo = GetHexFromChar(in[i * 2 + 1]);
		if (hi > 0xF || lo > 0xF)
			return false;
		buff[i] = (hi << 4) | lo;
	}
	return true;
}

bool GetKeysFromFile(char *path, key_storage_t* dumpedKeys) {
	gfx_puts("Grabbing keys from prod.keys...");
	if (!sd_mount()) {
		return false;
	}

	LIST_INIT(iniList); // Whatever we'll just let this die in memory hell
	if (!ini_parse(&iniList, path, false)) {
		debug_log_write("Init parse error in bis keys extract via file\n");
		return false;
	}

	// add biskeys, mkey 0, header_key, save_mac_key
	if (!AddKey(dumpedKeys->bis_key[0], getKey("bis_key_00", &iniList), SE_KEY_128_SIZE * 2)) {
		debug_log_write("bis key 0 extract via file error\n");
		ini_free(&iniList);
		return false;
	}
	if (!AddKey(dumpedKeys->bis_key[1], getKey("bis_key_01", &iniList), SE_KEY_128_SIZE * 2)) {
		debug_log_write("bis key 1 extract via file error\n");
		ini_free(&iniList);
		return false;
	}
	if (!AddKey(dumpedKeys->bis_key[2], getKey("bis_key_02", &iniList), SE_KEY_128_SIZE * 2)) {
		debug_log_write("bis key 2 extract via file error\n");
		ini_free(&iniList);
		return false;
	}
	// AddKey(dumpedKeys->master_key, getKey("master_key_00", &iniList), SE_KEY_128_SIZE);
	// AddKey(dumpedKeys->header_key, getKey("header_key", &iniList), SE_KEY_128_SIZE * 2);
	// AddKey(dumpedKeys->save_mac_key, getKey("save_mac_key", &iniList), SE_KEY_128_SIZE);

	gfx_puts(" Done");
	debug_log_write("bis key extract via file success\n");
	ini_free(&iniList);
	return true;
}