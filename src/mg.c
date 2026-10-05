#include "global.h"
#include "aurora.h"
#include "rfu.h"
#include "link.h"
#include "misc.h"
#include "mg.h"

#define MG_ASSERT(cond)           \
    if (!(cond))                  \
    {                             \
        gMgWork->done = 1;        \
        return;                   \
    }

void StartMgServer(const u8 *gameName, const u8 *userName, struct ContentSlot *slots, u8 type)
{
    u16 i;

    gMgWork = SetSlotCallback(MgServerTask);
    CpuFastFill(0, gMgWork, 0x12DC);
    gMgWork->done = 0;
    gMgWork->timer = 0;
    gMgWork->type = type;
    gMgWork->entries = gMgWork->entryBuffer;
    if (type == 1)
    {
        for (i = 0; slots[i].data != NULL; i++)
        {
            gMgWork->entries[i].header = slots[i].header;
            memcpy(&gMgWork->entries[i].data, slots[i].data, sizeof(struct ContentData));
        }
        gMgWork->numEntries = i;
    }
    memcpy(gMgWork->gameName, gameName, sizeof(gMgWork->gameName));
    memcpy(gMgWork->userName, userName, sizeof(gMgWork->userName));
    MgRfuLink_Init();
    SetMgState(MgState_Handshake);
}

void SetMgState(void (*func)(void))
{
    gMgWork->state = func;
    gMgWork->substate = 0;
    gMgWork->stateTimer = 0;
}

u16 MgServerTask(void *work)
{
    gMgWork = work;
    gMgWork->status = GetRfuSlotStatus();
    if (gMgWork->status & RFU_STATUS_SENDING)
        return 0;
    gMgWork->timer++;
    if (gMgWork->link.connected)
    {
        MgRfuLink_Update();
        gMgWork->state();
    }
    else if (MgRfuLink_Connect())
    {
        gMgWork->link.connected = 1;
    }
    else if (gMgWork->timer > 600)
    {
        gMgWork->done = 1;
    }
    if (gMgWork->done)
        return 1;
    return 0;
}

void MgRfuLink_Init(void)
{
    CpuFastFill(0, &gMgWork->link, sizeof(gMgWork->link));
    gMgWork->link.timer = 0;
    gMgWork->link.recvState = 0;
    gMgWork->link.sendState = 0;
    gMgWork->link.recvLen = 0;
    gMgWork->link.lastHeader = 8;
    gMgWork->link.dupCount = 0;
    gMgWork->link.recvDelay = 0;
    gMgWork->link.sendCounter = 0;
    gMgWork->link.connected = 0;
}

void MgRfuLink_Update(void)
{
    MgRfuLink_Recv();
    MgRfuLink_Send();
    if (gMgWork->link.timer > 600)
        gMgWork->done = 1;
}

bool8 MgRfuLink_InitSend(const u8 *data, u8 size)
{
    struct MgRfuLink *link = &gMgWork->link;

    if (size > 0xFC)
        return FALSE;
    if (size == 0)
        return FALSE;
    if (link->sendState)
        return FALSE;
    link->sendState = 2;
    link->sendIndex = 0;
    link->sendCount = (size + 11) / 12;
    memset(link->sendData, 0, sizeof(link->sendData));
    memcpy(link->sendData, data, size);
    return TRUE;
}

bool8 MgRfuLink_SendCommand(u8 cmd)
{
    bool8 ret = FALSE;
    struct MgRfuLink *link = &gMgWork->link;
    int i;

    if (link->sendState)
        return FALSE;
    link->sendPacket[0] = 0;
    link->sendPacket[1] = cmd;
    switch (cmd)
    {
    case RFUCMD_SEND_PLAYER_IDS:
    {
        int slot;

        link->sendPacket[2] = 2;
        link->sendPacket[3] = 0;
        slot = GetCurrentSlot();
        for (i = 0; i < 4; i++)
        {
            if (i == slot)
                link->sendPacket[4 + i] = 1;
            else
                link->sendPacket[4 + i] = 0;
        }
        link->sendState = 1;
        ret = TRUE;
        break;
    }
    case RFUCMD_SEND_BLOCK_REQ:
        link->sendPacket[2] = 0;
        link->sendPacket[3] = 0;
        link->sendState = 1;
        ret = TRUE;
        break;
    }
    return ret;
}

bool8 MgRfuLink_IsSendIdle(void)
{
    if (!gMgWork->link.sendState)
        return TRUE;
    return FALSE;
}

bool8 MgRfuLink_GetRecv(u8 *dest, u8 *size)
{
    if (gMgWork->link.recvLen && !gMgWork->link.recvDelay)
    {
        memcpy(dest, gMgWork->link.recvData, gMgWork->link.recvLen);
        *size = gMgWork->link.recvLen;
        gMgWork->link.recvLen = 0;
        return TRUE;
    }
    return FALSE;
}

bool8 MgRfuLink_Connect(void)
{
    bool8 ret = FALSE;
    struct MgRfuLink *link = &gMgWork->link;

    if (gMgWork->status & RFU_STATUS_RECV_NI_DONE)
    {
        if (!RfuRecvNI(link->recvPacket))
            link->recvBuffer[0] = link->recvPacket[0];
    }
    if (gMgWork->status & RFU_STATUS_SENDING)
    {
        ret = FALSE;
    }
    else
    {
        link->sendPacket[0] = 5;
        if (!RfuSendNI(link->sendPacket, 1))
            ret = TRUE;
    }
    return ret;
}

bool8 MgRfuLink_HasRecv(void)
{
    if (gMgWork->status & RFU_STATUS_RECV_READY)
        return TRUE;
    return FALSE;
}

bool8 MgRfuLink_IsNotSending(void)
{
    if (gMgWork->status & RFU_STATUS_SENDING)
        return FALSE;
    return TRUE;
}

void MgRfuLink_Recv(void)
{
    bool32 progress = FALSE;
    struct MgRfuLink *link = &gMgWork->link;
    u16 err;

    if (!MgRfuLink_HasRecv())
        goto end;
    err = RfuRecvUNI(link->recvPacket);
    if (err != 0)
        goto end;
    if (*(u16 *)link->recvPacket == 0)
    {
        progress = TRUE;
        goto delay;
    }
    if ((link->recvPacket[0] >> 5) == link->lastHeader)
    {
        if (++link->dupCount > 4)
        {
            gMgWork->done = 1;
            return;
        }
        goto delay;
    }
    link->lastHeader = link->recvPacket[0] >> 5;
    link->dupCount = progress;
    progress = TRUE;
    switch (link->recvState)
    {
    case 0:
        switch (link->recvPacket[1])
        {
        case RFUCMD_SEND_BLOCK_INIT:
            link->recvCount = link->recvPacket[2];
            if (link->recvCount > 32)
            {
                progress = FALSE;
                break;
            }
            link->recvMask = err;
            if (link->recvCount)
                link->recvState = progress;
            break;
        case RFUCMD_READY_CLOSE_LINK:
            link->sendState = 3;
            link->sendIndex = err;
            break;
        default:
            progress = FALSE;
            break;
        }
        break;
    case 1:
        switch (link->recvPacket[1])
        {
        case RFUCMD_SEND_BLOCK:
        {
            u8 index = link->recvPacket[0] & 0x1F;

            link->recvMask |= progress << index;
            memcpy(&link->recvBuffer[index * 12], &link->recvPacket[2], 12);
            if (link->recvMask == (progress << link->recvCount) - 1)
            {
                link->recvLen = link->recvCount * 12;
                memcpy(link->recvData, link->recvBuffer, link->recvLen);
                link->recvCount = err;
                link->recvMask = err;
                link->recvDelay = progress;
                link->recvState = err;
            }
            break;
        }
        case RFUCMD_READY_CLOSE_LINK:
            link->sendState = 3;
            link->sendIndex = err;
            return;
        default:
            progress = FALSE;
            break;
        }
        break;
    }
delay:
    if (link->recvDelay)
    {
        if (link->recvDelay > 3)
            link->recvDelay = 0;
        else
            link->recvDelay++;
    }
end:
    if (!progress)
    {
        if (link->recvState == 1)
            link->timer++;
    }
    else
    {
        link->timer = 0;
    }
}

void MgRfuLink_Send(void)
{
    bool8 sent = FALSE;
    struct MgRfuLink *link = &gMgWork->link;

    switch (link->sendState)
    {
    case 0:
        memset(link->sendPacket, 0, RFU_UNI_FRAME_SIZE);
        break;
    case 2:
        if (link->sendIndex == 0)
        {
            link->sendPacket[0] = sent;
            link->sendPacket[1] = RFUCMD_SEND_BLOCK_INIT;
            link->sendPacket[2] = link->sendCount;
            link->sendPacket[3] = sent;
            link->sendPacket[4] = 0x80;
            link->sendPacket[5] = sent;
            memset(&link->sendPacket[6], 0, 8);
        }
        else if (link->sendIndex <= link->sendCount)
        {
            link->sendPacket[0] = link->sendIndex - 1;
            link->sendPacket[1] = RFUCMD_SEND_BLOCK;
            memcpy(&link->sendPacket[2], &link->sendData[(link->sendIndex - 1) * 12], 12);
        }
        else
        {
            link->sendPacket[0] = sent;
            link->sendPacket[1] = RFUCMD_READY_EXIT_STANDBY;
            link->sendPacket[2] = link->sendCounter;
            link->sendPacket[3] = sent;
        }
        break;
    case 3:
        if (link->sendIndex == 0)
        {
            link->sendPacket[0] = sent;
            link->sendPacket[1] = RFUCMD_READY_CLOSE_LINK;
            link->sendPacket[2] = link->sendCounter;
            link->sendPacket[3] = sent;
        }
        else
        {
            memset(link->sendPacket, 0, RFU_UNI_FRAME_SIZE);
        }
        break;
    }
    if (MgRfuLink_IsNotSending())
    {
        memcpy((u32 *)link->echoPacket, (u32 *)link->recvPacket, 14);
        link->sendPacket[0] &= 0x1F;
        link->echoPacket[0] &= 0x1F;
        memset(link->filler_63E, 0, sizeof(link->filler_63E));
        if (!RfuSendUNI(link->sendPacket))
            sent = TRUE;
    }
    if (!sent)
    {
        link->timer++;
        return;
    }
    link->timer = 0;
    switch (link->sendState)
    {
    case 1:
        link->sendState = 0;
        break;
    case 2:
        link->sendIndex++;
        if (link->sendIndex > link->sendCount + 2)
        {
            link->sendCounter++;
            link->sendState = 0;
            link->sendIndex = 0;
            link->sendCount = 0;
        }
        break;
    case 3:
        if (link->sendIndex == 0)
        {
            RfuRequireAck();
            link->sendIndex = 1;
        }
        break;
    }
}

void MgState_Handshake(void)
{
    gMgWork->stateTimer++;
    switch (gMgWork->substate)
    {
    case 0:
        MgInitServerInfo(gMgWork->serverInfo);
        memcpy(gMgWork->buffer, gMgWork->serverInfo, sizeof(gMgWork->serverInfo));
        if (gMgWork->stateTimer > 2)
            MgNextSubstate();
        break;
    case 1:
        if (MgRfuLink_SendCommand(RFUCMD_SEND_PLAYER_IDS))
            MgNextSubstate();
        break;
    case 2:
    case 4:
        if (MgRfuLink_IsSendIdle())
            MgNextSubstate();
        break;
    case 3:
        if (MgRfuLink_SendCommand(RFUCMD_SEND_BLOCK_REQ))
            MgNextSubstate();
        break;
    case 5:
        if (MgRfuLink_InitSend(gMgWork->buffer, 0xC8))
            MgNextSubstate();
        break;
    case 6:
        if (MgRfuLink_IsSendIdle() && MgRfuLink_GetRecv(gMgWork->buffer, &gMgWork->bufferLen))
            MgNextSubstate();
        break;
    case 7:
        memcpy(gMgWork->clientInfo, gMgWork->buffer, sizeof(gMgWork->clientInfo));
        SetMgState(MgState_Server);
        break;
    }
    if (gMgWork->stateTimer > 1200)
        gMgWork->done = 1;
}

void MgState_Server(void)
{
    gMgWork->stateTimer++;
    switch (gMgWork->substate)
    {
    case 0:
        MgServer_Init();
        MgNextSubstate();
        break;
    case 1:
        MgServer_CallFunc();
        break;
    case 2:
        MgServer_Done();
        break;
    }
    if (gMgWork->stateTimer > 1200)
        gMgWork->done = 1;
}

void MgNextSubstate(void)
{
    gMgWork->substate++;
    gMgWork->stateTimer = 0;
}

void MgInitServerInfo(u8 *data)
{
    struct MgServerInfo *info = (struct MgServerInfo *)data;
    struct MgInfoBody *body = &info->body;

    body->number = GetTerminalId();
    memcpy(body->userName, sRfuUserName, sizeof(body->userName));
    body->unk13 = 1;
    body->unk14 = 0;
    body->unk1A = 2;
    body->unk0 = 0x4004;
    body->unk2 = 0x8000;
    body->unk10 = 0;
    memcpy(&info->header1, &sMgServerInfoHeader, 15);
    memcpy(&info->header2, &sMgServerInfoHeader, 15);
}

void MgServer_Init(void)
{
    struct MysteryGiftServer *svr;

    gMgWork->script = sMgScripts[gMgWork->type].script;
    svr = &gMgWork->server;
    svr->unused = 0;
    svr->funcId = FUNC_INIT;
    memset(svr->card, 0, sizeof(svr->card));
    memset(svr->news, 0, sizeof(svr->news));
    memset(svr->recvBuffer, 0, sizeof(svr->recvBuffer));
    memset(svr->linkGameData, 0, sizeof(svr->linkGameData));
    svr->script = gMgWork->script;
    svr->cmdidx = 0;
    svr->link.sendPlayerId = 0;
    svr->link.recvPlayerId = 1;
    svr->link.state = 0;
    svr->link.sendCRC = 0;
    svr->link.sendSize = 0;
    svr->link.sendCounter = 0;
    svr->link.recvCRC = 0;
    svr->link.recvSize = 0;
    svr->link.recvCounter = 0;
    svr->link.sendBuffer = NULL;
    svr->link.recvBuffer = NULL;
}

void MgServer_CallFunc(void)
{
    switch (gMgWork->server.funcId)
    {
    case FUNC_INIT:
        gMgWork->server.funcId = FUNC_RUN;
        gMgWork->stateTimer = 0;
        break;
    case FUNC_DONE:
        MgNextSubstate();
        gMgWork->stateTimer = 0;
        break;
    case FUNC_RECV:
        if (MysteryGiftLink_Recv(&gMgWork->server.link))
        {
            gMgWork->server.funcId = FUNC_RUN;
            gMgWork->stateTimer = 0;
        }
        break;
    case FUNC_SEND:
        if (MysteryGiftLink_Send(&gMgWork->server.link))
        {
            gMgWork->server.funcId = FUNC_RUN;
            gMgWork->stateTimer = 0;
        }
        break;
    case FUNC_RUN:
        MysteryGiftServer_Run(&gMgWork->server);
        gMgWork->stateTimer = 0;
        break;
    default:
        gMgWork->done = 1;
        break;
    }
}

void MgServer_Done(void)
{
}

u16 CalcCRC16WithTable(const u8 *data, u32 length)
{
    u16 i;
    u16 crc = 0x1121;
    u8 byte;

    for (i = 0; i < length; i++)
    {
        byte = crc >> 8;
        crc ^= data[i];
        crc = byte ^ sCrcTable[(u8)crc];
    }
    return ~crc;
}

struct SendRecvHeader
{
    u16 ident;
    u16 crc;
    u16 size;
};

bool8 MysteryGiftLink_Send(struct MysteryGiftLink *link)
{
    struct SendRecvHeader header;

    switch (link->state)
    {
    case 0:
        if (MgRfuLink_IsSendIdle())
        {
            header.ident = link->sendIdent;
            header.size = link->sendSize;
            header.crc = CalcCRC16WithTable(link->sendBuffer, link->sendSize);
            link->sendCRC = header.crc;
            link->sendCounter = 0;
            if (MgRfuLink_InitSend((u8 *)&header, sizeof(header)))
                link->state++;
        }
        break;
    case 1:
        if (MgRfuLink_IsSendIdle())
        {
            u32 blocksize = 252 * link->sendCounter;

            if (link->sendSize - blocksize <= 252)
            {
                if (MgRfuLink_InitSend((u8 *)link->sendBuffer + blocksize, link->sendSize - blocksize))
                {
                    link->sendCounter++;
                    link->state++;
                }
            }
            else
            {
                if (MgRfuLink_InitSend((u8 *)link->sendBuffer + blocksize, 252))
                    link->sendCounter++;
            }
        }
        break;
    case 2:
        if (MgRfuLink_IsSendIdle())
        {
            if (CalcCRC16WithTable(link->sendBuffer, link->sendSize) != link->sendCRC)
                gMgWork->done = 1;
            else
                link->state++;
        }
        break;
    case 3:
        link->state = 0;
        return TRUE;
    }
    return FALSE;
}

bool8 MysteryGiftLink_Recv(struct MysteryGiftLink *link)
{
    u8 buffer[0x100];
    u8 size;
    struct SendRecvHeader header;

    switch (link->state)
    {
    case 0:
        if (MgRfuLink_GetRecv(buffer, &size))
        {
            memcpy(&header, buffer, sizeof(header));
            link->recvSize = header.size;
            link->recvCRC = header.crc;
            if (link->recvSize > 0x400)
            {
                gMgWork->done = 1;
                return FALSE;
            }
            else if (link->recvIdent != header.ident)
            {
                gMgWork->done = 1;
                return FALSE;
            }
            else
            {
                link->recvCounter = 0;
                link->state++;
            }
        }
        break;
    case 1:
        if (MgRfuLink_GetRecv(buffer, &size))
        {
            u32 blocksize = link->recvCounter * 252;

            if (link->recvSize - blocksize <= 252)
            {
                memcpy((u8 *)link->recvBuffer + blocksize, buffer, link->recvSize - blocksize);
                link->recvCounter++;
                link->state++;
            }
            else
            {
                memcpy((u8 *)link->recvBuffer + blocksize, buffer, 252);
                link->recvCounter++;
            }
        }
        break;
    case 2:
        if (CalcCRC16WithTable(link->recvBuffer, link->recvSize) != link->recvCRC)
        {
            gMgWork->done = 1;
            return FALSE;
        }
        else
        {
            link->state++;
        }
        break;
    case 3:
        link->state = 0;
        return TRUE;
    }
    return FALSE;
}

void MysteryGiftLink_InitSend(struct MysteryGiftLink *link, u32 ident, const void *src, u32 size)
{
    link->state = 0;
    link->sendIdent = ident;
    link->sendCounter = 0;
    link->sendCRC = 0;
    if (size != 0)
        link->sendSize = size;
    else
        link->sendSize = 0x400;
    link->sendBuffer = src;
}

void MysteryGiftServer_Run(struct MysteryGiftServer *svr)
{
    const struct MysteryGiftServerCmd *cmd = &svr->script[svr->cmdidx];
    struct MysteryGiftLink *link = &svr->link;
    const void *ptr;

    svr->cmdidx++;
    switch (cmd->instr)
    {
    case SVR_RETURN:
        MG_ASSERT(cmd->ptr == NULL);
        svr->funcId = FUNC_DONE;
        svr->param = cmd->parameter;
        break;
    case SVR_SEND:
        svr->funcId = FUNC_SEND;
        break;
    case SVR_RECV:
        MG_ASSERT(cmd->ptr == NULL);
        link->state = 0;
        link->recvIdent = cmd->parameter;
        link->recvCounter = 0;
        link->recvCRC = 0;
        link->recvSize = 0;
        link->recvBuffer = svr->recvBuffer;
        svr->funcId = FUNC_RECV;
        break;
    case SVR_GOTO:
        MG_ASSERT(cmd->parameter == 0);
        svr->cmdidx = 0;
        svr->script = cmd->ptr;
        break;
    case SVR_GOTO_IF_EQ:
        if (svr->param == cmd->parameter)
        {
            svr->cmdidx = 0;
            svr->script = cmd->ptr;
        }
        break;
    case SVR_COPY_GAME_DATA:
        MG_ASSERT(cmd->parameter == 0);
        MG_ASSERT(cmd->ptr == NULL);
        memcpy(svr->linkGameData, svr->recvBuffer, sizeof(svr->linkGameData));
        break;
    case SVR_CHECK_GAME_DATA:
        MG_ASSERT(cmd->parameter == 0);
        MG_ASSERT(cmd->ptr == NULL);
        svr->param = MysteryGift_ValidateLinkGameData(svr->linkGameData);
        break;
    case SVR_CHECK_EXISTING_CARD:
        MG_ASSERT(cmd->parameter == 0);
        if (cmd->ptr != NULL)
            ptr = cmd->ptr;
        else
            ptr = svr->card;
        svr->param = MysteryGift_CompareCardFlags(ptr, svr->linkGameData);
        break;
    case SVR_READ_RESPONSE:
        MG_ASSERT(cmd->parameter == 0);
        MG_ASSERT(cmd->ptr == NULL);
        svr->param = *(u32 *)svr->recvBuffer;
        break;
    case SVR_CHECK_EXISTING_STAMPS:
        MG_ASSERT(cmd->parameter == 0);
        if (cmd->ptr != NULL)
            ptr = cmd->ptr;
        else
            ptr = &svr->stamp;
        svr->param = MysteryGift_CheckStamps(ptr, svr->linkGameData);
        break;
    case SVR_GET_CARD_STAT:
        MG_ASSERT(cmd->ptr == NULL);
        svr->param = MysteryGift_GetCardStat(svr->linkGameData, cmd->parameter);
        break;
    case SVR_CHECK_QUESTIONNAIRE:
        MG_ASSERT(cmd->parameter == 0);
        svr->param = MysteryGift_DoesQuestionnaireMatch(svr->linkGameData, cmd->ptr);
        break;
    case SVR_COMPARE:
        MG_ASSERT(cmd->parameter == 0);
        svr->param = MysteryGiftServer_Compare(cmd->ptr, *(void **)svr->recvBuffer);
        break;
    case SVR_LOAD_CARD:
        MG_ASSERT(cmd->parameter == 0);
        if (cmd->ptr != NULL)
            ptr = cmd->ptr;
        else
            ptr = svr->card;
        MysteryGiftLink_InitSend(link, MG_LINKID_CARD, ptr, sizeof(svr->card));
        break;
    case SVR_LOAD_NEWS:
        MG_ASSERT(cmd->parameter == 0);
        if (cmd->ptr != NULL)
            ptr = cmd->ptr;
        else
            ptr = svr->news;
        MysteryGiftLink_InitSend(link, MG_LINKID_NEWS, ptr, sizeof(svr->news));
        break;
    case SVR_LOAD_RAM_SCRIPT:
        if (cmd->ptr == NULL)
            MysteryGiftLink_InitSend(link, MG_LINKID_RAM_SCRIPT, svr->ramScript, svr->ramScriptSize);
        else
            MysteryGiftLink_InitSend(link, MG_LINKID_RAM_SCRIPT, cmd->ptr, cmd->parameter);
        break;
    case SVR_LOAD_STAMP:
        MG_ASSERT(cmd->parameter == 0);
        if (cmd->ptr != NULL)
            ptr = cmd->ptr;
        else
            ptr = &svr->stamp;
        MysteryGiftLink_InitSend(link, MG_LINKID_STAMP, ptr, sizeof(svr->stamp));
        break;
    case SVR_LOAD_UNK_2:
        MysteryGiftLink_InitSend(link, MG_LINKID_UNK_2, cmd->ptr, cmd->parameter);
        break;
    case SVR_LOAD_CLIENT_SCRIPT:
        if (cmd->ptr == NULL)
            MysteryGiftLink_InitSend(link, MG_LINKID_CLIENT_SCRIPT, svr->clientScript, svr->clientScriptSize);
        else
            MysteryGiftLink_InitSend(link, MG_LINKID_CLIENT_SCRIPT, cmd->ptr, cmd->parameter);
        break;
    case SVR_LOAD_EREADER_TRAINER:
        MG_ASSERT(cmd->parameter == 0);
        MysteryGiftLink_InitSend(link, MG_LINKID_EREADER_TRAINER, cmd->ptr, 0xBC);
        break;
    case SVR_LOAD_MSG:
        MysteryGiftLink_InitSend(link, MG_LINKID_DYNAMIC_MSG, cmd->ptr, cmd->parameter);
        break;
    case SVR_COPY_STAMP:
        MG_ASSERT(cmd->parameter == 0);
        svr->stamp = *(u32 *)cmd->ptr;
        break;
    case SVR_COPY_CARD:
        MG_ASSERT(cmd->parameter == 0);
        memcpy(svr->card, cmd->ptr, sizeof(svr->card));
        break;
    case SVR_COPY_NEWS:
        MG_ASSERT(cmd->parameter == 0);
        memcpy(svr->news, cmd->ptr, sizeof(svr->news));
        break;
    case SVR_SET_RAM_SCRIPT:
        svr->ramScript = cmd->ptr;
        svr->ramScriptSize = cmd->parameter;
        break;
    case SVR_SET_CLIENT_SCRIPT:
        svr->clientScript = cmd->ptr;
        svr->clientScriptSize = cmd->parameter;
        break;
    case SVR_LOAD_UNK_1:
        MysteryGiftLink_InitSend(link, MG_LINKID_UNK_1, cmd->ptr, cmd->parameter);
        break;
    case SVR_FIND_NEWS:
        svr->param = MysteryGift_FindEntry(svr->linkGameData);
        break;
    case SVR_COPY_FOUND_NEWS:
        memcpy(svr->news, &gMgWork->entries[gMgWork->entryIndex].data, sizeof(svr->news));
        break;
    case SVR_NOP:
        MysteryGiftServer_Unused(svr);
        break;
    default:
        MG_ASSERT(FALSE);
    }
}

void MysteryGiftServer_Unused(struct MysteryGiftServer *svr)
{
}

s8 MysteryGift_ValidateLinkGameData(const u8 *data)
{
    const struct MgLinkGameData *gameData = (const struct MgLinkGameData *)data;

    if (gameData->validationVar == 0x101
     && (gameData->validationGameVersion & 0xF)
     && (gameData->validationFlag1 & 1)
     && (gameData->validationFlag2 & 1)
     && (gameData->validationFlag3 & 1))
        return TRUE;
    return FALSE;
}

u32 MysteryGift_CompareCardFlags(const u8 *card, const u8 *data)
{
    u16 id = ((const struct MgLinkGameData *)data)->cardId;

    if (id == 0)
        return 0;
    if (*(u16 *)card == id)
        return 1;
    return 2;
}

u32 MysteryGift_CheckStamps(const u16 *stampData, const u8 *data)
{
    const struct MgLinkGameData *gameData = (const struct MgLinkGameData *)data;
    const struct WonderCardMetadata *meta = &gameData->cardMetadata;
    int count = 0;
    int matches = 0;
    int i;

    for (i = 0; i < gameData->maxStamps; i++)
    {
        if (meta->stampData[1][i] && meta->stampData[0][i])
        {
            count++;
            if ((meta->stampData[1][i] == stampData[1]) | (meta->stampData[0][i] == stampData[0]))
                matches++;
        }
    }
    count = gameData->maxStamps - count;
    if (count == 0)
        return 1;
    if (matches)
        return 3;
    if (count == 1)
        return 4;
    return 2;
}

u16 MysteryGift_GetCardStat(const u8 *data, u32 stat)
{
    const struct MgLinkGameData *gameData = (const struct MgLinkGameData *)data;
    int count;
    int i;

    switch (stat)
    {
    case 0:
        return gameData->cardMetadata.battlesWon;
    case 1:
        return gameData->cardMetadata.battlesLost;
    case 2:
        return gameData->cardMetadata.numTrades;
    case 3:
        count = 0;
        for (i = 0; i < gameData->maxStamps; i++)
        {
            if (gameData->cardMetadata.stampData[1][i] && gameData->cardMetadata.stampData[0][i])
                count++;
        }
        return count;
    case 4:
        return gameData->maxStamps;
    }
    gMgWork->done = 1;
    return 0;
}

s8 MysteryGift_DoesQuestionnaireMatch(const u8 *data, const u16 *words)
{
    const struct MgLinkGameData *gameData = (const struct MgLinkGameData *)data;
    int i;

    for (i = 0; i < 4; i++)
    {
        if (gameData->questionnaire[i] != words[i])
            return FALSE;
    }
    return TRUE;
}

u32 MysteryGiftServer_Compare(const void *a, const void *b)
{
    if (b < a)
        return 0;
    else if (b == a)
        return 1;
    else
        return 2;
}

s8 MysteryGift_FindEntry(const u8 *data)
{
    u8 noContent[8];
    int i;
    int index;
    bool32 found;

    memcpy(noContent, sNoContentHeader, sizeof(noContent));
    if (gMgWork->numEntries == 0)
        return FALSE;
    i = 0;
    found = FALSE;
    index = 0;
    for (; i < gMgWork->numEntries; i++)
    {
        if (MysteryGift_DoesQuestionnaireMatch(data, gMgWork->entries[i].header.questionnaire))
        {
            gMgWork->entryIndex = i;
            return TRUE;
        }
        if (*(s32 *)&gMgWork->entries[i].header.questionnaire[0] == -1 && *(s32 *)&gMgWork->entries[i].header.questionnaire[2] == -1)
        {
            index = i;
            found = TRUE;
        }
    }
    if (found)
    {
        gMgWork->entryIndex = index;
        return TRUE;
    }
    return FALSE;
}
