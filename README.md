# Aurora Ticket

A decompilation of the Aurora Ticket distribution cartridge (title
`AURORATICKET`, code `JPAJ`, USA). It builds a byte-for-byte copy of the
original:

    aurora_ticket.gba  sha1 94a21d133e3f1b1a2129f450a9bc753d12c13b09

The cartridge is a Mystery Gift server for Pokémon FireRed and LeafGreen. It
sends them a WONDER CARD and the script that gives the AURORA TICKET (for the
ship at Vermilion City's port) through the Wireless Adapter, and can also send
WONDER NEWS. It has setup screens ("Setup Menu", "POKEMON AI Setup", "Time
Setup"), a real-time clock and a flash save.

## Documentation

- [How the cartridge works](docs/how_it_works.md): start-up, the wireless
  slots, the Mystery Gift servers, WONDER NEWS, and the features this build
  doesn't use.
- [Save format](docs/save_format.md)
- [Changing what it sends](docs/modding.md): the card, the script (with a
  MYSTIC TICKET example), the news, text and graphics.

## Building

Requirements: `binutils-arm-none-eabi`, `make`, Python 3 (for `tools/`) and
[agbcc](https://github.com/pret/agbcc).

```sh
# build and install agbcc into tools/agbcc
git clone https://github.com/pret/agbcc
cd agbcc && ./build.sh && ./install.sh /path/to/Aurora-Ticket-Decomp && cd ..

make compare   # builds aurora_ticket.gba and checks its SHA-1
```

Objects go to `build/`.

## ROM layout

| Address | Contents | Source |
|---|---|---|
| 0x08000000 | Header, start-up and interrupt code | `asm/crt0.s` |
| 0x08000258 | The game's code: flash save, content tables, BG/OAM helpers, main loop, wireless link, setup menus, screens and text, Mystery Gift server, PC controller link | `src/*.c` (see below) |
| 0x08003780 | Nintendo's wireless link manager | `src/AgbRfu_LinkManager.c` |
| 0x08007790 | librfu RFU_V1021: STWI, interrupts, rfu, Sio32ID | `src/librfu_*.c` |
| 0x0800BDA4 | SIIRTC_V001 (RTC) | `src/siirtc.c` |
| 0x0800C66C | AGB flash library | `src/agb_flash.c` |
| 0x0800CB6C | BIOS calls | `asm/libagbsyscall.s` |
| 0x0800CB98 | agbcc's libgcc and libc | `tools/agbcc/lib` |
| 0x08012728 | The script sent to the games | `data/event_script.s` |
| 0x080128F8 | Tables, text, Mystery Gift scripts, WONDER CARD and NEWS, an unused font | `data/rodata.s` |
| 0x080516C0 | Graphics | `data/graphics.s`, `graphics/` |
| 0x0808E2C0 | The libraries' read-only data | `src/librfu_*.c`, ..., libc |
| 0x0808E5D4 | The `.data` section, copied to EWRAM (0x02000000) at start-up: the save buffer, content and slot buffers (all zero) | `src/save.c`, `src/content.c`, `src/link.c`, libc |

RAM variables that aren't initialized are placed at their original addresses
by `ld/syms.ld`.
`config/funcs.txt` names the functions by address.

## Notes on matching

- The C libraries come from pret's [pokeemerald](https://github.com/pret/pokeemerald).
  This cartridge has an older librfu (RFU_V1021, where Emerald has 1026), so
  the differences are under `LIBRFU_VERSION` (`include/librfu.h`):
  - `struct RfuStatic` is smaller, and four of its fields are separate
    variables.
  - The API buffer reserves 0x38 more bytes.
  - `rfu_REQ_stopMode` doesn't wait.
  - The link manager doesn't report some disconnects, and calls the game's own
    disconnect function (`RfuDisconnectSlots`).
- The original libc was linked with its objects sorted by name, so the build
  links a sorted copy of agbcc's `libc.a`, between two passes of `libgcc.a`.
- Compiled C is padded with 0 rather than a nop, as in pret's builds.

## Decompilation status

- In C: all of the game's code, plus librfu, the link manager, the RTC and
  flash libraries. Everything matches.
- Still assembly: only `asm/crt0.s` (start-up and the ARM interrupt handler)
  and `asm/libagbsyscall.s` (BIOS call stubs), which were assembly originally.
- Linked from agbcc: libgcc and libc.
- Data: in `data/*.s`, with the graphics in `graphics/`. Nothing is copied
  from the original ROM.

The game's modules, in link order:

| File | Contents |
|---|---|
| `save.c` | Flash save (two banks per sector, checksums) |
| `content.c` | Dated content tables and their text |
| `bg.c` | Tilemap printing and OAM helpers |
| `main.c` | Interrupts, VBlank, `AgbMain`, expiry lock |
| `rfu.c` | Wireless link glue over the link manager |
| `link.c` | Per-slot task manager, debug screen, hourly refresh |
| `setup.c` | Setup, time and POKEMON AI menus |
| `misc.c` | Title screen, keys, cursor, BCD, distribution task |
| `ui.c` | Fades, sprites, Shift-JIS text rendering |
| `mg.c` | Mystery Gift server (an older pokeemerald `mystery_gift_server.c`) and its wireless transport |
| `ctrl.c` | Link to a PC-side "JSPCONTROLLER" (clock, pages, stats) |

## Data

- `data/event_script.s` is the Pokémon field script the card server sends,
  with the commands in `asm/macros/event.inc`.
- `data/rodata.s` has the tables and text, the WONDER CARD and WONDER NEWS,
  and the Mystery Gift servers' scripts and the scripts they send to the
  games (`asm/macros/mg.inc`, named as in pokeemerald).
- The text the games show is written as `.string "..."`, which
  `tools/preproc.py` encodes with `charmap.txt` (the Gen 3 character set,
  Japanese and international) before assembling. `.string "...", 40` pads a
  fixed-size field.
- `graphics/` has each screen's tiles (`tiles.png`), palette (`palette.pal`,
  JASC) and tilemaps (`*.bin`). The build converts them with
  `tools/gbagfx.py`. The PNGs show one of the palette's 16-color rows, and
  only their pixel indices are used. The expired screen's palette is kept as
  `palette.gbapal` because some of its colors have the unused top bit set.
- `graphics/font/font.png` is a 16x16 font (ASCII, kana and kanji) that the
  cartridge doesn't use.

## Tools

- `tools/fdiff.py <function>` diffs a built function against the original
  ROM; `tools/fdiff.py --all` checks every function in `config/funcs.txt`.
- `tools/try.py <file.c> [-v] [function...]` compiles a C file and checks
  each function against the ROM at its original address, without changing
  the build (run `make` first).
- `tools/gbagfx.py` converts between PNG and 1/4/8bpp tiles, and between JASC
  and GBA palettes.
- `tools/preproc.py` encodes `.string` text in the data files.
- `tools/render_screens.py` draws each screen from `graphics/` into
  `build/screens/` (run `make` first).
