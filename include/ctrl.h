#ifndef GUARD_CTRL_H
#define GUARD_CTRL_H

// Talks to a PC-side "JSPCONTROLLER" over the wireless adapter: it sets the
// clock, picks the data pages to show and reports link statistics.

struct CtrlTime
{
    u32 dayOfWeek:3;
    u32 second:6;
    u32 minute:6;
    u32 hour:5;
    u32 day:5;
    u32 month:4;
    u32 year:3;
};

struct CtrlStats
{
    u32 sendCount;
    u32 inOrder;
    u32 skipped;
    u32 repeated;
};

// The PC controller's commands
#define CTRL_CMD_EXIT        0x1111
#define CTRL_CMD_SET_TIME    0x2222
#define CTRL_CMD_PAGE        0x3333
#define CTRL_CMD_STATS       0x4444
#define CTRL_CMD_STATS_START 0x5555

union RfuStatus
{
    struct
    {
        u32 value:16;
    } raw;
    struct
    {
        u16 unused:13;
        u16 recvNIDone:1;
        u16 sending:1;
        u16 recvReady:1;
    } bits;
};

struct CtrlWork
{
    /*0x0000*/ u16 cmd;
    /*0x0002*/ u8 sendPage;
    /*0x0003*/ u8 sendSubpage;
    /*0x0004*/ union
    {
        u8 data[0x40];
        struct CtrlStats stats;
    } send;
    /*0x0044*/ u8 filler44[4];
    /*0x0048*/ u16 recvCmd;
    /*0x004A*/ u8 recvPage;
    /*0x004B*/ u8 recvSubpage;
    /*0x004C*/ union
    {
        struct CtrlTime time;
        u32 value;
        u32 *ptr;
    } recv;
    /*0x0050*/ u8 filler50[8];
    /*0x0058*/ u32 sendCount;
    /*0x005C*/ u32 remote[3];
    /*0x0068*/ u32 lastValue;
    /*0x006C*/ u32 inOrder;
    /*0x0070*/ u32 skipped;
    /*0x0074*/ u32 repeated;
    /*0x0078*/ u8 pages[64][0x40];
    /*0x1078*/ u32 lastTime;
    /*0x107C*/ u16 lastCmd;
    /*0x107E*/ s8 page;
    /*0x107F*/ s8 subpage;
    /*0x1080*/ s8 statsState;
    /*0x1081*/ s8 done;
};

extern struct CtrlWork *gCtrlWork;
extern const char sText_JspController[];
extern const char sText_ChildTkd[];

void StartController(const u8 *gameName, const u8 *userName);
u16 ControllerTask(void *work);
void Controller_Recv(union RfuStatus status);
void Controller_Send(union RfuStatus status);
void Controller_HandleSetTime(void);
void Controller_HandleSetPage(void);
void Controller_HandleStats(void);
void Controller_CountValue(void);
void Controller_PrepareSend(void);

#endif // GUARD_CTRL_H
