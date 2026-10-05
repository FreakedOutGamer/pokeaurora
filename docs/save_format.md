# Save format

The cartridge saves to 128 KB of flash: 32 sectors of 4 KB (`src/save.c`,
with the flash library in `src/agb_flash.c`). In RAM the save is
`gSaveBuffer`, 16 sectors of 4 KB at the start of EWRAM.

## Layout

Buffer sector 0 is the header (`struct SaveHeader`, `include/aurora.h`):

| Offset | Size | Field | |
|---|---|---|---|
| 0x00 | 4 | `magic` | `"AURO"` (`sSaveMagic`) |
| 0x04 | 16 | `bank[16]` | which flash copy of each buffer sector is current (0 or 1) |
| 0x14 | 128 | `checksums[32]` | checksum of each flash sector |
| 0x94 | 4 | `unk94` | never written |
| 0x98 | 24 | `counters[6]` | `IncrementSaveCounter`, never called |
| 0xB0 | 4 | `distributionCount` | incremented for each game a gift is sent to |
| 0xB4 | 2 | `terminalId` | 1-10, chosen on the ID change screen and shown while sending |
| 0xFFC | 4 | `checksum` | checksum of the header |

A checksum is the 32-bit sum of the sector's words (the header's leaves out
its last word, the checksum itself).

Each buffer sector is kept twice in flash:

- the header in flash sectors 0 and 16;
- buffer sector *i* (1-15) in flash sector *i* (bank 0) or *i* + 16 (bank 1).

## Loading

`LoadSaveHeader` reads flash sector 0, then 16, and keeps the first with the
right magic and checksum. If neither is valid, the header is cleared, with
terminal ID 1. `LoadSaveSectors` then reads each buffer sector from its current
bank, or the other one if the checksum doesn't match, and clears it if neither
matches.

## Saving

The save code marks changed sectors in `sSectorDirty`.

- `SaveAll` writes each changed data sector to both banks, then the header to
  sectors 0 and 16.
- `SaveStep` saves a step at a time: first it flips the bank of each changed
  data sector, updates the checksums and writes the header to sector 0; then
  one data sector per call to its new bank; then the header to sector 16.
  If it's interrupted, loading falls back to each sector's other copy.

In this build, only `SetTerminalId` (the ID change screen) saves, with
`SaveAll`. `UpdatePeriodicSave`, which saves step by step while games are
connected (every `SAVE_INTERVAL`, an hour) or all at once when none are, isn't
called.

## Per-game blocks

Buffer sectors 1-15 are meant to hold a block for each kind of game, found by
the connected game's serial number (`sSaveBlockIds`, the same list as
`sAcceptedSerialNos`) with its size in `sSaveBlockSizes`. `ReadSaveBlock`,
`WriteSaveBlock` and `ClearSaveBlock` access the current slot's block. The only
serial number, 0x0002, has a block of size 0, and nothing calls these
functions.
