#ifndef GUARD_UI_H
#define GUARD_UI_H

#define BLD_TARGET_ALL 0x1F // BGs 0-3 and sprites

struct NumberPos
{
    u8 x;
    u8 y;
    u16 unused;
};

extern u32 gBg3ScrollY;
extern u8 gScreenState;
extern u8 gScreenAnimStep;
extern u8 gScreenAnimPage;
extern u8 gScreenAnimStopped;
extern u16 gScreenAnimTimer;
extern u16 gScreenFadeLevel;
extern u8 gScreenHold;
extern u8 gAdapterError;

extern const struct NumberPos sNumberPos[10];
extern const u16 sBgCnt[4];
extern const u32 sAdapterErrorGfx[];
extern const u32 sPleaseWaitGfx[];
extern const u16 sAdapterErrorTilemap[];
extern const u32 sPleaseWaitTilemap[];
extern const u32 sAdapterErrorPalette[];
extern const u32 sPleaseWaitPalette[];
extern const u32 sSendingGfx[];
extern const u32 sIdChangeGfx[];
extern const u32 sDigitsGfx[];
extern const u32 sSendingPalette[];
extern const u32 sIdChangePalette[];
extern const u32 sDigitsPalette[];
extern const u32 sSendingBgTilemap[];
extern const u32 sSendingTextTilemap[];
extern const u32 sIdSelectTilemap[];
extern const u32 sIdChangedTilemap[];

void FadeInScreen(void (*waitFunc)(void));
void InitScreen(void (*waitFunc)(void));
void ChooseNumberScreen(void);
void ScrollSendingScreen(u16 delay, u16 speed);
void FlashSendingScreen(u16 delay, u8 speed);
void ShowAdapterErrorScreen(void);
void ShowPleaseWaitScreen(void);
void ShowSendingScreen(void);
void ShowSendingBackground(void);
void ShowIdChangeScreen(void);
void ShowIdChangedScreen(void);
void UpdateDistributionScreen(void);
void SetAdapterError(u8 a);
void nullsub_080054FC(void);
void DrawText(const u8 *font, u8 *dest, int y, const u8 *str, u8 spacing, u8 start, u8 count, u32 *canvas);
void DrawTextColor(const u8 *font, u8 *dest, int y, const u8 *str, u8 spacing, u8 start, u8 count, u32 *canvas, u8 color);
void BlitGlyph(u32 *dest, u32 *src, int x, int y);
void LoadDigitSprites(void);
void SetBldDarken(u8 y, u8 target);
void SetBldLighten(u8 y, u8 target);
void SetScreenHold(u8 value);
u8 GetScreenHold(void);
u8 GetBitColor(u8 bit, u8 color);
u8 GetBit(u8 bit);
void SetBgOffset(u8 bg, u16 x, u16 y);
void SetNumberSprites(u16 tile, struct OamData *oam);
void SetNumberSpritesPos(u16 x, u16 y, struct OamData *oam);
void SetDigitSprites1(u8 digit, struct OamData *oam);
void SetDigitSprites2(u8 digit, struct OamData *oam);
void MoveBg(u8 bg, u16 x, u16 y);
void SetOamData(u16 tileNum, u8 x, u8 y, u8 size, u8 shape, u8 paletteNum, u8 priority, struct OamData *oam);
void SetOamPos(u16 x, u16 y, struct OamData *oam);
void FlipOam(struct OamData *oam);

#endif // GUARD_UI_H
