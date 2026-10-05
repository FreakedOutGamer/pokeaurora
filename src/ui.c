#include "global.h"
#include "aurora.h"
#include "content.h"
#include "misc.h"
#include "ui.h"

void FadeInScreen(void (*waitFunc)(void))
{
    u8 i;

    gScreenState = 0;
    SetScreenHold(1);
    i = 15;
    waitFunc();
    SetBldLighten(15, BLD_TARGET_ALL);
    gAdapterError = 0;
    gScreenAnimTimer = 0;
    gScreenFadeLevel = 0;
    gScreenAnimStopped = 0;
    gScreenAnimStep = 0;
    gScreenAnimPage = 0;
    InitScreen(waitFunc);
    gBg3ScrollY = 8;
    CpuFastFill(DISPLAY_HEIGHT, (void *)OAM, OAM_SIZE); // all sprites below the screen
    do
    {
        SetBldLighten(i, BLD_TARGET_ALL);
        i--;
        waitFunc();
    } while (i != 0);
    gScreenState = 1;
}

void InitScreen(void (*waitFunc)(void))
{
    waitFunc();
    REG_BG0CNT = sBgCnt[0];
    REG_BG1CNT = sBgCnt[1];
    REG_BG2CNT = sBgCnt[2];
    REG_BG3CNT = sBgCnt[3];
    REG_DISPCNT = DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_BG3_ON;
    ShowPleaseWaitScreen();
}

void ChooseNumberScreen(void)
{
    struct OamData oam[3];
    struct OamData digits[2];
    u16 running;
    u16 n;

    gScreenAnimStopped = 1;
    running = TRUE;
    n = GetTerminalId();
    WaitForVBlank();
    ShowIdChangeScreen();
    CpuFastFill(DISPLAY_HEIGHT, (void *)OAM, OAM_SIZE); // all sprites below the screen
    SetNumberSprites(0, oam);
    do
    {
        ReadKeys();
        if (gKeysNew == DPAD_UP)
        {
            if (n != 1 && n != 6)
                n--;
        }
        else if (gKeysNew == DPAD_DOWN)
        {
            if (n != 5 && n != 10)
                n++;
        }
        else if (gKeysNew == DPAD_RIGHT)
        {
            if (n <= 5)
                n += 5;
        }
        else if (gKeysNew == DPAD_LEFT)
        {
            if (n > 5)
                n -= 5;
        }
        if (gKeysNew == A_BUTTON)
        {
            running = FALSE;
            SetTerminalId(n);
            WaitForVBlank();
            ShowIdChangedScreen();
            CpuFastFill(DISPLAY_HEIGHT, (void *)OAM, OAM_SIZE); // all sprites below the screen
            SetDigitSprites2(n, digits);
            CpuSet(digits, (void *)OAM, 8);
            for (;;)
            {
                if (running > 120)
                    ResetGame();
                WaitForVBlank();
                running++;
            }
        }
        if (gKeysNew == B_BUTTON)
            ResetGame();
        SetNumberSpritesPos(sNumberPos[n - 1].x, sNumberPos[n - 1].y, oam);
        WaitForVBlank();
        CpuSet(oam, (void *)OAM, 12);
    } while (running);
}

void ScrollSendingScreen(u16 delay, u16 speed)
{
    if (gScreenState != 2)
    {
        if (gScreenState != 1 || GetScreenHold())
            return;
        gScreenState = 2;
    }
    if (gScreenAnimStopped)
        return;
    if (!gScreenAnimStep)
    {
        ShowSendingScreen();
        gScreenAnimStep = 1;
    }
    if (gScreenAnimTimer > delay)
    {
        gBg3ScrollY -= speed;
        SetBgOffset(3, 0, gBg3ScrollY);
        gScreenAnimTimer = 0;
    }
    else
    {
        gScreenAnimTimer++;
    }
}

void FlashSendingScreen(u16 delay, u8 speed)
{
    if (gScreenState != 2)
    {
        if (gScreenState != 1 || GetScreenHold())
            return;
        gScreenState = 2;
    }
    if (gScreenAnimStopped)
        return;
    {
        switch (gScreenAnimStep)
        {
        case 0:
            SetBldDarken(gScreenFadeLevel / speed, BLD_TARGET_ALL);
            if (gScreenFadeLevel / speed > 14)
                gScreenAnimStep = 1;
            else
                gScreenFadeLevel++;
            break;
        case 1:
            switch (gScreenAnimPage)
            {
            case 0:
                ShowSendingScreen();
                break;
            case 1:
                ShowSendingBackground();
                break;
            }
            gScreenAnimStep = 2;
            break;
        case 2:
            SetBldDarken(gScreenFadeLevel / speed, BLD_TARGET_ALL);
            if (gScreenFadeLevel / speed <= 0)
                gScreenAnimStep = 3;
            else
                gScreenFadeLevel--;
            break;
        case 3:
            if (++gScreenAnimTimer > delay)
            {
                gScreenAnimTimer = 0;
                gScreenAnimStep = 0;
                if (++gScreenAnimPage > 1)
                    gScreenAnimPage = 0;
            }
            break;
        }
    }
}

void ShowAdapterErrorScreen(void)
{
    CpuFastSet(sAdapterErrorGfx, (void *)BG_CHAR_ADDR(1), 0x1400);
    CpuFastSet(sAdapterErrorPalette, (void *)PLTT, 0x80);
    CpuSet(sAdapterErrorTilemap, (void *)BG_SCREEN_ADDR(1), 0x400);
    SetBldDarken(0, BLD_TARGET_ALL);
}

void ShowPleaseWaitScreen(void)
{
    CpuFastSet(sPleaseWaitGfx, (void *)BG_CHAR_ADDR(1), 1000);
    CpuFastSet(sPleaseWaitPalette, (void *)PLTT, 0x80);
    CpuFastSet(sPleaseWaitTilemap, (void *)BG_SCREEN_ADDR(1), 0x200);
}

void ShowSendingScreen(void)
{
    struct OamData oam[2];

    REG_DISPCNT = DISPCNT_BG2_ON | DISPCNT_BG3_ON | DISPCNT_OBJ_ON;
    CpuFastSet(sSendingGfx, (void *)BG_CHAR_ADDR(1), 0x2000);
    CpuFastSet(sSendingPalette, (void *)PLTT, 0x80);
    CpuFastSet(sSendingTextTilemap, (void *)BG_SCREEN_ADDR(2), 0x200);
    CpuFastSet(sSendingBgTilemap, (void *)BG_SCREEN_ADDR(3), 0x200);
    CpuFastFill(DISPLAY_HEIGHT, (void *)OAM, OAM_SIZE); // all sprites below the screen
    SetDigitSprites1(GetTerminalId(), oam);
    CpuSet(oam, (void *)OAM, 8);
}

void ShowSendingBackground(void)
{
    CpuFastSet(sSendingGfx, (void *)BG_CHAR_ADDR(1), 0x2000);
    CpuFastSet(sSendingPalette, (void *)PLTT, 0x80);
    CpuFastSet(sSendingBgTilemap, (void *)BG_SCREEN_ADDR(1), 0x200);
}

void ShowIdChangeScreen(void)
{
    REG_DISPCNT = DISPCNT_BG2_ON | DISPCNT_BG3_ON | DISPCNT_OBJ_ON;
    CpuFastSet(sIdChangeGfx, (void *)BG_CHAR_ADDR(1), 0x2000);
    CpuFastSet(sIdChangePalette, (void *)PLTT, 0x80);
    CpuFastSet(sIdSelectTilemap, (void *)BG_SCREEN_ADDR(2), 0x200);
}

void ShowIdChangedScreen(void)
{
    CpuFastSet(sIdChangedTilemap, (void *)BG_SCREEN_ADDR(2), 0x200);
}

void UpdateDistributionScreen(void)
{
    if (gAdapterError)
        ShowAdapterErrorScreen();
    else
        ScrollSendingScreen(0, 2);
}

void SetAdapterError(u8 a)
{
    gAdapterError = a;
    SetScreenHold(0);
}

void nullsub_080054FC(void)
{
}


void DrawText(const u8 *font, u8 *dest, int y, const u8 *str, u8 spacing, u8 start, u8 count, u32 *canvas)
{
    u8 tile[0x80];
    u8 n = 0;
    u16 x = 0;
    u16 pos = start * 2;

    while (str[pos] != 0 && n < count)
    {
        u16 c = (str[pos] << 8) + str[pos + 1];
        u16 glyph = (c - 0x8140) - ((c >> 8) - 0x81) * 0x40;
        u16 i;

        if (c > 0x889E)
            glyph -= 0x240;
        {
            int offset = glyph * 32;

            for (i = 0; i < 32; i++)
            {
                const u8 *bits = &font[offset + i];

                tile[i * 4 + 0] = GetBit(*bits & 0x80) | (GetBit(*bits & 0x40) << 4);
                tile[i * 4 + 1] = GetBit(*bits & 0x20) | (GetBit(*bits & 0x10) << 4);
                tile[i * 4 + 2] = GetBit(*bits & 0x08) | (GetBit(*bits & 0x04) << 4);
                tile[i * 4 + 3] = GetBit(*bits & 0x02) | (GetBit(*bits & 0x01) << 4);
            }
        }
        BlitGlyph(canvas, (u32 *)tile, (int)(dest + x), y);
        pos += 2;
        x += spacing;
        n++;
    }
}

void DrawTextColor(const u8 *font, u8 *dest, int y, const u8 *str, u8 spacing, u8 start, u8 count, u32 *canvas, u8 color)
{
    u8 tile[0x80];
    u16 pos;
    u8 n = 0;
    u16 x = 0;

    pos = start * 2;

    while (str[pos] != 0 && n < count)
    {
        u16 c = (str[pos] << 8) + str[pos + 1];
        u16 glyph = (c - 0x8140) - ((c >> 8) - 0x81) * 0x40;
        u16 i;

        if (c > 0x889E)
            glyph -= 0x240;
        {
            int offset = glyph * 32;

            for (i = 0; i < 32; i++)
            {
                const u8 *bits = &font[offset + i];

                tile[i * 4 + 0] = GetBitColor(*bits & 0x80, color) | (GetBitColor(*bits & 0x40, color) << 4);
                tile[i * 4 + 1] = GetBitColor(*bits & 0x20, color) | (GetBitColor(*bits & 0x10, color) << 4);
                tile[i * 4 + 2] = GetBitColor(*bits & 0x08, color) | (GetBitColor(*bits & 0x04, color) << 4);
                tile[i * 4 + 3] = GetBitColor(*bits & 0x02, color) | (GetBitColor(*bits & 0x01, color) << 4);
            }
        }
        BlitGlyph(canvas, (u32 *)tile, (int)(dest + x), y);
        pos += 2;
        x += spacing;
        n++;
    }
}

#define BLIT_ROWS(count)                                                                  \
    for (i = 0; i < (count); i++)                                                         \
    {                                                                                     \
        left = src[0];                                                                    \
        right = src[8];                                                                   \
        d[0] = (d[0] & (0xFFFFFFFF >> ((8 - x) * 4))) | (left << (x * 4));                \
        d[8] = (left >> ((8 - x) * 4)) | (right << (x * 4));                              \
        if (x != 0)                                                                       \
            d[16] = (d[16] & (0xFFFFFFFF << (x * 4))) | (right >> ((8 - x) * 4));         \
        d++;                                                                              \
        src++;                                                                            \
    }

void BlitGlyph(u32 *dest, u32 *src, int x, int y)
{
    int i;
    u32 *d;
    u32 left, right;

    dest += (y / 8) * 256 + (x / 8) * 8;
    x -= x / 8 * 8;
    y -= y / 8 * 8;
    d = dest + y;
    BLIT_ROWS(8 - y);
    dest += 0x100;
    d = dest;
    BLIT_ROWS(y);
    src += 8;
    BLIT_ROWS(8 - y);
    if (y != 0)
    {
        d = dest + 0x100;
        BLIT_ROWS(y);
    }
}

void LoadDigitSprites(void)
{
    CpuFastSet(sDigitsGfx, (void *)OBJ_VRAM0, 0x2000);
    CpuFastSet(sDigitsPalette, (void *)OBJ_PLTT, 0x80);
}

bool32 GetPlayerRank(const struct RankQuery *player, int *rank, int *total)
{
    return 0;
}

void GetTopRanking(struct RankEntry *entries, int count)
{
}

int ReturnTrue_08005A68(void)
{
    return 1;
}

int ReturnTrue_08005A6C(void)
{
    return 1;
}

int ReturnTrue_08005A70(void)
{
    return 1;
}

int ReturnFalse_08005A74(void)
{
    return 0;
}

void SetBldDarken(u8 y, u8 target)
{
    REG_BLDCNT = (u8)(BLDCNT_EFFECT_DARKEN | target);
    REG_BLDY = y;
}

void SetBldLighten(u8 y, u8 target)
{
    REG_BLDCNT = target | BLDCNT_EFFECT_LIGHTEN;
    REG_BLDY = y;
}

void SetScreenHold(u8 value)
{
    gScreenHold = value;
}

u8 GetScreenHold(void)
{
    return gScreenHold;
}

u8 GetBitColor(u8 bit, u8 color)
{
    if (bit)
        return color;
    return 0;
}

u8 GetBit(u8 bit)
{
    if (bit)
        return 1;
    return 0;
}

void SetBgOffset(u8 bg, u16 x, u16 y)
{
    switch (bg)
    {
    case 0:
        REG_BG0HOFS = x;
        REG_BG0VOFS = y;
        break;
    case 1:
        REG_BG1HOFS = x;
        REG_BG1VOFS = y;
        break;
    case 2:
        REG_BG2HOFS = x;
        REG_BG2VOFS = y;
        break;
    case 3:
        REG_BG3HOFS = x;
        REG_BG3VOFS = y;
        break;
    }
}

void SetNumberSprites(u16 tile, struct OamData *oam)
{
    SetOamData(tile, 0, 0, 2, 1, 0, 0, oam);
    SetOamData(tile + 4, 32, 0, 2, 1, 0, 0, oam + 1);
    SetOamData(tile + 8, 64, 0, 2, 1, 0, 0, oam + 2);
}

void SetNumberSpritesPos(u16 x, u16 y, struct OamData *oam)
{
    SetOamPos(x, y, oam);
    SetOamPos(x + 32, y, oam + 1);
    SetOamPos(x + 64, y, oam + 2);
}

void SetDigitSprites1(u8 digit, struct OamData *oam)
{
    if (digit <= 9)
    {
        SetOamData(0x40, 208, 10, 0, 0, 0, 0, oam);
        SetOamData(0x40 + digit * 2, 218, 10, 0, 0, 0, 0, oam + 1);
    }
    else
    {
        SetOamData(0x42, 208, 10, 0, 0, 0, 0, oam);
        SetOamData(0x40, 218, 10, 0, 0, 0, 0, oam + 1);
    }
    FlipOam(oam);
    FlipOam(oam + 1);
}

void SetDigitSprites2(u8 digit, struct OamData *oam)
{
    if (digit <= 9)
    {
        SetOamData(0x40, 124, 84, 0, 0, 0, 0, oam);
        SetOamData(0x40 + digit * 2, 134, 84, 0, 0, 0, 0, oam + 1);
    }
    else if (digit == 10)
    {
        SetOamData(0x42, 124, 84, 0, 0, 0, 0, oam);
        SetOamData(0x40, 134, 84, 0, 0, 0, 0, oam + 1);
    }
    FlipOam(oam);
    FlipOam(oam + 1);
}

void MoveBg(u8 bg, u16 x, u16 y)
{
    switch (bg)
    {
    case 0:
        REG_BG0HOFS += x;
        REG_BG0VOFS += y;
        break;
    case 1:
        REG_BG1HOFS += x;
        REG_BG1VOFS += y;
        break;
    case 2:
        REG_BG2HOFS += x;
        REG_BG2VOFS += y;
        break;
    case 3:
        REG_BG3HOFS += x;
        REG_BG3VOFS += y;
        break;
    }
}

void SetOamData(u16 tileNum, u8 x, u8 y, u8 size, u8 shape, u8 paletteNum, u8 priority, struct OamData *oam)
{
    oam->y = y;
    oam->affineMode = 0;
    oam->objMode = 0;
    oam->mosaic = 0;
    oam->bpp = 0;
    oam->shape = shape;
    oam->x = x;
    oam->matrixNum = 0;
    oam->hFlip = 0;
    oam->vFlip = 0;
    oam->size = size;
    oam->tileNum = tileNum;
    oam->priority = priority;
    oam->paletteNum = paletteNum;
    oam->affineParam = 0;
}

void SetOamPos(u16 x, u16 y, struct OamData *oam)
{
    oam->y = y;
    oam->x = x;
}

void FlipOam(struct OamData *oam)
{
    oam->hFlip = 1;
    oam->vFlip = 1;
}
