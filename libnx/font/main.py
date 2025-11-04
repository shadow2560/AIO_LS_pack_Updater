from PIL import Image, ImageFont, ImageDraw
from fontTools.ttLib import TTFont
import struct

font_paths = ['unscii-8.ttf']
output_path = 'default_font.bin'
cell_size = 16

# New indexed binary format:
# header: magic ('FN16'), version (1), cell_size (16), glyph_count, index_offset, bitmap_offset
# index: glyph_count * uint32 (sorted ascending codepoints; written little-endian to match C u32)
# bitmaps: glyph_count * (cell_size * 2) bytes (cell_size u16 big-endian per glyph — here 16 u16 => 32 bytes per glyph)

MAGIC = 0x464E3136  # 'FN16'
VERSION = 1

# Load PIL fonts for rendering
pil_fonts = [ImageFont.truetype(path, cell_size) for path in font_paths]

def get_supported_codepoints(ttf_path: str):
    ttf = TTFont(ttf_path)
    cmap = ttf.getBestCmap() or {}
    cps = [cp for cp in cmap.keys() if cp < 0x110000 and not (0xD800 <= cp <= 0xDFFF) and (cp & 0xFFFE) != 0xFFFE]
    cps.sort()
    return cps

def render_glyph(char, font):
    # 0 = black, 1 = white
    img = Image.new("1", (cell_size, cell_size), 0)
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), char, font=font, fill=1)

    rows = []
    for y in range(cell_size):
        row = 0
        for x in range(cell_size):
            if img.getpixel((x, y)):
                row |= (1 << (cell_size - 1 - x))
        # swap left 8 and right 8 bits
        swapped = ((row & 0xFF) << 8) | ((row >> 8) & 0xFF)
        rows.append(swapped & 0xFFFF)

    return b''.join(struct.pack('>H', r) for r in rows)



# Build union of supported codepoints across all fonts (priority order kept for rendering)
font_codepoint_sets = []
for path in font_paths:
    try:
        cps = set(get_supported_codepoints(path))
    except Exception:
        cps = set()
    font_codepoint_sets.append(cps)

all_codepoints = set()
for s in font_codepoint_sets:
    all_codepoints |= s

codepoints = sorted(all_codepoints)

glyph_bitmaps = []
for cp in codepoints:
    ch = chr(cp)
    # pick the first font that supports this codepoint
    chosen_font = None
    for idx, cps in enumerate(font_codepoint_sets):
        if cp in cps:
            chosen_font = pil_fonts[idx]
            break
    if chosen_font is None:
        # Should not happen since codepoints was built from union, but guard anyway
        chosen_font = pil_fonts[0]
    glyph_bitmaps.append(render_glyph(ch, chosen_font))

glyph_count = len(glyph_bitmaps)
index_offset = 4 + 2 + 2 + 4 + 4 + 4  # taille de l'entête
bitmap_offset = index_offset + glyph_count * 4

with open(output_path, 'wb') as f:
    # header
    f.write(struct.pack('<I', MAGIC))
    f.write(struct.pack('<H', VERSION))
    f.write(struct.pack('<H', cell_size))
    f.write(struct.pack('<I', glyph_count))
    f.write(struct.pack('<I', index_offset))
    f.write(struct.pack('<I', bitmap_offset))
    for cp in codepoints:
        f.write(struct.pack('<I', cp))
    # bitmaps
    for bmp in glyph_bitmaps:
        f.write(bmp)


