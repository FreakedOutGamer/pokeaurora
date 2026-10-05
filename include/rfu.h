#ifndef GUARD_RFU_H
#define GUARD_RFU_H

#include "librfu.h"
#include "AgbRfu_LinkManager.h"

// gRfuState
#define RFU_STATE_RUNNING 1
#define RFU_STATE_INIT    2  // start the link manager
#define RFU_STATE_CONNECT 3  // start accepting children
#define RFU_STATE_PAUSE   4  // skip two frames after the connections change

// GetRfuSlotStatus
#define RFU_STATUS_RECV_NI_DONE (1 << 13)
#define RFU_STATUS_SENDING      (1 << 14)
#define RFU_STATUS_RECV_READY   (1 << 15)

#define RFU_SERIAL_NO      0x7F7D // this cartridge's serial number
#define RFU_UNI_PACKET_SIZE 14
#define RFU_UNI_FRAME_SIZE  70
#define RFU_NI_FAIL_LIMIT   600

struct RfuSlot
{
    /*0x00*/ s8 sending;
    /*0x01*/ s8 sendingNI;
    /*0x02*/ u8 type;
    /*0x03*/ u8 size;
    /*0x04*/ u8 *data;
    /*0x08*/ s8 recvReady[2];
    /*0x0A*/ s8 recvEmpty[2];
    /*0x0C*/ s8 recvNIDone;
    /*0x0D*/ u8 recvBank;
    /*0x0E*/ u8 recvBuffer[2][RFU_UNI_PACKET_SIZE];
    /*0x2A*/ u8 recvNIBuffer[0x20];
    /*0x4A*/ u8 filler_4A[2];
};

extern u8 sRfuTimeoutCounters[4];
extern s8 sRfuPauseTimer;
extern u8 sRfuDisconnectTimer;
extern u8 sRfuGameDataTimer;
extern u16 sRfuIdleTimer;
extern INIT_PARAM sRfuInitParam;
extern struct RfuSlot gRfuSlots[4];
extern vu8 gRfuBusy;
extern u8 gRfuState;
extern u8 gRfuPrevState;
extern u8 gRfuConnected;
extern u8 gRfuConnectedOld;
extern u8 gRfuUnused1B4D;
extern u8 gRfuLost;
extern u8 gRfuDisconnectRequest;
extern u8 gRfuAckSlots;
extern s8 gRfuSendSlot;
extern u8 gRfuGameName[16];
extern u8 gRfuUserName[8];
extern u8 gRfuUnusedBuffer[0x11];
extern s8 gRfuFatalError;
extern s8 gRfuRetransmit;
extern s8 gRfuDisconnectPending;
extern u32 gRfuAPIBuffer[];

extern const u16 sAcceptedSerialNos[16];
extern void (*const sRfuConnectCallbacks[16])(const u8 *gname, const u8 *uname);

bool8 InitRfu(void);
void StopRfu(void);
bool8 ResetRfu(void);
void RfuMain(void);
void RfuLmanCallback(u8 msg, u8 paramCount);
void RfuMscCallback(u16 reqCommandId);
void InitRfuParams(void);
void RfuSendNextSlot(void);
void RfuCheckNISend(void);
void RfuCheckNIRecv(void);
void RfuCheckAcks(void);
void RfuStoreUNIRecv(void);
bool8 RfuHandleNewChildren(void);
bool8 RfuUpdatePause(void);
void RequestRfuDisconnect(int slot);
void RfuDisconnectSlots(int mask);
bool8 RfuDisconnectLost(void);
void RfuDisconnectRequested(void);
void RfuRequireAck(void);
void SetRfuGameName(const u8 *name);
void SetRfuUserName(const u8 *name);
void SetRfuUnusedBuffer(const u8 *src);
bool8 IsRfuSlotConnected(int slot);
void RfuUpdateGameData(void);
void ClearRfuSlot(int slot);
void ClearRfuSlots(u32 mask);
void RfuCheckReset(void);
int GetRfuConnectedCount(void);
u16 RfuSendUNI(u8 *data);
u16 RfuSendNI(u8 *data, u32 size);
u16 RfuRecvUNI(u8 *dest);
u16 RfuRecvNI(u8 *dest);
u16 GetRfuSlotStatus(void);

// Not yet decompiled
void ResetSlotCallbacks(int slot);

#endif // GUARD_RFU_H
