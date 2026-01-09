#include <string.h>
#include <ctype.h>
#include "fuse_check.h"
#include "../config.h"
#include <gfx_utils.h>
#include "../gfx/tui.h"
#include "../keys/keys.h"
#include <libs/fatfs/ff.h>
#include <sec/se.h>
#include <soc/fuse.h>
#include <soc/timer.h>
#include "../storage/emummc.h"
#include <storage/emmc.h>
#include "../storage/nx_emmc_bis.h"
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/sprintf.h>

#include "../tools.h"

const int entries_per_page = 15;

// Fuse-to-firmware mapping (from switchbrew.org/wiki/Fuses)
typedef struct {
	u8 major_min, minor_min;
	u8 major_max, minor_max;
	u8 prod_fuses;
	u8 dev_fuses;
} fuse_info_t;

static const fuse_info_t fuse_info[] = {
	{1,0,1,0, 1, 0},
	{2,0,2,3, 2, 0},
	{3,0,3,0, 3, 1},
	{3,1,3,2, 4, 1},
	{4,0,4,1, 5, 1},
	{5,0,5,1, 6, 1},
	{6,0,6,1, 7, 1},
	{6,2,6,2, 8, 1},
	{7,0,8,1, 9, 1},
	{8,1,8,1,10,1},
	{9,0,9,1,11,1},
	{9,1,9,2,12,1},
	{10,0,10,2,13,1},
	{11,0,12,1,14,1},
	{12,2,13,1,15,1},
	{13,2,14,1,16,1},
	{15,0,15,1,17,1},
	{16,0,16,1,18,1},
	{17,0,18,1,19,1},
	{19,0,19,1,20,1},
	{20,0,20,5,21,1},
	{21,0,21,1,22,1},
};

#define FUSE_INFO_COUNT (sizeof(fuse_info) / sizeof(fuse_info[0]))

// Unified database (NCA + Fuse Count)
typedef struct {
	u8 major;
	u8 minor;
	u8 patch;
	const char *nca;
} nca_map_t;

static const nca_map_t nca_db[] = {
	{21, 1, 0, "738da79326689ef4b72f702693bfc48a.nca"},
	{21, 0, 1, "e7273dd5b560d0ba282fc64206fecb56.nca"},
	{21, 0, 0, "4b0130c8b9d2174a6574f6247655acc0.nca"},

	{20, 5, 0, "23ce01f1fc55e55a783162d456e5ca58.nca"},
	{20, 4, 0, "c413a3965b7a3d4c9bfa5fb47a43dbcc.nca"},
	{20, 3, 0, "bc668b6d31f199b568d08e26d0b22cbe.nca"},
	{20, 2, 0, "8fefd5281d05b908f6537ac080dd55b9.nca"},
	{20, 1, 5, "5e67272eb312e5801f055fdbf9f12e39.nca"},
	{20, 1, 1, "5cb1ecaa7a31fe7ba1c37836a030ccb0.nca"},
	{20, 1, 0, "f58f9b84fba8df8227600d98cf326b5e.nca"},
	{20, 0, 1, "85872c8fc9e2d971b3848e454b408d1f.nca"},
	{20, 0, 0, "3a00a7cb68e2c6fd75bb6528a0a9cb66.nca"},

	{19, 0, 1, "79e18e7cbd5de5b700f054bb95e95efd.nca"},
	{19, 0, 0, "983b2ae2be700012c8a4c5ac9dd53e70.nca"},

	{18, 1, 0, "3d2e5985a13bbb3865739ce9dba73d2a.nca"},
	{18, 0, 1, "1515c2d9d495a230d364d7430ccf065d.nca"},
	{18, 0, 0, "65fbec7b2e2b9ba035233bdc24c5de6b.nca"},

	{17, 0, 1, "70c354afdf16bfb8322fbd7bf55db443.nca"},
	{17, 0, 0, "78bf4c8a7c585849d21d5bab580fff22.nca"},

	{16, 1, 0, "272fbf87314711813350d35ce275399a.nca"},
	{16, 0, 3, "c8828752f1e50e4fbd98f16544c3f6a6.nca"},
	{16, 0, 2, "d1b29e04b7816088a9aa0c3d6e307578.nca"},
	{16, 0, 1, "2496586f6a532bca8d35f0e877d8515f.nca"},
	{16, 0, 0, "93c146c1764caba2d5807580ff8d9554.nca"},

	{15, 0, 1, "f3e83267d2e6b135d1c9916679d24465.nca"},
	{15, 0, 0, "b5fb9334bd413d47d347d0c0e70ffbae.nca"},

	{14, 1, 2, "ae7f71c311913f1ca0ac599020f98b34.nca"},
	{14, 1, 1, "509cc8065a8c38f33ca6bb16223e6bb7.nca"},
	{14, 1, 0, "db493a87efd5f3b5b5d539c4d8f92f4e.nca"},
	{14, 0, 0, "d61042295220d7ac450d6ec839123700.nca"},

	{13, 2, 1, "9eb7dd136e156361dc6368f812175e90.nca"},
	{13, 2, 0, "6ab4d9b617765d1a40fba67fea5fc544.nca"},
	{13, 1, 0, "e9a8046639a10d656ea0e92254d7b8f6.nca"},
	{13, 0, 0, "bf2337ee88bd9f963a33b3ecbbc3732a.nca"},

	{12, 1, 0, "9d9d83d68d9517f245f3e8cd7f93c416.nca"},
	{12, 0, 3, "a1863a5c0e1cedd442f5e60b0422dc15.nca"},
	{12, 0, 2, "63d928b5a3016fe8cc0e76d2f06f4e98.nca"},
	{12, 0, 1, "e65114b456f9d0b566a80e53bade2d89.nca"},
	{12, 0, 0, "bd4185843550fbba125b20787005d1d2.nca"},

	{11, 0, 1, "56211c7a5ed20a5332f5cdda67121e37.nca"},
	{11, 0, 0, "594c90bcdbcccad6b062eadba0cd0e7e.nca"},

	{10, 2, 0, "26325de4db3909e0ef2379787c7e671d.nca"},
	{10, 1, 1, "5077973537f6735b564dd7475b779f87.nca"},
	{10, 1, 0, "fd1faed0ca750700d254c0915b93d506.nca"},
	{10, 0, 4, "34728c771299443420820d8ae490ea41.nca"},
	{10, 0, 3, "5b1df84f88c3334335bbb45d8522cbb4.nca"},
	{10, 0, 2, "e951bc9dedcd54f65ffd83d4d050f9e0.nca"},
	{10, 0, 1, "36ab1acf0c10a2beb9f7d472685f9a89.nca"},
	{10, 0, 0, "5625cdc21d5f1ca52f6c36ba261505b9.nca"},

	{9, 2, 0, "09ef4d92bb47b33861e695ba524a2c17.nca"},
	{9, 1, 0, "c5fbb49f2e3648c8cfca758020c53ecb.nca"},
	{9, 0, 1, "fd1ffb82dc1da76346343de22edbc97c.nca"},
	{9, 0, 0, "a6af05b33f8f903aab90c8b0fcbcc6a4.nca"},

	{8, 1, 1, "e9bb0602e939270a9348bddd9b78827b.nca"},
	{8, 1, 1, "724d9b432929ea43e787ad81bf09ae65.nca"},
	{8, 1, 0, "7eedb7006ad855ec567114be601b2a9d.nca"},
	{8, 0, 1, "6c5426d27c40288302ad616307867eba.nca"},
	{8, 0, 0, "4fe7b4abcea4a0bcc50975c1a926efcb.nca"},

	{7, 0, 1, "e6b22c40bb4fa66a151f1dc8db5a7b5c.nca"},
	{7, 0, 0, "c613bd9660478de69bc8d0e2e7ea9949.nca"},

	{6, 2, 0, "6dfaaf1a3cebda6307aa770d9303d9b6.nca"},
	{6, 1, 0, "1d21680af5a034d626693674faf81b02.nca"},
	{6, 0, 1, "663e74e45ffc86fbbaeb98045feea315.nca"},
	{6, 0, 0, "258c1786b0f6844250f34d9c6f66095b.nca"},
	{6, 0, 0, "286e30bafd7e4197df6551ad802dd815.nca"},

	{5, 1, 0, "fce3b0ea366f9c95fe6498b69274b0e7.nca"},
	{5, 0, 2, "c5758b0cb8c6512e8967e38842d35016.nca"},
	{5, 0, 1, "7f5529b7a092b77bf093bdf2f9a3bf96.nca"},
	{5, 0, 0, "faa857ad6e82f472863e97f810de036a.nca"},

	{4, 1, 0, "77e1ae7661ad8a718b9b13b70304aeea.nca"},
	{4, 0, 1, "d0e5d20e3260f3083bcc067483b71274.nca"},
	{4, 0, 0, "f99ac61b17fdd5ae8e4dda7c0b55132a.nca"},

	{3, 0, 2, "704129fc89e1fcb85c37b3112e51b0fc.nca"},
	{3, 0, 1, "9a78e13d48ca44b1987412352a1183a1.nca"},
	{3, 0, 0, "7bef244b45bf63efb4bf47a236975ec6.nca"},

	{2, 3, 0, "d1c991c53a8a9038f8c3157a553d876d.nca"},
	{2, 2, 0, "7f90353dff2d7ce69e19e07ebc0d5489.nca"},
	{2, 1, 0, "e9b3e75fce00e52fe646156634d229b4.nca"},
	{2, 0, 0, "7a1f79f8184d4b9bae1755090278f52c.nca"},

	{1, 0, 0, "a1b287e07f8455e8192f13d0e45a2aaf.nca"},
};

static u8 get_burnt_fuses() {
	u8 fuse_count = 0;
	u32 fuse_odm6 = fuse_read_odm(6);
	u32 fuse_odm7 = fuse_read_odm(7);

	for (u32 i = 0; i < 32; i++) {
		if ((fuse_odm6 >> i) & 1)
			fuse_count++;
	}

	for (u32 i = 0; i < 32; i++) {
		if ((fuse_odm7 >> i) & 1)
			fuse_count++;
	}

	return fuse_count;
}

u8 get_required_fuses(u8 major, u8 minor) {
	for (int i = 0; i < sizeof(fuse_info) / sizeof(fuse_info_t); i++) {
		if (major >= fuse_info[i].major_min && major <= fuse_info[i].major_max) {
			if (major > fuse_info[i].major_min || minor >= fuse_info[i].minor_min) {
				if (major < fuse_info[i].major_max || minor <= fuse_info[i].minor_max) {
					bool is_dev = fuse_read_hw_state() == FUSE_NX_HW_STATE_DEV;
					if (is_dev) {
						return fuse_info[i].dev_fuses;
					} else {
						return fuse_info[i].prod_fuses;
					}
				}
			}
		}
	}
	return 1;
}

static int get_burnt_fuses_idx(u8 burnt_fuses) {
	bool is_dev = fuse_read_hw_state() == FUSE_NX_HW_STATE_DEV;
	for (int i = 0; i < sizeof(fuse_info) / sizeof(fuse_info_t); i++) {
		if (is_dev) {
			if (fuse_info[i].dev_fuses == burnt_fuses) {
				return i;
			}
		} else {
			if (fuse_info[i].prod_fuses == burnt_fuses) {
				return i;
			}
		}
	}
	return -1;
}

static bool match_nca_filename(
	const char *fname,
	u8 *out_major,
	u8 *out_minor,
	u8 *out_patch
)
{
	for (size_t i = 0; i < (sizeof(nca_db) / sizeof(nca_db[0])); i++) {
		if (strcmp(fname, nca_db[i].nca) == 0) {
			*out_major = nca_db[i].major;
			*out_minor = nca_db[i].minor;
			*out_patch = nca_db[i].patch;
			return true;
		}
	}
	return false;
}

// Detect firmware from SystemVersion NCA in SYSTEM partition
static bool detect_firmware_from_nca(u8 *major, u8 *minor, u8 *patch)
{
	LIST_INIT(gpt);

	if (!mount_nand_part(&gpt, "SYSTEM", true, true, true, true, NULL, NULL, NULL, NULL))
		return false;

	DIR dir;
	FILINFO fno;
	bool found = false;

	if (f_opendir(&dir, "bis:/Contents/registered") == FR_OK) {
		while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {

			// Skip non-NCA
			size_t len = strlen(fno.fname);
			if (len < 4 || strcmp(fno.fname + len - 4, ".nca") != 0)
				continue;

			if (match_nca_filename(fno.fname, major, minor, patch)) {
				found = true;
				break;
			}
		}
		f_closedir(&dir);
	}

	unmount_nand_part(&gpt, false, true, true, true);
	return found;
}

static void print_centered(int y, const char *text) {
	int len = strlen(text);
	int x = (720 - (len * 16)) / 2; // Auto-calculate center
	gfx_con_setpos(x, y);
	gfx_puts(text);
}

void show_fuse_check(u8 burnt_fuses, u8 fw_major, u8 fw_minor, u8 fw_patch, u8 required_fuses) {
	gfx_clear_grey(0x1B);

	// Title
	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	print_centered(40, "NINTENDO SWITCH FUSE CHECKER");

	// System Information - single line
	gfx_con_setpos(100, 150);
	SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
	gfx_printf("Firmware: ");
	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	gfx_printf("%d.%d.%d", fw_major, fw_minor, fw_patch);

	gfx_con_setpos(100, 200);
	SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
	gfx_printf("Burnt Fuses: ");
	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	gfx_printf("%d", burnt_fuses);

	gfx_con_setpos(100, 250);
	SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
	gfx_printf("Required Fuses: ");
	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	gfx_printf("%d", required_fuses);

	// Status - large and clear
	gfx_con_setpos(100, 350);
	if (burnt_fuses < required_fuses) {
		SETCOLOR(COLOR_RED, COLOR_DEFAULT);
		gfx_puts("STATUS: CRITICAL ERROR");

		gfx_con_setpos(100, 400);
		SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
		gfx_printf("Missing %d fuse(s) - OFW WILL NOT BOOT!", required_fuses - burnt_fuses);

		gfx_con_setpos(100, 450);
		gfx_puts("System will black screen on OFW boot");

		gfx_con_setpos(100, 520);
		SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
		gfx_puts("What will work: CFW (Atmosphere), Semi-stock (Hekate nogc)");
	} else if (burnt_fuses > required_fuses) {
		SETCOLOR(COLOR_RED, COLOR_DEFAULT);
		gfx_puts("STATUS: CRITICAL ERROR (OVERBURNT)");

		gfx_con_setpos(100, 400);
		SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
		gfx_printf("Extra %d fuse(s) burnt - OFW WILL NOT BOOT!", burnt_fuses - required_fuses);

		gfx_con_setpos(100, 450);
		gfx_puts("System will black screen on OFW boot");

		gfx_con_setpos(100, 520);
		SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
		gfx_puts("What will work: CFW (Atmosphere), Semi-stock (Hekate nogc)");

		gfx_con_setpos(100, 570);
		SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
		int fw_idx = get_burnt_fuses_idx(burnt_fuses);
		if (fw_idx == -1) {
			gfx_printf("No downgrade below any known firmware possible.");
		} else {
			gfx_printf("Cannot downgrade below FW %d.%d.x", fuse_info[fw_idx].major_min, fuse_info[fw_idx].minor_min);
		}
	} else {
		SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
		gfx_puts("STATUS: PERFECT MATCH");

		gfx_con_setpos(100, 400);
		SETCOLOR(COLOR_WHITE, COLOR_DEFAULT);
		gfx_puts("Exact fuse count match - OFW WILL BOOT NORMALLY");

		gfx_con_setpos(100, 450);
		gfx_puts("All systems operational");
	}

	// Footer
	// SETCOLOR(COLOR_RED, COLOR_DEFAULT);
	// print_centered(650, "VOL+:Fuse Map | VOL- or power:Back to menu | vol+-: Screenshot");
	gfx_con_setpos(100, 650);
	save_screenshot_and_go_back("fuse_check");
}

void show_fuse_info_page(int scroll_offset) {
	// Calculate how many entries fit on screen (from y=200 to y=620, footer at 650)
	// const int entries_per_page = 15; // (620-200) / 28 rows spacing

	gfx_clear_grey(0x1B);
	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	print_centered(80, "SWITCHBREW FUSE MAP");

	SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
	gfx_con_setpos(50, 160);
	gfx_printf("System Version");
	gfx_con_setpos(320, 160);
	gfx_printf("Prod Fuses");
	gfx_con_setpos(560, 160);
	gfx_printf("Dev Fuses");

	int row_y = 200;
	if (FUSE_INFO_COUNT > 0) {

		// Display database entries with scrolling
		size_t start = scroll_offset;
		size_t end = start + entries_per_page;
		if (end > FUSE_INFO_COUNT) end = FUSE_INFO_COUNT;

		for (size_t i = start; i < end; i++, row_y += 28) {
			gfx_con_setpos(50, row_y);
			gfx_printf("%d.%d.x - %d.%d.x", fuse_info[i].major_min, fuse_info[i].minor_min, fuse_info[i].major_max, fuse_info[i].minor_max);

			gfx_con_setpos(400, row_y);
			gfx_printf("%2d", fuse_info[i].prod_fuses);

			gfx_con_setpos(640, row_y);
			gfx_printf("%2d", fuse_info[i].dev_fuses);
		}

		// Show scroll indicator if there are more entries
		if (FUSE_INFO_COUNT > entries_per_page) {
			gfx_con_setpos(540, 620);
			SETCOLOR(COLOR_CYAN, COLOR_DEFAULT);
			gfx_printf("[%d-%d/%d]", (int)start + 1, (int)end, (int)FUSE_INFO_COUNT);
		}
	}

	SETCOLOR(COLOR_RED, COLOR_DEFAULT);
	print_centered(650, "VOL+:Scroll Down | VOL-:Scroll Up | Power:Back | vol+-:Screenshot");
}

/*
void fuse_check() {
	bool orig_emummc_force_disable = h_cfg.emummc_force_disable;
	bool orig_emu_enabled = emu_cfg.enabled;
	h_cfg.emummc_force_disable = true;  // Force sysMMC for fuse check

	// Get burnt fuses
	u8 burnt_fuses = get_burnt_fuses();

	// Detect firmware version from NCA
	u8 fw_major = 0, fw_minor = 0, fw_patch = 0;
	bool fw_detected = false;

	if (emummc_storage_init_mmc() == 0) {
		// Try NCA detection
		fw_detected = detect_firmware_from_nca(&fw_major, &fw_minor, &fw_patch);
		emummc_storage_end();
	}

	if (!fw_detected) {
		// Default to a safe version if detection completely fails
		fw_major = 1;
		fw_minor = 0;
		fw_patch = 0;
	}

	// Calculate required fuses
	u8 required_fuses = get_required_fuses(fw_major, fw_minor);

	// Show results in horizontal layout (single page)
	show_fuse_check(burnt_fuses, fw_major, fw_minor, fw_patch, required_fuses);

	// Wait for button to exit, support info page, scrolling, and screenshot combo
	bool on_info_page = false;
	int scroll_offset = 0;
	// const int entries_per_page = 15;

	u32 btn_last = btn_read();

	while (true)
	{
		// Non-blocking button read
		u32 btn = btn_read();

		if (btn == (BTN_VOL_UP | BTN_VOL_DOWN))
		{
			// Wait for buttons release to avoid multiple screenshots
			while (btn_read() == (BTN_VOL_UP | BTN_VOL_DOWN))
				msleep(10);

			if (!save_fb_to_bmp("fuse_check"))
			{
				SETCOLOR(COLOR_GREEN, COLOR_DEFAULT);
				print_centered(620, "Screenshot saved!");
				msleep(1000);
			}
			else
			{
				SETCOLOR(COLOR_RED, COLOR_DEFAULT);
				print_centered(620, "Screenshot failed!");
				msleep(1000);
			}
			if (on_info_page)
				show_fuse_info_page(scroll_offset);
			else
				show_fuse_check(burnt_fuses, fw_major, fw_minor, fw_patch, required_fuses);

			btn_last = 0;
			continue;
		}

		// Only process button presses (ignore button releases and repeats)
		if (btn == btn_last)
		{
			msleep(10);
			continue;
		}

		btn_last = btn;

		// Ignore button releases (when btn becomes 0)
		if (!btn)
		{
			msleep(10);
			continue;
		}

		bool vol_up = btn & BTN_VOL_UP;
		bool vol_dn = btn & BTN_VOL_DOWN;

		// On main page: VOL+ toggles to info page, VOL- goes back to menu
		if (!on_info_page)
		{
			if (vol_up)
			{
				on_info_page = true;
				scroll_offset = 0; // Reset scroll when entering info page
				show_fuse_info_page(scroll_offset);
				continue;
			}

			if (vol_dn) {
				h_cfg.emummc_force_disable = orig_emummc_force_disable;
				emu_cfg.enabled = orig_emu_enabled;
				return;
			}
		}
		// On info page: VOL+ scrolls down, VOL- scrolls up, Power goes back to main
		else
		{
			if (vol_up)
			{
				// Scroll down (stop at bottom, don't change pages)
				int max_scroll = (int)FUSE_INFO_COUNT - entries_per_page;
				if (max_scroll < 0) max_scroll = 0;

				if (scroll_offset < max_scroll)
				{
					scroll_offset++;
					show_fuse_info_page(scroll_offset);
				}
				// If at bottom, do nothing (just stay there)
				continue;
			}

			if (vol_dn)
			{
				// Scroll up (stop at top, don't change pages)
				if (scroll_offset > 0)
				{
					scroll_offset--;
					show_fuse_info_page(scroll_offset);
				}
				// If at top, do nothing (just stay there)
				continue;
			}

			// Power: go back to main page when on fuse list
			if (btn & BTN_POWER)
			{
				on_info_page = false;
				show_fuse_check(burnt_fuses, fw_major, fw_minor, fw_patch, required_fuses);
				continue;
			}
		}

		// Power on main page: quit
		if (!on_info_page && (btn & BTN_POWER))
		{
			h_cfg.emummc_force_disable = orig_emummc_force_disable;
			emu_cfg.enabled = orig_emu_enabled;
			return;
		}
	}
}
*/

void fuse_check() {
	bool orig_emummc_force_disable = h_cfg.emummc_force_disable;
	bool orig_emu_enabled = emu_cfg.enabled;
	h_cfg.emummc_force_disable = true;  // Force sysMMC for fuse check

	// Get burnt fuses
	u8 burnt_fuses = get_burnt_fuses();

	// Detect firmware version from NCA
	u8 fw_major = 0, fw_minor = 0, fw_patch = 0;
	bool fw_detected = false;

	if (emummc_storage_init_mmc() == 0) {
		// Try NCA detection
		fw_detected = detect_firmware_from_nca(&fw_major, &fw_minor, &fw_patch);
		emummc_storage_end();
		sd_mount();
	}

	if (!fw_detected) {
		// Default to a safe version if detection completely fails
		fw_major = 1;
		fw_minor = 0;
		fw_patch = 0;
	}

	// Calculate required fuses
	u8 required_fuses = get_required_fuses(fw_major, fw_minor);

	show_fuse_check(burnt_fuses, fw_major, fw_minor, fw_patch, required_fuses);
			h_cfg.emummc_force_disable = orig_emummc_force_disable;
	emu_cfg.enabled = orig_emu_enabled;
}