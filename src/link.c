#include "global.h"
#include "siirtc.h"
#include "aurora.h"
#include "content.h"
#include "rfu.h"
#include "link.h"
#include "misc.h"

u8 gSlotWork[4][0x8000] = {0};

void SlotTasks_Main(void)
{
    int i;

    if (GetMainStateChanged())
        SlotTasks_Init();
    for (i = 0; i < 4; i++)
    {
        sCurrentSlot = i;
        REG_TM1CNT_H &= 0x7F;
        REG_TM1CNT_L = 0;
        REG_TM1CNT_H |= TIMER_ENABLE;
        if (sSlotCallbacks[i](gSlotWork[i]) == 0)
        {
            sSlotTimes[i] = REG_TM1CNT_L;
            if (sSlotTimes[i] > sSlotMaxTimes[i])
                sSlotMaxTimes[i] = sSlotTimes[i];
        }
        else
        {
            sSlotCallbacks[i] = SlotIdle;
            sSlotDebugCallbacks[i] = NULL;
            RequestRfuDisconnect(i);
        }
    }
    sCurrentSlot = -1;
    RfuMain();
    DebugScreen_Update();
    RfuMain();
    RfuUpdateGameData();
    UpdateHourly(0);
}

void SlotTasks_Init(void)
{
    int tries = 0;
    int i;

    for (i = 0; i < 4; i++)
    {
        ResetSlotStats(i);
        gSlotData[i].state = 0;
    }
    sDebugScreenActive = 0;
    sDebugSlot = -1;
    sSaveRequested = 0;
    REG_TM1CNT_H &= 0x3A;
    REG_TM1CNT_H |= TIMER_256CLK;
    for (i = 0; i < 4; i++)
    {
        sSlotCallbacks[i] = SlotIdle;
        sSlotDebugCallbacks[i] = NULL;
    }
    UpdateHourly(1);
    do
    {
        if (ResetRfu())
        {
            SetAdapterError(0);
            return;
        }
    } while (++tries < 14);
    SetAdapterError(1);
    for (;;)
        WaitForVBlank();
}

void ResetSlotStats(int slot)
{
    sSlotSerialNos[slot] = 0;
    sSlotTimes[slot] = 0;
    sSlotMaxTimes[slot] = 0;
}

void DebugScreen_Update(void)
{
    if (sDebugScreenActive)
    {
        if (sDebugSlot == -1)
            DebugScreen_DrawStats();
        else
            DebugScreen_RunSlot();
    }
}

void DebugScreen_DrawStats(void)
{
    int i;

    UpdateCursorAnim(0);
    ClearTilemap();
    for (i = 0; i < 4; i++)
    {
        u16 x = sDebugSlotPos[i].x;
        u16 y = sDebugSlotPos[i].y;

        PrintText(x, y, 0, sText_Slot);
        PrintDec(x + 4, y, 0, i);
    }
    PrintText(sDebugLabelPos_Serial.x, sDebugLabelPos_Serial.y, 13, sText_Serial);
    PrintText(sDebugLabelPos_Time.x, sDebugLabelPos_Time.y, 13, sText_SlotTime);
    PrintText(sDebugLabelPos_MaxTime.x, sDebugLabelPos_MaxTime.y, 13, sText_MaxTime);
    PrintText(sDebugLabelPos_Strength.x, sDebugLabelPos_Strength.y, 13, sText_Strength);
    for (i = 0; i < 4; i++)
    {
        PrintHex4(sDebugSerialPos[i].x, sDebugSerialPos[i].y, 2, sSlotSerialNos[i]);
        PrintHex4(sDebugTimePos[i].x, sDebugTimePos[i].y, 3, sSlotTimes[i]);
        PrintHex4(sDebugMaxTimePos[i].x, sDebugMaxTimePos[i].y, 1, sSlotMaxTimes[i]);
        PrintHex2(sDebugStrengthPos[i].x, sDebugStrengthPos[i].y, 6, gRfuLinkStatus->strength[i]);
    }
}

void DebugScreen_RunSlot(void)
{
    SlotFunc func = sSlotDebugCallbacks[sDebugSlot];

    if (func == NULL)
    {
        sDebugSlot = -1;
        gOamBuffer[0].affineMode = 0;
    }
    else
    {
        func(gSlotWork[sDebugSlot]);
    }
}

void CheckIdChangeKeys(void)
{
    if ((gKeysHeld & (A_BUTTON | B_BUTTON)) == (A_BUTTON | B_BUTTON))
    {
        StopRfu();
        ChooseNumberScreen();
        for (;;)
            ;
    }
}

void DebugScreen_HandleInput(void)
{
    if (IsCursorAnimating())
        return;
    if (gKeysNew & A_BUTTON)
    {
        int slot = (sDebugCursor - 4) >> 1;

        if (sSlotDebugCallbacks[slot] != NULL)
        {
            sDebugSlot = slot;
            gOamBuffer[0].affineMode = 2;
        }
    }
    if (gKeysRepeat & DPAD_UP)
    {
        if (sDebugCursor != 4)
        {
            sDebugCursor -= 2;
            StartCursorAnim(sDebugCursor, 2, 1);
        }
        else
        {
            sDebugCursor = 10;
            StartCursorAnim(10, 8, 1);
        }
    }
    if (gKeysRepeat & DPAD_DOWN)
    {
        if (sDebugCursor != 10)
        {
            sDebugCursor += 2;
            StartCursorAnim(sDebugCursor, 2, 1);
        }
        else
        {
            sDebugCursor = 4;
            StartCursorAnim(4, 8, 1);
        }
    }
}

void DebugScreen_HandleCancel(void)
{
    if (gKeysNew & B_BUTTON)
    {
        sDebugSlot = -1;
        gOamBuffer[0].affineMode = 0;
    }
}

void DebugScreen_Open(void)
{
    REG_DISPCNT = 0;
    REG_BLDCNT = 0;
    sDebugScreenActive = 1;
    CpuFastSet(sDebugGfx, (void *)BG_CHAR_ADDR(2), 0x880);
    CpuFastSet(sDebugBgPalette, (void *)PLTT, 0x80);
    REG_BG0CNT = BGCNT_CHARBASE(2);
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
    CpuFastSet(sDebugGfx, (void *)OBJ_VRAM0, 0x880);
    CpuFastSet(sDebugObjPalette, (void *)OBJ_PLTT, 0x80);
    CpuFastFill(0xA0, gOamBuffer, sizeof(gOamBuffer));
    ResetCursorAnim();
    if (sDebugCursor == 0)
        sDebugCursor = 4;
    HideAllSprites();
    gOamBuffer[0].affineMode = 0;
    gOamBuffer[0].tileNum = 0x106;
    gOamBuffer[0].paletteNum = 0;
    gOamBuffer[0].x = 0;
    gOamBuffer[0].y = sDebugCursor * 8;
    REG_DISPCNT = DISPCNT_OBJ_ON | DISPCNT_BG0_ON;
}

void DebugScreen_Close(void)
{
    REG_DISPCNT = 0;
    sDebugScreenActive = 0;
    sDebugSlot = -1;
    InitScreen(WaitForVBlank);
}

u16 SlotIdle(void *work)
{
    return 0;
}

void ResetSlotCallbacks(int slot)
{
    sSlotCallbacks[slot] = SlotIdle;
    sSlotDebugCallbacks[slot] = NULL;
}

int GetCurrentSlot(void)
{
    return sCurrentSlot;
}

void SetCurrentSlot(int slot)
{
    sCurrentSlot = slot;
}

void SetSlotSerialNo(int slot, u32 serialNo)
{
    ResetSlotStats(slot);
    sSlotSerialNos[slot] = serialNo;
}

void UpdatePeriodicSave(void)
{
    if (!IsSaving())
        return;
    if (GetRfuConnectedCount() != 0 && sSaveRequested == 0)
    {
        if (++sSaveTimer < SAVE_INTERVAL)
            return;
        sSaveRequested = 1;
    }
    sSaveTimer = 0;
    if (sSaveRequested)
    {
        if (!SaveStep())
            sSaveRequested = 0;
    }
    else
    {
        StopRfu();
        while (SaveStep())
            ;
        while (!ResetRfu())
            ;
    }
}

s8 IsDebugScreenActive(void)
{
    return sDebugScreenActive;
}

void ClearDebugScreenActive(void)
{
    sDebugScreenActive = 0;
}
struct ContentDate
{
    u32 year:8;
    u32 month:8;
    u32 day:8;
    u32 hour:8;
};

extern u8 sLastHour;

void UpdateHourly(bool8 force)
{
    struct SiiRtcInfo rtc;
    u8 buffer[17];
    struct ContentDate date;

    if (force)
        sLastHour = 0xFF;
    SiiRtcGetDateTime(&rtc);
    CheckExpiryReset(&rtc);
    if (rtc.hour != sLastHour)
    {
        sLastHour = rtc.hour;
        date.year = rtc.year;
        date.month = rtc.month;
        date.day = rtc.day;
        date.hour = rtc.hour;
        SetRfuUnusedBuffer(buffer);
        UpdateCurrentNews((u8 *)&date);
    }
}

void CheckExpiryReset(struct SiiRtcInfo *rtc)
{
    if (rtc->year < sExpiryDate[0])
        return;
    if (rtc->year > sExpiryDate[0])
        ResetGame();
    if (rtc->month < sExpiryDate[1])
        return;
    if (rtc->month > sExpiryDate[1])
        ResetGame();
    if (rtc->day < sExpiryDate[2])
        return;
    if (rtc->day > sExpiryDate[2])
        ResetGame();
    if (rtc->hour < sExpiryDate[3])
        return;
    ResetGame();
}

u32 GetCurrentSlotSerialNo(void)
{
    if (sCurrentSlot == -1)
        return 2;
    return sSlotSerialNos[sCurrentSlot];
}

void *SetSlotCallback(SlotFunc func)
{
    sSlotCallbacks[sCurrentSlot] = func;
    return gSlotWork[sCurrentSlot];
}

void *SetSlotDebugCallback(SlotFunc func)
{
    sSlotDebugCallbacks[sCurrentSlot] = func;
    return gSlotWork[sCurrentSlot];
}
