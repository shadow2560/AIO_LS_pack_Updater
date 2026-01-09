/*
 * Copyright (c) 2018 naehrwert
 *
 * Copyright (c) 2018-2021 CTCaer
 * Copyright (c) 2019-2021 shchmue
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "config.h"
#include <display/di.h>
#include <gfx_utils.h>
#include "gfx/tui.h"
#include <libs/fatfs/ff.h>
#include <mem/heap.h>
#include <mem/minerva.h>
#include <power/bq24193.h>
#include <power/max17050.h>
#include <power/max77620.h>
#include <rtc/max77620-rtc.h>
#include <soc/bpmp.h>
#include <soc/fuse.h>
#include <soc/hw_init.h>
#include <soc/timer.h>
#include "storage/emummc.h"
#include <storage/emmc.h>
#include "storage/nx_emmc_bis.h"
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/dirlist.h>
#include <utils/ini.h>
#include <utils/list.h>
#include <utils/sprintf.h>
#include <utils/util.h>

#include "fuse_check/fuse_check.h"
#include "keys/keys.h"
#include "prodinfogen/build_prodinfo.h"
#include "unbrick/unbrick.h"

#include "tools.h"
#include "gfx/display_log_bin.h"
#include "gfx/messages.h"

hekate_config h_cfg;
boot_cfg_t __attribute__((section ("._boot_cfg"))) b_cfg;
const volatile ipl_ver_meta_t __attribute__((section ("._ipl_version"))) ipl_ver = {
	.magic = LS_MAGIC,
	.version           = (LS_VER_MJ + '0') | ((LS_VER_MN + '0') << 8) | ((LS_VER_HF + '0') << 16) | ((LS_VER_RL) << 24),
	.rcfg.rsvd_flags   = 0,
	.rcfg.bclk_t210    = BPMP_CLK_LOWER_BOOST,
	.rcfg.bclk_t210b01 = BPMP_CLK_DEFAULT_BOOST
};

volatile nyx_storage_t *nyx_str = (nyx_storage_t *)NYX_STORAGE_ADDR;

const char* DONOR_PRODINFO_FILENAME = "sd:/switch/donor_prodinfo.bin";

bool physical_emmc_ok = true;
bool sysmmc_available = true;
bool emummc_available = true;
bool called_from_config_files = false;
bool called_from_AIO_LS_Pack_Updater = false;
bool menu_on_sysnand = false;
bool bis_from_console = true;
char emmc_id[9] = {0};  // 8 hex chars + null terminator
int emunand_count = 0;
int prev_sec_emunand = 0;
int cur_sec_emunand = 0;
emunand_entry_t *emunands = NULL;
bool have_sd = false;
bool have_minerva = false;
u32 COPY_BUF_SIZE;

typedef struct {
	const char *path;
	void (*func)(void);
	bool emunand;
} auto_action_t;

static void run_auto_action(const auto_action_t *a)
{
	sd_mount();
	if (f_stat(a->path, NULL) != FR_OK) {
		return;
	}

	if (!called_from_config_files) {
		called_from_config_files = true;
		log_init();
		if (bis_from_console) {
			log_printf(LOG_INFO, LOG_MSG_BIS_VIA_CONSOLE);
		} else {
			log_printf(LOG_INFO, LOG_MSG_BIS_VIA_FILE);
		}
		if (sysmmc_available) {
			log_printf(LOG_INFO, LOG_MSG_SYSMMC_AVAILABLE);
		}
		if (emummc_available) {
			log_printf(LOG_INFO, LOG_MSG_EMUMMC_AVAILABLE);
			if (emu_cfg.sector <= 0) {
				log_printf(LOG_INFO, LOG_MSG_EMUMMC_PATH, emu_cfg.path);
			} else {
				log_printf(LOG_INFO, LOG_MSG_EMUMMC_SECTOR, (u32)emu_cfg.sector);
			}
			log_printf(LOG_INFO, LOG_MSG_EMUMMC_NINTENDO_PATH, emu_cfg.nintendo_path);
		}
		log_printf(LOG_INFO, LOG_MSG_BATCH_BEGIN);
		cls();
	}

	if (a->emunand && !emummc_available) {
		log_printf(LOG_ERR, LOG_MSG_ERR_EMUMMC_NOT_AVAILABLE);
		return;
	}

	if (!a->emunand && !sysmmc_available) {
		log_printf(LOG_ERR, LOG_MSG_ERR_SYSMMC_NOT_AVAILABLE);
		return;
	}

	h_cfg.emummc_force_disable = !a->emunand;
	emu_cfg.enabled = a->emunand;
	menu_on_sysnand = !a->emunand;
	get_emmc_id(emmc_id);

	if (menu_on_sysnand) {
		log_printf(LOG_WARN, LOG_MSG_NEXT_BATCH_ON_SYSNAND);
	} else {
		log_printf(LOG_WARN, LOG_MSG_NEXT_BATCH_ON_EMUNAND);
	}

	a->func();

	sd_mount();
	f_unlink(a->path);
}

// This is a safe and unused DRAM region for our payloads.
#define RELOC_META_OFF      0x7C
#define PATCHED_RELOC_SZ    0x94
#define PATCHED_RELOC_STACK 0x40007000
#define PATCHED_RELOC_ENTRY 0x40010000
#define EXT_PAYLOAD_ADDR    0xC0000000
#define RCM_PAYLOAD_ADDR    (EXT_PAYLOAD_ADDR + ALIGN(PATCHED_RELOC_SZ, 0x10))
#define COREBOOT_END_ADDR   0xD0000000
#define COREBOOT_VER_OFF    0x41
#define CBFS_DRAM_EN_ADDR   0x4003e000
#define  CBFS_DRAM_MAGIC    0x4452414D // "DRAM"

static void *coreboot_addr;

void reloc_patcher(u32 payload_dst, u32 payload_src, u32 payload_size)
{
	memcpy((u8 *)payload_src, (u8 *)IPL_LOAD_ADDR, PATCHED_RELOC_SZ);

	reloc_meta_t *relocator = (reloc_meta_t *)(payload_src + RELOC_META_OFF);

	relocator->start = payload_dst - ALIGN(PATCHED_RELOC_SZ, 0x10);
	relocator->stack = PATCHED_RELOC_STACK;
	relocator->end   = payload_dst + payload_size;
	relocator->ep    = payload_dst;

	if (payload_size == 0x7000)
	{
		memcpy((u8 *)(payload_src + ALIGN(PATCHED_RELOC_SZ, 0x10)), coreboot_addr, 0x7000); //Bootblock
		*(vu32 *)CBFS_DRAM_EN_ADDR = CBFS_DRAM_MAGIC;
	}
}

int launch_payload(char *path, bool clear_screen)
{
	if (clear_screen)
		gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);
	if (!path)
		return 1;

	if (sd_mount())
	{
		FIL fp;
		if (f_open(&fp, path, FA_READ))
		{
			gfx_con.mute = false;
			EPRINTFARGS("Payload file is missing!\n(%s)", path);

			goto out;
		}

		// Read and copy the payload to our chosen address
		void *buf;
		u32 size = f_size(&fp);

		if (size < 0x30000)
			buf = (void *)RCM_PAYLOAD_ADDR;
		else
		{
			coreboot_addr = (void *)(COREBOOT_END_ADDR - size);
			buf = coreboot_addr;
			if (h_cfg.t210b01)
			{
				f_close(&fp);

				gfx_con.mute = false;
				EPRINTF("Coreboot not allowed on Mariko!");

				goto out;
			}
		}

		if (f_read(&fp, buf, size, NULL))
		{
			f_close(&fp);

			goto out;
		}

		f_close(&fp);

		sd_end();

		if (size < 0x30000)
		{
			reloc_patcher(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(size, 0x10));

			hw_deinit(false, byte_swap_32(*(u32 *)(buf + size - sizeof(u32))));
		}
		else
		{
			reloc_patcher(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, 0x7000);

			// Get coreboot seamless display magic.
			u32 magic = 0;
			char *magic_ptr = buf + COREBOOT_VER_OFF;
			memcpy(&magic, magic_ptr + strlen(magic_ptr) - 4, 4);
			hw_deinit(true, magic);
		}

		// Some cards (Sandisk U1), do not like a fast power cycle. Wait min 100ms.
		sdmmc_storage_init_wait_sd();

		void (*ext_payload_ptr)() = (void *)EXT_PAYLOAD_ADDR;

		// Launch our payload.
		(*ext_payload_ptr)();
	}

out:
	sd_end();
	return 1;
}

/*
void launch_tools()
{
	const u8 max_entries = 61;

	dirlist_t *filelist = NULL;
	char *file_sec = NULL;
	char *dir = NULL;
	ment_t *ments = NULL;

	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	ments = malloc(sizeof(ment_t) * (max_entries + 3));
	if (!ments)
	{
		EPRINTF("Out of memory (ments)\n");
		btn_wait();
		return;
	}

	if (!sd_mount())
		goto out;

	dir = (char*)malloc(256);
	if (!dir)
	{
		EPRINTF("Out of memory (dir)\n");
		goto out;
	}

	memcpy(dir, "sd:/bootloader/payloads", 24);

	filelist = dirlist(dir, NULL, 0);
	if (!filelist)
		goto out;

	u32 i = 0;
	u32 i_off = 2;
	u32 color_idx = 0;

	ments[0].type = MENT_BACK;
	ments[0].caption = "Back";
	ments[0].enabled = 1;
	ments[0].color = colors[(color_idx++) % 6];

	ments[1].type = MENT_CHGLINE;
	ments[1].enabled = 0;
	ments[1].color = colors[(color_idx++) % 6];

	if (!f_stat("sd:/atmosphere/reboot_payload.bin", NULL))
	{
		if (i_off < max_entries + 2)
		{
			ments[i_off].type = INI_CHOICE;
			ments[i_off].caption = "reboot_payload.bin";
			ments[i_off].data = "sd:/atmosphere/reboot_payload.bin";
			ments[i_off].color = COLOR_GREEN;
			ments[i_off].enabled = 1;
			i_off++;
		}
	}

	while (filelist->name[i] && (i + i_off) < (max_entries + 2))
	{
		ments[i + i_off].type = INI_CHOICE;
		ments[i + i_off].caption = filelist->name[i];
		ments[i + i_off].data = filelist->name[i];
		ments[i + i_off].color = COLOR_WHITE;
		ments[i + i_off].enabled = 1;
		i++;
	}

	if (i == 0 && i_off == 2)
	{
		EPRINTF("No payloads or modules found.");
		goto out;
	}

	memset(&ments[i + i_off], 0, sizeof(ment_t));

	menu_t menu = {
		.ents = ments,
		.caption = "Choose a file to launch",
		.x = 0,
		.y = 0
	};

	const char *sel = tui_do_menu(&menu);
	if (!sel)
		goto out;

	file_sec = bdk_strdup(sel);
	if (!file_sec) {
		EPRINTF("Out of memory (file_sec)\n");
		goto out;
	}

	if (memcmp(file_sec, "sd:/", 4) != 0) {
		s_printf(dir, "sd:/bootloader/payloads/%s", file_sec);
	} else {
		s_printf(dir, "%s", file_sec);
	}

	launch_payload(dir, true);
	EPRINTF("Failed to launch payload.");

out:
	sd_end();

	if (file_sec) free(file_sec);
	if (filelist) free(filelist);
	if (dir) free(dir);
	if (ments) free(ments);

	btn_wait();
}
*/

void launch_tools()
{
	u8 max_entries = 61;
	dirlist_t *filelist = NULL;
	char *file_sec = NULL;
	char *dir = NULL;

	ment_t *ments = (ment_t *)malloc(sizeof(ment_t) * (max_entries + 3));

	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	if (sd_mount())
	{
		dir = (char *)malloc(256);

		memcpy(dir, "sd:/bootloader/payloads", 24);

		filelist = dirlist(dir, NULL, 0);

		u32 i = 0;
		u32 i_off = 2;

		if (filelist)
		{
			// Build configuration menu.
			u32 color_idx = 0;

			ments[0].type = MENT_BACK;
			ments[0].caption = "Back";
			ments[0].enabled = 1;
			ments[0].color = colors[(color_idx++) % 6];
			ments[1].type = MENT_CHGLINE;
			ments[1].color = colors[(color_idx++) % 6];
			ments[1].enabled = 0;
			if (!f_stat("sd:/atmosphere/reboot_payload.bin", NULL))
			{
				ments[i_off].type = INI_CHOICE;
				ments[i_off].caption = "reboot_payload.bin";
				ments[i_off].color = COLOR_GREEN;
				ments[i_off].data = "sd:/atmosphere/reboot_payload.bin";
				ments[i_off].enabled = 1;
				i_off++;
			}

			while (true)
			{
				if (i > max_entries || !filelist->name[i])
					break;
				ments[i + i_off].type = INI_CHOICE;
				ments[i + i_off].caption = filelist->name[i];
				ments[i + i_off].color = COLOR_WHITE;
				ments[i + i_off].data = filelist->name[i];
				ments[i + i_off].enabled = 1;

				i++;
			}
		}

		if (i > 0)
		{
			memset(&ments[i + i_off], 0, sizeof(ment_t));
			menu_t menu = { ments, "Choose a file to launch", 0, 0 };

			file_sec = (char *)tui_do_menu(&menu);

			if (!file_sec)
			{
				free(ments);
				free(dir);
				free(filelist);
				// sd_end();

				return;
			}
		}
		else
			EPRINTF("No payloads or modules found.");

		free(ments);
		free(filelist);
	}
	else
	{
		free(ments);
		goto out;
	}

	if (file_sec)
	{
		if (memcmp("sd:/", file_sec, 4) != 0)
		{
			memcpy(dir + strlen(dir), "/", 2);
			memcpy(dir + strlen(dir), file_sec, strlen(file_sec) + 1);
		}
		else
			memcpy(dir, file_sec, strlen(file_sec) + 1);

		launch_payload(dir, true);
		EPRINTF("Failed to launch payload.");
	}

out:
	// sd_end();
	free(dir);

	btn_wait();
}

static void launch_hekate()
{
	sd_mount();
	if (!f_stat("bootloader/update.bin", NULL))
		launch_payload("bootloader/update.bin", false);
	else
	{
		gfx_clear_grey(0x1B);
		gfx_con_setpos(0, 0);
		EPRINTF("bootloader/update.bin not found!");
		gfx_printf("\n%kPress any button to return to menu.", colors[0]);
		btn_wait();
	}
}

static void launch_reboot_payload()
{
	sd_mount();
	if (!f_stat("payload.bin", NULL))
		launch_payload("payload.bin", false);
	else
	{
		gfx_clear_grey(0x1B);
		gfx_con_setpos(0, 0);
		EPRINTF("payload.bin not found on SD root!");
		gfx_printf("\n%kPress any button to return to menu.", colors[0]);
		btn_wait();
	}
}

static void _ipl_reload()
{
	hw_deinit(false, 0);

	// Reload payload.
	void (*ipl_ptr)() = (void *)IPL_LOAD_ADDR;
	(*ipl_ptr)();
}

power_state_t STATE_POWER_OFF           = POWER_OFF_RESET;
power_state_t STATE_REBOOT_FULL         = POWER_OFF_REBOOT;
power_state_t STATE_REBOOT_RCM          = REBOOT_RCM;
power_state_t STATE_REBOOT_BYPASS_FUSES = REBOOT_BYPASS_FUSES;

bool init_and_verify_bis_keys(bool from_file);

static void set_bis_keys_from_console() {
	cls();
	init_and_verify_bis_keys(false);
}

static void set_bis_keys_from_file() {
	cls();
	init_and_verify_bis_keys(true);
}

void switch_nand_work() {
	if (!emummc_available) {
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
		menu_on_sysnand = true;
		return;
	}
	if (menu_on_sysnand) {
		select_and_apply_emunand();
		if (emunand_count > 1 && prev_sec_emunand != cur_sec_emunand) {
			if (bis_from_console) {
				set_bis_keys_from_console();
			} else {
				set_bis_keys_from_file();
			}
		}
	} else {
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
		menu_on_sysnand = true;
	}
	get_emmc_id(emmc_id);
}

static void keys_dump() {
	cls();
	dump_keys(false);
}

static void dump_mariko_partial_keys() {
	cls();
	/*
	gfx_printf("%kThis dumps the results of writing zeros over consecutive 32-bit portions of each keyslot, the results of which can then be bruteforced quickly on a computer to recover keys from unreadable keyslots.\n\n", colors[1]);
	gfx_printf("%kThis includes the Mariko KEK and BEK as well as the unique SBK.\n\n", colors[2]);
	gfx_printf("%kThese are not useful for most users but are included for archival purposes.\n\n", colors[3]);
	gfx_printf("%kWarning: this wipes keyslots!\n", colors[4]);
	gfx_printf("The console must be completely restarted!\n");
	gfx_printf("Modchip must run again to fix the keys!\n\n");
	gfx_printf("%kPress \"vol+\" to launch the dump or any other keys to cancel.\n", COLOR_WHITE);
	*/
	gfx_printf("%kThis dumps the results of writing zeros over consecutive 32-bit portions of each keyslot, the results of which can then be bruteforced quickly on a computer to recover keys from unreadable keyslots.\n\n \
		%kThis includes the Mariko KEK and BEK as well as the unique SBK.\n\n \
	%kThese are not useful for most users but are included for archival purposes.\n\n \
		%kWarning: this wipes keyslots!\n \
		The console must be completely restarted!\n \
		Modchip must run again to fix the keys!\n\n \
	%kPress \"vol+\" to launch the dump or any other keys to cancel.\n", colors[1], colors[2], colors[3], colors[4], COLOR_WHITE);
	if (!wait_vol_plus()) {
		return;
	}
	if (h_cfg.t210b01) {
		int res = save_mariko_partial_keys(0, 16, false);
		if (res == 0 || res == 3) {
			// Force shutdown the console as the keyslots have been invalidated.
			gfx_printf("\n%kPress a button to shutdown the console.", COLOR_ORANGE);
			btn_wait();
			power_set_state(POWER_OFF_RESET);
			while (true)
				bpmp_halt();
		}
	}
}

static void build_prodinfo_from_scratch() {
	// return;
	cls();
	build_prodinfo(NULL, true);
}

static void build_prodinfo_from_donor() {
	// return;
	cls();
	build_prodinfo(DONOR_PRODINFO_FILENAME, true);
}

static void build_and_flash_prodinfo_from_scratch() {
	// return;
	cls();
	log_printf(LOG_INFO, LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_SCRATCH_BEGIN);
	if (!wait_vol_plus()) {
		return;
	}
	build_prodinfo_and_flash(true);
}

static void build_and_flash_prodinfo_from_donor() {
	// return;
	cls();
	log_printf(LOG_INFO, LOG_MSG_PRODINFO_BUILD_AND_FLASH_FROM_DONOR_BEGIN);
	if (!wait_vol_plus()) {
		return;
	}
	build_prodinfo_and_flash(false);
}

static void emmchacgen_package_flash() {
	cls();
	log_printf(LOG_INFO, LOG_MSG_UNBRICK_BEGIN);
	if (!wait_vol_plus()) {
		return;
	}
	unbrick("sd:/cdj_package_files", false);
}

static void emmchacgen_package_flash_with_wip() {
	cls();
	log_printf(LOG_INFO, LOG_MSG_UNBRICK_AND_WIP_BEGIN);
	if (!wait_vol_plus()) {
		return;
	}
	unbrick("sd:/cdj_package_files", true);
}

static void dump_prodinfo() {
	cls();
	char path[256];
	if (menu_on_sysnand) {
		s_printf(path, "sd:/LockSmith-RCM/backups/%s/PRODINFO_sysnand_dec.bin", emmc_id);
	} else {
		s_printf(path, "sd:/LockSmith-RCM/backups/%s/PRODINFO_emunand_dec.bin", emmc_id);
	}
	flash_or_dump_part(false, path, "PRODINFO", false);
}

static void restore_prodinfo() {
	cls();
		log_printf(LOG_INFO, LOG_MSG_RESTORE_PRODINFO_BEGIN);
	if (!wait_vol_plus()) {
		return;
	}
	char path[256];
	if (menu_on_sysnand) {
		s_printf(path, "sd:/LockSmith-RCM/backups/%s/PRODINFO_sysnand_dec.bin", emmc_id);
	} else {
		s_printf(path, "sd:/LockSmith-RCM/backups/%s/PRODINFO_emunand_dec.bin", emmc_id);
	}
	flash_or_dump_part(true, path, "PRODINFO", false);
}

ment_t ment_top[] = {
	MDEF_HANDLER("Switch nand work", switch_nand_work, COLOR_YELLOW),
	MDEF_HANDLER("Use console's biskeys", set_bis_keys_from_console, COLOR_GREEN),
	MDEF_HANDLER("Use biskeys from file (\"sd:/LockSmith-RCM/prod.keys\" file)", set_bis_keys_from_file, COLOR_RED),
	MDEF_CAPTION("---------------", COLOR_WHITE),
	MDEF_HANDLER("Dump keys", keys_dump, COLOR_TURQUOISE),
	MDEF_HANDLER("Dump Amiibo keys", derive_amiibo_keys, COLOR_TURQUOISE),
	MDEF_HANDLER("Dump Mariko Partials keys", dump_mariko_partial_keys, COLOR_TURQUOISE),
	MDEF_HANDLER("Fuse check", fuse_check, COLOR_TURQUOISE),
	MDEF_CAPTION("---------------", COLOR_WHITE),
	// MDEF_HANDLER("Dump PRODINFO", dump_prodinfo, COLOR_TURQUOISE),
	// MDEF_HANDLER("Restore PRODINFO", restore_prodinfo, COLOR_RED),
	MDEF_HANDLER("Build PRODINFO file from scratch", build_prodinfo_from_scratch, COLOR_TURQUOISE),
	MDEF_HANDLER("Build PRODINFO file from donor", build_prodinfo_from_donor, COLOR_TURQUOISE),
	MDEF_HANDLER("Build and flash PRODINFO file from scratch", build_and_flash_prodinfo_from_scratch, COLOR_RED),
	MDEF_HANDLER("Build and flash PRODINFO file from donor", build_and_flash_prodinfo_from_donor, COLOR_RED),
	MDEF_CAPTION("---------------", COLOR_WHITE),
	MDEF_HANDLER("Downgrade fix", fix_downgrade, COLOR_ORANGE),
	MDEF_HANDLER("Remove parental control", remove_parental_control, COLOR_ORANGE),
	MDEF_HANDLER("Wip nand", wip_nand, COLOR_ORANGE),
	MDEF_HANDLER("Remove erpt save", del_erpt_save, COLOR_RED),
	MDEF_HANDLER("Synchronize joycons from working nand", sync_joycons_between_nands, COLOR_RED),
	MDEF_HANDLER("Unbrick with EmmcHacGen package\n in \"sd:/cdj_package_files\"", emmchacgen_package_flash, COLOR_RED),
	MDEF_HANDLER("Wip and unbrick with EmmcHacGen package\n in \"sd:/cdj_package_files\"", emmchacgen_package_flash_with_wip, COLOR_RED),
	MDEF_CAPTION("---------------", COLOR_WHITE),
	MDEF_HANDLER("Payloads...", launch_tools, COLOR_GREEN),
	MDEF_HANDLER("Reboot to Hekate", launch_hekate, COLOR_TURQUOISE),
	MDEF_HANDLER("Reboot to Payload.bin", launch_reboot_payload, COLOR_TURQUOISE),
	MDEF_CAPTION("---------------", COLOR_WHITE),
	MDEF_HANDLER_EX("Reboot (OFW bypass fuses)", &STATE_REBOOT_BYPASS_FUSES, power_set_state_ex, COLOR_TURQUOISE),
	MDEF_HANDLER_EX("Reboot (OFW)", &STATE_REBOOT_FULL, power_set_state_ex, COLOR_RED),
	MDEF_HANDLER_EX("Reboot (RCM)", &STATE_REBOOT_RCM, power_set_state_ex, COLOR_ORANGE),
	MDEF_HANDLER("Reboot LockSmith-RCM", _ipl_reload, COLOR_TURQUOISE),
	MDEF_HANDLER_EX("Power off", &STATE_POWER_OFF, power_set_state_ex, COLOR_TURQUOISE),
	MDEF_END()
};

menu_t menu_top = { ment_top, NULL, 0, 0 };

void grey_out_menu_item(ment_t *menu)
{
	// menu->type = MENT_CAPTION;
	// menu->color = 0xFF555555;
	// menu->handler = NULL;
	menu->enabled = 0;
}

void reset_menu(menu_t *menu)
{
	for (int i = 0; menu->ents[i].type != MENT_END; i++)
	{
		switch (menu->ents[i].type)
		{
		case MENT_HANDLER:
		case MENT_MENU:
		case MENT_DATA:
		case MENT_BACK:
		case MENT_HDLR_RE:
			menu->ents[i].enabled = 1;
			break;

		default:
			menu->ents[i].enabled = 0;
			break;
		}
	}
}

#define AUTO_BASE "sd:/LockSmith-RCM/"
static const auto_action_t auto_actions[] = {
	{ AUTO_BASE "fix_dg_sysnand", fix_downgrade, false },
	{ AUTO_BASE "fix_dg_emunand", fix_downgrade, true  },
	{ AUTO_BASE "wip_sysnand",    wip_nand,     false },
	{ AUTO_BASE "wip_emunand",    wip_nand,     true  },
	{ AUTO_BASE "rm_parental_control_sysnand",    remove_parental_control,     false },
	{ AUTO_BASE "rm_parental_control_emunand",    remove_parental_control,     true  },
	{ AUTO_BASE "unbrick_sysnand",    emmchacgen_package_flash,     false },
	{ AUTO_BASE "unbrick_emunand",    emmchacgen_package_flash,     true  },
	{ AUTO_BASE "unbrick_and_wip_sysnand",    emmchacgen_package_flash_with_wip,     false },
	{ AUTO_BASE "unbrick_and_wip_emunand",    emmchacgen_package_flash_with_wip,     true  },
	{ AUTO_BASE "rm_erpt_sysnand",    del_erpt_save,     false },
	{ AUTO_BASE "rm_erpt_emunand",    del_erpt_save,     true  },
	{ AUTO_BASE "sync_joycons_sysnand",    sync_joycons_between_nands,     false },
	{ AUTO_BASE "sync_joycons_emunand",    sync_joycons_between_nands,     true  },
	{ AUTO_BASE "prodinfogen_flash_scratch_sysnand",    build_and_flash_prodinfo_from_scratch,     false },
	{ AUTO_BASE "prodinfogen_flash_scratch_emunand",    build_and_flash_prodinfo_from_scratch,     true  },
	{ AUTO_BASE "prodinfogen_flash_donor_sysnand",    build_and_flash_prodinfo_from_donor,     false },
	{ AUTO_BASE "prodinfogen_flash_donor_emunand",    build_and_flash_prodinfo_from_donor,     true  },
	{ AUTO_BASE "dump_keys_sysnand",    keys_dump,     false },
	{ AUTO_BASE "dump_keys_emunand",    keys_dump,     true  },
	{ AUTO_BASE "dump_amiibo_keys",    derive_amiibo_keys,     false  },
};

void mask_emmc_need_for_menu() {
	grey_out_menu_item(&ment_top[5]);
	grey_out_menu_item(&ment_top[6]);
	grey_out_menu_item(&ment_top[7]);
	grey_out_menu_item(&ment_top[9]);
	grey_out_menu_item(&ment_top[10]);
	grey_out_menu_item(&ment_top[11]);
	grey_out_menu_item(&ment_top[12]);
	grey_out_menu_item(&ment_top[14]);
	grey_out_menu_item(&ment_top[15]);
	grey_out_menu_item(&ment_top[16]);
	grey_out_menu_item(&ment_top[17]);
	grey_out_menu_item(&ment_top[18]);
	grey_out_menu_item(&ment_top[19]);
	grey_out_menu_item(&ment_top[20]);
}

void mask_file_load_keys_need_for_menu() {
	grey_out_menu_item(&ment_top[2]);
	grey_out_menu_item(&ment_top[4]);
	grey_out_menu_item(&ment_top[5]);
	grey_out_menu_item(&ment_top[6]);
	grey_out_menu_item(&ment_top[7]);
	grey_out_menu_item(&ment_top[9]);
	grey_out_menu_item(&ment_top[10]);
	grey_out_menu_item(&ment_top[11]);
	grey_out_menu_item(&ment_top[12]);
}

void mask_no_sd_menu_options() {
	grey_out_menu_item(&ment_top[0]);
	grey_out_menu_item(&ment_top[1]);
	grey_out_menu_item(&ment_top[2]);
	grey_out_menu_item(&ment_top[4]);
	grey_out_menu_item(&ment_top[5]);
	grey_out_menu_item(&ment_top[6]);
	grey_out_menu_item(&ment_top[9]);
	grey_out_menu_item(&ment_top[10]);
	grey_out_menu_item(&ment_top[11]);
	grey_out_menu_item(&ment_top[12]);
	grey_out_menu_item(&ment_top[18]);
		grey_out_menu_item(&ment_top[19]);
		grey_out_menu_item(&ment_top[20]);
		grey_out_menu_item(&ment_top[22]);
		grey_out_menu_item(&ment_top[23]);
		grey_out_menu_item(&ment_top[24]);
}

void mask_specific_menu_options() {
	// Grey out "switch nand work " and "sync joycons" if emunand or sysnand not present.
	if (!emummc_available || !sysmmc_available) {
		grey_out_menu_item(&ment_top[0]); // switch between sysnand and emunand work
		grey_out_menu_item(&ment_top[18]); // Syncronize joycons
	}

	if (f_stat("sd:/LockSmith-RCM/prod.keys", NULL)) {
		grey_out_menu_item(&ment_top[2]);
	}

	// Grey out dump mariko specific keys option if not on Mariko  console.
	if (!h_cfg.t210b01) {
		grey_out_menu_item(&ment_top[6]);
	}

	// Grey out PRODINFO build (and build and flash) from donor if  donor file  not found.
	if (f_stat(DONOR_PRODINFO_FILENAME, NULL)) {
		grey_out_menu_item(&ment_top[10]);
		grey_out_menu_item(&ment_top[12]);
	}

	// Grey out unbrick via EmmcHacGen package options if   files and folders not found.
	if (f_stat("cdj_package_files", NULL) || f_stat("cdj_package_files/BCPKG2-1-Normal-Main.bin", NULL) || f_stat("cdj_package_files/BCPKG2-2-Normal-Sub.bin", NULL) || f_stat("cdj_package_files/BCPKG2-3-SafeMode-Main.bin", NULL) || f_stat("cdj_package_files/BCPKG2-4-SafeMode-Sub.bin", NULL) || f_stat("cdj_package_files/BOOT0.bin", NULL) || f_stat("cdj_package_files/BOOT1.bin", NULL) || f_stat("cdj_package_files/SYSTEM/Contents/placehld", NULL) || f_stat("cdj_package_files/SYSTEM/Contents/registered", NULL) || f_stat("cdj_package_files/SYSTEM/save", NULL)) {
		grey_out_menu_item(&ment_top[19]);
		grey_out_menu_item(&ment_top[20]);
	}

	// Grey out Hekate reboot if update.bin not found.
	if (f_stat("bootloader/update.bin", NULL)) {
		grey_out_menu_item(&ment_top[23]);
	}

	// Grey out Payload.bin reboot if not found.
	if (f_stat("payload.bin", NULL)) {
		grey_out_menu_item(&ment_top[24]);
	}

	// Grey out reboot to RCM option if on Mariko or patched console, else grey out reboot OFW options if auto-rcm enabled
	if (h_cfg.t210b01 || h_cfg.rcm_patched) {
		grey_out_menu_item(&ment_top[28]);
	} else {
		if (is_autorcm_enabled()) {
			grey_out_menu_item(&ment_top[26]);
			grey_out_menu_item(&ment_top[27]);
		}
	}

// Grey out reboot OFW options if sysnand not founded
	if (!sysmmc_available) {
		grey_out_menu_item(&ment_top[26]);
		grey_out_menu_item(&ment_top[27]);
	}
}

static bool internal_call = false;
bool init_and_verify_bis_keys(bool from_file) {
	if (!internal_call) {
		reset_menu(&menu_top);
	}
	gfx_con.mute = true;
	bool keys_derived;
	if (!from_file) {
		internal_call = true;
		bis_from_console = true;
		keys_derived = prepare_bis_keys(false, NULL);
		grey_out_menu_item(&ment_top[1]);
	} else {
		keys_derived = prepare_bis_keys(true, NULL);
		if (keys_derived) {
			bis_from_console = false;
			mask_file_load_keys_need_for_menu();
		} else {
			bis_from_console = true;
			internal_call = true;
			keys_derived = prepare_bis_keys(false, NULL);
			grey_out_menu_item(&ment_top[1]);
			grey_out_menu_item(&ment_top[2]);
		}
	}
	gfx_con.mute = false;
	if (!keys_derived) {
		debug_log_write("Key derivation error\n");
		mask_emmc_need_for_menu();
		grey_out_menu_item(&ment_top[4]);
		cls();
EPRINTF("\nFailed to derive keys!\nMost menu function will be disabled and auto functions will be completly disabled.\nPress any key to continue...\n");
		btn_wait();
		gfx_clear_grey(0x1B);
		gfx_con_setpos(0, 0);
		internal_call = false;
		return false;
	}
	LIST_INIT(gpt);
	bool orig_emummc_force_disable = h_cfg.emummc_force_disable;
	bool orig_emu_enabled = emu_cfg.enabled;

	// Check sysnand
	if (physical_emmc_ok) {
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
		if (!mount_nand_part(&gpt, "PRODINFO", true, true, false, true, NULL, NULL, NULL, NULL)) {
			debug_log_write("Sysnand PRODINFO read  error\n");
			sysmmc_available = false;
			menu_on_sysnand = false;
		} else {
			unmount_nand_part(&gpt, false, true, true, false);
		}
		if (!mount_nand_part(&gpt, "SYSTEM", true, true, true, true, NULL, NULL, NULL, NULL)) {
			debug_log_write("Sysnand SYSTEM read  error\n");
			sysmmc_available = false;
			menu_on_sysnand = false;
		} else {
			unmount_nand_part(&gpt, false, true, true, true);
			sysmmc_available = true;
		}
	}

	// Check emunand
	if (emu_cfg.sector != 0 || emu_cfg.path != NULL) {
		h_cfg.emummc_force_disable = false;
		emu_cfg.enabled = true;
		if (!mount_nand_part(&gpt, "PRODINFO", true, true, false, true, NULL, NULL, NULL, NULL)) {
			debug_log_write("Emunand PRODINFO read  error\n");
			emummc_available = false;
			menu_on_sysnand = true;
		} else {
			unmount_nand_part(&gpt, false, true, true, false);
		}
		if (!mount_nand_part(&gpt, "SYSTEM", true, true, true, true, NULL, NULL, NULL, NULL)) {
			debug_log_write("Emunand SYSTEM read  error\n");
			emummc_available = false;
			menu_on_sysnand = true;
		} else {
			unmount_nand_part(&gpt, false, true, true, true);
			emummc_available = true;
		}
	}

	if (!sysmmc_available && !emummc_available && internal_call) {
		mask_emmc_need_for_menu();
		cls();
EPRINTF("\nFailed to init EMMC or to discover an emunand config!\nMost menu function will be disabled and auto functions will be completly disabled.\nPress any key to continue...\n");
		btn_wait();
		gfx_clear_grey(0x1B);
		gfx_con_setpos(0, 0);
		h_cfg.emummc_force_disable = true;
		emu_cfg.enabled = false;
		menu_on_sysnand = true;
		internal_call = false;
		return false;
	}
	if (!sysmmc_available && !emummc_available && !internal_call) {
		if (from_file) {
			internal_call = true;
			bool test_fallback = init_and_verify_bis_keys(false);
			if (!test_fallback) {
				mask_emmc_need_for_menu();
				cls();
		EPRINTF("\nFailed to init EMMC or to discover an emunand config!\nMost menu function will be disabled and auto functions will be completly disabled.\nPress any key to continue...\n");
				btn_wait();
				gfx_clear_grey(0x1B);
				gfx_con_setpos(0, 0);
				h_cfg.emummc_force_disable = true;
				emu_cfg.enabled = false;
				menu_on_sysnand = true;
				internal_call = false;
				return false;
			}
		}
	}

	h_cfg.emummc_force_disable = orig_emummc_force_disable;
	emu_cfg.enabled = orig_emu_enabled;

	mask_specific_menu_options();
	internal_call = false;
	get_emmc_id(emmc_id);
	return true;
}

void init_payload() {
	mask_specific_menu_options();
	grey_out_menu_item(&ment_top[1]);

	if (f_stat("sd:/switch/AIO_LS_pack_Updater/called_via_AIO_LS_pack_Updater", NULL) == FR_OK) {
		called_from_AIO_LS_Pack_Updater = true;
		f_unlink("sd:/switch/AIO_LS_pack_Updater/called_via_AIO_LS_pack_Updater");
	}

	if (called_from_AIO_LS_Pack_Updater) {
		easy_rename("atmosphere/stratosphere.romfs.temp", "atmosphere/stratosphere.romfs");
		easy_rename("atmosphere/package3.temp", "atmosphere/package3");
		easy_rename("switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro.temp", "switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro");
		f_unlink("switch/AIO_LS_pack_Updater/daybreak_auto.nro");
		f_unlink("switch/AIO_LS_pack_Updater/Daybreak-cli.nro");
		if (f_stat("payload.bin.temp", NULL) == FR_OK) {
			f_copy("bootloader/update.bin", "payload.bin.temp", NULL);
		}
		if (f_stat("payload.bin.temp", NULL) == FR_OK) {
			f_copy("atmosphere/reboot_payload.bin", "payload.bin.temp", NULL);
		}
		easy_rename("payload.bin.temp", "payload.bin");
	}

	bool keys_derived;
	if (f_stat("sd:/LockSmith-RCM/prod.keys", NULL)) {
		keys_derived = init_and_verify_bis_keys(false);
	} else {
		keys_derived = init_and_verify_bis_keys(true);
	}
	if (!have_minerva) {
		cls();
		gfx_printf("%kMinerva not enabled, functions will be slower down.\n%kPress any key to continue", COLOR_ORANGE, COLOR_WHITE);
		btn_wait();
	}
	if (!have_sd) {
		mask_specific_menu_options();
		mask_no_sd_menu_options();
		cls();
		gfx_printf("%kSD can't be mounted, most function will be disabled and batch will not be executed.\n%kPress any key to continue", COLOR_RED, COLOR_WHITE);
		btn_wait();
		return;
	}
	if (keys_derived) {
		for (u32 i = 0; i < ARRAY_SIZE(auto_actions); i++)
			run_auto_action(&auto_actions[i]);
		if (called_from_config_files && !called_from_AIO_LS_Pack_Updater) {
			log_printf(LOG_INFO, LOG_MSG_BATCH_END);
			cls();
			gfx_con_setpos(100, 640);
			gfx_printf("%kPress any key to view batch log.", COLOR_YELLOW);
			btn_wait();
			msleep(1000);
			show_log_viewer();
			log_free();
		}
	}
}

void check_physical_nand() {
	emmc_initialize(true);
	if (!emmc_set_partition(EMMC_BOOT0)) {
		physical_emmc_ok = false;
		sysmmc_available = false;
	}
}

extern void pivot_stack(u32 stack_top);

void ipl_main()
{
	// Override DRAM ID if needed.
	if (ipl_ver.rcfg.rsvd_flags & RSVD_FLAG_DRAM_8GB)
		fuse_force_8gb_dramid();

	// Do initial HW configuration. This is compatible with consecutive reruns without a reset.
	hw_init();

	// Pivot the stack under IPL. (Only max 4KB is needed).
	pivot_stack(IPL_LOAD_ADDR);

	// Tegra/Horizon configuration goes to 0x80000000+, package2 goes to 0xA9800000, we place our heap in between.
	heap_init((void*)IPL_HEAP_START);

	// Set bootloader's default configuration.
	set_default_configuration();

	// Initialize display.
	display_init();

	// Overclock BPMP.
	bpmp_clk_rate_set(h_cfg.t210b01 ? ipl_ver.rcfg.bclk_t210b01 : ipl_ver.rcfg.bclk_t210);

	// Mount SD Card.
	if (!sd_mount()) {
		// h_cfg.errors |= ERR_SD_BOOT_EN;
		have_sd = false;
	} else {
		// h_cfg.errors |= 0;
		have_sd = true;
	}

	// Check if watchdog was fired previously.
	if (watchdog_fired())
		goto skip_lp0_minerva_config;

	// Enable watchdog protection to avoid SD corruption based hanging in LP0/Minerva config.
	watchdog_start(5000000 / 2, TIMER_FIQENABL_EN); // 5 seconds.

	// Train DRAM and switch to max frequency.
	if (minerva_init((minerva_str_t *)&nyx_str->minerva)) { //!TODO: Add Tegra210B01 support to minerva.
		// h_cfg.errors |= ERR_LIBSYS_MTC;
		have_minerva = false;
	} else {
		have_minerva = true;
	}

	// Disable watchdog protection.
	watchdog_end();

skip_lp0_minerva_config:
	COPY_BUF_SIZE = (!have_minerva) ? 0x10000 : 0x800000;
	// Initialize display window, backlight and gfx console.
	u32 *fb = display_init_window_a_pitch();
	gfx_init_ctxt(fb, 720, 1280, 720);
	gfx_con_init();

// Initialize backlight PWM.
	display_backlight_pwm_init();

	minerva_change_freq(FREQ_800);

	// Load emuMMC configuration from SD.
	emummc_load_cfg();
	debug_log_start();
	build_emunand_list();

check_physical_nand();

	select_and_apply_emunand();

	gfx_clear_grey(0x1B);
	display_backlight_brightness(150, 1000);

init_payload();

	if (called_from_config_files || called_from_AIO_LS_Pack_Updater) {
		// If the console is a patched or Mariko unit
		if (h_cfg.t210b01 || h_cfg.rcm_patched) {
			power_set_state(POWER_OFF_REBOOT);
		} else {
			if (f_stat("payload.bin", NULL) == FR_OK)
				launch_payload("payload.bin", false);

			if (f_stat("bootloader/update.bin", NULL) == FR_OK)
				launch_payload("bootloader/update.bin", false);

			if (f_stat("atmosphere/reboot_payload.bin", NULL) == FR_OK)
				launch_payload("atmosphere/reboot_payload.bin", false);

			EPRINTF("Failed to launch payload.");
		}
	} else {
		while (true) {
			tui_do_menu(&menu_top);
		}
	}

	// Halt BPMP if we managed to get out of execution.
	while (true)
	bpmp_halt();
}
