#!/usr/bin/env python3
"""Compare a built function against the original ROM.

usage: tools/fdiff.py <function> [original_address]
       tools/fdiff.py --all

The original address defaults to the address in config/funcs.txt, then to
the XXXXXXXX of sub_XXXXXXXX names, then to the function's address in the
build. Prints a side-by-side instruction diff and exits non-zero on mismatch.
--all checks every function named in config/funcs.txt.
"""
import os
import re
import subprocess
import sys
import signal

signal.signal(signal.SIGPIPE, signal.SIG_DFL)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASEROM = os.path.join(ROOT, 'JPAJ - Aurora Ticket (USA).gba')
ELF = os.path.join(ROOT, 'aurora_ticket.elf')
OBJDUMP = 'arm-none-eabi-objdump'


def symbols(elf):
    out = subprocess.run(['arm-none-eabi-nm', '-n', elf], capture_output=True, text=True).stdout
    funcs = []
    for line in out.splitlines():
        p = line.split()
        if len(p) == 3 and p[1] in 'TR':
            funcs.append((int(p[0], 16), p[2]))
    syms = {}
    for i, (a, n) in enumerate(funcs):
        end = next((b for b, _ in funcs[i + 1:] if b > a), a)
        syms[n] = (a, end - a)
    return syms


def disasm(data, base, mode):
    tmp = os.path.join(ROOT, 'build', '.fdiff.bin')
    open(tmp, 'wb').write(data)
    args = [OBJDUMP, '-D', '-b', 'binary', '-marm', '--adjust-vma=%#x' % base]
    if mode == 'thumb':
        args.append('-Mforce-thumb')
    out = subprocess.run(args + [tmp], capture_output=True, text=True).stdout
    lines = []
    for line in out.splitlines():
        m = re.match(r'\s*([0-9a-f]+):\s+([0-9a-f ]+?)\s{2,}(.*)', line)
        if m:
            lines.append((int(m.group(1), 16) - base, m.group(2).replace(' ', ''), m.group(3)))
    return lines


def funcs_table():
    table = {}
    for line in open(os.path.join(ROOT, 'config', 'funcs.txt')):
        p = line.split()
        if len(p) >= 2 and not p[0].startswith('#'):
            table[p[1]] = int(p[0], 16)
    return table


def loose_equal(la, lb, ta, tb, moved):
    """Ignore differences that only come from code or data moving."""
    ma, mb = ta.split()[:1], tb.split()[:1]
    if not ma or ma != mb:
        return False
    op = ma[0]
    # bl / b to relocated targets
    if op.startswith('bl') and len(la[1]) == 8 and len(lb[1]) == 8:
        return True
    if moved and op.startswith('b') and la[1][:1] == lb[1][:1]:
        return True
    # literal pool words holding ROM / RAM addresses
    if len(la[1]) == 8 and len(lb[1]) == 8 and la[1][:2] == lb[1][:2] and la[1][:2] in ('02', '03', '08'):
        return True
    return False


def main():
    func = sys.argv[1]
    elf = ELF
    syms = symbols(elf)
    table = funcs_table()
    if func == '--all':
        if not table:
            # every global function in the build
            out = subprocess.run(['arm-none-eabi-readelf', '-sW', elf], capture_output=True, text=True).stdout
            for line in out.splitlines():
                p = line.split()
                if len(p) >= 8 and p[3] == 'FUNC' and p[4] == 'GLOBAL' and syms.get(p[7], (0, 0))[1] > 0:
                    table[p[7]] = int(p[1], 16) & ~1
        bad = []
        for name, orig in sorted(table.items(), key=lambda x: x[1]):
            if name in syms:
                r = subprocess.run([sys.executable, __file__, name, '%x' % orig], capture_output=True, text=True)
                if r.returncode:
                    bad.append(name)
        print('mismatching:', ' '.join(bad) if bad else 'none')
        sys.exit(1 if bad else 0)
    if func not in syms:
        sys.exit('%s not found in %s' % (func, elf))
    addr, size = syms[func]
    if len(sys.argv) > 2:
        orig_addr = int(sys.argv[2], 16)
    elif func in table:
        orig_addr = table[func]
    elif re.search(r'_([0-9A-Fa-f]{8})$', func):
        orig_addr = int(re.search(r'([0-9A-Fa-f]{8})$', func).group(1), 16)
    else:
        orig_addr = addr
    elfbin = subprocess.run([OBJDUMP, '-s', '--start-address=%#x' % addr,
                             '--stop-address=%#x' % (addr + size), elf], capture_output=True, text=True).stdout
    built = bytearray()
    for line in elfbin.splitlines():
        m = re.match(r'\s*([0-9a-f]{7,8}) ((?:[0-9a-f]{2,8} ){1,4})', line + ' ')
        if m:
            built += bytes.fromhex(m.group(2).replace(' ', ''))
    built = bytes(built[:size])
    rom, base = open(BASEROM, 'rb').read(), 0x08000000
    orig = rom[orig_addr - base: orig_addr - base + size]
    mode = 'thumb'
    if addr != orig_addr or True:
        # normalise literal pool words holding (possibly relocated) addresses
        b2 = bytearray(built)
        for i in range((4 - addr % 4) % 4, min(len(built), len(orig)) - 3, 4):
            wb = int.from_bytes(built[i:i + 4], 'little')
            wo = int.from_bytes(orig[i:i + 4], 'little')
            if addr <= wb < addr + size and orig_addr <= wo < orig_addr + size:
                # pointer into the function itself (jump table): compare offsets
                if wb - addr == wo - orig_addr:
                    b2[i:i + 4] = orig[i:i + 4]
            elif built[i + 3] == orig[i + 3] and built[i + 3] in (2, 3, 8) and built[i + 2] == orig[i + 2]:
                b2[i:i + 4] = orig[i:i + 4]
        built = bytes(b2)
    a = disasm(built, addr, mode)
    b = disasm(orig, orig_addr, mode)
    bad = 0
    for i in range(max(len(a), len(b))):
        la = a[i] if i < len(a) else (0, '', '')
        lb = b[i] if i < len(b) else (0, '', '')
        ta = re.sub(r'\s*[;@].*', '', la[2])
        tb = re.sub(r'\s*[;@].*', '', lb[2])
        same = la[1] == lb[1] or loose_equal(la, lb, ta, tb, addr != orig_addr)
        mark = '  ' if same else '!!'
        if not same:
            bad += 1
        print('%s %04x %-8s %-32s | %-8s %s' % (mark, la[0], la[1], ta, lb[1], tb))
    print('%s: %d differing instructions' % (func, bad) if bad else '%s: OK' % func)
    sys.exit(1 if bad else 0)


main()
