#!/usr/bin/env python3
"""Check C functions against the original ROM without changing the build.

usage: tools/try.py <file.c> [-v] [function...]

Compiles <file.c> the way the Makefile compiles the game's C, then links each
function on its own at its original address (from config/funcs.txt, or the
XXXXXXXX of a sub_XXXXXXXX name) and compares it with the ROM. Everything
else is resolved against the current build (aurora_ticket.elf, so run make
first), ld/syms.ld and config/funcs.txt. -v prints a side-by-side diff of
each mismatching function; naming functions limits the check to them.
"""
import difflib
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASEROM = os.path.join(ROOT, 'JPAJ - Aurora Ticket (USA).gba')
ELF = os.path.join(ROOT, 'aurora_ticket.elf')
TMP = os.path.join(ROOT, 'build', 'try')
P = 'arm-none-eabi-'


def run(args, **kw):
    r = subprocess.run(args, capture_output=True, text=True, **kw)
    if r.returncode:
        sys.stderr.write(r.stdout + r.stderr)
        sys.exit(2)
    return r.stdout


def nm(path):
    out = {}
    for line in run([P + 'nm', path]).splitlines():
        p = line.split()
        if len(p) == 3:
            out[p[2]] = (int(p[0], 16), p[1])
        elif len(p) == 2:
            out[p[1]] = (0, p[0])
    return out


FUNCS = {}
for line in open(os.path.join(ROOT, 'config', 'funcs.txt')):
    p = line.split()
    if len(p) >= 2 and not p[0].startswith('#'):
        FUNCS[p[1]] = int(p[0], 16)
LDSYMS = {}
for m in re.finditer(r'^(\w+) = (0x[0-9A-Fa-f]+);', open(os.path.join(ROOT, 'ld', 'syms.ld')).read(), re.M):
    LDSYMS[m.group(1)] = int(m.group(2), 16)


def orig_address(name):
    if name in FUNCS:
        return FUNCS[name]
    m = re.search(r'([0-9A-Fa-f]{8})$', name)
    if m:
        return int(m.group(1), 16)
    return None


def disasm(data, base):
    tmp = os.path.join(TMP, 'dis.bin')
    open(tmp, 'wb').write(data)
    out = run([P + 'objdump', '-D', '-b', 'binary', '-marm', '-Mforce-thumb',
               '--adjust-vma=%#x' % base, tmp])
    lines = []
    for line in out.splitlines():
        m = re.match(r'\s*([0-9a-f]+):\s+([0-9a-f ]+?)\s{2,}(.*)', line)
        if m:
            lines.append((int(m.group(1), 16), m.group(2).replace(' ', ''), m.group(3)))
    return lines


def split_functions(asm):
    """Split agbcc output into (header, [(name, text)])."""
    lines = asm.split('\n')
    starts = []
    for i, l in enumerate(lines):
        m = re.match(r'\s*\.type\s+(\w+),function', l)
        if m:
            j = i
            while j > 0 and not lines[j - 1].strip().startswith('.align'):
                j -= 1
            starts.append((j - 1, m.group(1)))
    header = '\n'.join(l for l in lines[:starts[0][0]] if not l.startswith('.gcc2'))
    funcs = []
    for k, (s, name) in enumerate(starts):
        e = starts[k + 1][0] if k + 1 < len(starts) else len(lines)
        out = []
        text = True
        for l in lines[s:e]:
            t = l.strip()
            if t.startswith('.comm') or t.startswith('.lcomm'):
                continue
            if t.startswith('.section') or t in ('.data', '.bss'):
                text = '.text' in t
            elif t == '.text':
                text = True
            if text:
                out.append(l)
        funcs.append((name, '\n'.join(out)))
    return header, funcs


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    src = args[0]
    only = set(args[1:])
    verbose = '-v' in sys.argv
    os.makedirs(TMP, exist_ok=True)
    base = os.path.join(TMP, os.path.basename(src)[:-2])
    run(['cpp', '-nostdinc', '-undef', '-I', 'include', '-I', 'tools/agbcc/include',
         '-iquote', 'include', src, '-o', base + '.i'], cwd=ROOT)
    run(['tools/agbcc/bin/agbcc', '-mthumb-interwork', '-Wimplicit', '-Wparentheses',
         '-O2', '-fhex-asm', base + '.i', '-o', base + '.s'], cwd=ROOT)
    header, funcs = split_functions(open(base + '.s').read())
    elfsyms = nm(ELF)
    rom = open(BASEROM, 'rb').read()

    bad = 0
    for name, text in funcs:
        if only and name not in only:
            continue
        want = orig_address(name)
        if want is None:
            print('%-28s no original address' % name)
            bad += 1
            continue
        fbase = base + '.' + name
        open(fbase + '.s', 'w').write(header + '\n.text\n' + text + '\n\t.align 2, 0\n')
        run([P + 'as', '-mcpu=arm7tdmi', fbase + '.s', '-o', fbase + '.o'], cwd=ROOT)
        own = nm(fbase + '.o')
        args = []
        for sym, (a, t) in own.items():
            if t != 'U':
                if sym in elfsyms:
                    args += ['-N', sym]
            elif sym not in elfsyms:
                if sym in LDSYMS:
                    args += ['--add-symbol', '%s=%#x,global' % (sym, LDSYMS[sym])]
                elif orig_address(sym) is not None:
                    args += ['--add-symbol', '%s=.text:%#x,function,global' % (sym, (orig_address(sym) - 0x08000000) | 1)]
        run([P + 'objcopy'] + args + [ELF, fbase + '.syms.elf'])
        open(fbase + '.ld', 'w').write('SECTIONS { .text %#x : { *(.text) } /DISCARD/ : { *(*) } }\n' % want)
        run([P + 'ld', '-T', fbase + '.ld', '-R', fbase + '.syms.elf', '-o', fbase + '.elf', fbase + '.o'], cwd=ROOT)
        run([P + 'objcopy', '-O', 'binary', '-j', '.text', fbase + '.elf', fbase + '.bin'])
        b = open(fbase + '.bin', 'rb').read()
        o = rom[want - 0x08000000:want - 0x08000000 + len(b)]
        ok = b == o
        if not ok and len(b) % 4 == 0 and b[-2:] == b'\0\0':
            # the padding after the function may differ
            ok = b[:-2] == o[:-2]
        print('%-28s %s' % (name, 'OK' if ok else 'MISMATCH'))
        if not ok:
            bad += 1
            if verbose:
                da, do = disasm(b, want), disasm(o, want)
                norm = lambda l: re.sub(r'\s*[;@].*', '', l[2])
                sm = difflib.SequenceMatcher(None, [norm(l) for l in da], [norm(l) for l in do], autojunk=False)
                for tag, i1, i2, j1, j2 in sm.get_opcodes():
                    if tag == 'equal':
                        continue
                    for k in range(max(i2 - i1, j2 - j1)):
                        la = da[i1 + k] if i1 + k < i2 else (0, '', '')
                        lo = do[j1 + k] if j1 + k < j2 else (0, '', '')
                        print('!! %08x %-36s | %08x %s' % (la[0], norm(la)[:36], lo[0], norm(lo)))
                    print('   --')
    sys.exit(1 if bad else 0)


main()
