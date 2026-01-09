#include <string.h>

#include "display_log_bin.h"
#include <gfx_utils.h>
#include "gfx.h"
#include <libs/fatfs/ff.h>
#include <mem/heap.h>
#include <utils/btn.h>
#include <utils/sprintf.h>
#include <utils/types.h>

#include "../tools.h"
#include "messages.h"

static u32 log_color_from_level(u8 lvl) {
	switch (lvl) {
		case LOG_OK:   return COLOR_GREEN;
		case LOG_WARN: return COLOR_ORANGE;
		case LOG_ERR:  return COLOR_RED;
		default:       return COLOR_WHITE;
	}
}

static int format_log_entry(char *out, int out_sz, const log_entry_t *e) {
	return s_printf(out, g_log_messages[e->msg_id],
		log_arg_value((log_entry_t *)e, 0),
		log_arg_value((log_entry_t *)e, 1),
		log_arg_value((log_entry_t *)e, 2),
		log_arg_value((log_entry_t *)e, 3)
	);
}

static int build_log_view_index(log_view_item_t *items, int max_items) {
	int count = 0;

	for (u32 i = 0; i < g_log_count && count < max_items; i++) {
		char tmp[256];
		int len = format_log_entry(tmp, sizeof(tmp), &g_log_buf[i]);

		int lines = (len + MAX_CHARS_PER_LINE - 1) / MAX_CHARS_PER_LINE;
		if (lines < 1) lines = 1;

		items[count].index  = i;
		items[count].height = lines;
		count++;
	}

	return count;
}

static void draw_log_entry(const log_entry_t *e, int y_start) {
	char buf[256];
	int len = format_log_entry(buf, sizeof(buf), e);
	int pos = 0;
	int y = y_start;

	u32 color = log_color_from_level(e->level);

	while (pos < len) {
		int chunk = MIN(MAX_CHARS_PER_LINE, len - pos);

		char line[MAX_CHARS_PER_LINE + 1];
		memcpy(line, buf + pos, chunk);
		line[chunk] = 0;

		gfx_con_setpos(0, y);
		gfx_printf("%k%s", color, line);

		pos += chunk;
		y += FONT_H;
	}
}

static int compute_max_top(const log_view_item_t *items, int item_count) {
	int used = 0;

	for (int i = item_count - 1; i >= 0; i--) {
		used += items[i].height;

		if (used >= MAX_VISIBLE_LINES)
			return i;
	}

	return 0;
}

void show_log_viewer() {
	if (g_log_count == 0 || g_log_count > LOG_MAX_ENTRIES) {
		cls();
		gfx_printf("%kNo log to display, press any key to continue%k", COLOR_RED, COLOR_WHITE);
		btn_wait();
		return;
	}
	log_view_item_t *items = malloc(sizeof(*items) * g_log_count);
	if (!items) {
		gfx_printf("%kOut of memory (log viewer)%k", COLOR_RED, COLOR_WHITE);
		btn_wait();
		return;
	}
	// static log_view_item_t items[LOG_MAX_ENTRIES];

	int item_count = build_log_view_index(items, LOG_MAX_ENTRIES);
	if (!item_count) {
		free(items);
		return;
	}

	int top = 0;
	int max_top = compute_max_top(items, item_count);

	while (1) {
		gfx_clear_grey(0x1B);
		gfx_con_setpos(0, 0);

		display_title();

		int y = (TITLE_LINES + SEPARATOR_LINES) * FONT_H;
		int used = 0;

		for (int i = top; i < item_count; i++) {
			if (used + items[i].height > MAX_VISIBLE_LINES)
				break;

			const log_entry_t *e = &g_log_buf[items[i].index];
			draw_log_entry(e, y);

			y += items[i].height * FONT_H;
			used += items[i].height;
		}

		gfx_con_setpos(0, SCREEN_H - FOOTER_LINES * FONT_H);
		gfx_printf("%kVOL+ Up  VOL- Down  POWER Exit", COLOR_WHITE);

		u8 k = 0;
		while (1) {
			k = btn_read();
			if (k == BTN_VOL_UP || k == BTN_VOL_DOWN || k == BTN_POWER) {
				break;
			}
		}
		if (k == BTN_VOL_UP && top > 0) {
			top--;
		} else if (k == BTN_VOL_DOWN && top < max_top) {
			top++;
		} else if (k == BTN_POWER) {
			log_export_txt("sd:/LockSmith-RCM/log.txt");
			break;
		}
	}
	free(items);
}
