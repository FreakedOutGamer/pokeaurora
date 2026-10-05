#include "global.h"
#include "aurora.h"
#include "content.h"

extern const char sFmt_02X[];
extern const char sFmt_04X[];
extern const char sFmt_08X[];
extern const char sFmt_d[];
extern const char sFmt_02d[];
extern const char sFmt_03d[];

void ClearTilemapRect(int x, int y, int width, int height)
{
    int i, j;

    for (i = 0; i < height; i++)
    {
        u16 *dest = gBgTilemap + x + (y + i) * 32;

        for (j = 0; j < width; j++)
            *dest++ = 0;
    }
}

void ClearTilemap(void)
{
    memset(gBgTilemap, 0, sizeof(gBgTilemap));
}

void HideSprite(int i)
{
    u16 *attr0 = (u16 *)&gOamBuffer[i];

    *attr0 = ST_OAM_AFFINE_ERASE << 8;
}

void HideAllSprites(void)
{
    int i;

    for (i = 0; i < 128; i++)
    {
        u16 *attr0 = (u16 *)&gOamBuffer[i];

        *attr0 = ST_OAM_AFFINE_ERASE << 8;
    }
}

void PrintText(int x, int y, int palette, const u8 *str)
{
    u16 *dest = gBgTilemap + x + y * 32;

    while (*str)
    {
        *dest = *str | (palette << 12);
        dest++;
        str++;
    }
}

void PrintHex2(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_02X, value);
    PrintText(x, y, palette, buf);
}

void PrintHex4(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_04X, value);
    PrintText(x, y, palette, buf);
}

void PrintHex8(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_08X, value);
    PrintText(x, y, palette, buf);
}

void PrintDec(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_d, value);
    PrintText(x, y, palette, buf);
}

void PrintDec2(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_02d, value);
    PrintText(x, y, palette, buf);
}

void PrintDec3(int x, int y, int palette, int value)
{
    char buf[16];

    sprintf(buf, sFmt_03d, value);
    PrintText(x, y, palette, buf);
}
