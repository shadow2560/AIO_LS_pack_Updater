#include "messages.h"
#include <string.h>
#include <stdarg.h>
#include <gfx_utils.h>
#include "../gfx/gfx.h"
#include <libs/fatfs/ff.h>
#include <mem/heap.h>
#include <utils/sprintf.h>
#include <utils/types.h>

const char *g_log_messages[LOG_MSG_COUNT] = {
	[LOG_MSG_malloc_error]   = "Error when allocating memory",
	[LOG_MSG_BIS_VIA_CONSOLE]   = "Bis keys from console used",
	[LOG_MSG_BIS_VIA_FILE]   = "Bis keys from file used",
	[LOG_MSG_ERR_SYSMMC_NOT_AVAILABLE]   = "Sysnand not available!",
	[LOG_MSG_ERR_EMUMMC_NOT_AVAILABLE]   = "EmuNAND not available!",
	[LOG_MSG_SYSMMC_AVAILABLE]   = "Sysnand available",
	[LOG_MSG_EMUMMC_AVAILABLE]   = "EmuNAND available",
	[LOG_MSG_EMUMMC_PATH]   = "EmuNAND path: %s",
	[LOG_MSG_EMUMMC_SECTOR]   = "EmuNAND sector: %d",
	[LOG_MSG_EMUMMC_NINTENDO_PATH]   = "EmuNAND nintendo path: %s",
	[LOG_MSG_ERR_SD_MOUNT]   = "Unable to mount SD.",
	[LOG_MSG_ERR_INIT_EMMC]   = "Emmc initialize failed.",
	[LOG_MSG_ERR_SET_PARTITION]   = "Unable to set partition.",
	[LOG_MSG_ERR_FOUND_PARTITION]   = "Partition '%s' not found",
	[LOG_MSG_ERROR_PRODINFO_READ]   = "Unable to read PRODINFO.",
	[LOG_MSG_ERROR_PRODINFO_MAGIC_READ]   = "Invalid CAL0 magic. Check BIS key 0.",
	[LOG_MSG_ERR_MOUNT_PARTITION]   = "Unable to mount %s partition.",
	[LOG_MSG_DELETE_SAVE_SYSTEM]   = "Deleting save file %s from SYSTEM...",
	[LOG_MSG_DELETE_SAVE_USER]   = "Deleting save file %s from USER...",
	[LOG_MSG_ERR_OPEN_FILE]   = "Cannot open file '%s'",
	[LOG_MSG_ERR_OPEN_FOLDER]   = "Cannot open folder '%s'",
	[LOG_MSG_ERR_FILE_READ]   = "Error when reading file.",
	[LOG_MSG_FILE_RENAME]   = "Renaming/moving file '%s' to '%s'",
	[LOG_MSG_DELETE_FILE_SUCCESS]   = "File deleted.",
	[LOG_MSG_DELETE_FILE_WARNING]   = "File not found (may already be deleted).",
	[LOG_MSG_DELETE_FILE_ERROR]   = "Failed to delete file (error: %d).",
	[LOG_MSG_ERR_EMPTY_FILE]   = "File is empty",
	[LOG_MSG_FLASH_PARTITION_BEGIN]   = "Flashing '%s' on '%s'",
	[LOG_MSG_FLASH_PARTITION_FILE_TO_BIG]   = "File too big for partition",
	[LOG_MSG_FLASH_PARTITION_FILE_NOT_ALLIGNED]   = "Error: file size not sector aligned.",
	[LOG_MSG_FLASH_PARTITION_ERR_PARTITION_WRITE]   = "Error when flashing partition.",
	[LOG_MSG_FLASH_PARTITION_SUCCESS]   = "Flash of partition done.",
	[LOG_MSG_DUMP_PARTITION_BEGIN]   = "Dumping '%s' to '%s'",
	[LOG_MSG_DUMP_PARTITION_NOT_ALLIGNED]   = "Error: partition size not aligned.",
	[LOG_MSG_dump_PARTITION_FILE_TO_BIG]   = "Partition too big for SD remaining space",
	[LOG_MSG_DUMP_PARTITION_ERR_PARTITION_WRITE]   = "Error when dumping partition.",
	[LOG_MSG_DUMP_PARTITION_SUCCESS]   = "Dump of partition done.",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SYS_TO_EMU_SYSTEM]   = "File '%s' transfert from sysnand to emunand on SYSTEM partition",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SYS_TO_EMU_USER]   = "File '%s' transfert from sysnand to emunand on USER partition",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_EMU_TO_SYS_SYSTEM]   = "File '%s' transfert from emunand to sysnand on SYSTEM partition",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_EMU_TO_SYS_USER]   = "File '%s' transfert from emunand to sysnand on USER partition",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_OPEN_SRC]   = "Error when opening source file",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_TOO_LARGE]   = "File too large.",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_NO_DATA_READ]   = "No data read from source or read error",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_TRUNCATED]   = "File truncated: source larger than RAM buffer",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_OPEN_DST]   = "Error when opening dest file.",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_WRITE_DST]   = "Write error on dest file.",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_INCOMPLET_TRANSFER]   = "Error: Incomplete file transfer.",
	[LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SUCCESS]   = "File transfer done",
	[LOG_MSG_FOLDER_COPY_BEGIN]   = "Copying '%s' to '%s'...",
	[LOG_MSG_FOLDER_DELETE_BEGIN]   = "Removing '%s'...",
	[LOG_MSG_FOLDER_COPY_ERROR]   = "Copy failed: %s (%d)",
	[LOG_MSG_FOLDER_DELETE_END]   = "Folder removed",
	[LOG_MSG_FOLDER_COPY_END]   = "Folder copied",
	[LOG_MSG_BATCH_BEGIN]   = "Batch Begin",
	[LOG_MSG_NEXT_BATCH_ON_SYSNAND]   = "Next function will be on sysnand",
	[LOG_MSG_NEXT_BATCH_ON_EMUNAND]   = "Next function will be on emunand",
	[LOG_MSG_UNBRICK_BEGIN]   = "Press \"vol+\" to launch the unbrick process or any other keys to cancel.",
	[LOG_MSG_UNBRICK_AND_WIP_BEGIN]   = "Press \"vol+\" to launch the unbrick and wip process or any other keys to cancel.",
	[LOG_MSG_UNBRICK_FOLDER_ERROR]   = "Folders and files for unbrick are missing, can't continue.",
	[LOG_MSG_UNBRICK_SUCCESS] = "Unbrick done",
	[LOG_MSG_UNBRICK_AND_WIP_SUCCESS] = "Unbrick and wip done",
	[LOG_MSG_WIP_BEGIN] = "Press \"vol+\" to launch the wip process or any other keys to cancel.",
	[LOG_MSG_WIP_SUCCESS] = "Wip done",
	[LOG_MSG_DG_BEGIN] = "Press \"vol+\" to launch the downgrade fix process or any other keys to cancel.",
	[LOG_MSG_DG_SUCCESS] = "Downgrade fix done",
	[LOG_MSG_RM_ERPT_BEGIN] = "Press \"vol+\" to launch the ERPT deletion process or any other keys to cancel.",
	[LOG_MSG_RM_ERPT_SUCCESS] = "ERPT deletion done",
	[LOG_MSG_RM_PARENTAL_CONTROL_BEGIN] = "Press \"vol+\" to launch the parental control deletion process or any other keys to cancel.",
	[LOG_MSG_RM_PARENTAL_CONTROL_SUCCESS] = "Parental control deletion done",
	[LOG_MSG_SYNCH_JOYCONS_BEGIN] = "Press \"vol+\" to launch the synchronization of joycons process or any other keys to cancel.",
	[LOG_MSG_SYNCH_JOYCONS_SUCCESS] = "synchronization of joycons done",
	[LOG_MSG_RESTORE_PRODINFO_BEGIN]   = "Press \"vol+\" to launch the restore PRODINFO process or any other keys to cancel.",
	[LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_SCRATCH_BEGIN]   = "Press \"vol+\" to launch the build and flash of scratch PRODINFO process or any other keys to cancel.",
	[LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_DONOR_BEGIN]   = "Press \"vol+\" to launch the build and flash of donor PRODINFO process or any other keys to cancel.",
	[LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_SCRATCH_SUCCESS]   = "Build and flash of scratch PRODINFO done",
	[LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_DONOR_SUCCESS]   = "Build and flash of donor PRODINFO done",
	[LOG_MSG_PRODINFOGEN_BEGIN]   = "ProdinfoGen begin",
	[LOG_MSG_PRODINFOGEN_WARN_CREATE]   = "Something went wrong, writing output anyway...",
	[LOG_MSG_PRODINFOGEN_SAVE_FILE_SUCCESS]   = "Wrote %d bytes to %s",
	[LOG_MSG_PRODINFOGEN_SAVE_FILE_ERROR]   = "Unable to save generated PRODINFO to SD.",
	[LOG_MSG_PRODINFOGEN_ERR_FIND_DONOR_BIN]   = "Couldn't find donor PRODINFO at sd:/switch/donor_prodinfo.bin",
	[LOG_MSG_PRODINFOGEN_ERR_TOO_SMALL]   = "Donor PRODINFO is too small! (%d < %d)",
	[LOG_MSG_PRODINFOGEN_ERR_TOO_BIG]   = "Donor PRODINFO is too big! (%d > %d)",
	[LOG_MSG_PRODINFOGEN_ERR_INVALID]   = "Donor PRODINFO seems invalid.",
	[LOG_MSG_PRODINFOGEN_WARN_KEY4X]   = "Donor's device_key_4x has not been supplied, extended keys decryption might fail!",
	[LOG_MSG_PRODINFOGEN_WARN_KEY_EGC]   = "Could not decrypt donor extended GameCard key!",
	[LOG_MSG_PRODINFOGEN_ERR_MASTER_KEY]   = "Couldn't get master_key_00.",
	[LOG_MSG_PRODINFOGEN_ERR_READ_DONOR_KEYS]   = "Error parsing sd:/switch/donor.keys.",
	[LOG_MSG_PRODINFOGEN_WARN_FALLBACK_DONOR_TO_SCRATCH]   = "Couldn't import from donor. Generating from scratch.",
	[LOG_MSG_PRODINFOGEN_WRITING_FILE]   = "Writing output file...",
	[LOG_MSG_PRODINFOGEN_KEYS_DUMP_END]   = "Keygen part done in %d us",
	[LOG_MSG_PRODINFOGEN_END]   = "Done in %d us",
	[LOG_MSG_KEYS_DUMP_BEGIN]   = "Lockpick-RCM keys dump begin",
	[LOG_MSG_KEYS_DUMP_ERROR_CRC_SSL_KEY]   = "Crc16 error reading SSL key.",
	[LOG_MSG_KEYS_DUMP_ERROR_CRC_DEVICE_KEY]   = "Crc16 error reading device key.",
	[LOG_MSG_KEYS_DUMP_INVALID_TICKET_KEYPAIR]   = "Invalid eticket keypair.",
	[LOG_MSG_KEYS_DUMP_SSL_INVALID_GMAC]   = "SSL keypair has invalid GMac.",
	[LOG_MSG_KEYS_DUMP_ERR_MASTER_KEYS]   = "Unable to derive master keys for %s.",
	[LOG_MSG_KEYS_DUMP_ERR_READ_KEYBLOBS]   = "Unable to read keyblobs.",
	[LOG_MSG_KEYS_DUMP_WARN_CORRUPT_KEYBLOBS]   = "Keyblob %x corrupt.",
	[LOG_MSG_KEYS_DUMP_ERR_RUN_KEYGEN]   = "Failed to run keygen.",
	[LOG_MSG_KEYS_DUMP_ERR_READ_E1_SAVE]   = "Unable to open e1 save. Skipping.",
	[LOG_MSG_KEYS_DUMP_ERR_PROCESS_ES_SAVE]   = "Failed to process es save.",
	[LOG_MSG_KEYS_DUMP_ERR_LOCATE_TICKET_LIST]   = "Unable to locate ticket_list.bin in save.",
	[LOG_MSG_KEYS_DUMP_ERR_LOCATE_TICKET]   = "Unable to locate ticket.bin in save.",
	[LOG_MSG_KEYS_DUMP_ERR_OPEN_SD_SEED_VECTOR]   = "Unable to open SD seed vector. Skipping.",
	[LOG_MSG_KEYS_DUMP_ERR_READ_SD_SEED_VECTOR]   = "Unable to read SD seed vector. Skipping.",
	[LOG_MSG_KEYS_DUMP_ERR_OPEN_NS_APPMAN]   = "Unable to open ns_appman save.\nSkipping SD seed.",
	[LOG_MSG_KEYS_DUMP_TITLE_KEYS_FOUNDED]   = "Found %d titlekeys.",
	[LOG_MSG_KEYS_DUMP_ERR_SSL_KEY_DERIVATION]   = "Unable to derive SSL key.",
	[LOG_MSG_KEYS_DUMP_ERR_ETICKET_KEY_DERIVATION]   = "Unable to derive ETicket key.",
	[LOG_MSG_KEYS_DUMP_ERR_GET_SD_SEED]   = "Unable to get SD seed.",
	[LOG_MSG_KEYS_DUMP_ERR_TITLEKEYS_DERIVATION]   = "Unable to derive titlekeys.",
	[LOG_MSG_KEYS_DUMP_KEYS_FOUNDED]   = "Found %d %s keys.",
	[LOG_MSG_KEYS_DUMP_KEYS_FOUNDED_VIA]   = "Found through master_key_%02x.",
	[LOG_MSG_KEYS_DUMP_SAVE_FILES]   = "Wrote %d bytes to %s",
	[LOG_MSG_KEYS_DUMP_ERR_SAVE_KEYS_FILE]   = "Unable to save keys to SD.",
	[LOG_MSG_KEYS_DUMP_ERR_SAVE_TITLEKEYS_FILE]   = "Unable to save titlekeys to SD.",
	[LOG_MSG_KEYS_DUMP_ERR_SET_KEYSLOT]   = "Unable to set crypto keyslots! Try launching payload differently or flash Spacecraft-NX if using a modchip.",
	[LOG_MSG_KEYS_DUMP_ERR_EMMC_NOT_INITIALIZED]   = "eMMC not initialized, skipping SD seed and titlekeys.",
	[LOG_MSG_KEYS_DUMP_ERR_BIS_KEYS_NEEDED]   = "Missing needed BIS keys, skipping SD seed and titlekeys.",
	[LOG_MSG_KEYS_DUMP_END]   = "Lockpick totally done in %d us",
	[LOG_MSG_AMIIBO_KEYS_DUMP_BEGIN]   = "Dumping amiibo key...",
	[LOG_MSG_AMIIBO_KEYS_DUMP_ERR_MASTER_KEY]   = "Unable to derive master keys for NFC.",
	[LOG_MSG_AMIIBO_KEYS_DUMP_ERR_HASH]   = "Amiibo hash mismatch, skipping save.",
	[LOG_MSG_AMIIBO_KEYS_DUMP_SUCCESS]   = "Wrote Amiibo keys to %s",
	[LOG_MSG_AMIIBO_KEYS_DUMP_ERR_SAVE_FILE]   = "Unable to save Amiibo keys to SD.",
	[LOG_MSG_BATCH_END]     = "Batch end"
};

log_entry_t* g_log_buf;
u32 g_log_count = 0;
char* g_log_str_pool;
u32  g_log_str_pos = 0;

void log_init() {
	g_log_buf = malloc(sizeof(log_entry_t) * LOG_MAX_ENTRIES);
	if (!g_log_buf) {
		g_log_count = LOG_MAX_ENTRIES + 1;
		return;
	}
	g_log_str_pool = (char*)malloc(LOG_STR_POOL_SIZE);
	if (!g_log_str_pool) {
		g_log_count = LOG_MAX_ENTRIES + 1;
		free(g_log_buf);
		g_log_buf = NULL;
		return;
	}
	memset(g_log_buf, 0, sizeof(log_entry_t) * LOG_MAX_ENTRIES);
	
}

void log_free() {
	if (g_log_str_pool) free(g_log_str_pool);
	if (g_log_buf) free(g_log_buf);
	g_log_str_pool = NULL;
	g_log_buf = NULL;
	g_log_count = 0;
	g_log_str_pos = 0;
}

static u32 log_store_string(const char *s)
{
	if (!s)
		return 0;

	u32 max = LOG_STR_POOL_SIZE - g_log_str_pos;
	if (max < 2)
		return 0;

	u32 len = strlen(s);
	if (len >= max)
		len = max - 1;

	u32 off = g_log_str_pos;

	memcpy(&g_log_str_pool[g_log_str_pos], s, len);
	g_log_str_pool[g_log_str_pos + len] = 0;   // ← TERMINAISON GARANTIE

	g_log_str_pos += len + 1;

	return off;
}

void log_printf(log_level_t lvl, log_msg_id_t id, ...) {
	const char *fmt = g_log_messages[id];
	bool store = called_from_config_files && !called_from_AIO_LS_Pack_Updater && g_log_count < LOG_MAX_ENTRIES;

	log_entry_t *e = NULL;
	if (store) {
		e = &g_log_buf[g_log_count++];
		memset(e, 0, sizeof(*e));
		e->level = lvl;
		e->msg_id = id;
		e->argc = 0;
	}

	u32 color = COLOR_WHITE;
	if (lvl == LOG_OK) color = COLOR_GREEN;
	else if (lvl == LOG_WARN) color = COLOR_ORANGE;
	else if (lvl == LOG_ERR) color = COLOR_RED;

	gfx_printf("%k", color);

	va_list ap;
	va_start(ap, id);

	u32 args[LOG_MAX_ARGS] = {0};
	u8  argc = 0;

	for (const char *p = fmt; *p && argc < LOG_MAX_ARGS; p++) {
		if (*p != '%')
			continue;

		p++;
		if (*p == '%')
			continue;

		u32 v = 0;
		switch (*p) {
			case 'd':
			case 'x':
			case 'X':
				v = va_arg(ap, u32);
				if (store && e && e->argc < LOG_MAX_ARGS) {
					e->arg_type[e->argc] = LOG_ARG_U32;
					e->argv[e->argc++] = v;
				}
				args[argc++] = v;
				break;

			case 'p':
				v = va_arg(ap, u32);
				if (store && e && e->argc < LOG_MAX_ARGS) {
					e->arg_type[e->argc] = LOG_ARG_PTR;
					e->argv[e->argc++] = v;
				}
				args[argc++] = v;
				break;

			case 's':
				const char *str = va_arg(ap, char *);
				if (store && e && e->argc < LOG_MAX_ARGS) {
					u32 off = str ? log_store_string(str) : 0;
					e->arg_type[e->argc] = LOG_ARG_STR;
					e->argv[e->argc++] = off;
				}
				args[argc++] = (u32)(str ? str : "<null>");
				break;
		}
	}

	va_end(ap);

	gfx_printf(fmt,
		argc > 0 ? args[0] : 0,
		argc > 1 ? args[1] : 0,
		argc > 2 ? args[2] : 0,
		argc > 3 ? args[3] : 0
	);
	gfx_printf("\n");
}

u32 log_arg_value(log_entry_t *e, int i) {
	if (i >= e->argc)
		return 0;

	if (e->arg_type[i] == LOG_ARG_STR) {
		u32 off = e->argv[i];
		if (off >= LOG_STR_POOL_SIZE)
			return (u32)"<invalid str>";
		return (u32)&g_log_str_pool[off];
	}

	return e->argv[i];
}

void log_export_txt(const char *path) {
	if (g_log_count == 0 || g_log_count > LOG_MAX_ENTRIES) {
		return;
	}
	mkdir_recursive(path);
	FIL fp;
	if (f_open(&fp, path, FA_CREATE_ALWAYS | FA_WRITE))
		return;

	for (u32 i = 0; i < g_log_count; i++) {
		log_entry_t *e = &g_log_buf[i];
		const char *fmt = g_log_messages[e->msg_id];

		char line[256];
		char *p = line;

		*p++ = '0' + e->level;
		*p++ = ' ';
		*p++ = '-';
		*p++ = ' ';

		p += s_printf(p, fmt,
			log_arg_value(e, 0),
			log_arg_value(e, 1),
			log_arg_value(e, 2),
			log_arg_value(e, 3)
		);

		*p++ = '\n';
		*p = 0;

		UINT bw;
		f_write(&fp, line, strlen(line), &bw);
	}

	f_close(&fp);
}