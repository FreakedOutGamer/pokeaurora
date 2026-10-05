#include "global.h"
#include "siirtc.h"
#include "aurora.h"
#include "content.h"

typedef void (*IntrFunc)(void);

extern IntrFunc gIntrTable[14];
extern vu8 gVBlankDone;
extern u8 gVBlankCounter;
extern u32 gIntrMainBuf[0x80];
extern u32 gPreservedValue;
extern u8 gMainState;
extern u8 gPrevMainState;
extern s8 gMainStateChanged;
extern void (*const sMainCallbacks[])(void);
extern const u8 __data_rom_start[];
extern const u8 __data_rom_end[];
extern const u8 sExpiredGfx[];
extern const u8 sExpiredTilemap[];
extern const u8 sExpiredPalette[];

void IntrMain(void);
void IntrDummy(void);
void VBlankIntr(void);
void WaitForVBlank(void);
void InitMain(void);
void UpdateMainState(void);
void SetMainState(u8 state);
void ShowExpiredScreen(void);
bool8 InitRfu(void);
bool8 IsDebugScreenActive(void);
void ReadKeys(void);
void UpdateDistributionScreen(void);
void FadeInScreen(void (*func)(void), int a, void *b);
void rfu_LMAN_syncVBlank(void);

void InitIntrTable(void)
{
    gIntrTable[0] = IntrDummy;
    gIntrTable[1] = VBlankIntr;
    gIntrTable[2] = IntrDummy;
    gIntrTable[3] = IntrDummy;
    gIntrTable[4] = IntrDummy;
    gIntrTable[5] = IntrDummy;
    gIntrTable[6] = IntrDummy;
    gIntrTable[7] = IntrDummy;
    gIntrTable[8] = IntrDummy;
    gIntrTable[9] = IntrDummy;
    gIntrTable[10] = IntrDummy;
    gIntrTable[11] = IntrDummy;
    gIntrTable[12] = IntrDummy;
    gIntrTable[13] = IntrDummy;
    gVBlankDone = 0;
}

void VBlankIntr(void)
{
    gVBlankCounter++;
    if (IsDebugScreenActive())
    {
        DmaCopy32(3, gBgTilemap, BG_VRAM, sizeof(gBgTilemap));
        DmaCopy32(3, gOamBuffer, OAM, OAM_SIZE);
    }
    else
    {
        UpdateDistributionScreen();
    }
    rfu_LMAN_syncVBlank();
    gVBlankDone = 1;
}

void IntrDummy(void)
{
}

void WaitForVBlank(void)
{
    gVBlankDone = 0;
    while (gVBlankDone == 0)
        ;
}

int GetVBlankCounter(void)
{
    return gVBlankCounter;
}

void SetVBlankCounter(u8 value)
{
    gVBlankCounter = value;
}

void AgbMain(void)
{
    InitMain();
    SetMainState(MAIN_STATE_TITLE);
    UpdateMainState();
    for (;;)
    {
        ReadKeys();
        sMainCallbacks[gMainState]();
        UpdateMainState();
        WaitForVBlank();
    }
}

void InitMain(void)
{
    int i;
    u32 *ptr;

    REG_WAITCNT = WAITCNT_SRAM_4 | WAITCNT_WS0_N_3 | WAITCNT_WS0_S_1 | WAITCNT_PREFETCH_ENABLE;
    DmaFill32(3, 0, EWRAM_START, 0x40000);
    for (i = 0, ptr = (u32 *)IWRAM_START; i < 0x1EE0; ptr++, i++)
    {
        if (ptr != &gPreservedValue)
            CpuFill32(0, ptr, 4);
    }
    DmaFill32(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0x20, OAM, OAM_SIZE);
    DmaFill32(3, 0, PLTT, PLTT_SIZE);
    DmaCopy32(3, __data_rom_start, EWRAM_START, __data_rom_end - __data_rom_start);
    DmaCopy32(3, IntrMain, gIntrMainBuf, sizeof(gIntrMainBuf));
    INTR_VECTOR = gIntrMainBuf;
    InitIntrTable();
    if (!InitRfu())
    {
        for (;;)
            ;
    }
    REG_DISPSTAT = DISPSTAT_VBLANK_INTR;
    REG_IF = 0xFFFF;
    REG_IE = INTR_FLAG_VBLANK | INTR_FLAG_SERIAL;
    REG_IME = 1;
    srand(REG_TM0CNT_L);
    gPrevMainState = 0;
    FadeInScreen(WaitForVBlank, 2, gIntrTable + 6);
}

void UpdateMainState(void)
{
    if (gMainState == gPrevMainState)
        gMainStateChanged = 0;
    else
        gMainStateChanged = 1;
    gPrevMainState = gMainState;
}

void SetMainState(u8 state)
{
    gMainState = state;
}

void MainState_None(void)
{
}

s8 GetMainStateChanged(void)
{
    return gMainStateChanged;
}

void CheckExpiry(void)
{
    struct SiiRtcInfo rtc;

    SiiRtcGetDateTime(&rtc);
    if (rtc.year < sExpiryDate[0])
        return;
    if (rtc.year > sExpiryDate[0])
        ShowExpiredScreen();
    if (rtc.month < sExpiryDate[1])
        return;
    if (rtc.month > sExpiryDate[1])
        ShowExpiredScreen();
    if (rtc.day < sExpiryDate[2])
        return;
    if (rtc.day > sExpiryDate[2])
        ShowExpiredScreen();
    if (rtc.hour < sExpiryDate[3])
        return;
    ShowExpiredScreen();
}

void ShowExpiredScreen(void)
{
    REG_IME = 0;
    REG_BLDCNT = 0;
    DmaCopy32(3, sExpiredGfx, BG_VRAM, 0x10000);
    DmaCopy32(3, sExpiredTilemap, BG_SCREEN_ADDR(8), 0x1000);
    DmaCopy32(3, sExpiredPalette, PLTT, 0x200);
    REG_BG0CNT = BGCNT_SCREENBASE(8) | BGCNT_256COLOR;
    REG_DISPCNT = DISPCNT_BG0_ON;
    DmaStop(0);
    DmaStop(1);
    DmaStop(2);
    DmaStop(3);
    for (;;)
        ;
}

u32 GetPreservedValue(void)
{
    return gPreservedValue;
}

void SetPreservedValue(u32 value)
{
    gPreservedValue = value;
}
