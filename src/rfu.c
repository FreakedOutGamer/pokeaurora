#include "global.h"
#include "aurora.h"
#include "rfu.h"
#include "link.h"
#include "misc.h"

extern IntrFunc gIntrTable[];

bool8 InitRfu(void)
{
    if (rfu_initializeAPI(gRfuAPIBuffer, RFU_API_BUFF_SIZE_RAM, gIntrTable, TRUE) != 0)
        return FALSE;
    rfu_setTimerInterrupt(0, gIntrTable + 4);
    return TRUE;
}

void StopRfu(void)
{
    rfu_REQ_stopMode();
    rfu_waitREQComplete();
}

bool8 ResetRfu(void)
{
    int i;

    gRfuBusy = 0;
    gRfuFatalError = 0;
    gRfuRetransmit = 0;
    gRfuConnected = 0;
    gRfuConnectedOld = 0;
    gRfuUnused1B4D = 0;
    gRfuLost = 0;
    gRfuDisconnectRequest = 0;
    gRfuAckSlots = 0;
    gRfuDisconnectPending = 0;
    gRfuSendSlot = -1;
    for (i = 0; i < 4; i++)
        ClearRfuSlot(i);
    SetRfuGameData(0);
    InitRfuParams();
    if (rfu_LMAN_REQBN_softReset_and_checkID() != RFU_ID
     || rfu_LMAN_initializeManager(RfuLmanCallback, RfuMscCallback))
        return FALSE;
    for (i = 0; i < 4; i++)
        rfu_setRecvBuffer(TYPE_NI, i, gRfuSlots[i].recvNIBuffer, sizeof(gRfuSlots[i].recvNIBuffer));
    rfu_clearAllSlot();
    gRfuState = RFU_STATE_INIT;
    return TRUE;
}

void RfuMain(void)
{
    while (gRfuBusy == 1)
        ;
    RfuCheckReset();
    if (RfuUpdatePause())
        return;
    if (RfuHandleNewChildren())
        return;
    rfu_LMAN_manager_entity(0);
    if (RfuDisconnectLost())
        return;
    RfuSendNextSlot();
    RfuCheckNISend();
    RfuCheckNIRecv();
    switch (gRfuState)
    {
    case RFU_STATE_INIT:
        rfu_LMAN_initializeRFU(&sRfuInitParam);
        gRfuState = RFU_STATE_RUNNING;
        break;
    case RFU_STATE_CONNECT:
        if (rfu_LMAN_establishConnection(1, 0, 120, (u16 *)sAcceptedSerialNos) == 0)
            gRfuState = RFU_STATE_RUNNING;
        break;
    }
}

void RfuLmanCallback(u8 msg, u8 paramCount)
{
    switch (msg)
    {
    case LMAN_MSG_RECV_DATA_REQ_COMPLETED:
        RfuCheckAcks();
        RfuStoreUNIRecv();
        gRfuBusy = 0;
        break;
    case LMAN_MSG_INITIALIZE_COMPLETED:
        gRfuState = RFU_STATE_CONNECT;
        break;
    case LMAN_MSG_NEW_CHILD_CONNECT_ACCEPTED:
        gRfuConnected |= lman.param[0];
        break;
    case LMAN_MSG_NEW_CHILD_CONNECT_DETECTED:
    case LMAN_MSG_NEW_CHILD_CONNECT_REJECTED:
        break;
    case LMAN_MSG_LINK_LOSS_DETECTED_AND_DISCONNECTED:
    case LMAN_MSG_LINK_RECOVERY_FAILED_AND_DISCONNECTED:
    case LMAN_MSG_LINK_DISCONNECTED_BY_USER:
        gRfuConnected &= ~lman.param[0];
        ClearRfuSlots(lman.param[0]);
        if (gRfuConnected == 0 && gRfuState != RFU_STATE_PAUSE)
        {
            gRfuPrevState = gRfuState;
            gRfuState = RFU_STATE_PAUSE;
        }
        break;
    case LMAN_MSG_REQ_API_ERROR:
    case LMAN_MSG_WATCH_DOG_TIMER_ERROR:
        gRfuBusy = 0;
        gRfuFatalError = 1;
        break;
    case LMAN_MSG_RFU_FATAL_ERROR:
        gRfuBusy = 0;
        break;
    case LMAN_MSG_CLOCK_SLAVE_MS_CHANGE_ERROR_BY_DMA:
        gRfuFatalError = 1;
        break;
    }
}

void RfuMscCallback(u16 reqCommandId)
{
    rfu_REQ_recvData();
}

void InitRfuParams(void)
{
    sRfuInitParam.maxMFrame = 4;
    sRfuInitParam.MC_TimerCount = 32;
    sRfuInitParam.availSlot_flag = 0;
    sRfuInitParam.mboot_flag = 0;
    sRfuInitParam.serialNo = RFU_SERIAL_NO;
    sRfuInitParam.gameName = gRfuGameName;
    sRfuInitParam.userName = gRfuUserName;
    sRfuInitParam.fastSearchParent_flag = 0;
    sRfuInitParam.linkRecovery_enable = 1;
    sRfuInitParam.linkRecovery_period = 120;
    sRfuInitParam.NI_failCounter_limit = RFU_NI_FAIL_LIMIT;
}

void RfuSendNextSlot(void)
{
    struct RfuLinkStatus *status = gRfuLinkStatus;
    int i, j;

    if (status->parentChild != MODE_PARENT)
        return;
    if (gRfuRetransmit)
    {
        gRfuBusy = 1;
        rfu_REQ_PARENT_resumeRetransmitAndChange();
        return;
    }
    if (status->linkLossSlotFlag != 0 || gRfuDisconnectPending)
        return;
    gRfuSendSlot = (gRfuSendSlot + 1) & 3;
    for (i = 1; i <= 4; i++)
    {
        int slot = gRfuSendSlot;

        if (((gRfuConnected >> slot) & 1) && gRfuSlots[slot].sending && !gRfuSlots[slot].sendingNI)
        {
            u8 *data;
            u32 size;

            for (j = 0; j < 4; j++)
                rfu_clearSlot(TYPE_UNI_SEND, j);
            data = gRfuSlots[slot].data;
            size = gRfuSlots[slot].size;
            if (gRfuSlots[slot].type == TYPE_UNI)
            {
                if (rfu_UNI_setSendData(1 << slot, data, size) == 0)
                    rfu_UNI_readySendData(slot);
            }
            else
            {
                if (rfu_NI_setSendData(1 << slot, 4, data, size) == 0)
                    gRfuSlots[slot].sendingNI = 1;
            }
            goto send;
        }
        gRfuSendSlot++;
        gRfuSendSlot &= 3;
    }
    gRfuSendSlot = -1;
send:
    gRfuBusy = 1;
    rfu_LMAN_REQ_sendData(TRUE);
}

void RfuCheckNISend(void)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (gRfuSlotStatusNI[i]->send.state != 0)
        {
            if (gRfuSlotStatusNI[i]->send.state == SLOT_STATE_SEND_SUCCESS)
            {
                rfu_clearSlot(TYPE_NI_SEND, i);
                gRfuSlots[i].sending = 0;
                gRfuSlots[i].sendingNI = 0;
            }
            if (gRfuSlotStatusNI[i]->send.state == SLOT_STATE_SEND_FAILED
             || gRfuSlotStatusNI[i]->send.failCounter >= RFU_NI_FAIL_LIMIT)
            {
                rfu_clearSlot(TYPE_NI_SEND, i);
                gRfuConnected &= ~(1 << i);
                gRfuLost |= 1 << i;
                ClearRfuSlots(1 << i);
            }
        }
    }
}

void RfuCheckNIRecv(void)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (gRfuSlotStatusNI[i]->recv.state != 0)
        {
            if (gRfuSlotStatusNI[i]->recv.state == SLOT_STATE_RECV_SUCCESS)
            {
                rfu_clearSlot(TYPE_NI_RECV, i);
                gRfuSlots[i].recvNIDone = 1;
            }
            if (gRfuSlotStatusNI[i]->recv.state == SLOT_STATE_RECV_FAILED
             || gRfuSlotStatusNI[i]->recv.failCounter >= RFU_NI_FAIL_LIMIT)
            {
                rfu_clearSlot(TYPE_NI_RECV, i);
                gRfuConnected &= ~(1 << i);
                gRfuLost |= 1 << i;
                ClearRfuSlots(1 << i);
            }
        }
    }
}

void RfuCheckAcks(void)
{
    u8 noAck = (gRfuAckSlots ^ lman.parentAck_flag) & gRfuAckSlots;
    int i;

    for (i = 0; i < 4; i++)
    {
        if (!((noAck >> i) & 1))
        {
            sRfuTimeoutCounters[i] = 0;
        }
        else if (++sRfuTimeoutCounters[i] > 3)
        {
            gRfuConnected &= ~(1 << i);
            gRfuLost |= 1 << i;
            gRfuAckSlots &= ~(1 << i);
            sRfuTimeoutCounters[i] = 0;
        }
    }
    if (((gRfuConnected & ~gRfuLost) & lman.parentAck_flag) == (gRfuConnected & ~gRfuLost) || gRfuSendSlot == -1)
    {
        gRfuRetransmit = 0;
        if (gRfuLost)
            gRfuDisconnectPending = 1;
    }
    else
    {
        gRfuRetransmit = 1;
        gRfuDisconnectPending = 0;
    }
}

void RfuStoreUNIRecv(void)
{
    int i;

    if (gRfuRetransmit)
        return;
    for (i = 0; i < 4; i++)
    {
        if (((gRfuConnected >> i) & 1) && gRfuSlotStatusUNI[i]->recv.newDataFlag)
        {
            gRfuSlots[i].recvReady[gRfuSlots[i].recvBank] = 1;
            gRfuSlots[i].recvEmpty[gRfuSlots[i].recvBank] = 0;
            gRfuSlots[i].recvBank ^= 1;
            rfu_setRecvBuffer(TYPE_UNI, i, gRfuSlots[i].recvBuffer[gRfuSlots[i].recvBank], RFU_UNI_PACKET_SIZE);
            rfu_UNI_clearRecvNewDataFlag(i);
        }
    }
    if (gRfuSendSlot != -1 && gRfuSlotStatusNI[gRfuSendSlot]->send.state == 0)
        gRfuSlots[gRfuSendSlot].sending = 0;
}

bool8 RfuHandleNewChildren(void)
{
    u8 newSlots = (gRfuConnected ^ gRfuConnectedOld) & gRfuConnected;
    int i, j;

    for (i = 0; i < 4; i++)
    {
        if ((newSlots >> i) & 1)
        {
            for (j = 0; j < 16 && gRfuLinkStatus->partner[i].serialNo != sAcceptedSerialNos[j]; j++)
                ;
            SetCurrentSlot(i);
            sRfuConnectCallbacks[j](gRfuLinkStatus->partner[i].gname, gRfuLinkStatus->partner[i].uname);
            SetSlotSerialNo(i, gRfuLinkStatus->partner[i].serialNo);
            SetCurrentSlot(-1);
        }
    }
    gRfuConnectedOld = gRfuConnected;
    if (newSlots)
    {
        if (gRfuState == RFU_STATE_PAUSE)
            return TRUE;
        gRfuPrevState = gRfuState;
        gRfuState = RFU_STATE_PAUSE;
        return TRUE;
    }
    return FALSE;
}

bool8 RfuUpdatePause(void)
{
    if (gRfuState != RFU_STATE_PAUSE)
        return FALSE;
    if (++sRfuPauseTimer > 1)
    {
        sRfuPauseTimer = 0;
        gRfuState = gRfuPrevState;
    }
    return TRUE;
}

void RequestRfuDisconnect(int slot)
{
    if (((gRfuConnected >> slot) & 1) && !((gRfuLost >> slot) & 1))
        gRfuDisconnectRequest |= 1 << slot;
}

void RfuDisconnectSlots(int mask)
{
    int i;

    gRfuConnected &= ~mask;
    gRfuLost |= mask;
    for (i = 0; i < 4; i++)
    {
        if ((mask >> i) & 1)
        {
            ResetSlotCallbacks(i);
            ClearRfuSlot(i);
        }
    }
    gRfuDisconnectRequest &= ~mask;
}

bool8 RfuDisconnectLost(void)
{
    int i;

    RfuDisconnectRequested();
    if (gRfuLost == 0 || (gRfuDisconnectPending == 0 && ++sRfuDisconnectTimer < 0xFF))
        return FALSE;
    sRfuDisconnectTimer = 0;
    rfu_REQ_disconnect(gRfuLost);
    rfu_waitREQComplete();
    for (i = 0; i < 4; i++)
    {
        if ((gRfuLost >> i) & 1)
            rfu_clearSlot(TYPE_UNI_SEND | TYPE_UNI_RECV | TYPE_NI_SEND | TYPE_NI_RECV, i);
    }
    gRfuLost = 0;
    gRfuDisconnectPending = 0;
    if (gRfuState != RFU_STATE_PAUSE)
    {
        gRfuPrevState = gRfuState;
        gRfuState = RFU_STATE_PAUSE;
    }
    return TRUE;
}

void RfuDisconnectRequested(void)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (((gRfuDisconnectRequest >> i) & 1) && gRfuSlots[i].sending == 0)
        {
            gRfuConnected &= ~(1 << i);
            gRfuLost |= 1 << i;
            ClearRfuSlot(i);
            gRfuDisconnectRequest &= ~(1 << i);
        }
    }
}

void RfuRequireAck(void)
{
    int slot = GetCurrentSlot();

    if (((gRfuConnected >> slot) & 1) && !((gRfuLost >> slot) & 1))
        gRfuAckSlots |= 1 << slot;
}

void SetRfuGameName(const u8 *name)
{
    memcpy(gRfuGameName, name, 13);
}

void SetRfuUserName(const u8 *name)
{
    memcpy(gRfuUserName, name, 8);
}

void SetRfuUnusedBuffer(const u8 *src)
{
    memcpy(gRfuUnusedBuffer, src, 17);
}

bool8 IsRfuSlotConnected(int slot)
{
    if ((gRfuConnected >> slot) & 1)
        return TRUE;
    return FALSE;
}

void RfuUpdateGameData(void)
{
    if (gRfuRetransmit == 0 && gRfuLinkStatus->linkLossSlotFlag == 0 && gRfuState != RFU_STATE_PAUSE && gRfuDisconnectPending == 0
     && ++sRfuGameDataTimer != 42 && sRfuGameDataTimer > 53)
    {
        bool8 allConnected;

        if ((gRfuConnected & 0xF) == 0xF)
            allConnected = TRUE;
        else
            allConnected = FALSE;
        SetRfuGameData(allConnected);
        while (gRfuBusy == 1)
            ;
        rfu_REQ_configGameData(0, RFU_SERIAL_NO, gRfuGameName, gRfuUserName);
        sRfuGameDataTimer = 0;
        gRfuPrevState = gRfuState;
        gRfuState = RFU_STATE_PAUSE;
        rfu_waitREQComplete();
    }
}

void ClearRfuSlot(int slot)
{
    int i;

    gRfuSlots[slot].sending = 0;
    gRfuSlots[slot].sendingNI = 0;
    gRfuSlots[slot].type = 0;
    gRfuSlots[slot].size = 0;
    gRfuSlots[slot].data = NULL;
    gRfuSlots[slot].recvBank = 0;
    gRfuSlots[slot].recvNIDone = 0;
    for (i = 0; i < 2; i++)
    {
        gRfuSlots[slot].recvReady[i] = 0;
        gRfuSlots[slot].recvEmpty[i] = 1;
        memset(gRfuSlots[slot].recvBuffer[i], 0, RFU_UNI_PACKET_SIZE);
    }
    memset(gRfuSlots[slot].recvNIBuffer, 0, sizeof(gRfuSlots[slot].recvNIBuffer));
    rfu_setRecvBuffer(TYPE_UNI, slot, gRfuSlots[slot].recvBuffer[0], RFU_UNI_PACKET_SIZE);
}

void ClearRfuSlots(u32 mask)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if ((1 << i) & mask)
        {
            ClearRfuSlot(i);
            ResetSlotCallbacks(i);
        }
    }
}

void RfuCheckReset(void)
{
    if (gRfuFatalError)
        ResetGame();
    if (gRfuState != RFU_STATE_RUNNING || gRfuConnected != 0)
        sRfuIdleTimer = 0;
    else if (++sRfuIdleTimer > 599)
        ResetRfu();
}

int GetRfuConnectedCount(void)
{
    int count = 0;
    int i;

    for (i = 0; i < 4; i++)
        count += (gRfuConnected >> i) & 1;
    return count;
}

u16 RfuSendUNI(u8 *data)
{
    int slot = GetCurrentSlot();

    if (slot == -1 || gRfuSlots[slot].sending
     || (!gRfuSlots[slot].recvEmpty[0] && !gRfuSlots[slot].recvEmpty[1]))
        return TRUE;
    gRfuSlots[slot].type = TYPE_UNI;
    gRfuSlots[slot].data = data;
    gRfuSlots[slot].size = RFU_UNI_FRAME_SIZE;
    gRfuSlots[slot].sending = 1;
    return FALSE;
}

u16 RfuSendNI(u8 *data, u32 size)
{
    int slot = GetCurrentSlot();

    if (slot == -1 || gRfuSlots[slot].sending || gRfuSlots[slot].sendingNI || gRfuSlots[slot].recvNIDone
     || gRfuSlotStatusNI[slot]->send.state || gRfuSlotStatusNI[slot]->recv.state
     || (!gRfuSlots[slot].recvEmpty[0] && !gRfuSlots[slot].recvEmpty[1]))
        return TRUE;
    gRfuSlots[slot].type = TYPE_NI;
    gRfuSlots[slot].data = data;
    gRfuSlots[slot].size = size;
    gRfuSlots[slot].sending = 1;
    return FALSE;
}

u16 RfuRecvUNI(u8 *dest)
{
    int slot = GetCurrentSlot();
    u8 bank;

    if (!gRfuSlots[slot].recvReady[0] && !gRfuSlots[slot].recvReady[1])
        return TRUE;
    bank = gRfuSlots[slot].recvBank;
    if (!gRfuSlots[slot].recvReady[bank])
        bank ^= 1;
    memcpy(dest, gRfuSlots[slot].recvBuffer[bank], RFU_UNI_PACKET_SIZE);
    memset(gRfuSlots[slot].recvBuffer[bank], 0, RFU_UNI_PACKET_SIZE);
    gRfuSlots[slot].recvReady[bank] = 0;
    gRfuSlots[slot].recvEmpty[bank] = 1;
    return FALSE;
}

u16 RfuRecvNI(u8 *dest)
{
    int slot = GetCurrentSlot();

    if (!gRfuSlots[slot].recvNIDone)
        return TRUE;
    memcpy(dest, gRfuSlots[slot].recvNIBuffer, sizeof(gRfuSlots[slot].recvNIBuffer));
    memset(gRfuSlots[slot].recvNIBuffer, 0, sizeof(gRfuSlots[slot].recvNIBuffer));
    gRfuSlots[slot].recvNIDone = 0;
    return FALSE;
}

struct RfuSlotStatusBits
{
    u16 unused:13;
    u16 recvNIDone:1;
    u16 sending:1;
    u16 recvReady:1;
};

u16 GetRfuSlotStatus(void)
{
    struct RfuSlotStatusBits status;
    int slot = GetCurrentSlot();

    status.sending = gRfuSlots[slot].sending;
    status.recvReady = gRfuSlots[slot].recvReady[0] | gRfuSlots[slot].recvReady[1];
    status.recvNIDone = gRfuSlots[slot].recvNIDone;
    return *(u16 *)&status;
}
