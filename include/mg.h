#ifndef GUARD_MG_H
#define GUARD_MG_H

#include "content.h"

// Server script commands (MysteryGiftServer_Run), as pokeemerald's. The data's
// scripts use the same values (asm/macros/mg.inc).
#define SVR_RETURN                0
#define SVR_SEND                  1
#define SVR_RECV                  2
#define SVR_GOTO                  3
#define SVR_GOTO_IF_EQ            4
#define SVR_COPY_GAME_DATA        5
#define SVR_CHECK_GAME_DATA       6
#define SVR_CHECK_EXISTING_CARD   7
#define SVR_READ_RESPONSE         8
#define SVR_CHECK_EXISTING_STAMPS 9
#define SVR_GET_CARD_STAT         10
#define SVR_CHECK_QUESTIONNAIRE   11
#define SVR_COMPARE               12
#define SVR_LOAD_CARD             13
#define SVR_LOAD_NEWS             14
#define SVR_LOAD_RAM_SCRIPT       15
#define SVR_LOAD_STAMP            16
#define SVR_LOAD_UNK_2            17
#define SVR_LOAD_CLIENT_SCRIPT    18
#define SVR_LOAD_EREADER_TRAINER  19
#define SVR_LOAD_MSG              20
#define SVR_COPY_STAMP            21
#define SVR_COPY_CARD             22
#define SVR_COPY_NEWS             23
#define SVR_SET_RAM_SCRIPT        24
#define SVR_SET_CLIENT_SCRIPT     25
// This cartridge's own
#define SVR_LOAD_UNK_1            29
#define SVR_FIND_NEWS             30
#define SVR_COPY_FOUND_NEWS       31
#define SVR_NOP                   32

// What a link transfer carries
#define MG_LINKID_CLIENT_SCRIPT   0x10
#define MG_LINKID_GAME_DATA       0x11
#define MG_LINKID_GAME_STAT       0x12
#define MG_LINKID_RESPONSE        0x13
#define MG_LINKID_READY_END       0x14
#define MG_LINKID_DYNAMIC_MSG     0x15
#define MG_LINKID_CARD            0x16
#define MG_LINKID_NEWS            0x17
#define MG_LINKID_STAMP           0x18
#define MG_LINKID_RAM_SCRIPT      0x19
#define MG_LINKID_EREADER_TRAINER 0x1A
#define MG_LINKID_UNK_1           0x1B
#define MG_LINKID_UNK_2           0x1C

// The link commands in the first bytes of each packet (the high bytes of
// pokeemerald's RFUCMD_*)
#define RFUCMD_SEND_BLOCK_INIT    0x88
#define RFUCMD_SEND_BLOCK         0x89
#define RFUCMD_READY_CLOSE_LINK   0x5F
#define RFUCMD_READY_EXIT_STANDBY 0x66
#define RFUCMD_SEND_PLAYER_IDS    0x77
#define RFUCMD_SEND_BLOCK_REQ     0xA1

// What the server is doing (MgServer_CallFunc)
enum
{
    FUNC_INIT,
    FUNC_DONE,
    FUNC_RECV,
    FUNC_SEND,
    FUNC_RUN,
};

// The wireless transport for one Mystery Gift client: NI handshake, then
// 14-byte packets carried in 70-byte UNI frames.
struct MgRfuLink
{
    /*0x000*/ s32 timer;
    /*0x004*/ u8 recvBuffer[0x200];
    /*0x204*/ u8 recvData[0x200];
    /*0x404*/ u8 sendData[0x200];
    /*0x604*/ u8 sendCounter;
    /*0x605*/ u8 recvState;
    /*0x606*/ u8 lastHeader;
    /*0x607*/ u8 dupCount;
    /*0x608*/ u8 recvCount;
    /*0x609*/ u8 filler_609[3];
    /*0x60C*/ u32 recvMask;
    /*0x610*/ u8 recvDelay;
    /*0x611*/ u8 sendState;
    /*0x612*/ u8 sendCount;
    /*0x613*/ u8 sendIndex;
    /*0x614*/ u8 recvPacket[14];
    /*0x622*/ u8 sendPacket[14];
    /*0x630*/ u8 echoPacket[14];
    /*0x63E*/ u8 filler_63E[42];
    /*0x668*/ u8 recvLen;
    /*0x669*/ u8 connected;
};

struct MysteryGiftLink
{
    s32 state;
    u8 sendPlayerId;
    u8 recvPlayerId;
    u16 recvIdent;
    u16 recvCounter;
    u16 recvCRC;
    u16 recvSize;
    u16 sendIdent;
    u16 sendCounter;
    u16 sendCRC;
    u16 sendSize;
    void *recvBuffer;
    const void *sendBuffer;
};

struct MysteryGiftServerCmd
{
    u32 instr;
    u32 parameter;
    const void *ptr;
};

struct MysteryGiftServer
{
    /*0x000*/ u32 unused;
    /*0x004*/ u32 param;
    /*0x008*/ u32 funcId;
    /*0x00C*/ u32 cmdidx;
    /*0x010*/ const struct MysteryGiftServerCmd *script;
    /*0x014*/ u8 recvBuffer[0x400];
    /*0x414*/ u8 card[0x14C];
    /*0x560*/ u8 news[0x1BC];
    /*0x71C*/ u8 linkGameData[0x64];
    /*0x780*/ const void *ramScript;
    /*0x784*/ u32 ramScriptSize;
    /*0x788*/ const void *clientScript;
    /*0x78C*/ u32 clientScriptSize;
    /*0x790*/ u32 stamp;
    /*0x794*/ struct MysteryGiftLink link;
};

struct MgHeader
{
    u8 data[15];
};

struct MgInfoBody
{
    /*0x00*/ u16 unk0;
    /*0x02*/ u16 unk2;
    /*0x04*/ u32 number;
    /*0x08*/ u8 userName[8];
    /*0x10*/ u8 unk10;
    /*0x11*/ u8 filler11[2];
    /*0x13*/ u8 unk13;
    /*0x14*/ u32 unk14;
    /*0x18*/ u16 unk18;
    /*0x1A*/ u16 unk1A;
};

struct MgServerInfo
{
    /*0x00*/ struct MgHeader header1;
    /*0x10*/ struct MgInfoBody body;
    /*0x2C*/ struct MgHeader header2;
};

struct WonderCardMetadata
{
    u16 battlesWon;
    u16 battlesLost;
    u16 numTrades;
    u16 iconSpecies;
    u16 stampData[2][7];
};

struct MgLinkGameData
{
    /*0x00*/ u32 validationVar;
    /*0x04*/ u16 validationFlag1;
    /*0x06*/ u16 filler6;
    /*0x08*/ u32 validationFlag2;
    /*0x0C*/ u16 validationFlag3;
    /*0x0E*/ u16 fillerE;
    /*0x10*/ u32 validationGameVersion;
    /*0x14*/ u16 cardId;
    /*0x16*/ u16 questionnaire[4];
    /*0x1E*/ u16 filler1E;
    /*0x20*/ struct WonderCardMetadata cardMetadata;
    /*0x44*/ u8 maxStamps;
    /*0x45*/ u8 playerName[8];
    /*0x4D*/ u8 playerTrainerId[4];
    /*0x52*/ u16 easyChatProfile[6];
    /*0x5E*/ u8 romHeaderGameCode[4];
    /*0x62*/ u8 romHeaderSoftwareVersion;
};

struct MgEntry
{
    struct ContentHeader header;
    struct ContentData data;
};

struct MgScriptInfo
{
    u32 unk0;
    u32 unk4;
    const struct MysteryGiftServerCmd *script;
    const u8 *name;
};

struct MgWork
{
    /*0x0000*/ u8 done;
    /*0x0001*/ u8 gameName[13];
    /*0x000E*/ u8 userName[8];
    /*0x0016*/ u8 type;
    /*0x0018*/ s32 timer;
    /*0x001C*/ u16 status;
    /*0x0020*/ struct MgRfuLink link;
    /*0x068C*/ void (*state)(void);
    /*0x0690*/ u8 substate;
    /*0x0694*/ s32 stateTimer;
    /*0x0698*/ u8 buffer[0xC8];
    /*0x0760*/ u8 bufferLen;
    /*0x0761*/ u8 filler_761[0x337];
    /*0x0A98*/ u8 serverInfo[0x3C];
    /*0x0AD4*/ u8 clientInfo[0x3C];
    /*0x0B10*/ const struct MysteryGiftServerCmd *script;
    /*0x0B14*/ u16 entryIndex;
    /*0x0B18*/ struct MysteryGiftServer server;
    /*0x12CC*/ u8 filler_12CC[8];
    /*0x12D4*/ u16 numEntries;
    /*0x12D8*/ struct MgEntry *entries;
    /*0x12DC*/ struct MgEntry entryBuffer[1];
};

extern struct MgWork *gMgWork;
extern const struct MgScriptInfo sMgScripts[];
extern const struct MgHeader sMgServerInfoHeader;
extern const u16 sCrcTable[256];
extern const u8 sNoContentHeader[8];

void StartMgServer(const u8 *gameName, const u8 *userName, struct ContentSlot *slots, u8 type);
void SetMgState(void (*func)(void));
u16 MgServerTask(void *work);
void MgRfuLink_Init(void);
void MgRfuLink_Update(void);
bool8 MgRfuLink_InitSend(const u8 *data, u8 size);
bool8 MgRfuLink_SendCommand(u8 cmd);
bool8 MgRfuLink_IsSendIdle(void);
bool8 MgRfuLink_GetRecv(u8 *dest, u8 *size);
bool8 MgRfuLink_Connect(void);
bool8 MgRfuLink_HasRecv(void);
bool8 MgRfuLink_IsNotSending(void);
void MgRfuLink_Recv(void);
void MgRfuLink_Send(void);
void MgState_Handshake(void);
void MgState_Server(void);
void MgNextSubstate(void);
void MgInitServerInfo(u8 *info);
void MgServer_Init(void);
void MgServer_CallFunc(void);
void MgServer_Done(void);
u16 CalcCRC16WithTable(const u8 *data, u32 length);
bool8 MysteryGiftLink_Send(struct MysteryGiftLink *link);
bool8 MysteryGiftLink_Recv(struct MysteryGiftLink *link);
void MysteryGiftLink_InitSend(struct MysteryGiftLink *link, u32 ident, const void *src, u32 size);
void MysteryGiftServer_Run(struct MysteryGiftServer *svr);
void MysteryGiftServer_Unused(struct MysteryGiftServer *svr);
s8 MysteryGift_ValidateLinkGameData(const u8 *data);
u32 MysteryGift_CompareCardFlags(const u8 *card, const u8 *data);
u32 MysteryGift_CheckStamps(const u16 *stamp, const u8 *data);
u16 MysteryGift_GetCardStat(const u8 *data, u32 stat);
s8 MysteryGift_DoesQuestionnaireMatch(const u8 *data, const u16 *words);
u32 MysteryGiftServer_Compare(const void *a, const void *b);
s8 MysteryGift_FindEntry(const u8 *data);

#endif // GUARD_MG_H
