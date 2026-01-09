#ifndef _TOOLS_H
#define _TOOLS_H

#include "config.h"

#include <libs/fatfs/ff.h>
#include <storage/emmc.h>
#include <utils/types.h>
#include <utils/list.h>

#define DEBUG 0

extern bool have_sd;
extern bool have_minerva;

#define BIS_CLUSTER_SECTORS   32
#define MAX_PATH_LEN    256
extern u32 COPY_BUF_SIZE;
#define RAM_COPY_BUF_SIZE (8 * 1024 * 1024)

typedef struct {
	u32 x;
	u32 y;
} ui_pos_t;

typedef struct {
	char *name;
	char *base_path;
} emunand_entry_t;
#define MAX_EMUNANDS 12

extern hekate_config h_cfg;
extern bool physical_emmc_ok;
extern bool sysmmc_available;
extern bool emummc_available;
extern bool menu_on_sysnand;
extern bool called_from_config_files;
extern bool called_from_AIO_LS_Pack_Updater;
extern bool bis_from_console;
extern char emmc_id[9];
extern int emunand_count;
extern int prev_sec_emunand;
extern int cur_sec_emunand;
extern emunand_entry_t *emunands;

/* Legacy color definitions - Kept for backward compatibility */
#define COLOR_RED    0xFFFF0000  // Now matches Hekate TXT_CLR_ERROR
#define COLOR_YELLOW 0xFFFFDD00  // Now matches Hekate TXT_CLR_WARNING
#define COLOR_GREEN  0xFF40FF00
#define COLOR_BLUE   0xFF00DDFF
#define COLOR_VIOLET 0xFF8040FF
#define COLOR_DEFAULT 0xFF1B1B1B

/* Hekate-style color palette - Authentic Hekate colors from bootloader/gfx/gfx.h */
#define COLOR_WHITE      0xFFFFFFFF  // Pure white
#define COLOR_SOFT_WHITE 0xFFCCCCCC  // Hekate default text (TXT_CLR_DEFAULT)
#define COLOR_CYAN       0xFF00CCFF  // Hekate light cyan (TXT_CLR_CYAN_L)
#define COLOR_CYAN_L     0xFF00CCFF  // Hekate light cyan (TXT_CLR_CYAN_L)
#define COLOR_TURQUOISE  0xFF00FFCC  // Hekate turquoise (TXT_CLR_TURQUOISE)
#define COLOR_ORANGE     0xFFFFBA00  // Hekate orange (TXT_CLR_ORANGE)
#define COLOR_GREENISH   0xFF96FF00  // Hekate toxic green (TXT_CLR_GREENISH)
#define COLOR_WARNING    0xFFFFDD00  // Hekate warning yellow (TXT_CLR_WARNING)
#define COLOR_ERROR      0xFFFF0000  // Hekate error red (TXT_CLR_ERROR)
#define COLOR_GREEN_D    0xFF008800  // Hekate dark green (TXT_CLR_GREEN_D)
#define COLOR_RED_D      0xFF880000  // Hekate dark red (TXT_CLR_RED_D)
#define COLOR_GREY       0xFF888888  // Hekate grey (TXT_CLR_GREY)
#define COLOR_GREY_M     0xFF555555  // Hekate medium grey (TXT_CLR_GREY_M)
#define COLOR_GREY_DM    0xFF444444  // Hekate darker grey (TXT_CLR_GREY_DM)
#define COLOR_GREY_D     0xFF303030  // Hekate darkest grey (TXT_CLR_GREY_D)

#define SETCOLOR(fg, bg) gfx_con_setcol(fg, 1, bg)
#define RESETCOLOR SETCOLOR(COLOR_WHITE, COLOR_DEFAULT)

static const u32 colors[6] = {COLOR_CYAN_L, COLOR_TURQUOISE, COLOR_GREENISH, COLOR_SOFT_WHITE, COLOR_ORANGE, COLOR_WHITE};

#if DEBUG
#define debug_log_start() debug_log_start_impl()
#define debug_log_write(...) debug_log_write_impl(__VA_ARGS__)
#else
#define debug_log_start() do {} while (0)
#define debug_log_write(...) do {} while (0)
#endif

char *bdk_strdup(const char *s);
void mkdir_recursive(const char *path);
void debug_log_start_impl();
void debug_log_write_impl(const char *text, ...);
void test_total_heap();
void debug_dump_gpt(link_t *gpt);
bool get_emmc_id(char *emmc_id_out);
bool mount_nand_part(link_t *gpt, const char *part_name, bool nand_open, bool set_partition, bool fatfs_mount, bool test_loaded_keys, u64 *part_size_bytes_buf, bool *is_boot_buf, bool *is_bis_buf, emmc_part_t *part_buf);
void unmount_nand_part(link_t *gpt, bool is_boot_part, bool is_bis, bool nand_close, bool fatfs_unmount);
bool wait_vol_plus();
bool delete_save_from_nand(const char* savename, bool on_system_part);
void ui_spinner_begin();
void ui_spinner_draw();
void ui_spinner_clear();
bool flash_or_dump_part(bool flash, const char *sd_filepath, const char *part_name, bool file_encrypted);
// bool flash_part_from_sd_file(const char *sd_filepath, const char *part_name, bool src_encrypted);
// bool dump_part_to_sd_file(const char *sd_filepath, const char *part_name, bool dst_encrypted);
FRESULT easy_rename(const char* old, const char* new);
FRESULT f_copy(const char *src, const char *dst, BYTE *buf);
bool f_transfer_from_nands(const char *file_path, bool on_system_part); // limited to transfer a 8 MB file max
void save_screenshot_and_go_back(const char* filename);
void display_title();
void cls();
FRESULT f_cp_or_rm_rf(const char *src_root, const char *dst_root);
bool is_autorcm_enabled();
void build_emunand_list();
void select_and_apply_emunand();
void emunand_list_free();
int save_fb_to_bmp(const char* filename);

#endif