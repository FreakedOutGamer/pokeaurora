#!/usr/bin/env python3
"""Convert between GBA graphics formats and PNG/JASC palettes.

usage:
  gbagfx.py in.png out.{1bpp,4bpp,8bpp} [-mwidth N] [-mheight N] [-num_tiles N]
  gbagfx.py in.{1bpp,4bpp,8bpp} out.png -width N [-palette in.gbapal]
            [-palnum N] [-mwidth N] [-mheight N]
  gbagfx.py in.pal out.gbapal
  gbagfx.py in.gbapal out.pal

Tiles are 8x8 pixels, laid out left to right, top to bottom; with -mwidth and
-mheight they are grouped in metatiles of that many tiles (e.g. 16x16 glyphs
are -mwidth 2 -mheight 2). -width is the image's width in metatiles. 4bpp and
8bpp images are indexed PNGs; only the pixel indices are converted back, so
the palette in a PNG is for viewing. 1bpp is stored with the leftmost pixel in
the high bit.
"""
import struct
import sys
import zlib


def read_png(path):
    data = open(path, 'rb').read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        sys.exit('%s: not a PNG' % path)
    pos = 8
    idat = b''
    plte = []
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            width, height, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
        elif kind == b'PLTE':
            plte = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
        elif kind == b'IDAT':
            idat += body
    if ctype not in (0, 3) or interlace:
        sys.exit('%s: must be an indexed or grayscale, non-interlaced PNG' % path)
    raw = zlib.decompress(idat)
    stride = (width * depth + 7) // 8
    bpp = max(1, depth // 8)
    rows = []
    prev = bytearray(stride)
    pos = 0
    for _ in range(height):
        ftype = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if ftype == 1:
                line[i] = (line[i] + a) & 0xFF
            elif ftype == 2:
                line[i] = (line[i] + b) & 0xFF
            elif ftype == 3:
                line[i] = (line[i] + (a + b) // 2) & 0xFF
            elif ftype == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else b if pb <= pc else c
                line[i] = (line[i] + pred) & 0xFF
        prev = line
        pixels = []
        mask = (1 << depth) - 1
        for x in range(width):
            bit = x * depth
            pixels.append((line[bit // 8] >> (8 - depth - bit % 8)) & mask)
        rows.append(pixels)
    return width, height, rows, plte


def write_png(path, width, height, rows, palette, depth):
    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body) & 0xFFFFFFFF)

    raw = bytearray()
    for pixels in rows:
        raw.append(0)
        line = bytearray((width * depth + 7) // 8)
        for x, p in enumerate(pixels):
            bit = x * depth
            line[bit // 8] |= p << (8 - depth - bit % 8)
        raw += line
    out = b'\x89PNG\r\n\x1a\n'
    out += chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, depth, 3, 0, 0, 0))
    out += chunk(b'PLTE', b''.join(bytes(c) for c in palette))
    out += chunk(b'IDAT', zlib.compress(bytes(raw), 9))
    out += chunk(b'IEND', b'')
    open(path, 'wb').write(out)


def tile_positions(count, width, mwidth, mheight):
    """(x, y) in tiles of each tile, for an image `width` metatiles wide."""
    per_meta = mwidth * mheight
    for t in range(count):
        meta, sub = divmod(t, per_meta)
        mx, my = meta % width, meta // width
        yield mx * mwidth + sub % mwidth, my * mheight + sub // mwidth


def tiles_to_png(src, dst, bits, width, palette, mwidth, mheight):
    data = open(src, 'rb').read()
    tile_size = 8 * bits
    count = len(data) // tile_size
    if count * tile_size != len(data):
        sys.exit('%s: not a whole number of tiles' % src)
    metas = -(-count // (mwidth * mheight))
    height = -(-metas // width)
    w, h = width * mwidth * 8, height * mheight * 8
    rows = [[0] * w for _ in range(h)]
    for t, (tx, ty) in enumerate(tile_positions(count, width, mwidth, mheight)):
        tile = data[t * tile_size:(t + 1) * tile_size]
        for y in range(8):
            for x in range(8):
                if bits == 1:
                    p = (tile[y] >> (7 - x)) & 1
                elif bits == 4:
                    p = (tile[y * 4 + x // 2] >> (4 * (x & 1))) & 15
                else:
                    p = tile[y * 8 + x]
                rows[ty * 8 + y][tx * 8 + x] = p
    write_png(dst, w, h, rows, palette, bits if bits != 4 else 4)


def png_to_tiles(src, dst, bits, mwidth, mheight, num_tiles):
    w, h, rows, _ = read_png(src)
    width = w // (mwidth * 8)
    count = (w // 8) * (h // 8)
    if num_tiles is not None:
        count = num_tiles
    out = bytearray()
    for tx, ty in tile_positions(count, width, mwidth, mheight):
        for y in range(8):
            row = rows[ty * 8 + y][tx * 8:tx * 8 + 8]
            if bits == 1:
                out.append(sum((p & 1) << (7 - x) for x, p in enumerate(row)))
            elif bits == 4:
                out += bytes((row[x] & 15) | (row[x + 1] & 15) << 4 for x in range(0, 8, 2))
            else:
                out += bytes(row)
    open(dst, 'wb').write(out)


def read_gbapal(path, strict=True):
    data = open(path, 'rb').read()
    colors = []
    for (c,) in struct.iter_unpack('<H', data):
        if c & 0x8000 and strict:
            sys.exit('%s: color %#x has bit 15 set' % (path, c))
        colors.append(((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3))
    return colors


def write_jasc(path, colors):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n%d\n' % len(colors))
        for c in colors:
            f.write('%d %d %d\n' % c)


def read_jasc(path):
    lines = open(path).read().split()
    if lines[:2] != ['JASC-PAL', '0100']:
        sys.exit('%s: not a JASC palette' % path)
    n = int(lines[2])
    vals = list(map(int, lines[3:3 + n * 3]))
    return [tuple(vals[i:i + 3]) for i in range(0, n * 3, 3)]


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        sys.exit(__doc__)
    src, dst = args[0], args[1]
    opts = {}
    i = 2
    while i < len(args):
        opts[args[i].lstrip('-')] = args[i + 1]
        i += 2
    ext_in, ext_out = src.rsplit('.', 1)[1], dst.rsplit('.', 1)[1]
    mwidth = int(opts.get('mwidth', 1))
    mheight = int(opts.get('mheight', 1))
    bits = {'1bpp': 1, '4bpp': 4, '8bpp': 8}
    if ext_in == 'png' and ext_out in bits:
        num = int(opts['num_tiles']) if 'num_tiles' in opts else None
        png_to_tiles(src, dst, bits[ext_out], mwidth, mheight, num)
    elif ext_in in bits and ext_out == 'png':
        b = bits[ext_in]
        if 'palette' in opts:
            pal = read_gbapal(opts['palette'], strict=False)
            if b == 4:
                n = int(opts.get('palnum', 0))
                pal = pal[n * 16:n * 16 + 16]
        elif b == 1:
            pal = [(255, 255, 255), (0, 0, 0)]
        else:
            pal = [(i * 255 // ((1 << b) - 1),) * 3 for i in range(1 << b)]
        pal = (pal + [(0, 0, 0)] * (1 << b))[:1 << b]
        tiles_to_png(src, dst, b, int(opts['width']), pal, mwidth, mheight)
    elif ext_in == 'pal' and ext_out == 'gbapal':
        colors = read_jasc(src)
        open(dst, 'wb').write(b''.join(
            struct.pack('<H', (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10) for r, g, b in colors))
    elif ext_in == 'gbapal' and ext_out == 'pal':
        write_jasc(dst, read_gbapal(src))
    else:
        sys.exit('don\'t know how to convert .%s to .%s' % (ext_in, ext_out))


main()
