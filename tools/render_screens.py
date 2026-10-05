#!/usr/bin/env python3
"""Render the cartridge's screens from graphics/ as the game shows them.

usage: tools/render_screens.py [out_dir]

Draws each screen's tilemap with its tiles and palette (the same pairs the
code loads, see src/ui.c and src/main.c) into out_dir (build/screens by
default), to check the graphics after editing them. Run `make` first: it
reads the built .4bpp/.8bpp and .gbapal files.
"""
import os
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# name: (tiles, palette, tilemap, bpp, width and height in tiles)
SCREENS = {
    'adapter_error': ('adapter_error/tiles.4bpp', 'adapter_error/palette.gbapal', 'adapter_error/tilemap.bin', 4, 32, 32),
    'please_wait': ('please_wait/tiles.4bpp', 'please_wait/palette.gbapal', 'please_wait/tilemap.bin', 4, 32, 32),
    'sending_text': ('sending/tiles.4bpp', 'sending/palette.gbapal', 'sending/text.bin', 4, 32, 32),
    'sending_background': ('sending/tiles.4bpp', 'sending/palette.gbapal', 'sending/background.bin', 4, 32, 32),
    'id_select': ('id_change/tiles.4bpp', 'id_change/palette.gbapal', 'id_change/select.bin', 4, 32, 32),
    'id_changed': ('id_change/tiles.4bpp', 'id_change/palette.gbapal', 'id_change/changed.bin', 4, 32, 32),
    'expired': ('expired/tiles.8bpp', 'expired/palette.gbapal', 'expired/tilemap.bin', 8, 32, 32),
}


def read(path):
    return open(os.path.join(ROOT, 'graphics', path), 'rb').read()


def palette(data):
    return [((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3)
            for (c,) in struct.iter_unpack('<H', data)]


def write_png(path, width, height, rows):
    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body) & 0xFFFFFFFF)

    raw = b''.join(b'\0' + b''.join(bytes(p) for p in row) for row in rows)
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n'
                           + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(raw, 9))
                           + chunk(b'IEND', b''))


def render(tiles, pal, tilemap, bpp, w, h):
    tile_size = 8 * bpp
    rows = [[(0, 0, 0)] * (w * 8) for _ in range(h * 8)]
    for i in range(w * h):
        entry, = struct.unpack_from('<H', tilemap, i * 2)
        num, hflip, vflip, palnum = entry & 0x3FF, entry >> 10 & 1, entry >> 11 & 1, entry >> 12
        tile = tiles[num * tile_size:(num + 1) * tile_size]
        if len(tile) < tile_size:
            continue
        bx, by = i % w * 8, i // w * 8
        for y in range(8):
            for x in range(8):
                if bpp == 4:
                    color = pal[palnum * 16 + (tile[y * 4 + x // 2] >> (4 * (x & 1)) & 15)]
                else:
                    color = pal[tile[y * 8 + x]]
                rows[by + (7 - y if vflip else y)][bx + (7 - x if hflip else x)] = color
    return rows


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'build', 'screens')
    os.makedirs(out, exist_ok=True)
    for name, (tiles, pal, tilemap, bpp, w, h) in SCREENS.items():
        rows = render(read(tiles), palette(read(pal)), read(tilemap), bpp, w, h)
        path = os.path.join(out, name + '.png')
        write_png(path, w * 8, h * 8, rows)
        print(path)


main()
