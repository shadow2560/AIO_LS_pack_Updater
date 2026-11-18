#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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
	RGB565_FROM_RGB8(27,26,27),		// background color (custom dark)
	CONSOLE_BG_CUSTOM,		// flags (use custom BG)
	RGB565_FROM_RGB8(27,26,27),		// base/default background
	true,	// baseBgCustom
	false	//console initialized
};


static PrintConsole currentCopy;

static PrintConsole* currentConsole = &currentCopy;

typedef struct ConsoleTextBuffer
{
	PrintConsole* console;
	u32* cells;
	int width;
	int height;
	u32* historyCells;
	int historyCapacity;
	int historyAllocatedCapacity;
	int historyWidth;
	int historyCount;
	int historyStart;
	u64 historyScrollCount;
	u64 historyGeneration;
	struct ConsoleTextBuffer* next;
} ConsoleTextBuffer;

static ConsoleTextBuffer* consoleTextBuffers;

static ConsoleTextBuffer* consoleFindTextBuffer(PrintConsole* console)
{
	ConsoleTextBuffer* buffer = consoleTextBuffers;
	while (buffer) {
		if (buffer->console == console)
			return buffer;
		buffer = buffer->next;
	}
	return NULL;
}

static void consoleClearTextHistory(ConsoleTextBuffer* buffer)
{
	if (!buffer)
		return;
	buffer->historyCount = 0;
	buffer->historyStart = 0;
	buffer->historyScrollCount = 0;
	buffer->historyGeneration++;
}

static void consoleStoreHistoryLine(ConsoleTextBuffer* buffer, const u32* line)
{
	if (!buffer)
		return;
	if (buffer->historyScrollCount >= ULLONG_MAX - (u64)buffer->height) {
		consoleClearTextHistory(buffer);
	}
	buffer->historyScrollCount++;
	if (!buffer->historyCells || buffer->historyCapacity <= 0 || !line)
		return;

	int index;
	if (buffer->historyCount < buffer->historyCapacity) {
		index = (buffer->historyStart + buffer->historyCount) % buffer->historyCapacity;
		buffer->historyCount++;
	} else {
		index = buffer->historyStart;
		buffer->historyStart = (buffer->historyStart + 1) % buffer->historyCapacity;
	}
	int copyWidth = buffer->width < buffer->historyWidth ? buffer->width : buffer->historyWidth;
	u32* destination = buffer->historyCells + (size_t)index * buffer->historyWidth;
	memcpy(destination, line, (size_t)copyWidth * sizeof(u32));
	for (int x = copyWidth; x < buffer->historyWidth; x++)
		destination[x] = ' ';
}

static void consoleReverseHistoryRows(u32* cells, int width, int first, int last)
{
	while (first < last) {
		u32* firstRow = cells + (size_t)first * width;
		u32* lastRow = cells + (size_t)last * width;
		for (int x = 0; x < width; x++) {
			u32 value = firstRow[x];
			firstRow[x] = lastRow[x];
			lastRow[x] = value;
		}
		first++;
		last--;
	}
}

static void consoleNormalizeHistory(ConsoleTextBuffer* buffer)
{
	if (!buffer || !buffer->historyCells || buffer->historyCapacity <= 0 ||
		buffer->historyStart <= 0)
		return;

	consoleReverseHistoryRows(buffer->historyCells, buffer->historyWidth, 0, buffer->historyStart - 1);
	consoleReverseHistoryRows(buffer->historyCells, buffer->historyWidth, buffer->historyStart,
		buffer->historyCapacity - 1);
	consoleReverseHistoryRows(buffer->historyCells, buffer->historyWidth, 0,
		buffer->historyCapacity - 1);
	buffer->historyStart = 0;
}

static const u32* consoleGetTextBufferLine(const ConsoleTextBuffer* buffer, int line)
{
	if (!buffer || line < 0 || line >= buffer->historyCount + buffer->height)
		return NULL;
	if (line < buffer->historyCount) {
		const int index = (buffer->historyStart + line) % buffer->historyCapacity;
		return buffer->historyCells + (size_t)index * buffer->historyWidth;
	}
	return buffer->cells + (size_t)(line - buffer->historyCount) * buffer->width;
}

static bool consoleResizeTextBuffer(ConsoleTextBuffer* buffer, int width, int height)
{
	if (!buffer)
		return false;
	if (width <= 0 || height <= 0) {
		free(buffer->cells);
		buffer->cells = NULL;
		// A temporarily hidden console (zero-sized window) must not lose its scrollback.
		// The history allocation and retained lines are reused when the window is restored.
		buffer->width = width;
		buffer->height = height;
		return false;
	}
	if (buffer->cells && buffer->width == width && buffer->height == height)
		return true;
	if ((size_t)width > SIZE_MAX / (size_t)height / sizeof(u32)) {
		free(buffer->cells);
		buffer->cells = NULL;
		free(buffer->historyCells);
		buffer->historyCells = NULL;
		buffer->historyCapacity = 0;
		buffer->historyAllocatedCapacity = 0;
		buffer->historyWidth = 0;
		consoleClearTextHistory(buffer);
		buffer->width = width;
		buffer->height = height;
		return false;
	}

	u32* cells = (u32*)malloc((size_t)width * (size_t)height * sizeof(u32));
	if (!cells) {
		free(buffer->cells);
		buffer->cells = NULL;
		free(buffer->historyCells);
		buffer->historyCells = NULL;
		buffer->historyCapacity = 0;
		buffer->historyAllocatedCapacity = 0;
		buffer->historyWidth = 0;
		consoleClearTextHistory(buffer);
		buffer->width = width;
		buffer->height = height;
		return false;
	}
	for (size_t i = 0; i < (size_t)width * (size_t)height; i++)
		cells[i] = ' ';

	if (buffer->cells) {
		int copyWidth = buffer->width < width ? buffer->width : width;
		int copyHeight = buffer->height < height ? buffer->height : height;
		for (int y = 0; y < copyHeight; y++)
			memcpy(cells + (size_t)y * width, buffer->cells + (size_t)y * buffer->width, (size_t)copyWidth * sizeof(u32));
	}

	if (buffer->historyCells && width > buffer->historyWidth && buffer->historyAllocatedCapacity > 0 &&
		(size_t)width <= SIZE_MAX / (size_t)buffer->historyAllocatedCapacity / sizeof(u32)) {
		const int oldHistoryWidth = buffer->historyWidth;
		consoleNormalizeHistory(buffer);
		size_t historySize = (size_t)width * (size_t)buffer->historyAllocatedCapacity * sizeof(u32);
		u32* historyCells = (u32*)realloc(buffer->historyCells, historySize);
		if (historyCells) {
			for (int line = buffer->historyCount - 1; line >= 0; line--) {
				u32* destination = historyCells + (size_t)line * width;
				memmove(destination, historyCells + (size_t)line * oldHistoryWidth,
					(size_t)oldHistoryWidth * sizeof(u32));
				for (int x = oldHistoryWidth; x < width; x++)
					destination[x] = ' ';
			}
			buffer->historyCells = historyCells;
			buffer->historyWidth = width;
		}
	}

	free(buffer->cells);
	buffer->cells = cells;
	buffer->width = width;
	buffer->height = height;
	return true;
}

static void consoleStoreTextCell(int x, int y, u32 codepoint)
{
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(currentConsole);
	if (!buffer || !buffer->cells || x < 1 || y < 1 || x > buffer->width || y > buffer->height)
		return;
	buffer->cells[(size_t)(y - 1) * buffer->width + (x - 1)] = codepoint;
}

static void consoleScrollTextBuffer(void)
{
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(currentConsole);
	if (!buffer || !buffer->cells || buffer->height <= 0 || buffer->width <= 0)
		return;
	consoleStoreHistoryLine(buffer, buffer->cells);
	if (buffer->height > 1)
		memmove(buffer->cells, buffer->cells + buffer->width,
			(size_t)(buffer->height - 1) * buffer->width * sizeof(u32));
	for (int x = 0; x < buffer->width; x++)
		buffer->cells[(size_t)(buffer->height - 1) * buffer->width + x] = ' ';
}

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

bool consoleEnableTextBuffer(PrintConsole* console)
{
	if (!console)
		console = currentConsole;
	if (console->windowWidth <= 0 || console->windowHeight <= 0)
		return false;

	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	bool created = false;
	if (!buffer) {
		buffer = (ConsoleTextBuffer*)calloc(1, sizeof(ConsoleTextBuffer));
		if (!buffer)
			return false;
		buffer->console = console;
		buffer->next = consoleTextBuffers;
		consoleTextBuffers = buffer;
		created = true;
	}
	if (consoleResizeTextBuffer(buffer, console->windowWidth, console->windowHeight))
		return true;
	if (created)
		consoleDisableTextBuffer(console);
	return false;
}

bool consoleConfigureTextBufferHistory(PrintConsole* console, bool enabled, int maxLines)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || maxLines < 0 ||
		(enabled && (maxLines == 0 || maxLines > INT_MAX - buffer->height)))
		return false;

	if (!enabled) {
		free(buffer->historyCells);
		buffer->historyCells = NULL;
		buffer->historyCapacity = 0;
		buffer->historyAllocatedCapacity = 0;
		buffer->historyWidth = 0;
		consoleClearTextHistory(buffer);
		return true;
	}

	if (!buffer->historyCells)
		buffer->historyWidth = buffer->width;
	if ((size_t)buffer->historyWidth > SIZE_MAX / (size_t)maxLines / sizeof(u32))
		return false;
	if (buffer->historyCapacity == maxLines && buffer->historyCells)
		return true;

	if (maxLines > buffer->historyAllocatedCapacity) {
		size_t allocationSize = (size_t)buffer->historyWidth * (size_t)maxLines * sizeof(u32);
		u32* historyCells = buffer->historyCells ?
			(u32*)realloc(buffer->historyCells, allocationSize) : (u32*)malloc(allocationSize);
		if (!historyCells)
			return false;
		buffer->historyCells = historyCells;
		buffer->historyAllocatedCapacity = maxLines;
	}

	consoleNormalizeHistory(buffer);
	int keepLines = buffer->historyCount;
	if (keepLines > maxLines)
		keepLines = maxLines;
	if (keepLines < buffer->historyCount) {
		memmove(buffer->historyCells,
			buffer->historyCells + (size_t)(buffer->historyCount - keepLines) * buffer->historyWidth,
			(size_t)keepLines * buffer->historyWidth * sizeof(u32));
	}
	if (maxLines < buffer->historyAllocatedCapacity) {
		size_t allocationSize = (size_t)buffer->historyWidth * (size_t)maxLines * sizeof(u32);
		u32* historyCells = (u32*)realloc(buffer->historyCells, allocationSize);
		if (historyCells) {
			buffer->historyCells = historyCells;
			buffer->historyAllocatedCapacity = maxLines;
		}
	}
	buffer->historyCapacity = maxLines;
	buffer->historyCount = keepLines;
	buffer->historyStart = 0;
	return true;
}

void consoleDisableTextBuffer(PrintConsole* console)
{
	if (!console)
		console = currentConsole;

	ConsoleTextBuffer** link = &consoleTextBuffers;
	while (*link) {
		if ((*link)->console == console) {
			ConsoleTextBuffer* buffer = *link;
			*link = buffer->next;
			free(buffer->cells);
			free(buffer->historyCells);
			free(buffer);
			return;
		}
		link = &(*link)->next;
	}
}

bool consoleIsTextBufferEnabled(PrintConsole* console)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	return buffer && buffer->cells;
}

int consoleGetLastTextLine(PrintConsole* console)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || buffer->width <= 0 || buffer->height <= 0)
		return -1;

	for (int y = buffer->historyCount + buffer->height - 1; y >= 0; y--) {
		const u32* line = consoleGetTextBufferLine(buffer, y);
		const int lineWidth = y < buffer->historyCount ? buffer->historyWidth : buffer->width;
		for (int x = 0; x < lineWidth; x++) {
			if (line[x] != ' ')
				return y;
		}
	}
	return -1;
}

int consoleGetLastVisibleTextLine(PrintConsole* console)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || buffer->width <= 0 || buffer->height <= 0)
		return -1;

	for (int y = buffer->height - 1; y >= 0; y--) {
		const u32* line = buffer->cells + (size_t)y * buffer->width;
		for (int x = 0; x < buffer->width; x++) {
			if (line[x] != ' ')
				return y;
		}
	}
	return -1;
}

int consoleGetTextHistoryLineCount(PrintConsole* console)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	return buffer && buffer->cells ? buffer->historyCount : 0;
}

bool consoleGetTextBufferLineId(PrintConsole* console, int line, u64* generation, u64* lineId)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || line < 0 || line >= buffer->historyCount + buffer->height ||
		!generation || !lineId)
		return false;
	*generation = buffer->historyGeneration;
	*lineId = buffer->historyScrollCount - (u64)buffer->historyCount + (u64)line;
	return true;
}

int consoleGetTextBufferLineIndex(PrintConsole* console, u64 generation, u64 lineId)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || generation != buffer->historyGeneration)
		return -1;
	u64 firstLineId = buffer->historyScrollCount - (u64)buffer->historyCount;
	u64 lastLineId = buffer->historyScrollCount + (u64)buffer->height - 1;
	if (lineId < firstLineId || lineId > lastLineId)
		return -1;
	return (int)(lineId - firstLineId);
}

static size_t consoleCodepointUtf8Size(u32 codepoint)
{
	if (codepoint <= 0x7F)
		return 1;
	if (codepoint <= 0x7FF)
		return 2;
	if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
		return 3;
	if (codepoint <= 0xFFFF)
		return 3;
	if (codepoint <= 0x10FFFF)
		return 4;
	return 3;
}

static size_t consoleWriteCodepointUtf8(char* output, size_t offset, size_t outputSize, u32 codepoint)
{
	if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
		codepoint = 0xFFFD;
	size_t length = consoleCodepointUtf8Size(codepoint);
	if (!output || offset + length >= outputSize)
		return offset;
	if (length == 1) {
		output[offset] = (char)codepoint;
	} else if (length == 2) {
		output[offset] = (char)(0xC0 | (codepoint >> 6));
		output[offset + 1] = (char)(0x80 | (codepoint & 0x3F));
	} else if (length == 3) {
		output[offset] = (char)(0xE0 | (codepoint >> 12));
		output[offset + 1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		output[offset + 2] = (char)(0x80 | (codepoint & 0x3F));
	} else {
		output[offset] = (char)(0xF0 | (codepoint >> 18));
		output[offset + 1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
		output[offset + 2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		output[offset + 3] = (char)(0x80 | (codepoint & 0x3F));
	}
	return offset + length;
}

size_t consoleGetText(PrintConsole* console, int startLine, int endLine, char* output, size_t outputSize)
{
	if (!console)
		console = currentConsole;
	ConsoleTextBuffer* buffer = consoleFindTextBuffer(console);
	if (!buffer || !buffer->cells || startLine < 0 || endLine < startLine ||
		endLine >= buffer->historyCount + buffer->height)
		return 0;

	size_t required = 0;
	size_t written = 0;
	bool outputFull = false;
	for (int y = startLine; y <= endLine; y++) {
		const u32* line = consoleGetTextBufferLine(buffer, y);
		const int lineWidth = y < buffer->historyCount ? buffer->historyWidth : buffer->width;
		int lastColumn = lineWidth - 1;
		while (lastColumn >= 0 && line[lastColumn] == ' ')
			lastColumn--;

		for (int x = 0; x <= lastColumn; x++) {
			u32 codepoint = line[x];
			size_t length = consoleCodepointUtf8Size(codepoint);
			required += length;
			if (output && !outputFull) {
				size_t nextWritten = consoleWriteCodepointUtf8(output, written, outputSize, codepoint);
				if (nextWritten == written)
					outputFull = true;
				else
					written = nextWritten;
			}
		}
		if (y < endLine) {
			required++;
			if (output && !outputFull) {
				if (written + 1 < outputSize)
					output[written++] = '\n';
				else
					outputFull = true;
			}
		}
	}

	if (output && outputSize)
		output[written < outputSize ? written : outputSize - 1] = '\0';
	return required;
}

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
			consoleClearTextHistory(consoleFindTextBuffer(currentConsole));
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
	case 49: // reset background color to base theme
		if (currentConsole->baseBgCustom) {
			escapeSeq.color.flags |= CONSOLE_BG_CUSTOM;
		} else {
			escapeSeq.color.flags &= ~CONSOLE_BG_CUSTOM;
		}
		escapeSeq.color.bg = currentConsole->baseBg;
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
		ConsoleTextBuffer* textBuffer = consoleFindTextBuffer(console);
		if (textBuffer)
			consoleResizeTextBuffer(textBuffer, console->windowWidth, console->windowHeight);
		consoleCls(2);
		return console;
	}

	return currentConsole;
}

//---------------------------------------------------------------------------------
void consoleExit(PrintConsole* console) {
//---------------------------------------------------------------------------------

	if (!console) console = currentConsole;
	consoleDisableTextBuffer(console);

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
void consoleSetBackgroundColorRgb(PrintConsole* console, u8 r, u8 g, u8 b) {
//---------------------------------------------------------------------------------
	if(!console) console = currentConsole;
	u16 color = RGB565_FROM_RGB8(r, g, b);
	console->bg = color;
	console->flags |= CONSOLE_BG_CUSTOM;
	// Also set the base/default background so ANSI '49' (Default) uses this theme.
	console->baseBg = color;
	console->baseBgCustom = true;
}

//---------------------------------------------------------------------------------
void consoleNewRow(void) {
//---------------------------------------------------------------------------------
	currentConsole->cursorY++;

	if(currentConsole->cursorY  > currentConsole->windowHeight)  {
		currentConsole->renderer->scrollWindow(currentConsole);
		consoleScrollTextBuffer();
		currentConsole->cursorY = currentConsole->windowHeight;
		consoleClearLine(2);
	}
}

//---------------------------------------------------------------------------------
void consoleDrawChar(int c) {
//---------------------------------------------------------------------------------
	consoleStoreTextCell(currentConsole->cursorX, currentConsole->cursorY, (u32)c);

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
	ConsoleTextBuffer* textBuffer = consoleFindTextBuffer(console);
	if (textBuffer)
		consoleResizeTextBuffer(textBuffer, width, height);

}
