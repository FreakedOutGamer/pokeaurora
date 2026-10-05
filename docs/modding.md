# Changing what the cartridge sends

Everything the cartridge sends is in `data/`, as text. After a change, build
with `make` (`make compare` will fail, since the ROM no longer matches).

## The WONDER CARD

`sWonderCard` in `data/rodata.s`:

```
sWonderCard:
	wonder_card 0x3e8, 0xffff, 0, 0, 2, 0, 0
	.string "         “AURORA TICKET”", 40
	.string "     Exchange Card 2004 FALL", 40
	...
```

- `wonder_card flagId, iconSpecies, idNumber, type, bgType, sendType, maxStamps`.
  `flagId` decides which "received" flag the game checks: in pokefirered,
  card IDs from 1000 (`WONDER_CARD_FLAG_OFFSET`) stand for
  `FLAG_RECEIVED_AURORA_TICKET` (1000), `FLAG_RECEIVED_MYSTIC_TICKET` (1001),
  and so on. The server compares it with the game's card
  (`SVR_CHECK_EXISTING_CARD`) to tell whether it already has this one.
  `iconSpecies` 0xFFFF means no icon.
- Then eight lines of 40 bytes: title, subtitle, four lines of body and two of
  footer. Text that's too long is an error.

## The script

`data/event_script.s` is the field script the game saves and runs when the
player talks to the deliveryman, written with pokefirered's commands
(`asm/macros/event.inc`). It has to fit in the game's RAM script, 995 bytes.
Its jumps and messages use `vgoto_if` and `vmessage`, which are relative to
`setvaddress` at its start, so labels work as usual.

### Example: the MYSTIC TICKET

To send the MYSTIC TICKET (for Navel Rock) instead, with the values from
pokefirered:

1. Add the constants to `asm/macros/event.inc`:

   ```
   	.set ITEM_MYSTIC_TICKET, 370
   	.set FLAG_RECEIVED_MYSTIC_TICKET, 0x2A8
   	.set FLAG_ENABLE_SHIP_NAVEL_ROCK, 0x84A
   ```

2. In `data/event_script.s`, use `ITEM_MYSTIC_TICKET`,
   `FLAG_RECEIVED_MYSTIC_TICKET` and `FLAG_ENABLE_SHIP_NAVEL_ROCK` in place of
   the AURORA TICKET's item and flags. Drop or replace the
   `FLAG_FOUGHT_DEOXYS` check (Navel Rock has `FLAG_FOUGHT_LUGIA` and
   `FLAG_FOUGHT_HO_OH`), and change the messages.
3. Give `sWonderCard` flag ID 1001 and new text.

## WONDER NEWS

`sContentEntries` in `data/rodata.s` is the list of news, each shown between
two dates (BCD: `0x04, 0x02, 0x27, 0x00` is 2004-02-27, 00:00), up to the entry
starting with 0xFF. Each has four questionnaire words (0xFFFF matches any
game), `wonder_news id, sendType, bgType`, a title and ten lines of 40 bytes.
The news server only runs if bit 0 of `sDistributionType` is set (it's 2, the
card server).

## Text

`.string` text is encoded with `charmap.txt`, which has both the international
and the Japanese character sets, so a string can only use one game's set
(English FireRed and LeafGreen here). `{PLAYER}` is the player's name, `\n` a
new line, `\p` a new box, `\l` scrolls, and `$` ends a message. The fixed-size
fields (`.string "...", 40`) don't need the `$`.

## Graphics

Edit the PNGs in `graphics/`. Only the pixels' palette indices are used: keep
the image indexed, the same size, and use colors from the screen's palette
(`palette.pal`, which you can edit as well). Tilemaps (`*.bin`) are raw GBA
screen entries. `tools/render_screens.py` draws each screen after `make`, to
check the result in `build/screens/`.
