#include "global.h"
#include "aurora.h"
#include "content.h"
#include "rfu.h"
#include "link.h"
#include "misc.h"
#include "mg.h"

void TitleScreen_Main(void)
{
    GetMainStateChanged();
    TitleScreen_Wait();
}

void TitleScreen_Init(void)
{
    SetVBlankCounter(0);
    ClearTilemap();
    PrintText(sTitlePos.x, sTitlePos.y, 0, sText_Title);
    PrintText(sVersionPos.x, sVersionPos.y, 0, sText_Version);
    HideAllSprites();
}

void TitleScreen_Blink(void)
{
    int counter = GetVBlankCounter();

    if (counter < 60)
        PrintText(sPressAPos.x, sPressAPos.y, 3, sText_PressA);
    else if (counter < 108)
        ClearTilemapRect(0, sPressAPos.y, 32, 1);
    else
        SetVBlankCounter(0);
}

void TitleScreen_HandleInput(void)
{
    if (gKeysNew & A_BUTTON)
        SetMainState(MAIN_STATE_SLOTS);
}

void TitleScreen_Wait(void)
{
    if (++sTitleTimer > 7)
    {
        sTitleTimer = 0;
        SetMainState(MAIN_STATE_SLOTS);
    }
}

void ReadKeys(void)
{
    u16 keys = REG_KEYINPUT ^ KEYS_MASK;
    int i;

    gKeysNew = keys & ~gKeysHeld;
    gKeysHeld = keys;
    gKeysRepeat = gKeysNew;
    for (i = 0; i < 10; i++)
    {
        if ((keys >> i) & 1)
        {
            sKeyRepeatTimers[i]++;
            if (sKeyRepeatTimers[i] == 30)
            {
                gKeysRepeat |= 1 << i;
            }
            else if (sKeyRepeatTimers[i] == 40)
            {
                gKeysRepeat |= 1 << i;
                sKeyRepeatTimers[i] = 30;
            }
        }
        else
        {
            sKeyRepeatTimers[i] = 0;
        }
    }
}

void ResetCursorAnim(void)
{
    sCursorTarget = CURSOR_NONE;
}

void StartCursorAnim(int target, int speed, int vertical)
{
    sCursorTarget = target;
    sCursorSpeed = speed;
    sCursorVertical = vertical;
}

void UpdateCursorAnim(int sprite)
{
    int pos;
    int dir;

    if (sCursorTarget == CURSOR_NONE)
        return;
    if (sCursorVertical)
        pos = gOamBuffer[sprite].y;
    else
        pos = gOamBuffer[sprite].x;
    if (sCursorTarget * 8 > pos)
        dir = 1;
    else
        dir = -1;
    pos += sCursorSpeed * dir;
    if (sCursorVertical)
        gOamBuffer[sprite].y = pos;
    else
        gOamBuffer[sprite].x = pos;
    if (pos == sCursorTarget * 8)
        sCursorTarget = CURSOR_NONE;
}

bool8 IsCursorAnimating(void)
{
    if (sCursorTarget == CURSOR_NONE)
        return FALSE;
    return TRUE;
}

void WaitFrames(int frames)
{
    do
    {
        WaitForVBlank();
    } while (--frames != 0);
}

void ResetGame(void)
{
    rfu_setREQCallback(NULL);
    rfu_REQ_stopMode();
    rfu_waitREQComplete();
    if (rfu_waitREQComplete())
    {
        for (;;)
            ;
    }
    REG_IME = 0;
    DmaStop(0);
    DmaStop(1);
    DmaStop(2);
    DmaStop(3);
    SoftReset(RESET_ALL & ~(RESET_IWRAM | RESET_SIO_REGS));
}

u32 BcdToInt(u32 bcd)
{
    u32 value = 0;
    int i;

    for (i = 0; i < 8; i++)
        value += Pow(10, i) * ((bcd >> (i * 4)) & 0xF);
    return value;
}

u32 Pow(u32 base, u32 exp)
{
    if (exp == 0)
        return 1;
    if (exp == 1)
        return base;
    return Pow(base, exp - 1) * base;
}

u32 IntToBcd(u32 value)
{
    u32 bcd = 0;
    int i;

    for (i = 0; i < 8; i++)
        bcd |= Mod(Div(value, Pow(10, i)), 10) << (i * 4);
    return bcd;
}

u32 BcdIncrement(u32 bcd)
{
    int i;

    bcd++;
    for (i = 0; i < 8; i++)
    {
        if (((bcd >> (i * 4)) & 0xF) != 10)
            return bcd;
        bcd -= 10 << (i * 4);
        bcd += 0x10 << (i * 4);
    }
    return 0;
}

u32 BcdDecrement(u32 bcd)
{
    int i;

    bcd--;
    for (i = 0; i < 8; i++)
    {
        if (((bcd >> (i * 4)) & 0xF) != 0xF)
            return bcd;
        bcd -= 6 << (i * 4);
    }
    return 0x99999999;
}

void SetRfuGameData(bool8 full)
{
    struct RfuGameData data;
    int value = (s8)full;
    int isFull = value;

    memset(&data, 0, sizeof(data));
    data.language = 2;
    data.unknown = TRUE;
    data.version = 4;
    data.playerGender = 1;
    data.startedActivity = isFull;
    data.trainerId[0] = 1;
    data.trainerId[1] = 0;
    if (sDistributionType == 1)
        data.hasNews = TRUE;
    if (sDistributionType == 2)
        data.hasCard = TRUE;
    if (isFull)
        data.partnerInfo[0] = 0x83;
    SetRfuGameName((u8 *)&data);
    SetRfuUserName(sRfuUserName);
}

void StartDistribution(const u8 *gameName, const u8 *userName)
{
    struct DistributionWork *work = SetSlotCallback(DistributionTask);

    CpuFastFill(0, work, sizeof(*work));
    memcpy(work->gameName, gameName, sizeof(work->gameName));
    memcpy(work->userName, userName, sizeof(work->userName));
}

u16 DistributionTask(void *data)
{
    struct DistributionWork *work = data;
    struct ContentSlot *slots;

    s32 strength = gRfuLinkStatus->strength[GetCurrentSlot()];

    // The game must be held close to the cartridge.
    if (strength < DISTRIBUTION_MIN_STRENGTH)
        return 1;
    if (++work->timer > 59)
    {
        GetCurrentSlotNews(&slots);
        BuildRankingNews((u16 *)work->gameName, work->userName);
        StartMgServer(work->gameName, work->userName, slots, sDistributionType & 1);
        IncrementDistributionCount();
    }
    return 0;
}
