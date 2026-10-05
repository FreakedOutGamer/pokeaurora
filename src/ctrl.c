#include "global.h"
#include "siirtc.h"
#include "aurora.h"
#include "rfu.h"
#include "link.h"
#include "misc.h"
#include "ctrl.h"

void StartController(const u8 *gameName, const u8 *userName)
{
    gCtrlWork = SetSlotCallback(ControllerTask);
    CpuFastFill(0, gCtrlWork, sizeof(struct CtrlWork));
    if (strcmp(gameName, sText_JspController) != 0 || strcmp(userName, sText_ChildTkd) != 0)
        gCtrlWork->done = 1;
    gCtrlWork->page = -1;
}

u16 ControllerTask(void *work)
{
    union RfuStatus status;

    gCtrlWork = work;
    status.raw.value = GetRfuSlotStatus();
    if (((union RfuStatus *)&status)->bits.sending && !gCtrlWork->done)
        return 0;
    Controller_Recv(status);
    Controller_Send(status);
    if (gCtrlWork->done)
        return 1;
    return 0;
}

void Controller_Recv(union RfuStatus status)
{
    if (status.bits.recvReady && !RfuRecvUNI((u8 *)&gCtrlWork->recvCmd))
    {
        gCtrlWork->lastCmd = gCtrlWork->recvCmd;
        Controller_HandleSetTime();
        Controller_HandleSetPage();
        Controller_HandleStats();
    }
}

void Controller_Send(union RfuStatus status)
{
    u16 err;

    if (status.bits.sending)
        return;
    gCtrlWork->cmd = gCtrlWork->lastCmd;
    if (gCtrlWork->lastCmd == CTRL_CMD_PAGE)
    {
        gCtrlWork->sendPage = gCtrlWork->page;
        gCtrlWork->sendSubpage = gCtrlWork->subpage;
        memcpy(gCtrlWork->send.data, gCtrlWork->pages[gCtrlWork->subpage], sizeof(gCtrlWork->send.data));
    }
    Controller_PrepareSend();
    err = RfuSendUNI((u8 *)gCtrlWork);
    if (err == 0)
    {
        if (gCtrlWork->cmd == CTRL_CMD_EXIT)
            gCtrlWork->done = 1;
        gCtrlWork->lastCmd = err;
        if (gCtrlWork->statsState == 2)
            gCtrlWork->sendCount++;
    }
}

void Controller_HandleSetTime(void)
{
    struct SiiRtcInfo rtc;

    if (gCtrlWork->lastCmd == CTRL_CMD_SET_TIME && gCtrlWork->recv.value != gCtrlWork->lastTime)
    {
        struct CtrlTime time = gCtrlWork->recv.time;

        gCtrlWork->lastTime = gCtrlWork->recv.value;
        rtc.year = IntToBcd(time.year + 3);
        rtc.month = IntToBcd(time.month);
        rtc.day = IntToBcd(time.day);
        rtc.dayOfWeek = time.dayOfWeek;
        rtc.hour = IntToBcd(time.hour);
        rtc.minute = IntToBcd(time.minute);
        rtc.second = IntToBcd(time.second);
        while (!SiiRtcSetDateTime(&rtc))
            ;
        UpdateHourly(TRUE);
    }
}

void Controller_HandleSetPage(void)
{
    if (gCtrlWork->lastCmd == CTRL_CMD_PAGE)
    {
        gCtrlWork->subpage = gCtrlWork->recvSubpage;
        if (gCtrlWork->recvPage != gCtrlWork->page)
            gCtrlWork->page = gCtrlWork->recvPage;
    }
}

void Controller_HandleStats(void)
{
    if (gCtrlWork->lastCmd == CTRL_CMD_STATS_START)
    {
        if (gCtrlWork->statsState != 1)
        {
            memset(&gCtrlWork->sendCount, 0, 0x10);
            memset(&gCtrlWork->lastValue, 0, 0x10);
            gCtrlWork->lastValue = -1;
            gCtrlWork->statsState = 1;
        }
    }
    else if (gCtrlWork->lastCmd == CTRL_CMD_STATS)
    {
        Controller_CountValue();
        gCtrlWork->remote[0] = gCtrlWork->recv.ptr[1];
        gCtrlWork->remote[1] = gCtrlWork->recv.ptr[2];
        gCtrlWork->remote[2] = gCtrlWork->recv.ptr[3];
    }
    else
    {
        gCtrlWork->statsState = 0;
    }
}

void Controller_CountValue(void)
{
    u32 diff = gCtrlWork->recv.value - gCtrlWork->lastValue;

    if (diff == 1)
        gCtrlWork->inOrder++;
    else if (diff > 1)
        gCtrlWork->skipped++;
    else
        gCtrlWork->repeated++;
    gCtrlWork->lastValue = gCtrlWork->recv.value;
}

void Controller_PrepareSend(void)
{
    struct CtrlStats stats;

    switch (gCtrlWork->statsState)
    {
    case 0:
        break;
    case 1:
        gCtrlWork->cmd = CTRL_CMD_STATS_START;
        gCtrlWork->statsState = 2;
        break;
    default:
        gCtrlWork->cmd = CTRL_CMD_STATS;
        stats.sendCount = gCtrlWork->sendCount;
        stats.inOrder = gCtrlWork->inOrder;
        stats.skipped = gCtrlWork->skipped;
        stats.repeated = gCtrlWork->repeated;
        gCtrlWork->send.stats = stats;
        break;
    }
}
