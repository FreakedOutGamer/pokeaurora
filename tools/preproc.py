#!/usr/bin/env python3
"""Encode the game's text in an assembly file.

usage: tools/preproc.py <file.s> <charmap.txt>

Writes the file to stdout with each `.string "..."` replaced by the bytes of
the text in the game's character set (charmap.txt). Inside the quotes, `{NAME}`
is a control code from the charmap and `{0xNN}` a raw byte; \\n, \\l and \\p
are the line, scroll and page breaks, and `$` ends the string. Nothing is added
at the end, so terminators are written out. `.string "...", N` pads the text
with 0 to N bytes, for the fixed-size text fields.
"""
import re
import sys


def load_charmap(path):
    chars, names = {}, {}
    for line in open(path, encoding='utf-8'):
        line = line.split('@', 1)[0].strip() if not line.lstrip().startswith("'@'") else line.strip()
        if not line:
            continue
        key, _, value = line.rpartition('=')
        key = key.strip()
        code = bytes(int(v, 16) for v in value.split())
        if key.startswith("'"):
            text = key[1:-1]
            if text.startswith('\\'):
                text = {'\\n': '\n', '\\l': '\v', '\\p': '\f', "\\'": "'", '\\\\': '\\'}[text]
            chars[text] = code
        else:
            names[key] = code
    return chars, names


def encode(text, chars, names, where):
    out = bytearray()
    i = 0
    while i < len(text):
        c = text[i]
        if c == '\\':
            esc = text[i + 1]
            c = {'n': '\n', 'l': '\v', 'p': '\f', '"': '"', "'": "'", '\\': '\\'}.get(esc)
            if c is None:
                sys.exit('%s: unknown escape \\%s' % (where, esc))
            i += 2
        elif c == '{':
            j = text.index('}', i)
            token = text[i + 1:j]
            if token in names:
                out += names[token]
            elif re.fullmatch(r'0x[0-9A-Fa-f]{1,2}', token):
                out.append(int(token, 16))
            else:
                sys.exit('%s: unknown control code {%s}' % (where, token))
            i = j + 1
            continue
        else:
            i += 1
        if c not in chars:
            sys.exit('%s: %r is not in the charmap' % (where, c))
        out += chars[c]
    return out


def main():
    src, charmap = sys.argv[1], sys.argv[2]
    chars, names = load_charmap(charmap)
    for n, line in enumerate(open(src, encoding='utf-8'), 1):
        m = re.match(r'(\s*(?:\w+:\s*)?)\.string\s+"((?:[^"\\]|\\.)*)"\s*(?:,\s*(\w+))?\s*(@.*)?$', line.rstrip('\n'))
        if m:
            where = '%s:%d' % (src, n)
            data = encode(m.group(2), chars, names, where)
            if m.group(3):
                size = int(m.group(3), 0)
                if len(data) > size:
                    sys.exit('%s: text is %d bytes, longer than %d' % (where, len(data), size))
                data += bytes(size - len(data))
            line = '%s.byte %s\n' % (m.group(1), ', '.join('0x%02X' % b for b in data))
        sys.stdout.write(line)


main()
