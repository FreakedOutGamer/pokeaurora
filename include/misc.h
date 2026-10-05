#ifndef GUARD_MISC_H
#define GUARD_MISC_H

#define CURSOR_NONE 0x7F // sCursorTarget when the cursor isn't moving
#define DISTRIBUTION_MIN_STRENGTH 0xCC // of the wireless signal

struct RfuGameData
{
    u8 language:4;
    u8 hasNews:1;
    u8 hasCard:1;
    u8 unknown:1;
    u8 isChampion:1;
    u8 hasNationalDex:1;
    u8 gameClear:1;
    u8 version:4;
    u8 unused:2;
    u8 trainerId[2];
    u8 partnerInfo[4];
    u16 tradeSpecies:10;
    u16 tradeType:6;
    u8 activity:7;
    u8 startedActivity:1;
    u8 playerGender:1;
    u8 tradeLevel:7;
    u8 padding[4];
};

struct DistributionWork
{
    u32 timer;
    u8 gameName[13];
    u8 userName[8];
};

extern u8 sTitleTimer;
extern u8 sKeyRepeatTimers[10];
extern s8 sCursorTarget;
extern s8 sCursorSpeed;
extern s8 sCursorVertical;

extern const u8 sDistributionType;
extern const struct SlotPos sTitlePos;
extern const struct SlotPos sVersionPos;
extern const struct SlotPos sPressAPos;
extern const u8 sText_Title[];
extern const u8 sText_Version[];
extern const u8 sText_PressA[];
extern const u8 sRfuUserName[];

void TitleScreen_Main(void);
void TitleScreen_Init(void);
void TitleScreen_Blink(void);
void TitleScreen_HandleInput(void);
void TitleScreen_Wait(void);
void ReadKeys(void);
void ResetCursorAnim(void);
void StartCursorAnim(int target, int speed, int vertical);
void UpdateCursorAnim(int sprite);
bool8 IsCursorAnimating(void);
void WaitFrames(int frames);
void ResetGame(void);
u32 BcdToInt(u32 bcd);
u32 Pow(u32 base, u32 exp);
u32 IntToBcd(u32 value);
u32 BcdIncrement(u32 bcd);
u32 BcdDecrement(u32 bcd);
void SetRfuGameData(bool8 full);
void StartDistribution(const u8 *gameName, const u8 *userName);
u16 DistributionTask(void *work);

#endif // GUARD_MISC_H
