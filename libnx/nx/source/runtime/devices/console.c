#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include "runtime/devices/console.h"
#include "display/framebuffer.h"

#include "default_font_bin.h"

// --- Minimal UTF-8 decoder for the console ---
typedef struct {
	uint32_t codepoint;
	int remaining;
} Utf8State;

static Utf8State g_utf8 = {0, 0};

// --- New indexed font format (FN16) ---
typedef struct {
	u32 magic;         // 'FN16' = 0x464E3136
	u16 version;       // 1
	u16 cell_size;     // 16
	u32 glyph_count;   // number of entries in the index
	u32 index_offset;  // offset relative to the start of the data
	u32 bitmap_offset; // offset relative to the start of the data
} __attribute__((packed)) Fn16Header;

#define FN16_MAGIC 0x464E3136u

static inline bool _fontIsFn16(const void* gfx, Fn16Header* out_hdr) {
	if (!gfx) return false;
	const Fn16Header* h = (const Fn16Header*)gfx;
	if (h->magic == FN16_MAGIC && h->cell_size == 16 && h->version >= 1) {
		if (out_hdr) *out_hdr = *h;
		return true;
	}
	return false;
}

static int _fn16LookupGlyph(const void* gfx, u32 codepoint) {
	Fn16Header hdr;
	if (!_fontIsFn16(gfx, &hdr)) return -1;
	const u32* index = (const u32*)((const u8*)gfx + hdr.index_offset);
	// Binary search
	s32 lo = 0, hi = (s32)hdr.glyph_count - 1;
	while (lo <= hi) {
		s32 mid = lo + ((hi - lo) >> 1);
		u32 v = index[mid];
		if (v == codepoint) return mid;
		if (v < codepoint) lo = mid + 1; else hi = mid - 1;
	}
	return -1;
}

static int _fn16LookupFallback(const void* gfx) {
	// Try U+FFFD, then '?', then space
	int idx = _fn16LookupGlyph(gfx, 0xFFFD);
	if (idx >= 0) return idx;
	idx = _fn16LookupGlyph(gfx, '?');
	if (idx >= 0) return idx;
	idx = _fn16LookupGlyph(gfx, ' ');
	return idx;
}

static const u8 colorCube[] = {
	0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff,
};

static const u8 grayScale[] = {
	0x08, 0x12, 0x1c, 0x26, 0x30, 0x3a, 0x44, 0x4e,
	0x58, 0x62, 0x6c, 0x76, 0x80, 0x8a, 0x94, 0x9e,
	0xa8, 0xb2, 0xbc, 0xc6, 0xd0, 0xda, 0xe4, 0xee,
};

//The below width/height is for 720p.
static PrintConsole defaultConsole =
{
	//Font:
	{
		default_font_bin, //font gfx
		0, //first ascii character in the set
		256, //number of characters in the font set
		16, //tile width
		16, //tile height
	},
	NULL,   //renderer
	1,1,	//cursorX cursorY
	1,1,	//prevcursorX prevcursorY
	80,		//console width
	45,		//console height
	1,		//window x
	1,		//window y
	80,		//window width
	45,		//window height
	3,		//tab size
	7,		// foreground color
	0,		// background color
	0,		// flags
	false	//console initialized
};


static PrintConsole currentCopy;

static PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

static void consoleNewRow(void);
static void consolePrintChar(int c);
static void consoleDrawChar(int c);

//---------------------------------------------------------------------------------
static void consoleCls(int mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp,rowTemp;

	switch (mode)
	{
		case 0:
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			while(i++ < ((currentConsole->windowHeight * currentConsole->windowWidth) - (rowTemp * currentConsole->windowWidth + colTemp)))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
		case 1:
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			currentConsole->cursorY  = 1;
			currentConsole->cursorX  = 1;

			while (i++ < (rowTemp * currentConsole->windowWidth + colTemp))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
		case 2:
		{
			currentConsole->cursorY  = 1;
			currentConsole->cursorX  = 1;

			while(i++ < currentConsole->windowHeight * currentConsole->windowWidth)
				consolePrintChar(' ');

			currentConsole->cursorY  = 1;
			currentConsole->cursorX  = 1;
			break;
		}
	}
}
//---------------------------------------------------------------------------------
static void consoleClearLine(int mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp;

	switch (mode)
	{
		case 0:
			colTemp = currentConsole->cursorX;

			for (i=0; i < currentConsole->windowWidth - colTemp + 1; i++) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		case 1:
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 1;

			for(i=0; i < colTemp - 1; i++) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		case 2:
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 1;

			for(i=0; i < currentConsole->windowWidth; i++) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
	}
}

//---------------------------------------------------------------------------------
static inline void consolePosition(int x, int y) {
//---------------------------------------------------------------------------------
	// invalid position
	if(x < 0 || y < 0)
		return;

	// 1-based, but we'll take a 0
	if(x < 1)
		x = 1;
	if(y < 1)
		y = 1;

	// clip to console edge
	if(x > currentConsole->windowWidth)
		x = currentConsole->windowWidth;
	if(y > currentConsole->windowHeight)
		y = currentConsole->windowHeight;

	currentConsole->cursorX = x;
	currentConsole->cursorY = y;
}

#define _ANSI_MAXARGS 16

static struct
{
	struct
	{
		int flags;
		u32 fg;
		u32 bg;
	} color;
	int argIdx;
	int args[_ANSI_MAXARGS];
	int colorArgCount;
	unsigned int colorArgs[3];
	bool hasArg;
	enum
	{
		ESC_NONE,
		ESC_START,
		ESC_BUILDING_UNKNOWN,
		ESC_BUILDING_FORMAT_FG,
		ESC_BUILDING_FORMAT_BG,
		ESC_BUILDING_FORMAT_FG_NONRGB,
		ESC_BUILDING_FORMAT_BG_NONRGB,
		ESC_BUILDING_FORMAT_FG_RGB,
		ESC_BUILDING_FORMAT_BG_RGB,
	} state;
} escapeSeq;

static void consoleSetColorState(int code)
{
	switch(code)
	{
	case 0: // reset
		escapeSeq.color.flags = 0;
		escapeSeq.color.bg    = 0;
		escapeSeq.color.fg    = 7;
		break;

	case 1: // bold
		escapeSeq.color.flags &= ~CONSOLE_COLOR_FAINT;
		escapeSeq.color.flags |= CONSOLE_COLOR_BOLD;
		break;

	case 2: // faint
		escapeSeq.color.flags &= ~CONSOLE_COLOR_BOLD;
		escapeSeq.color.flags |= CONSOLE_COLOR_FAINT;
		break;

	case 3: // italic
		escapeSeq.color.flags |= CONSOLE_ITALIC;
		break;

	case 4: // underline
		escapeSeq.color.flags |= CONSOLE_UNDERLINE;
		break;
	case 5: // blink slow
		escapeSeq.color.flags &= ~CONSOLE_BLINK_FAST;
		escapeSeq.color.flags |= CONSOLE_BLINK_SLOW;
		break;
	case 6: // blink fast
		escapeSeq.color.flags &= ~CONSOLE_BLINK_SLOW;
		escapeSeq.color.flags |= CONSOLE_BLINK_FAST;
		break;
	case 7: // reverse video
		escapeSeq.color.flags |= CONSOLE_COLOR_REVERSE;
		break;
	case 8: // conceal
		escapeSeq.color.flags |= CONSOLE_CONCEAL;
		break;
	case 9: // crossed-out
		escapeSeq.color.flags |= CONSOLE_CROSSED_OUT;
		break;
	case 21: // bold off
		escapeSeq.color.flags &= ~CONSOLE_COLOR_BOLD;
		break;

	case 22: // normal color
		escapeSeq.color.flags &= ~CONSOLE_COLOR_BOLD;
		escapeSeq.color.flags &= ~CONSOLE_COLOR_FAINT;
		break;

	case 23: // italic off
		escapeSeq.color.flags &= ~CONSOLE_ITALIC;
		break;

	case 24: // underline off
		escapeSeq.color.flags &= ~CONSOLE_UNDERLINE;
		break;

	case 25: // blink off
		escapeSeq.color.flags &= ~CONSOLE_BLINK_SLOW;
		escapeSeq.color.flags &= ~CONSOLE_BLINK_FAST;
		break;

	case 27: // reverse off
		escapeSeq.color.flags &= ~CONSOLE_COLOR_REVERSE;
		break;

	case 29: // crossed-out off
		escapeSeq.color.flags &= ~CONSOLE_CROSSED_OUT;
		break;

	case 30 ... 37: // writing color
		escapeSeq.color.flags &= ~CONSOLE_FG_CUSTOM;
		escapeSeq.color.fg     = code - 30;
		break;

	case 38: // custom foreground color
		escapeSeq.state = ESC_BUILDING_FORMAT_FG;
		escapeSeq.colorArgCount = 0;
		break;

	case 39: // reset foreground color
		escapeSeq.color.flags &= ~CONSOLE_FG_CUSTOM;
		escapeSeq.color.fg     = 7;
		break;
	case 40 ... 47: // screen color
		escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
		escapeSeq.color.bg = code - 40;
		break;
	case 48: // custom background color
		escapeSeq.state = ESC_BUILDING_FORMAT_BG;
		escapeSeq.colorArgCount = 0;
		break;
	case 49: // reset background color
		escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
		escapeSeq.color.bg = 0;
		break;
	case 90 ... 97: // bright foreground
		escapeSeq.color.flags &= ~CONSOLE_COLOR_FAINT;
		escapeSeq.color.flags |= CONSOLE_COLOR_FG_BRIGHT;
		escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
		escapeSeq.color.fg = code - 90;
		break;
	case 100 ... 107: // bright background
		escapeSeq.color.flags &= ~CONSOLE_COLOR_FAINT;
		escapeSeq.color.flags |= CONSOLE_COLOR_BG_BRIGHT;
		escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
		escapeSeq.color.bg = code - 100;
		break;
	}
}

static void consoleHandleColorEsc(int argCount)
{
	escapeSeq.color.bg = currentConsole->bg;
	escapeSeq.color.fg = currentConsole->fg;
	escapeSeq.color.flags = currentConsole->flags;

	for (int arg = 0; arg < argCount; arg++)
	{
		int code = escapeSeq.args[arg];
		switch (escapeSeq.state)
		{
		case ESC_BUILDING_UNKNOWN:
			consoleSetColorState(code);
			break;
		case ESC_BUILDING_FORMAT_FG:
			if (code == 5)
				escapeSeq.state = ESC_BUILDING_FORMAT_FG_NONRGB;
			else if (code == 2)
				escapeSeq.state = ESC_BUILDING_FORMAT_FG_RGB;
			else
				escapeSeq.state = ESC_BUILDING_UNKNOWN;
			break;
		case ESC_BUILDING_FORMAT_BG:
			if (code == 5)
				escapeSeq.state = ESC_BUILDING_FORMAT_BG_NONRGB;
			else if (code == 2)
				escapeSeq.state = ESC_BUILDING_FORMAT_BG_RGB;
			else
				escapeSeq.state = ESC_BUILDING_UNKNOWN;
			break;
		case ESC_BUILDING_FORMAT_FG_NONRGB:
			if (code <= 15) {
				escapeSeq.color.fg  = code;
				escapeSeq.color.flags &= ~CONSOLE_FG_CUSTOM;
			} else if (code <= 231) {
				code -= 16;
				unsigned int r = code / 36;
				unsigned int g = (code - r * 36) / 6;
				unsigned int b = code - r * 36 - g * 6;

				escapeSeq.color.fg  = RGB565_FROM_RGB8 (colorCube[r], colorCube[g], colorCube[b]);
				escapeSeq.color.flags |= CONSOLE_FG_CUSTOM;
			} else if (code <= 255) {
				code -= 232;

				escapeSeq.color.fg  = RGB565_FROM_RGB8 (grayScale[code], grayScale[code], grayScale[code]);
				escapeSeq.color.flags |= CONSOLE_FG_CUSTOM;
			}
			escapeSeq.state = ESC_BUILDING_UNKNOWN;
			break;
		case ESC_BUILDING_FORMAT_BG_NONRGB:
			if (code <= 15) {
				escapeSeq.color.bg  = code;
				escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
			} else if (code <= 231) {
				code -= 16;
				unsigned int r = code / 36;
				unsigned int g = (code - r * 36) / 6;
				unsigned int b = code - r * 36 - g * 6;

				escapeSeq.color.bg  = RGB565_FROM_RGB8 (colorCube[r], colorCube[g], colorCube[b]);
				escapeSeq.color.flags |= CONSOLE_BG_CUSTOM;
			} else if (code <= 255) {
				code -= 232;

				escapeSeq.color.bg  = RGB565_FROM_RGB8 (grayScale[code], grayScale[code], grayScale[code]);
				escapeSeq.color.flags |= CONSOLE_BG_CUSTOM;
			}
			escapeSeq.state = ESC_BUILDING_UNKNOWN;
			break;
		case ESC_BUILDING_FORMAT_FG_RGB:
			escapeSeq.colorArgs[escapeSeq.colorArgCount++] = code;
			if(escapeSeq.colorArgCount == 3)
			{
				escapeSeq.color.fg = RGB565_FROM_RGB8(escapeSeq.colorArgs[0], escapeSeq.colorArgs[1], escapeSeq.colorArgs[2]);
				escapeSeq.color.flags |= CONSOLE_FG_CUSTOM;
				escapeSeq.state = ESC_BUILDING_UNKNOWN;
			}
			break;
		case ESC_BUILDING_FORMAT_BG_RGB:
			escapeSeq.colorArgs[escapeSeq.colorArgCount++] = code;
			if(escapeSeq.colorArgCount == 3)
			{
				escapeSeq.color.bg = RGB565_FROM_RGB8(escapeSeq.colorArgs[0], escapeSeq.colorArgs[1], escapeSeq.colorArgs[2]);
				escapeSeq.color.flags |= CONSOLE_BG_CUSTOM;
				escapeSeq.state = ESC_BUILDING_UNKNOWN;
			}
		default:
			break;
		}
	}
	escapeSeq.argIdx = 0;

	currentConsole->bg = escapeSeq.color.bg;
	currentConsole->fg = escapeSeq.color.fg;
	currentConsole->flags = escapeSeq.color.flags;

}

//---------------------------------------------------------------------------------
static ssize_t con_write(struct _reent *r,void *fd,const char *ptr, size_t len) {
//---------------------------------------------------------------------------------

    int i = 0;
    int count = 0;
    const unsigned char *tmp = (const unsigned char*)ptr;

    if(!tmp) return -1;

    while(i < (int)len) {

        unsigned char b = *(tmp++);
        i++; count++;
		// If we're not currently inside an ANSI escape sequence, perform UTF-8 decoding
		if (escapeSeq.state == ESC_NONE) {
			// Currently in the middle of a UTF-8 sequence?
			if (g_utf8.remaining > 0) {
				if ((b & 0xC0) == 0x80) {
					// continuation byte
					g_utf8.codepoint = (g_utf8.codepoint << 6) | (b & 0x3F);
					g_utf8.remaining--;
					if (g_utf8.remaining == 0) {
						uint32_t cp = g_utf8.codepoint;
						// full codepoint decoded, print it
						consolePrintChar((int)cp);
					}
					continue; // byte consumed by the UTF-8 decoder
				} else {
					// invalid continuation byte -> reset state and reprocess this byte
					g_utf8.remaining = 0;
				}
			}

			// Starting a new UTF-8 sequence
			if (b < 0x80) {
				// ASCII -> let the ANSI state machine handle it (ESC, etc.)
			} else if (b >= 0xC2 && b <= 0xDF) {
				g_utf8.codepoint = b & 0x1F;
				g_utf8.remaining = 1;
				continue;
			} else if (b >= 0xE0 && b <= 0xEF) {
				g_utf8.codepoint = b & 0x0F;
				g_utf8.remaining = 2;
				continue;
			} else if (b >= 0xF0 && b <= 0xF4) {
				g_utf8.codepoint = b & 0x07;
				g_utf8.remaining = 3;
				continue;
			} else {
				// invalid standalone byte
				consolePrintChar('?');
				continue;
			}
		}

		// Original path: handle ANSI sequences and ASCII
		char chr = (char)b;

        switch (escapeSeq.state)
        {
        case ESC_NONE:
            if (chr == 0x1b)
                escapeSeq.state = ESC_START;
            else
                consolePrintChar((int)(unsigned char)chr);
            break;
        case ESC_START:
            if (chr == '[')
            {
                escapeSeq.state = ESC_BUILDING_UNKNOWN;
                escapeSeq.hasArg = false;
                memset(escapeSeq.args, 0, sizeof(escapeSeq.args));
                escapeSeq.color.bg = currentConsole->bg;
                escapeSeq.color.fg = currentConsole->fg;
                escapeSeq.color.flags = currentConsole->flags;
                escapeSeq.argIdx = 0;
            }
            else
            {
                consolePrintChar(0x1b);
                consolePrintChar(chr);
                escapeSeq.state = ESC_NONE;
            }
            break;
        case ESC_BUILDING_UNKNOWN:
            switch (chr)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                escapeSeq.hasArg = true;
                escapeSeq.args[escapeSeq.argIdx] = escapeSeq.args[escapeSeq.argIdx] * 10 + (chr - '0');
                break;
            case ';':
                if (escapeSeq.hasArg) {
                    if (escapeSeq.argIdx < _ANSI_MAXARGS) {
                        escapeSeq.argIdx++;
                    }
                }
                escapeSeq.hasArg = false;
                break;
            //---------------------------------------			// Cursor directional movement
            //---------------------------------------
            case 'A':
                if (!escapeSeq.hasArg && !escapeSeq.argIdx)
                    escapeSeq.args[0] = 1;
                currentConsole->cursorY  =  currentConsole->cursorY - escapeSeq.args[0];
                if (currentConsole->cursorY < 1)
                    currentConsole->cursorY = 1;
                escapeSeq.state = ESC_NONE;
                break;
            case 'B':
                if (!escapeSeq.hasArg && !escapeSeq.argIdx)
                    escapeSeq.args[0] = 1;
                currentConsole->cursorY  =  currentConsole->cursorY + escapeSeq.args[0];
                if (currentConsole->cursorY > currentConsole->windowHeight)
                    currentConsole->cursorY = currentConsole->windowHeight;
                escapeSeq.state = ESC_NONE;
                break;
            case 'C':
                if (!escapeSeq.hasArg && !escapeSeq.argIdx)
                    escapeSeq.args[0] = 1;
                currentConsole->cursorX  =  currentConsole->cursorX  + escapeSeq.args[0];
                if (currentConsole->cursorX > currentConsole->windowWidth)
                    currentConsole->cursorX = currentConsole->windowWidth;
                escapeSeq.state = ESC_NONE;
                break;
            case 'D':
                if (!escapeSeq.hasArg && !escapeSeq.argIdx)
                    escapeSeq.args[0] = 1;
                currentConsole->cursorX  =  currentConsole->cursorX  - escapeSeq.args[0];
                if (currentConsole->cursorX < 1)
                    currentConsole->cursorX = 1;
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Cursor position movement
            //---------------------------------------
            case 'H':
            case 'f':
                consolePosition(escapeSeq.args[1], escapeSeq.args[0]);
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Screen clear
            //---------------------------------------
            case 'J':
                if (escapeSeq.argIdx == 0 && !escapeSeq.hasArg) {
                    escapeSeq.args[0] = 0;
                }
                consoleCls(escapeSeq.args[0]);
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Line clear
            //---------------------------------------
            case 'K':
                if (escapeSeq.argIdx == 0 && !escapeSeq.hasArg) {
                    escapeSeq.args[0] = 0;
                }
                consoleClearLine(escapeSeq.args[0]);
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Save cursor position
            //---------------------------------------
            case 's':
                currentConsole->prevCursorX  = currentConsole->cursorX ;
                currentConsole->prevCursorY  = currentConsole->cursorY ;
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Load cursor position
            //---------------------------------------
            case 'u':
                currentConsole->cursorX  = currentConsole->prevCursorX ;
                currentConsole->cursorY  = currentConsole->prevCursorY ;
                escapeSeq.state = ESC_NONE;
                break;
            //---------------------------------------
            // Color scan codes
            //---------------------------------------
            case 'm':
                if (escapeSeq.argIdx == 0 && !escapeSeq.hasArg) escapeSeq.args[escapeSeq.argIdx++] = 0;
                if (escapeSeq.hasArg) escapeSeq.argIdx++;
                consoleHandleColorEsc(escapeSeq.argIdx);
                escapeSeq.state = ESC_NONE;
                break;
            default:
                // unsupported escape; ignore
                escapeSeq.state = ESC_NONE;
                break;
            }
        default:
            break;
        }
    }

    return count;
}
// ...existing code...

static const devoptab_t dotab_stdout = {
	.name    = "con",
	.write_r = con_write,
};

const devoptab_t* __nx_get_console_dotab(void) {
	return &dotab_stdout;
}

ConsoleRenderer* getDefaultConsoleRenderer(void);

//---------------------------------------------------------------------------------
PrintConsole* consoleInit(PrintConsole* console) {
//---------------------------------------------------------------------------------

	static bool didFirstConsoleInit = false;

	if(!didFirstConsoleInit) {
		devoptab_list[STD_OUT] = &dotab_stdout;
		setvbuf(stdout, NULL, _IONBF, 0);
		didFirstConsoleInit = true;
	}

	if(console) {
		currentConsole = console;
	} else {
		console = currentConsole;
	}

	*currentConsole = defaultConsole;
	if (!console->renderer) {
		console->renderer = getDefaultConsoleRenderer();
	}

	if (!console->consoleInitialised && console->renderer->init(console)) {
		console->consoleInitialised = true;
		consoleCls(2);
		return console;
	}

	return currentConsole;
}

//---------------------------------------------------------------------------------
void consoleExit(PrintConsole* console) {
//---------------------------------------------------------------------------------

	if (!console) console = currentConsole;

	if (console->consoleInitialised) {
		console->renderer->deinit(console);
		console->consoleInitialised = false;
	}
}

//---------------------------------------------------------------------------------
void consoleUpdate(PrintConsole* console) {
//---------------------------------------------------------------------------------

	if (!console) console = currentConsole;

	if (console->consoleInitialised) {
		console->renderer->flushAndSwap(console);
	}
}

//---------------------------------------------------------------------------------
PrintConsole *consoleSelect(PrintConsole* console) {
//---------------------------------------------------------------------------------
	PrintConsole *tmp = currentConsole;
	currentConsole = console;
	return tmp;
}

//---------------------------------------------------------------------------------
void consoleSetFont(PrintConsole* console, ConsoleFont* font) {
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->font = *font;

}

//---------------------------------------------------------------------------------
void consoleNewRow(void) {
//---------------------------------------------------------------------------------
	currentConsole->cursorY++;

	if(currentConsole->cursorY  > currentConsole->windowHeight)  {
		currentConsole->renderer->scrollWindow(currentConsole);
		currentConsole->cursorY = currentConsole->windowHeight;
		consoleClearLine(2);
	}
}

//---------------------------------------------------------------------------------
void consoleDrawChar(int c) {
//---------------------------------------------------------------------------------
	// Fast-path clear for l'espace: remplir la cellule en arrière-plan
	if (c == ' ') {
		currentConsole->renderer->drawChar(
			currentConsole,
			currentConsole->cursorX - 1 + currentConsole->windowX - 1,
			currentConsole->cursorY - 1 + currentConsole->windowY - 1,
			-1 /* sentinel: fill background cell */);
		return;
	}

	int glyph_index = -1;

	// Nouveau format indexé ?
	if (_fontIsFn16(currentConsole->font.gfx, NULL)) {
		glyph_index = _fn16LookupGlyph(currentConsole->font.gfx, (u32)c);
		if (glyph_index < 0) {
			glyph_index = _fn16LookupFallback(currentConsole->font.gfx);
			if (glyph_index < 0) return; // rien à dessiner
		}
	} else {
		// Ancien comportement: table linéaire
		int idx = c - currentConsole->font.asciiOffset;
		if (idx < 0 || idx > currentConsole->font.numChars) return;
		glyph_index = idx;
	}

	currentConsole->renderer->drawChar(
		currentConsole,
		currentConsole->cursorX - 1 + currentConsole->windowX - 1,
		currentConsole->cursorY - 1 + currentConsole->windowY - 1,
		glyph_index);
}

//---------------------------------------------------------------------------------
void consolePrintChar(int c) {
//---------------------------------------------------------------------------------
	int tabspaces;

	if (c==0) return;

	switch(c) {
		/*
		The only special characters we will handle are tab (\t), carriage return (\r), line feed (\n)
		and backspace (\b).
		Carriage return & line feed will function the same: go to next line and put cursor at the beginning.
		For everything else, use VT sequences.

		Reason: VT sequences are more specific to the task of cursor placement.
		The special escape sequences \b \f & \v are archaic and non-portable.
		*/
		case '\b':
			currentConsole->cursorX--;

			if(currentConsole->cursorX < 1) {
				if(currentConsole->cursorY > 1) {
					currentConsole->cursorX = currentConsole->windowWidth;
					currentConsole->cursorY--;
				} else {
					currentConsole->cursorX = 1;
				}
			}

			consoleDrawChar(' ');
			break;

		case '\t':
			tabspaces = currentConsole->tabSize - ((currentConsole->cursorX - 1) % currentConsole->tabSize);
			if (currentConsole->cursorX + tabspaces > currentConsole->windowWidth)
				tabspaces = currentConsole->windowWidth - currentConsole->cursorX;
			for(int i=0; i<tabspaces; i++) consolePrintChar(' ');
			break;
		case '\n':
			consoleNewRow();
		case '\r':
			currentConsole->cursorX  = 1;
			break;
		default:
			if(currentConsole->cursorX  > currentConsole->windowWidth) {
				currentConsole->cursorX  = 1;
				consoleNewRow();
			}
			consoleDrawChar(c);
			++currentConsole->cursorX ;
			break;
	}
}

//---------------------------------------------------------------------------------
void consoleClear(void) {
//---------------------------------------------------------------------------------
	consoleCls(2);
}

//---------------------------------------------------------------------------------
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height) {
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	if (x < 1) x = 1;
	if (y < 1) y = 1;

	if (x + width > console->consoleWidth ) width = console->consoleWidth + 1 - x;
	if (y + height > console->consoleHeight ) height = console->consoleHeight + 1 - y;

	console->windowWidth = width;
	console->windowHeight = height;
	console->windowX = x;
	console->windowY = y;

	console->cursorX = 1;
	console->cursorY = 1;

}
