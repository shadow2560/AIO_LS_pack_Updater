#ifndef _DISPLAY_LOG_BIN_H_
#define _DISPLAY_LOG_BIN_H_

#include <utils/types.h>
#include <gfx_utils.h>
#include "../gfx/messages.h"

extern const char *g_log_messages[LOG_MSG_COUNT];
extern log_entry_t* g_log_buf;
extern u32 g_log_count;

typedef struct {
	u16 index;       // index dans g_log_buf[]
	u16 height;      // lignes écran nécessaires
} log_view_item_t;

#define SCREEN_W            720
#define SCREEN_H            1280

#define FONT_W              16
#define FONT_H              28

#define TITLE_LINES         2
#define FOOTER_LINES        2
#define SEPARATOR_LINES     1

#define MAX_VISIBLE_LINES   ((SCREEN_H / FONT_H) - (TITLE_LINES + FOOTER_LINES + 2 * SEPARATOR_LINES))

#define MAX_CHARS_PER_LINE  (SCREEN_W / FONT_W)

void show_log_viewer();

#endif