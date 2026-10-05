# How the cartridge works

The Aurora Ticket cartridge is a Mystery Gift server. Up to four FireRed or
LeafGreen games connect to it through the Wireless Adapter at once, and each
one is sent a WONDER CARD and the script that gives the AURORA TICKET.

## Start-up (`src/main.c`)

`AgbMain` clears RAM and VRAM (except `gPreservedValue`, which survives a soft
reset), copies `.data` to EWRAM, installs the interrupt handler, starts
librfu and fades in the "Please Wait..." screen. The main loop then calls
`sMainCallbacks[gMainState]` every frame:

| State | Function | |
|---|---|---|
| `MAIN_STATE_NONE` | `MainState_None` | does nothing |
| `MAIN_STATE_TITLE` | `TitleScreen_Main` | waits 8 frames, then goes on to distributing. Its title screen ("JoySpot test program", "Ver. 0.4", `TitleScreen_Init`) is never drawn. |
| `MAIN_STATE_SLOTS` | `SlotTasks_Main` | distributing |
| `MAIN_STATE_SETUP` | `SetupMenu_Main` | the setup, time and POKéMON AI menus. Nothing sets this state. |

## Distributing (`src/link.c`, `src/rfu.c`)

`SlotTasks_Init` starts the wireless link manager (`ResetRfu`). It tries 14
times. If the adapter doesn't answer, it shows the "check the Wireless Adapter"
screen and stops.

The cartridge is the parent, with serial number `RFU_SERIAL_NO` (0x7F7D), and
accepts children whose serial number is in `sAcceptedSerialNos` (0x0002). The
game name it advertises is built by `SetRfuGameData` (as Emerald's
`RfuGameData`), and its user name is "ターミナル" ("Terminal").

Each of the four child slots runs a task: a function and a 32 KB work buffer
(`gSlotWork`). `SlotTasks_Main` calls each slot's task once a frame, times it
with timer 1, and frees the slot when the task returns nonzero. Then it runs
the wireless code (`RfuMain`):

- `RfuHandleNewChildren`: when a game connects, it calls its serial number's
  entry in `sRfuConnectCallbacks`, `StartDistribution`, which gives the slot
  `DistributionTask`.
- `RfuSendNextSlot`: sends each slot's pending UNI or NI data in turn.
- `RfuCheckAcks`, `RfuCheckNISend`, `RfuCheckNIRecv`, `RfuStoreUNIRecv`:
  delivery, timeouts and received data. A slot that times out is disconnected.
- `RfuUpdateGameData`: refreshes the advertised game data every 54 frames,
  marking it full when four games are connected.
- `RfuCheckReset`: resets the game after a link manager error, and restarts the
  link after 600 calls with nothing connected (`RfuMain` runs twice a frame,
  so about 5 seconds).

## Sending a gift (`src/misc.c`, `src/mg.c`)

`DistributionTask` drops the game if its signal is weaker than
`DISTRIBUTION_MIN_STRENGTH` (it must be held close). After a second, it
starts the Mystery Gift server (`StartMgServer`) on the slot and counts the
distribution in the save.

The server is an older version of pokeemerald's `mystery_gift_server.c` over
its own packet transport (`MgRfuLink_*`: 14-byte packets with pokeemerald's
`RFUCMD_*` commands, carrying 12 bytes each):

1. Handshake: it sends its server info ("GameFreak inc.", the terminal ID)
   and reads the game's.
2. It runs a server script (`data/rodata.s`). `sDistributionType & 1` picks
   it from `sMgScripts`:
   - 0, the card server (this cartridge: `sDistributionType` is 2). It sends a
     client script that makes the game send its data, checks it, and then
     sends `sWonderCard` and `sAuroraTicketScript`, or asks the player to toss
     their old card, or tells them they already have it.
   - 1, the news server. It finds the WONDER NEWS current for the game
     (`MysteryGift_FindEntry`, below) and sends it.
3. The client scripts it sends are run by the game (pokeemerald's `CLI_*`
   commands), and end by returning a result the game shows a message for.

## WONDER NEWS (`src/content.c`)

Every hour (`UpdateHourly`), the cartridge reads the clock and lists the news
in `sContentEntries` whose start and end dates include it (BCD
year/month/day/hour), after `sDefaultContentEntry`, the ranking news. A news
entry also has four questionnaire words that a game's answers must match
(0xFFFF matches anything). `BuildRankingNews` fills in the ranking for each
game, but the ranking functions are stubs, so it says the player has no
battle record.

## Screens (`src/ui.c`)

While distributing, the screen shows "Now Sending AURORATICKET" with the
terminal ID, scrolling its background (`ScrollSendingScreen`), or the adapter
error screen. The terminal ID (1-10) is chosen on the "ID番号の変更" screen
(`ChooseNumberScreen`), opened by holding A+B (`CheckIdChangeKeys`). Nothing
calls `CheckIdChangeKeys` in this build.

`UpdateHourly` also calls `CheckExpiryReset`, which resets the game once the
clock reaches `sExpiryDate` (BCD 00-01-01 00:00 here). The expired screen
("このカートリッジは使用できません", "this cartridge can't be used",
`ShowExpiredScreen`) is only shown by `CheckExpiry`, which nothing calls.

## Leftovers

Present but not reached in this build:

- The debug screen (`DebugScreen_*` in `src/link.c`): each slot's serial
  number, time and signal strength, with the debug font.
- The setup menus (`src/setup.c`): the clock and POKéMON AI settings.
- The PC controller (`src/ctrl.c`): a "JSPCONTROLLER"/"CHILDTKD" child that
  sets the clock and reads statistics. It isn't in `sRfuConnectCallbacks`.
- The title screen and the expired screen (above).
- `FlashSendingScreen`, an alternative animation for the sending screen.
- The 16x16 font in `graphics/font/` and the Japanese WONDER CARD.
