#ifndef GUARD_AURORA_H
#define GUARD_AURORA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Flash save

#define SECTOR_SIZE 0x1000

struct SaveHeader
{
    /*0x00*/ u32 magic;
    /*0x04*/ u8 bank[16];           // which of the two copies of each sector is current
    /*0x14*/ u32 checksums[32];     // per flash sector (bank * 16 + sector)
    /*0x94*/ u32 unk94;
    /*0x98*/ u32 counters[6];
    /*0xB0*/ u32 distributionCount;
    /*0xB4*/ u16 terminalId;         // 1-10, chosen on the ID change screen
    /*0xB6*/ u8 filler_B6[SECTOR_SIZE - 0xB6 - 4];
    /*0xFFC*/ u32 checksum;
};

union SaveBuffer
{
    struct SaveHeader header;
    u8 sectors[16][SECTOR_SIZE];
};

extern union SaveBuffer gSaveBuffer;
extern s8 sSectorDirty[16];
extern s8 sSaveState;
extern const u32 sSaveMagic;
extern const u16 sSaveBlockIds[16];
extern const u32 sSaveBlockSizes[16];

void InitSave(void);
bool8 LoadSaveHeader(void);
void ClearSaveHeader(void);
void LoadSaveSectors(void);
u32 CalcSectorChecksum(int sector);
s8 SaveStep(void);
void SaveAll(void);
bool8 IsSaving(void);
s8 GetSaveState(void);
u32 GetSaveUnk94(void);
void IncrementSaveCounter(int i);
void IncrementDistributionCount(void);
void *GetSaveDataStart(void);
u32 GetTerminalId(void);
void SetTerminalId(u16 value);
bool32 ReadSaveBlock(u32 offset, void *dest, u32 size);
bool32 WriteSaveBlock(u32 offset, const void *src, u32 size);
bool32 ClearSaveBlock(void);
bool8 IsSaveBlockRangeValid(u32 offset, u32 size);
void *GetSaveBlock(void);

void ReadFlash(u32 sectorNum, u32 offset, void *dest, u32 size);
u32 ProgramFlashSectorAndVerify(u32 sectorNum, void *src);

// Main

// gMainState: the index of the function the main loop runs (sMainCallbacks)
#define MAIN_STATE_NONE  0
#define MAIN_STATE_TITLE 1
#define MAIN_STATE_SLOTS 2  // distributing
#define MAIN_STATE_SETUP 3

extern const u8 sExpiryDate[4];
extern u16 gKeysHeld;
extern u16 gKeysNew;
extern u16 gKeysRepeat;

void WaitForVBlank(void);
s8 GetMainStateChanged(void);
int GetVBlankCounter(void);
void SetVBlankCounter(u8 value);
void SetMainState(u8 state);

// Graphics

void ClearTilemapRect(int x, int y, int width, int height);
void ClearTilemap(void);
void HideSprite(int i);
void HideAllSprites(void);
void PrintText(int x, int y, int palette, const u8 *str);
void PrintHex2(int x, int y, int palette, int value);
void PrintHex4(int x, int y, int palette, int value);
void PrintHex8(int x, int y, int palette, int value);
void PrintDec(int x, int y, int palette, int value);
void PrintDec2(int x, int y, int palette, int value);
void PrintDec3(int x, int y, int palette, int value);

extern u16 gBgTilemap[0x280];
extern struct OamData gOamBuffer[128];

int GetCurrentSlot(void);
void SetCurrentSlot(int slot);

#endif // GUARD_AURORA_H
