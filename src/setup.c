#include "global.h"
#include "siirtc.h"
#include "aurora.h"
#include "link.h"
#include "misc.h"

extern struct SiiRtcInfo sSetupRtc;
extern s8 sSetupPage;
extern s8 sSetupUnused;
extern s8 sMainCursor;
extern s8 sTimeCursor;
extern s8 sAiCursor;
extern s8 sAiMode;
extern s8 sAiLevel;
extern s8 sAiPair;

extern const u8 sRtcMin[7];
extern const u8 sRtcMax[7];
extern const s8 sAiSettingsMin[3];
extern const s8 sAiSettingsMax[3];
extern const u8 sText_Border[];
extern const u8 sText_SetupMenu[];
extern const u8 sText_Time[];
extern const u8 sText_PokemonAi[];
extern const u8 sText_TimeSetup[];
extern const u8 sText_Year[];
extern const u8 sText_Month[];
extern const u8 sText_Day[];
extern const u8 sText_Week[];
extern const u8 sText_Hour[];
extern const u8 sText_Minute[];
extern const u8 sText_Second[];
extern const u8 sText_Set[];
extern const u8 sText_PokemonAiSetup[];
extern const u8 sText_Mode[];
extern const u8 sText_Level[];
extern const u8 sText_Pair[];
extern const u8 sText_Dashes[];
extern const u8 sFlashPalettes[0x40];

void SetupMenu_Init(void);
void SetupMenu_ShowMain(void);
void SetupMenu_ShowTime(void);
void SetupMenu_ShowAi(void);
void SetupMenu_InitGfx(void);
void SetupMenu_Draw(void);
void SetupMenu_DrawTime(void);
void SetupMenu_DrawAi(void);
void SetupMenu_HandleInput(void);
void SetupMenu_HandleMainInput(void);
void SetupMenu_HandleTimeInput(void);
void SetupMenu_HandleAiInput(void);
void SetupMenu_ValidateRtc(void);
void FlashPalette(void);

void SetupMenu_Main(void)
{
    if (GetMainStateChanged())
        SetupMenu_Init();
    SetupMenu_Draw();
    SetupMenu_HandleInput();
}

void SetupMenu_Init(void)
{
    SetupMenu_InitGfx();
    ResetCursorAnim();
    if (sMainCursor == 0)
        sMainCursor = 4;
    if (sTimeCursor == 0)
        sTimeCursor = 4;
    if (sAiCursor == 0)
        sAiCursor = 4;
    sSetupUnused = 2;
    HideAllSprites();
    gOamBuffer[0].affineMode = 0;
    gOamBuffer[0].tileNum = 0x106;
    gOamBuffer[0].paletteNum = 0;
    gOamBuffer[0].x = 16;
    SetupMenu_ShowMain();
    if (sAiPair == 0)
        sAiPair = sAiSettingsMin[2];
    SetupMenu_ValidateRtc();
}

void SetupMenu_ShowMain(void)
{
    sSetupPage = 0;
    ClearTilemap();
    PrintText(0, 0, 0, sText_Border);
    PrintText(0, 1, 0, sText_SetupMenu);
    PrintText(0, 2, 0, sText_Border);
    PrintText(0, 4, 0, sText_Time);
    PrintText(0, 5, 0, sText_PokemonAi);
    gOamBuffer[0].y = sMainCursor * 8;
}

void SetupMenu_ShowTime(void)
{
    sSetupPage = 1;
    ClearTilemap();
    PrintText(0, 0, 0, sText_Border);
    PrintText(0, 1, 0, sText_TimeSetup);
    PrintText(0, 2, 0, sText_Border);
    PrintText(0, 4, 0, sText_Year);
    PrintText(0, 5, 0, sText_Month);
    PrintText(0, 6, 0, sText_Day);
    PrintText(0, 7, 0, sText_Week);
    PrintText(0, 8, 0, sText_Hour);
    PrintText(0, 9, 0, sText_Minute);
    PrintText(0, 10, 0, sText_Second);
    PrintText(0, 12, 1, sText_Set);
    gOamBuffer[0].y = sTimeCursor * 8;
    while (!SiiRtcGetDateTime(&sSetupRtc))
        ;
}

void SetupMenu_ShowAi(void)
{
    sSetupPage = 2;
    ClearTilemap();
    PrintText(0, 0, 0, sText_Border);
    PrintText(0, 1, 0, sText_PokemonAiSetup);
    PrintText(0, 2, 0, sText_Border);
    PrintText(0, 4, 0, sText_Mode);
    PrintText(0, 5, 0, sText_Level);
    PrintText(0, 6, 0, sText_Pair);
    gOamBuffer[0].y = sAiCursor * 8;
}

void SetupMenu_InitGfx(void)
{
    REG_DISPCNT = 0;
    REG_BLDCNT = 0;
    CpuFastSet(sDebugGfx, (void *)BG_CHAR_ADDR(2), 0x880);
    CpuFastSet(sDebugBgPalette, (void *)PLTT, 0x80);
    REG_BG0CNT = BGCNT_CHARBASE(2);
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
    CpuFastSet(sDebugGfx, (void *)OBJ_VRAM0, 0x880);
    CpuFastSet(sDebugObjPalette, (void *)OBJ_PLTT, 0x80);
    CpuFastFill(0xA0, gOamBuffer, sizeof(gOamBuffer));
    REG_DISPCNT = DISPCNT_OBJ_ON | DISPCNT_BG0_ON;
}

void SetupMenu_Draw(void)
{
    UpdateCursorAnim(0);
    switch (sSetupPage)
    {
    case 1:
        SetupMenu_DrawTime();
        break;
    case 2:
        SetupMenu_DrawAi();
        break;
    }
}

void SetupMenu_DrawTime(void)
{
    PrintHex2(12, 4, 0, sSetupRtc.year);
    PrintHex2(12, 5, 0, sSetupRtc.month);
    PrintHex2(12, 6, 0, sSetupRtc.day);
    PrintHex2(12, 7, 0, sSetupRtc.dayOfWeek);
    PrintHex2(12, 8, 0, sSetupRtc.hour);
    PrintHex2(12, 9, 0, sSetupRtc.minute);
    PrintHex2(12, 10, 0, sSetupRtc.second);
}

void SetupMenu_DrawAi(void)
{
    switch (sAiMode)
    {
    case 0:
        PrintDec2(12, 4, 0, 0);
        PrintText(12, 5, 0, sText_Dashes);
        PrintText(12, 6, 0, sText_Dashes);
        break;
    case 1:
        PrintDec2(12, 4, 0, 1);
        PrintDec2(12, 5, 3, sAiLevel);
        PrintText(12, 6, 0, sText_Dashes);
        break;
    case 2:
        PrintDec2(12, 4, 0, 2);
        PrintText(12, 5, 0, sText_Dashes);
        PrintDec2(12, 6, 3, sAiPair);
        break;
    }
}

void SetupMenu_HandleInput(void)
{
    if (IsCursorAnimating())
        return;
    switch (sSetupPage)
    {
    case 0:
        SetupMenu_HandleMainInput();
        break;
    case 1:
        SetupMenu_HandleTimeInput();
        break;
    case 2:
        SetupMenu_HandleAiInput();
        break;
    }
}

void SetupMenu_HandleMainInput(void)
{
    if (gKeysNew & A_BUTTON)
    {
        if (sMainCursor == 4)
            SetupMenu_ShowTime();
        else
            SetupMenu_ShowAi();
    }
    if (gKeysNew & (B_BUTTON | START_BUTTON))
    {
        REG_DISPCNT = 0;
        ClearDebugScreenActive();
        InitScreen(WaitForVBlank);
        SetMainState(MAIN_STATE_SLOTS);
    }
    if (gKeysRepeat & DPAD_UP)
    {
        if (sMainCursor != 4)
        {
            sMainCursor--;
            StartCursorAnim(sMainCursor, 2, 1);
        }
        else
        {
            sMainCursor = 5;
            StartCursorAnim(5, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_DOWN)
    {
        if (sMainCursor != 5)
        {
            sMainCursor++;
            StartCursorAnim(sMainCursor, 2, 1);
        }
        else
        {
            sMainCursor = 4;
            StartCursorAnim(4, 8, 1);
        }
    }
}

void SetupMenu_HandleTimeInput(void)
{
    int field = sTimeCursor - 4;

    if ((gKeysNew & A_BUTTON) && sTimeCursor == 12)
    {
        while (!SiiRtcSetDateTime(&sSetupRtc))
            ;
        FlashPalette();
    }
    if (gKeysNew & B_BUTTON)
        SetupMenu_ShowMain();
    if (gKeysRepeat & DPAD_UP)
    {
        if (sTimeCursor != 4)
        {
            sTimeCursor--;
            if (sTimeCursor == 11)
                sTimeCursor--;
            StartCursorAnim(sTimeCursor, 2, 1);
        }
        else
        {
            sTimeCursor = 12;
            StartCursorAnim(12, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_DOWN)
    {
        if (sTimeCursor != 12)
        {
            sTimeCursor++;
            if (sTimeCursor == 11)
                sTimeCursor++;
            StartCursorAnim(sTimeCursor, 2, 1);
        }
        else
        {
            sTimeCursor = 4;
            StartCursorAnim(4, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_LEFT)
    {
        u8 *value = (u8 *)&sSetupRtc + field;

        if (*value != sRtcMin[field])
            *value = BcdDecrement(*value);
        else
            *value = sRtcMax[field];
    }
    if (gKeysRepeat & DPAD_RIGHT)
    {
        u8 *value = (u8 *)&sSetupRtc + field;

        if (*value != sRtcMax[field])
            *value = BcdIncrement(*value);
        else
            *value = sRtcMin[field];
    }
    if (gKeysNew & START_BUTTON)
    {
        while (!SiiRtcSetDateTime(&sSetupRtc))
            ;
        FlashPalette();
    }
}

void SetupMenu_HandleAiInput(void)
{
    int field = sAiCursor - 4;

    if (gKeysNew & B_BUTTON)
        SetupMenu_ShowMain();
    if (gKeysRepeat & DPAD_UP)
    {
        if (sAiCursor != 4)
        {
            sAiCursor--;
            StartCursorAnim(sAiCursor, 2, 1);
        }
        else
        {
            sAiCursor = 6;
            StartCursorAnim(6, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_DOWN)
    {
        if (sAiCursor != 6)
        {
            sAiCursor++;
            StartCursorAnim(sAiCursor, 2, 1);
        }
        else
        {
            sAiCursor = 4;
            StartCursorAnim(4, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_LEFT)
    {
        if (sAiCursor == 4 || (sAiCursor == 5 && sAiMode == 1) || (sAiCursor == 6 && sAiMode == 2))
        {
            if ((&sAiMode)[field] != sAiSettingsMin[field])
                (&sAiMode)[field]--;
            else
                (&sAiMode)[field] = sAiSettingsMax[field];
        }
    }
    if (gKeysRepeat & DPAD_RIGHT)
    {
        if (sAiCursor == 4 || (sAiCursor == 5 && sAiMode == 1) || (sAiCursor == 6 && sAiMode == 2))
        {
            if ((&sAiMode)[field] != sAiSettingsMax[field])
                (&sAiMode)[field]++;
            else
                (&sAiMode)[field] = sAiSettingsMin[field];
        }
    }
}

void SetupMenu_ValidateRtc(void)
{
    bool8 changed = FALSE;
    int i;

    while (!SiiRtcGetDateTime(&sSetupRtc))
        ;
    for (i = 0; i < 7; i++)
    {
        u8 *value = (u8 *)&sSetupRtc + i;

        if (*value < sRtcMin[i] || *value > sRtcMax[i])
        {
            *value = sRtcMin[i];
            changed = TRUE;
        }
    }
    if (changed)
    {
        while (!SiiRtcSetDateTime(&sSetupRtc))
            ;
    }
}

void FlashPalette(void)
{
    u8 palettes[0x40];
    int i;

    memcpy(palettes, sFlashPalettes, sizeof(palettes));
    for (i = 0; i < 4; i++)
    {
        CpuFastSet(&palettes[(i & 1) * 32], (void *)PLTT, 8);
        WaitForVBlank();
        WaitForVBlank();
    }
}

void GetAiSettings(u8 *mode, u8 *level, u8 *pair)
{
    *mode = sAiMode;
    *level = sAiLevel;
    *pair = sAiPair;
}
