#include <string.h>
#include <stdarg.h>

#include "tools.h"
#include "config.h"
#include <gfx_utils.h>
#include "gfx/tui.h"
#include "keys/keys.h"
#include <libs/fatfs/ff.h>
#include <mem/heap.h>
#include <soc/timer.h>
#include "storage/emummc.h"
#include <storage/emmc.h>
#include "storage/nx_emmc_bis.h"
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/ini.h>
#include <utils/list.h>
#include <utils/sprintf.h>

#include "gfx/messages.h"

bool emunand_probe_path(const char *path)
{
	FIL fp;
	char tmp[256];

	// Tester raw_based
	s_printf(tmp, "%s/raw_based", path);
	if (!f_open(&fp, tmp, FA_READ))
	{
		u32 sector = 0;
		if (!f_read(&fp, &sector, 4, NULL) && sector) {
			f_close(&fp);

			s_printf(tmp, "%s/Nintendo", path);
			if (!f_stat(tmp, NULL)) {
				// out->sector = sector;
				// s_printf(out->base_path, "%s", path);
				// s_printf(out->nintendo_path, "%s/Nintendo", path);
				return true;
			}
		}
		f_close(&fp);
	}

	// Tester file_based
	s_printf(tmp, "%s/file_based", path);
	if (!f_stat(tmp, NULL))
	{
		s_printf(tmp, "%s/Nintendo", path);
		if (!f_stat(tmp, NULL)) {
			// out->sector = 0;
			// s_printf(out->base_path, "%s", path);
			// s_printf(out->nintendo_path, "%s/Nintendo", path);
			return true;
		}
	}

	return false;
}

char *bdk_strdup(const char *s)
{
	if (!s)
		return NULL;

	size_t len = strlen(s) + 1;
	char *dst = malloc(len);
	if (!dst) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		return NULL;
	}

	memcpy(dst, s, len);
	return dst;
}

void mkdir_recursive(const char *path) {
	char tmp[256];
	char *p = NULL;

	s_printf(tmp, "%s", path);
	tmp[sizeof(tmp) - 1] = '\0';

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			f_mkdir(tmp);
			*p = '/';
		}
	}
}

void emunand_list_build(link_t *inilist, bool count_only) {
	LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, inilist, link){
		if (ini_sec->type == INI_CHOICE){
			LIST_FOREACH_ENTRY(ini_kv_t, kv, &ini_sec->kvs, link) {
				if (!strcmp("emupath", kv->key) && emunand_count < MAX_EMUNANDS) {
					// emunand_entry_t e;
					// s_printf(e.name, "%s", ini_sec->name);
					if (emunand_probe_path(kv->val)) {
						if (!count_only) {
							emunands[emunand_count].name = bdk_strdup(ini_sec->name);
							emunands[emunand_count].base_path = bdk_strdup(kv->val);
							// emunands[emunand_count] = e;
						}
						emunand_count++;
					}
				}
			}
		}   
	}
}

void build_emunand_list() {
	if (emu_cfg.sector != 0 || emu_cfg.path) {
		emunand_count++;
	}
	LIST_INIT(list);
	if (!f_stat("sd:/bootloader/hekate_ipl.ini", NULL)) {
		if (!ini_parse(&list, "sd:/bootloader/hekate_ipl.ini", false)) {
			debug_log_write("Parsing hekate_ipl.ini failed.\n");
		} else {
			emunand_list_build(&list, true);
			ini_free(&list);
			list_init(&list);
		}
	}
	if (!f_stat("sd:/bootloader/ini", NULL)) {
		if (!ini_parse(&list, "sd:/bootloader/ini", true)) {
			debug_log_write("Parsing bootloader/ini folder  failed.\n");
		} else {
			emunand_list_build(&list, true);
			ini_free(&list);
			list_init(&list);
		}
	}
	emunands = malloc(emunand_count * sizeof(emunand_entry_t));
	if (!emunands) {
		return;
	}
	emunand_count = 0;
	if (emu_cfg.sector != 0 || emu_cfg.path) {
		debug_log_write("Default emunand added.\n");
		emunands[emunand_count].name = bdk_strdup("default_config");
		emunands[emunand_count].base_path = bdk_strdup(emu_cfg.path);
		// s_printf(emunands[0].name, "default_config");
		// s_printf(emunands[0].base_path, "%s", emu_cfg.path);
		emunand_count++;
	}
	if (!f_stat("sd:/bootloader/hekate_ipl.ini", NULL)) {
		if (!ini_parse(&list, "sd:/bootloader/hekate_ipl.ini", false)) {
			debug_log_write("Parsing hekate_ipl.ini failed.\n");
		} else {
			emunand_list_build(&list, false);
			ini_free(&list);
			list_init(&list);
		}
	}
	if (!f_stat("sd:/bootloader/ini", NULL)) {
		if (!ini_parse(&list, "sd:/bootloader/ini", true)) {
			debug_log_write("Parsing bootloader/ini folder  failed.\n");
		} else {
			emunand_list_build(&list, false);
			ini_free(&list);
			list_init(&list);
		}
	}
}

void apply_emunand(const emunand_entry_t *e) {
	debug_log_write("applying emunand %s\n", e->name);
	if (strcmp(e->name, "default_config") == 0) {
		emummc_load_cfg();
	} else {
		emummc_set_path(e->base_path);
	}
	emu_cfg.enabled = true;
	h_cfg.emummc_force_disable = false;
	menu_on_sysnand = false;

	/*
	if (e->sector == 0) {
		s_printf(emu_cfg.path, "%s", e->base_path);
	} else {
		emu_cfg.path = NULL;
	}

	s_printf(emu_cfg.nintendo_path, "%s", e->nintendo_path);
	*/
}

int emunand_select_menu() {
	int idx = 0;
	int cnt = emunand_count;

	gfx_clear_partial_grey(0x1B, 0, 1256);

	while (1) {
		gfx_con_setpos(0, 0);
		display_title();
		gfx_putc('\n');
		gfx_printf("Select emuNAND:\n\n");

		for (int i = 0; i < cnt; i++) {
			if (i == idx)
				gfx_con_setcol(0xFF1B1B1B, 1, 0xFFCCCCCC);
			else
				gfx_con_setcol(0xFFCCCCCC, 1, 0xFF1B1B1B);

			gfx_printf(" %s\n", emunands[i].name);
		}

		gfx_con_setcol(0xFFCCCCCC, 1, 0xFF1B1B1B);
		gfx_con_setpos(0, 1191);
		gfx_printf("VOL: Move | PWR: Select");

		u32 btn = btn_wait();

		if (btn & BTN_VOL_DOWN) {
			idx++;
			if (idx >= cnt)
				idx = 0;
		} else if (btn & BTN_VOL_UP) {
			if (idx == 0)
				idx = cnt - 1;
			else
				idx--;
		} else if (btn & BTN_POWER) {
			return idx;
		}

		gfx_clear_partial_grey(0x1B, 0, 1256);
	}
}

void select_and_apply_emunand() {
	if (emunand_count == 0) {
		menu_on_sysnand = true;
		emummc_available = false;
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
		debug_log_write("No emunand, switching to sysnand\n");
		return;
	}

	if (emunand_count == 1) {
		debug_log_write("Applying default emunand\n");
		apply_emunand(&emunands[0]);
		return;
	}

	int sel = emunand_select_menu();
	if (sel >= 0 && sel < emunand_count) {
		debug_log_write("Applying emunand %s\n", emunands[sel]);
		apply_emunand(&emunands[sel]);
		prev_sec_emunand = cur_sec_emunand;
		cur_sec_emunand = sel;
	}
}

void emunand_list_free() {
	if (!emunands)
		return;

	for (int i = 0; i < emunand_count; i++) {
		if (emunands[i].name)
			free(emunands[i].name);

		if (emunands[i].base_path)
			free(emunands[i].base_path);
	}

	free(emunands);
	emunands = NULL;
	emunand_count = 0;
}

void debug_log_start_impl() {
	if (!f_stat("sd:/locksmith-rcm.log", NULL)) {
		f_unlink("sd:/locksmith-rcm.log");
	}
	FIL debug_log_file;
	f_open(&debug_log_file, "sd:/locksmith-rcm.log", FA_CREATE_ALWAYS | FA_WRITE);
	f_close(&debug_log_file);
}

void debug_log_write_impl(const char *text, ...) {
	FIL debug_log_file;
	UINT bw;

	char buffer[4096];
	va_list args;
	va_start(args, text);
	s_vprintf(buffer, text, args);
	va_end(args);

	f_open(&debug_log_file, "sd:/locksmith-rcm.log", FA_OPEN_APPEND | FA_WRITE);
	f_write(&debug_log_file, buffer, strlen(buffer), &bw);
	// f_sync(&debug_log_file);
	f_close(&debug_log_file);
}

void test_total_heap() {
	size_t total_test = 0;
	while (1) {
		void *p = malloc(1024 * 1024); // 1MB
		if (!p) break;
		total_test++;
	}
	debug_log_write("Max heap ~ %u MB\n", total_test);
}

void debug_dump_gpt(link_t *gpt) {
	// emmc_part_t *p;

	debug_log_write("==== GPT partition table ====\n");

	LIST_FOREACH_ENTRY(emmc_part_t, p, gpt, link)
	{
		debug_log_write(
			"Part: name='%s' lba_start=%d lba_end=%d sectors=%d\n",
			p->name,
			(u32)p->lba_start,
			(u32)p->lba_end,
			(u32)(p->lba_end - p->lba_start + 1)
		);
	}

	debug_log_write("==== END GPT ====\n");
}

static bool part_is_encrypted(emmc_part_t *part) {
	switch (part->index) {
		case 0:  /* PRODINFO */
		case 1:  /* PRODINFOF */
		case 8:  /* SAFE */
		case 9:  /* SYSTEM */
		case 10: /* USER */
			return true;
			break;
		default:
			return false;
			break;
	}
	return false;
}

bool mount_nand_part(link_t *gpt, const char *part_name, bool nand_open, bool set_partition, bool fatfs_mount, bool test_loaded_keys, u64 *part_size_bytes_buf, bool *is_boot_buf, bool *is_bis_buf, emmc_part_t *part_buf) {
	bool is_boot = false;
	bool use_bis = false;
	u64 part_size_bytes = 0;
	emmc_part_t *part = NULL;
	if (nand_open) {
				sd_mount();
		if (emummc_storage_init_mmc()) {
			log_printf(LOG_ERR, LOG_MSG_ERR_INIT_EMMC);
			debug_log_write(g_log_messages[LOG_MSG_ERR_INIT_EMMC]);
			debug_log_write("\n");
			return false;
		}
	}

	/* Select partition context */
	if (strcmp(part_name, "BOOT0") == 0) {
		if (set_partition && !emummc_storage_set_mmc_partition(EMMC_BOOT0)) {
			log_printf(LOG_ERR, LOG_MSG_ERR_SET_PARTITION);
			return false;
		}
		is_boot = true;
		part_size_bytes = (u64)emmc_storage.ext_csd.boot_mult << 17; // boot size from ext_csd
	} else if (strcmp(part_name, "BOOT1") == 0) {
		if (set_partition && !emummc_storage_set_mmc_partition(EMMC_BOOT1)) {
			log_printf(LOG_ERR, LOG_MSG_ERR_SET_PARTITION);
			return false;
		}
		is_boot = true;
		part_size_bytes = (u64)emmc_storage.ext_csd.boot_mult << 17;
	} else {
		if (set_partition && !emummc_storage_set_mmc_partition(EMMC_GPP)) {
			log_printf(LOG_ERR, LOG_MSG_ERR_SET_PARTITION);
			return false;
		}
		emmc_gpt_parse(gpt);
		part = emmc_part_find(gpt, part_name);
		if (!part) {
			log_printf(LOG_ERR, LOG_MSG_ERR_FOUND_PARTITION, part_name);
			debug_log_write(g_log_messages[LOG_MSG_ERR_FOUND_PARTITION], part_name);
			debug_log_write("\n");
			emmc_gpt_free(gpt);
			list_init(gpt);
			emummc_storage_end();
			sd_mount();
			return false;
		}
		part_size_bytes = (u64)(part->lba_end - part->lba_start + 1) * EMMC_BLOCKSIZE;

		if (part_buf != NULL) {
			*part_buf = *part;
		}

		/* determine if partition is BIS-encrypted */
		use_bis = part_is_encrypted(part);
		if (use_bis) {
			/* init bis for this partition (cache disabled) */
			nx_emmc_bis_init(part, false, 0);
			// nx_emmc_bis_init(part);
		}
		if (strcmp(part_name, "PRODINFO") == 0 && test_loaded_keys) {
			u8 sector[0x200];
			u32 magic;
			if (!nx_emmc_bis_read(0, 1, sector)) {
				log_printf(LOG_ERR, LOG_MSG_ERROR_PRODINFO_READ);
				nx_emmc_bis_finalize();
				emmc_gpt_free(gpt);
				list_init(gpt);
				emummc_storage_end();
				sd_mount();
				return false;
			}
			magic = *(u32 *)(sector + 0x0); // 0x0 is the offset of the CAL0 magic
			if (magic != MAGIC_CAL0) {
				log_printf(LOG_ERR, LOG_MSG_ERROR_PRODINFO_MAGIC_READ);
				nx_emmc_bis_finalize();
				emmc_gpt_free(gpt);
				list_init(gpt);
				emummc_storage_end();
				sd_mount();
				return false;
			}
		}
		if ((fatfs_mount || test_loaded_keys) && (strcmp(part_name, "PRODINFOF") == 0 || strcmp(part_name, "SAFE") == 0 || strcmp(part_name, "SYSTEM") == 0 || strcmp(part_name, "USER") == 0)) {
			FRESULT test_mount = f_mount(&emmc_fs, "bis:", 1);
			if (test_mount) {
				log_printf(LOG_ERR, LOG_MSG_ERR_MOUNT_PARTITION, part_name);
				debug_log_write("Failed to mounte partition, error %d.\n", (u32) test_mount);
				nx_emmc_bis_finalize();
				emmc_gpt_free(gpt);
				list_init(gpt);
				emummc_storage_end();
				sd_mount();
				return false;
			}
			if (test_loaded_keys && !fatfs_mount) {
				f_mount(NULL, "bis:", 1);
			}
		}
	}
	if (part_size_bytes_buf != NULL) {
	*part_size_bytes_buf = part_size_bytes;
	}
	if (is_boot_buf != NULL) {
	*is_boot_buf = is_boot;
	}
	if (is_bis_buf != NULL) {
		*is_bis_buf = use_bis;
	}
	return true;
}

void unmount_nand_part(link_t *gpt, bool is_boot_part, bool is_bis, bool nand_close, bool fatfs_unmount) {
	if (fatfs_unmount) {
		f_mount(NULL, "bis:", 1);
	}
	if (!is_boot_part) {
		if (is_bis) {
			nx_emmc_bis_finalize();
		}
		if (gpt != NULL) {
			emmc_gpt_free(gpt);
			list_init(gpt);
		}
	}
	if (nand_close) {
		emummc_storage_end();
		sd_mount();
	}
}

bool wait_vol_plus() {
	if (!called_from_config_files && !called_from_AIO_LS_Pack_Updater) {
		u8 btn = btn_wait();
		if (btn != BTN_VOL_UP) {
			return false;
		}
	}
	return true;
}

bool delete_save_from_nand(const char* savename, bool on_system_part) {
	if (!wait_vol_plus()) {
		return false;
	}
	if (on_system_part) {
		log_printf(LOG_INFO, LOG_MSG_DELETE_SAVE_SYSTEM, savename);
	} else {
		log_printf(LOG_INFO, LOG_MSG_DELETE_SAVE_USER, savename);
	}
	bool success = false;
	LIST_INIT(gpt);

	if (on_system_part) {
		if (!mount_nand_part(&gpt, "SYSTEM", true, true, true, true, NULL, NULL, NULL, NULL)) {
			return false;
		}
	} else {
		if (!mount_nand_part(&gpt, "USER", true, true, true, true, NULL, NULL, NULL, NULL)) {
			return false;
		}
	}

	char buff[27];
	s_printf(buff, "bis:/save/%s", savename);
	FRESULT fr = f_unlink(buff);
	if (fr == FR_OK) {
		gfx_printf("\n\n");
		log_printf(LOG_OK, LOG_MSG_DELETE_FILE_SUCCESS);
		gfx_printf("\n");
		success = true;
		debug_log_write(g_log_messages[LOG_MSG_DELETE_FILE_SUCCESS]);
		debug_log_write("\n");
	} else if (fr == FR_NO_FILE) {
		gfx_printf("\n\n");
		log_printf(LOG_WARN, LOG_MSG_DELETE_FILE_WARNING);
		gfx_printf("\n");
		gfx_printf("\n\n%kFile not found (may already be deleted).\n\n", COLOR_YELLOW);
		success = true;
		debug_log_write(g_log_messages[LOG_MSG_DELETE_FILE_WARNING]);
		debug_log_write("\n");
	} else {
		gfx_printf("\n\n");
		log_printf(LOG_ERR, LOG_MSG_DELETE_FILE_ERROR, (u32)fr);
		gfx_printf("\n");
		debug_log_write(g_log_messages[LOG_MSG_DELETE_FILE_ERROR], (u32)fr);
		debug_log_write("\n");
	}

unmount_nand_part(&gpt, false, true, true, true);
	return success;
}

static ui_pos_t spinner_pos;
static int spinner_timer_count = 0;
static bool spinner_visible = false;
void ui_spinner_begin() {
	gfx_con_getpos(&spinner_pos.x, &spinner_pos.y);
	spinner_timer_count = 0;
	spinner_visible = false;
	
}

void ui_spinner_draw(int pass_count) {
	if (spinner_timer_count != pass_count) {
		spinner_timer_count++;
		return;
	} else {
		spinner_timer_count = 0;
	}
	const char spin_chars[] = "-";

	gfx_con_setpos(spinner_pos.x, spinner_pos.y);
	if (spinner_visible) {
		gfx_printf("%s", spin_chars);
		spinner_visible = false;
	} else {
		gfx_printf(" ");
		spinner_visible = true;
	}
	gfx_con_setpos(spinner_pos.x, spinner_pos.y);
}

void ui_spinner_clear() {
	spinner_timer_count = 0;
	spinner_visible = false;
	gfx_con_setpos(spinner_pos.x, spinner_pos.y);
	gfx_printf(" ");
	gfx_con_setpos(spinner_pos.x, spinner_pos.y);
}

// Helper function to get eMMC ID (hex string from CID serial, matching Hekate format)
bool get_emmc_id(char *emmc_id_out) {
	// Initialize eMMC if not already done
	if (!emmc_storage.initialized && emummc_storage_init_mmc()) {
		return false;
	}

	// Convert CID serial to hexadecimal string (without leading zeros, like Hekate)
	s_printf(emmc_id_out, "%x", emmc_storage.cid.serial);

	emummc_storage_end();
	sd_mount();

	return true;
}

static bool sd_has_enough_space(u64 required_bytes) {
	FATFS *fs;
	DWORD fre_clust;
	u64 free_bytes;

	if (f_getfree("sd:", &fre_clust, &fs) != FR_OK)
		return false;

	free_bytes = (u64)fre_clust * fs->csize * 512;
	return free_bytes >= required_bytes;
}

bool flash_or_dump_part(bool flash, const char *sd_filepath, const char *part_name, bool file_encrypted) {
	bool is_boot = false;
	bool use_bis = false;
	u64 part_size_bytes = 0;
	FRESULT fr;
	FIL fp;
	emmc_part_t part;
	u64 filesize = 0;
	u8 *buff = NULL;

bool return_value = false;

	sd_mount();

	if (flash) {
		log_printf(LOG_INFO, LOG_MSG_FLASH_PARTITION_BEGIN, sd_filepath, part_name);
		debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_BEGIN], sd_filepath, part_name);
		debug_log_write("\n");
		fr = f_open(&fp, sd_filepath, FA_READ);
		if (fr != FR_OK) {
			log_printf(LOG_ERR, LOG_MSG_ERR_OPEN_FILE, sd_filepath);
			debug_log_write(g_log_messages[LOG_MSG_ERR_OPEN_FILE], sd_filepath);
			debug_log_write("\n");
			return return_value;
		}
		filesize = f_size(&fp);
		if (filesize == 0) {
			log_printf(LOG_ERR, LOG_MSG_ERR_EMPTY_FILE);
			debug_log_write(g_log_messages[LOG_MSG_ERR_EMPTY_FILE]);
			debug_log_write("\n");
			f_close(&fp);
			return return_value;
		}
	} else {
	log_printf(LOG_INFO, LOG_MSG_DUMP_PARTITION_BEGIN, part_name, sd_filepath);
	debug_log_write(g_log_messages[LOG_MSG_DUMP_PARTITION_BEGIN], part_name, sd_filepath);
	debug_log_write("\n");
	}

	LIST_INIT(gpt);
	bool test_part;
	if (file_encrypted) {
		test_part = mount_nand_part(&gpt, part_name, true, true, false, false, &part_size_bytes, &is_boot, &use_bis, &part);
	} else {
		test_part = mount_nand_part(&gpt, part_name, true, true, false, true, &part_size_bytes, &is_boot, &use_bis, &part);
	}
	if (!test_part) {
		if (flash) f_close(&fp);
		return return_value;
	}

	if (flash) {
		if (filesize > part_size_bytes) {
			log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_FILE_TO_BIG);
			debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_FILE_TO_BIG]);
			debug_log_write("\n");
			goto cleanup;
		}
		// For safety: require sector-aligned input (avoid implicit padding issues when writing encrypted partitions)
		if ((filesize % EMMC_BLOCKSIZE) != 0) {
			log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_FILE_NOT_ALLIGNED);
			debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_FILE_NOT_ALLIGNED]);
			debug_log_write("\n");
			goto cleanup;
		}
	} else {
		if ((part_size_bytes % EMMC_BLOCKSIZE) != 0) {
			log_printf(LOG_ERR, LOG_MSG_DUMP_PARTITION_NOT_ALLIGNED);
			goto cleanup;
		}
		if (!sd_has_enough_space(part_size_bytes)) {
			log_printf(LOG_ERR, LOG_MSG_dump_PARTITION_FILE_TO_BIG);
			goto cleanup;
		}
		char dirpath[256];
		s_printf(dirpath, "%s", sd_filepath);
		char *last_slash = strrchr(dirpath, '/');
		if (last_slash) {
			*last_slash = 0;
			mkdir_recursive(dirpath);
		}
		fr = f_open(&fp, sd_filepath, FA_WRITE | FA_CREATE_ALWAYS);
		if (fr != FR_OK) {
			log_printf(LOG_ERR, LOG_MSG_ERR_OPEN_FILE, sd_filepath);
			goto cleanup;
		}
	}

	buff = malloc(COPY_BUF_SIZE);
	if (!buff) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		goto cleanup;
	}

	u32 lba_start = is_boot ? 0 : (use_bis ? 0 : part.lba_start);
	u32 curLba = lba_start;
	u64 totalSectorsSrc;
	if (flash) {
		totalSectorsSrc = f_size(&fp) / EMMC_BLOCKSIZE;
	} else {
		totalSectorsSrc = part_size_bytes / EMMC_BLOCKSIZE;;
	}

	ui_spinner_begin();
	while (totalSectorsSrc > 0){
		ui_spinner_draw(1);
		int Res = 0;
		u32 num = MIN(totalSectorsSrc, COPY_BUF_SIZE / EMMC_BLOCKSIZE);

		if (flash) {
			if ((f_read(&fp, buff, num * EMMC_BLOCKSIZE, NULL))){
				log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
				goto cleanup;
				break;
			}
			if (use_bis && file_encrypted) {
				Res = !nx_emmc_bis_write(curLba, num, buff);
			} else  {
				Res = emummc_storage_write(curLba, num, buff);
			}
			if (!Res){
				log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_ERR_PARTITION_WRITE);
				goto cleanup;
				break;
			}
		} else {
			if (use_bis && file_encrypted) {
				Res = nx_emmc_bis_read(curLba, num, buff);
			} else {
				Res = emummc_storage_read(curLba, num, buff);
			}
			if (!Res) {
				log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
				goto cleanup;
			}
			UINT bw;
			fr = f_write(&fp, buff, num * EMMC_BLOCKSIZE, &bw);
			if (fr != FR_OK || bw != num * EMMC_BLOCKSIZE) {
				log_printf(LOG_ERR, LOG_MSG_DUMP_PARTITION_ERR_PARTITION_WRITE);
				goto cleanup;
			}
		}

		curLba += num;
		totalSectorsSrc -= num;
	}

	return_value = true;
	if (flash) {
		log_printf(LOG_OK, LOG_MSG_FLASH_PARTITION_SUCCESS);
		debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_SUCCESS]);
		debug_log_write("\n");
	} else {
		log_printf(LOG_OK, LOG_MSG_DUMP_PARTITION_SUCCESS);
		debug_log_write(g_log_messages[LOG_MSG_DUMP_PARTITION_SUCCESS]);
		debug_log_write("\n");
	}

cleanup:
ui_spinner_clear();
	if (buff) free(buff);
	f_close(&fp);

	unmount_nand_part(&gpt, is_boot, use_bis, true, false);
	return return_value;
}

/*
bool flash_part_from_sd_file(const char *sd_filepath, const char *part_name, bool src_encrypted) {
	// const u32 boot_part_size_bytes = (u64)emmc_storage.ext_csd.boot_mult << 17; // boot size from ext_csd
	bool is_boot = false;
	bool use_bis = false;
	u64 part_size_bytes = 0;
	FRESULT fr;
	FIL fp;
	emmc_part_t part;
	u64 filesize;
	u8 *buff = NULL;

bool return_value = false;

	sd_mount();

	log_printf(LOG_INFO, LOG_MSG_FLASH_PARTITION_BEGIN, sd_filepath, part_name);
	debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_BEGIN], sd_filepath, part_name);
	debug_log_write("\n");

	fr = f_open(&fp, sd_filepath, FA_READ);
	if (fr != FR_OK) {
		log_printf(LOG_ERR, LOG_MSG_ERR_OPEN_FILE, sd_filepath);
		debug_log_write(g_log_messages[LOG_MSG_ERR_OPEN_FILE], sd_filepath);
		debug_log_write("\n");
		return return_value;
	}

	filesize = f_size(&fp);
	if (filesize == 0) {
		log_printf(LOG_ERR, LOG_MSG_ERR_EMPTY_FILE);
		debug_log_write(g_log_messages[LOG_MSG_ERR_EMPTY_FILE]);
		debug_log_write("\n");
		f_close(&fp);
		return return_value;
	}

	LIST_INIT(gpt);
	bool test_part;
	if (src_encrypted) {
		test_part = mount_nand_part(&gpt, part_name, true, true, false, false, &part_size_bytes, &is_boot, &use_bis, &part);
	} else {
		test_part = mount_nand_part(&gpt, part_name, true, true, false, true, &part_size_bytes, &is_boot, &use_bis, &part);
	}
	if (!test_part) {
		f_close(&fp);
		return return_value;
	}

	if (filesize > part_size_bytes) {
		log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_FILE_TO_BIG);
		debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_FILE_TO_BIG]);
		debug_log_write("\n");
		goto cleanup;
	}

	// For safety: require sector-aligned input (avoid implicit padding issues when writing encrypted partitions)
	if ((filesize % EMMC_BLOCKSIZE) != 0) {
		log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_FILE_NOT_ALLIGNED);
		debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_FILE_NOT_ALLIGNED]);
		debug_log_write("\n");
		goto cleanup;
	}

	buff = malloc(COPY_BUF_SIZE);
	if (!buff) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		goto cleanup;
	}

	u32 lba_start = 0;
	// u32 lba_end = 0;
	if (is_boot) {
		// lba_end = (boot_part_size_bytes / EMMC_BLOCKSIZE) - 1;
	} else {
		if (use_bis) {
			// lba_end = part.lba_end - part.lba_start;
		} else {
			lba_start = part.lba_start;
			// lba_end = part.lba_end;
		}
	}
	u32 curLba = lba_start;
	// u32 totalSectorsDest = lba_end - lba_start + 1;
	u64 totalSizeSrc = f_size(&fp);
	u32 totalSectorsSrc = totalSizeSrc / EMMC_BLOCKSIZE;

	ui_spinner_begin();
	while (totalSectorsSrc > 0){
		ui_spinner_draw(1);
		u32 num = MIN(totalSectorsSrc, COPY_BUF_SIZE / EMMC_BLOCKSIZE);

		if ((f_read(&fp, buff, num * EMMC_BLOCKSIZE, NULL))){
			log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
			goto cleanup;
			break;
		}

		int writeRes = 0;
		if (use_bis && src_encrypted) {
			writeRes = !nx_emmc_bis_write(curLba, num, buff);
		} else  {
			writeRes = emummc_storage_write(curLba, num, buff);
		}

		if (!writeRes){
			log_printf(LOG_ERR, LOG_MSG_FLASH_PARTITION_ERR_PARTITION_WRITE);
			goto cleanup;
			break;
		}

		curLba += num;
		totalSectorsSrc -= num;
	}

	return_value = true;
	log_printf(LOG_OK, LOG_MSG_FLASH_PARTITION_SUCCESS);
	debug_log_write(g_log_messages[LOG_MSG_FLASH_PARTITION_SUCCESS]);
	debug_log_write("\n");

cleanup:
ui_spinner_clear();
	if (buff) free(buff);
	f_close(&fp);

	unmount_nand_part(&gpt, is_boot, use_bis, true, false);
	return return_value;
}

bool dump_part_to_sd_file(const char *sd_filepath, const char *part_name, bool dst_encrypted) {
	bool is_boot = false;
	bool use_bis = false;
	u64 part_size_bytes = 0;
	FRESULT fr;
	FIL fp;
	emmc_part_t part;
	u8 *buff = NULL;
	bool return_value = false;

	sd_mount();

	log_printf(LOG_INFO, LOG_MSG_DUMP_PARTITION_BEGIN, part_name, sd_filepath);
	debug_log_write(g_log_messages[LOG_MSG_DUMP_PARTITION_BEGIN], part_name, sd_filepath);
	debug_log_write("\n");

	LIST_INIT(gpt);
	bool ok;
	if (dst_encrypted) {
		ok = mount_nand_part(&gpt, part_name, true, true, false, false, &part_size_bytes, &is_boot, &use_bis, &part);
	} else {
		ok = mount_nand_part(&gpt, part_name, true, true, false, true, &part_size_bytes, &is_boot, &use_bis, &part);
	}

	if (!ok)
		return false;

	if ((part_size_bytes % EMMC_BLOCKSIZE) != 0) {
		log_printf(LOG_ERR, LOG_MSG_DUMP_PARTITION_NOT_ALLIGNED);
		goto cleanup;
	}

	if (!sd_has_enough_space(part_size_bytes)) {
		log_printf(LOG_ERR, LOG_MSG_dump_PARTITION_FILE_TO_BIG);
		goto cleanup;
	}

	char dirpath[256];
	s_printf(dirpath, "%s", sd_filepath);
	char *last_slash = strrchr(dirpath, '/');
	if (last_slash) {
		*last_slash = 0;
		mkdir_recursive(dirpath);
	}

	fr = f_open(&fp, sd_filepath, FA_WRITE | FA_CREATE_ALWAYS);
	if (fr != FR_OK) {
		log_printf(LOG_ERR, LOG_MSG_ERR_OPEN_FILE, sd_filepath);
		goto cleanup;
	}

	buff = malloc(COPY_BUF_SIZE);
	if (!buff) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		goto cleanup;
	}

	u32 lba_start = is_boot ? 0 : (use_bis ? 0 : part.lba_start);
	u32 curLba = lba_start;
	u64 remaining = part_size_bytes / EMMC_BLOCKSIZE;

	ui_spinner_begin();
	while (remaining > 0) {
		ui_spinner_draw(1);
		u32 num = MIN(remaining, COPY_BUF_SIZE / EMMC_BLOCKSIZE);

		int readRes;
		if (use_bis && dst_encrypted) {
			readRes = nx_emmc_bis_read(curLba, num, buff);
		} else {
			readRes = emummc_storage_read(curLba, num, buff);
		}

		if (!readRes) {
			log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
			goto cleanup;
		}

		UINT bw;
		fr = f_write(&fp, buff, num * EMMC_BLOCKSIZE, &bw);
		if (fr != FR_OK || bw != num * EMMC_BLOCKSIZE) {
			log_printf(LOG_ERR, LOG_MSG_DUMP_PARTITION_ERR_PARTITION_WRITE);
			goto cleanup;
		}

		curLba += num;
		remaining -= num;
	}

	return_value = true;
	log_printf(LOG_OK, LOG_MSG_DUMP_PARTITION_SUCCESS);
	debug_log_write(g_log_messages[LOG_MSG_DUMP_PARTITION_SUCCESS]);
	debug_log_write("\n");

cleanup:
	ui_spinner_clear();
	if (buff) free(buff);
	f_close(&fp);
	unmount_nand_part(&gpt, is_boot, use_bis, true, false);
	return return_value;
}
*/

bool f_transfer_from_nands(const char *file_path, bool on_system_part) {
	if (menu_on_sysnand) {
		if (on_system_part) {
			log_printf(LOG_INFO, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SYS_TO_EMU_SYSTEM, file_path);
		} else {
			log_printf(LOG_INFO, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SYS_TO_EMU_USER, file_path);
		}
	} else {
		if (on_system_part) {
			log_printf(LOG_INFO, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_EMU_TO_SYS_SYSTEM, file_path);
		} else {
			log_printf(LOG_INFO, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_EMU_TO_SYS_USER, file_path);
		}
	}

	if (!sysmmc_available) {
		log_printf(LOG_ERR, LOG_MSG_ERR_SYSMMC_NOT_AVAILABLE);
		return false;
	}
	if (!emummc_available) {
		log_printf(LOG_ERR, LOG_MSG_ERR_EMUMMC_NOT_AVAILABLE);
		return false;
	}

	/*
	if (menu_on_sysnand) {
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
	} else {
		h_cfg.emummc_force_disable = false;
		emu_cfg.enabled = true;
	}
	*/

	LIST_INIT(gpt);

	if (on_system_part) {
		if (!mount_nand_part(&gpt, "SYSTEM", true, true, true, true, NULL, NULL, NULL, NULL)) {
			return false;
		}
	} else {
		if (!mount_nand_part(&gpt, "USER", true, true, true, true, NULL, NULL, NULL, NULL)) {
			return false;
		}
	}

	FIL fs;
	FRESULT res;
	UINT br;
	char full_file_path[256];
	s_printf(full_file_path, "bis:/%s", file_path);

	res = f_open(&fs, full_file_path, FA_READ);
	if (res != FR_OK) {
		log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_OPEN_SRC);
		unmount_nand_part(&gpt, false, true, true, true);
		return false;
	}

	u32 buf_size = RAM_COPY_BUF_SIZE;
	u8 *ram_buf = (u8 *)malloc(buf_size);
	if (!ram_buf) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		f_close(&fs);
		unmount_nand_part(&gpt, false, true, true, true);
		return false;
	}

	u32 total_read = 0;
	while (1) {
		UINT want = (buf_size - total_read) > 4096 ? 4096 : (buf_size - total_read);
		if (want == 0) {
			log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_TOO_LARGE);
			free(ram_buf);
			f_close(&fs);
			unmount_nand_part(&gpt, false, true, true, true);
			return false;
		}

		res = f_read(&fs, ram_buf + total_read, want, &br);
		if (res != FR_OK) {
			log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
			free(ram_buf);
			f_close(&fs);
			unmount_nand_part(&gpt, false, true, true, true);
			return false;
		}
		if (br == 0) {
			break;
		}
		total_read += br;
	}

	f_close(&fs);
	unmount_nand_part(&gpt, false, true, false, true);

	if (total_read == 0) {
		log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_NO_DATA_READ);
		free(ram_buf);
		return false;
	}

	if (total_read >= buf_size) {
		log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_FILE_TRUNCATED);
		free(ram_buf);
		return false;
	}

	bool orig_emummc_force_disable = h_cfg.emummc_force_disable;
	bool orig_emu_enabled = emu_cfg.enabled;
	if (menu_on_sysnand) {
		h_cfg.emummc_force_disable = false;
		emu_cfg.enabled = true;
	} else {
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
	}

	if (on_system_part) {
		if (!mount_nand_part(&gpt, "SYSTEM", false, false, true, true, NULL, NULL, NULL, NULL)) {
			free(ram_buf);
			h_cfg.emummc_force_disable = orig_emummc_force_disable;
			emu_cfg.enabled = orig_emu_enabled;
			return false;
		}
	} else {
		if (!mount_nand_part(&gpt, "USER", false, false, true, true, NULL, NULL, NULL, NULL)) {
			free(ram_buf);
			h_cfg.emummc_force_disable = orig_emummc_force_disable;
			emu_cfg.enabled = orig_emu_enabled;
			return false;
		}
	}

	FIL fd;
	res = f_open(&fd, full_file_path, FA_WRITE | FA_CREATE_ALWAYS);
	if (res != FR_OK) {
		log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_OPEN_DST);
		unmount_nand_part(&gpt, false, true, true, true);
		free(ram_buf);
		h_cfg.emummc_force_disable = orig_emummc_force_disable;
		emu_cfg.enabled = orig_emu_enabled;
		return false;
	}

	u32 written = 0;
	const u32 write_chunk = COPY_BUF_SIZE;
	while (written < total_read) {
		u32 left = total_read - written;
		u32 do_write = left > write_chunk ? write_chunk : left;
		UINT bw;
		res = f_write(&fd, ram_buf + written, do_write, &bw);
		if (res != FR_OK || bw != do_write) {
			log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_WRITE_DST);
			break;
		}
		written += bw;
	}

	f_close(&fd);
	unmount_nand_part(&gpt, false, true, true, true);

	free(ram_buf);
	h_cfg.emummc_force_disable = orig_emummc_force_disable;
	emu_cfg.enabled = orig_emu_enabled;

	if (written != total_read) {
		log_printf(LOG_ERR, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_ERR_INCOMPLET_TRANSFER);
		return false;
	}

	log_printf(LOG_OK, LOG_MSG_FILE_TRANSFERT_FROM_NANDS_SUCCESS);
	return true;
}

bool is_autorcm_enabled() {
	if (!physical_emmc_ok) {
		return true;
	}
	u8 mod0, mod1;
	u8 *tempbuf = (u8 *)malloc(0x200);

	// Get the correct RSA modulus byte masks.
	nx_emmc_get_autorcm_masks(&mod0, &mod1);

	// Get 1st RSA modulus.
	emmc_set_partition(EMMC_BOOT0);
	sdmmc_storage_read(&emmc_storage, 0x200 / EMMC_BLOCKSIZE, 1, tempbuf);

	// Check if 2nd byte of modulus is correct.
	bool enabled = false;
	if (tempbuf[0x11] == mod1)
		if (tempbuf[0x10] != mod0)
			enabled = true;

	free(tempbuf);

	return enabled;
}

void save_screenshot_and_go_back(const char* filename) {
	if (called_from_config_files || called_from_AIO_LS_Pack_Updater) {
		return;
	}
	gfx_printf("\n%kPress VOL+ to save a screenshot\n or another button to return to the menu.\n\n", COLOR_WHITE);
	u8 btn = btn_wait();
	if (btn == BTN_VOL_UP) {
		int res = save_fb_to_bmp(filename);
		if (!res) {
			gfx_printf("%kScreenshot sd:/LockSmith-RCM/screenshots/%s saved.", COLOR_GREEN, filename);
		} else {
			EPRINTF("Screenshot failed.");
		}
		gfx_printf("\n%kPress a button to return to the menu.", COLOR_WHITE);
		btn_wait();
	}
}

void display_title() {
	if (menu_on_sysnand) {
		gfx_printf("[%kLockSmith-RCM v%d.%d.%d] - SYSNAND WORK%k\n\n",
			COLOR_RED, LS_VER_MJ, LS_VER_MN, LS_VER_HF, COLOR_SOFT_WHITE);
	} else {
		gfx_printf("[%kLockSmith-RCM v%d.%d.%d] - EMUNAND WORK%k\n\n",
			COLOR_BLUE, LS_VER_MJ, LS_VER_MN, LS_VER_HF, COLOR_SOFT_WHITE);
	}
}

void cls() {
	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);
	display_title();
}

FRESULT easy_rename(const char* old, const char* new) {
	log_printf(LOG_INFO, LOG_MSG_FILE_RENAME, old, new);
	FRESULT res = FR_OK;
	if (f_stat(old, NULL) != FR_OK) {
		return res;
	}
	if (f_stat(old, NULL) == FR_OK) {
		if (f_stat(new, NULL) == FR_OK) {
			res = f_unlink(new);
		}
		if (res == FR_OK) {
			res = f_rename(old, new);
		}
	}
	return res;
}

FRESULT f_copy(const char *src, const char *dst, BYTE *buf)
{
	FIL fs, fd;
	FRESULT fr;
	UINT r, w;

	debug_log_write("Copying file '%s' -> '%s'\n", src, dst);

	fr = f_open(&fs, src, FA_READ);
	if (fr != FR_OK)
		return fr;

	fr = f_open(&fd, dst, FA_WRITE | FA_CREATE_ALWAYS);
	if (fr != FR_OK) {
		f_close(&fs);
		return fr;
	}

	bool null_buf = false;
	if (buf == NULL) {
		null_buf = true;
		buf = malloc(COPY_BUF_SIZE);
		if (!buf) {
			log_printf(LOG_ERR, LOG_MSG_malloc_error);
			return FR_NOT_ENOUGH_CORE;
		}
	}

	ui_spinner_begin();
	do {
		ui_spinner_draw(1);
		fr = f_read(&fs, buf, COPY_BUF_SIZE, &r);
		if (fr != FR_OK) {
			log_printf(LOG_ERR, LOG_MSG_ERR_FILE_READ);
			break;
		}

		if (r == 0)
			break;

		fr = f_write(&fd, buf, r, &w);
		if (fr != FR_OK || w != r) {
			fr = FR_DISK_ERR;
			break;
		}
	} while (1);

	f_sync(&fd);

	ui_spinner_clear();
	f_close(&fs);
	f_close(&fd);
	FILINFO fno;
	f_stat(dst, &fno);
	if (null_buf) free(buf);
	return fr;
}

#define MAX_STACK_DEPTH 12
typedef struct {
	char path[MAX_PATH_LEN];
	BYTE stage;   // 0 = scan, 1 = delete dir itself
} dir_stack_t;

typedef struct {
	DIR  dir[MAX_STACK_DEPTH];
	char src_stack[MAX_STACK_DEPTH][MAX_PATH_LEN];
	char dst_stack[MAX_STACK_DEPTH][MAX_PATH_LEN];
	char src[MAX_PATH_LEN];
	char dst[MAX_PATH_LEN];
	FILINFO fno[MAX_STACK_DEPTH];
	BYTE *copy_buf;
} cp_rm_ctx_t;

FRESULT f_cp_or_rm_rf(const char *src_root, const char *dst_root)
{
	const bool do_copy = (dst_root != NULL);
	FRESULT res;

	cp_rm_ctx_t *ctx = malloc(sizeof(cp_rm_ctx_t));
	if (!ctx) {
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		return FR_NOT_ENOUGH_CORE;
	}

	ctx->copy_buf = malloc(COPY_BUF_SIZE);
	if (!ctx->copy_buf) {
		free(ctx);
		log_printf(LOG_ERR, LOG_MSG_malloc_error);
		return FR_NOT_ENOUGH_CORE;
	}

	int sp = 0;

	s_printf(ctx->src_stack[0], "%s", src_root);

	if (do_copy) {
		log_printf(LOG_INFO, LOG_MSG_FOLDER_COPY_BEGIN, src_root, dst_root);
		debug_log_write(g_log_messages[LOG_MSG_FOLDER_COPY_BEGIN], src_root, dst_root);
		debug_log_write("\n");
		s_printf(ctx->dst_stack[0], "%s", dst_root);
		f_mkdir(dst_root);
	} else {
		log_printf(LOG_INFO, LOG_MSG_FOLDER_DELETE_BEGIN, src_root);
		debug_log_write(g_log_messages[LOG_MSG_FOLDER_DELETE_BEGIN], src_root);
		debug_log_write("\n");
	}

	res = f_opendir(&ctx->dir[0], ctx->src_stack[0]);
	if (res != FR_OK) {
		log_printf(LOG_ERR, LOG_MSG_ERR_OPEN_FOLDER, ctx->src_stack[0]);
		debug_log_write(g_log_messages[LOG_MSG_ERR_OPEN_FOLDER], ctx->src_stack[0]);
		debug_log_write("\n");
		goto out;
	}

	while (sp >= 0) {
		char name[256 + 1];
		res = f_readdir(&ctx->dir[sp], &ctx->fno[sp]);
		if (res != FR_OK || ctx->fno[sp].fname[0] == 0) {
			f_closedir(&ctx->dir[sp]);
			if (!do_copy && sp > 0)
				f_unlink(ctx->src_stack[sp]);
			sp--;
			continue;
		}

	s_printf(name, "%s", ctx->fno[sp].fname);

		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		s_printf(ctx->src, "%s/%s", ctx->src_stack[sp], name);
		if (do_copy)
			s_printf(ctx->dst, "%s/%s", ctx->dst_stack[sp], name);

		if (ctx->fno[sp].fattrib & AM_DIR) {
			if (sp + 1 >= MAX_STACK_DEPTH)
				continue;

			if (do_copy)
				f_mkdir(ctx->dst);

			s_printf(ctx->src_stack[sp + 1], "%s", ctx->src);
			if (do_copy)
				s_printf(ctx->dst_stack[sp + 1], "%s", ctx->dst);

			if (f_opendir(&ctx->dir[sp + 1], ctx->src) == FR_OK)
				sp++;
			continue;
		}

		if (do_copy) {
			res = f_copy(ctx->src, ctx->dst, ctx->copy_buf);
			if (res != FR_OK) {
				log_printf(LOG_ERR, LOG_MSG_FOLDER_COPY_ERROR, ctx->src, res);
				debug_log_write(g_log_messages[LOG_MSG_FOLDER_COPY_ERROR], ctx->src, res);
				debug_log_write("\n");
				goto out;
			}
		} else {
			debug_log_write("Delete %s\n", ctx->src);
			f_unlink(ctx->src);
		}
	}

	if (!do_copy) {
		log_printf(LOG_INFO, LOG_MSG_FOLDER_DELETE_END);
		debug_log_write(g_log_messages[LOG_MSG_FOLDER_DELETE_END]);
		debug_log_write("\n\n");
		f_unlink(src_root);
	} else {
		log_printf(LOG_INFO, LOG_MSG_FOLDER_COPY_END);
		debug_log_write(g_log_messages[LOG_MSG_FOLDER_COPY_END]);
		debug_log_write("\n\n");
	}

out:
	free(ctx->copy_buf);
	free(ctx);
	return FR_OK;
}

int save_fb_to_bmp(const char* filename)
{
	// Disallow screenshots if less than 2s passed.
	static u32 timer = 0;
	if (get_tmr_ms() < timer)
		return 1;

	const u32 file_size = 0x384000 + 0x36;
	u8 *bitmap = malloc(file_size);
	u32 *fb = malloc(0x384000);
	u32 *fb_ptr = gfx_ctxt.fb;

	// Reconstruct FB for bottom-top, portrait bmp.
	for (int y = 1279; y > -1; y--)
	{
		for (u32 x = 0; x < 720; x++)
			fb[y * 720 + x] = *fb_ptr++;
	}

	memcpy(bitmap + 0x36, fb, 0x384000);

	typedef struct _bmp_t
	{
		u16 magic;
		u32 size;
		u32 rsvd;
		u32 data_off;
		u32 hdr_size;
		u32 width;
		u32 height;
		u16 planes;
		u16 pxl_bits;
		u32 comp;
		u32 img_size;
		u32 res_h;
		u32 res_v;
		u64 rsvd2;
	} __attribute__((packed)) bmp_t;

	bmp_t *bmp = (bmp_t *)bitmap;

	bmp->magic    = 0x4D42;
	bmp->size     = file_size;
	bmp->rsvd     = 0;
	bmp->data_off = 0x36;
	bmp->hdr_size = 40;
	bmp->width    = 720;
	bmp->height   = 1280;
	bmp->planes   = 1;
	bmp->pxl_bits = 32;
	bmp->comp     = 0;
	bmp->img_size = 0x384000;
	bmp->res_h    = 2834;
	bmp->res_v    = 2834;
	bmp->rsvd2    = 0;

	sd_mount();

	// f_mkdir("sd:/LockSmith-RCM");
	// f_mkdir("sd:/LockSmith-RCM/screenshots");
	char path[256];
	s_printf(path, "sd:/LockSmith-RCM/screenshots/%s.bmp", filename);
	mkdir_recursive(path);
	int i = 0;
	if (f_stat(path, NULL) == FR_OK) {
		for (i = 1; i <= 99999; i++) {
			// int v = i;
			// char num[6];
			// num[5] = 0;
			// num[4] = '0' + (v % 10); v /= 10;
			// num[3] = '0' + (v % 10); v /= 10;
			// num[2] = '0' + (v % 10); v /= 10;
			// num[1] = '0' + (v % 10); v /= 10;
			// num[0] = '0' + (v % 10);
			// s_printf(path, "sd:/LockSmith-RCM/screenshots/%s_%s.bmp", filename, num);
			s_printf(path, "sd:/LockSmith-RCM/screenshots/%s_%d.bmp", filename, i);
			if (f_stat(path, NULL) != FR_OK) {
				break;
			}
		}
	}

	// Save screenshot and log.
	int res = -1;
	if (i <= 100000) res = sd_save_to_file(bitmap, file_size, path);

	// sd_unmount();

	free(bitmap);
	free(fb);

	// Set timer to 2s.
	timer = get_tmr_ms() + 2000;

	return res;
}