#ifndef GUARD_LINK_H
#define GUARD_LINK_H

struct SiiRtcInfo;
struct ContentSlot;

typedef u16 (*SlotFunc)(void *work);

struct SlotPos
{
    u16 x;
    u16 y;
};

// Per-slot data that's only ever cleared.
struct SlotData
{
    u16 state;
    u8 unused[0x52];
};

// While games are connected, the save is written once an hour (in frames).
#define SAVE_INTERVAL (60 * 60 * 60)

extern u32 sSaveTimer;
extern struct SlotData gSlotData[4];
extern SlotFunc sSlotCallbacks[4];
extern SlotFunc sSlotDebugCallbacks[4];
extern u16 sSlotSerialNos[4];
extern u16 sSlotTimes[4];
extern u16 sSlotMaxTimes[4];
extern s8 sCurrentSlot;
extern s8 sDebugSlot;
extern s8 sDebugCursor;
extern s8 sSaveRequested;
extern s8 sDebugScreenActive;
extern u8 gSlotWork[4][0x8000];

extern const struct SlotPos sDebugSlotPos[4];
extern const struct SlotPos sDebugLabelPos_Serial;
extern const struct SlotPos sDebugLabelPos_Time;
extern const struct SlotPos sDebugLabelPos_MaxTime;
extern const struct SlotPos sDebugLabelPos_Strength;
extern const struct SlotPos sDebugSerialPos[4];
extern const struct SlotPos sDebugTimePos[4];
extern const struct SlotPos sDebugMaxTimePos[4];
extern const struct SlotPos sDebugStrengthPos[4];
extern const u8 sText_Slot[];
extern const u8 sText_Serial[];
extern const u8 sText_SlotTime[];
extern const u8 sText_MaxTime[];
extern const u8 sText_Strength[];
extern const u32 sDebugGfx[];
extern const u32 sDebugBgPalette[];
extern const u32 sDebugObjPalette[];

void SlotTasks_Main(void);
void SlotTasks_Init(void);
void ResetSlotStats(int slot);
void DebugScreen_Update(void);
void DebugScreen_DrawStats(void);
void DebugScreen_RunSlot(void);
void CheckIdChangeKeys(void);
void DebugScreen_HandleInput(void);
void DebugScreen_HandleCancel(void);
void DebugScreen_Open(void);
void DebugScreen_Close(void);
u16 SlotIdle(void *work);
void ResetSlotCallbacks(int slot);
int GetCurrentSlot(void);
void SetCurrentSlot(int slot);
void SetSlotSerialNo(int slot, u32 serialNo);
void UpdatePeriodicSave(void);
s8 IsDebugScreenActive(void);
void ClearDebugScreenActive(void);

void UpdateHourly(bool8 force);
void CheckExpiryReset(struct SiiRtcInfo *rtc);
u32 GetCurrentSlotSerialNo(void);
void *SetSlotCallback(SlotFunc func);
void *SetSlotDebugCallback(SlotFunc func);

// Not yet decompiled
void InitScreen(void (*func)(void));
void ChooseNumberScreen(void);
void SetAdapterError(int a);

#endif // GUARD_LINK_H
