#include "global.h"
#include "aurora.h"
#include "content.h"

struct ContentSlot gContentSlots[4][16] = {0};
struct ContentData gContentData[4] = {0};

void UpdateCurrentNews(const u8 *date)
{
    int count = 0;
    int idx = -1;
    int i, j;

    if (IsDateOnOrAfter(date, sDefaultContentEntry.start) && IsDateOnOrBefore(date, sDefaultContentEntry.end))
    {
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
                gContentSlots[i][count].header.questionnaire[j] = sDefaultContentEntry.header.questionnaire[j];
            gContentSlots[i][count].data = &gContentData[i];
        }
        count++;
    }
    while (idx++, sContentEntries[idx].start[0] != 0xFF)
    {
        if (IsDateOnOrAfter(date, sContentEntries[idx].start) && IsDateOnOrBefore(date, sContentEntries[idx].end))
        {
            for (i = 0; i < 4; i++)
            {
                for (j = 0; j < 4; j++)
                    gContentSlots[i][count].header.questionnaire[j] = sContentEntries[idx].header.questionnaire[j];
                gContentSlots[i][count].data = &sContentEntries[idx].data;
            }
            count++;
            if (count == 15)
                break;
        }
    }
    for (i = 0; i < 4; i++)
        gContentSlots[i][count].data = NULL;
}

void GetCurrentSlotNews(struct ContentSlot **out)
{
    *out = gContentSlots[GetCurrentSlot()];
}

void BuildRankingNews(const u16 *gameName, const u8 *userName)
{
    u8 emptyRow[15];
    struct RankEntry top[5];
    struct RankQuery player;
    int rank, total;
    int slot;
    int i, n;

    memcpy(emptyRow, sText_EmptyRankingRow, sizeof(emptyRow));
    slot = GetCurrentSlot();
    {
        struct ContentData *base = gContentData;
        struct ContentData *dst = &base[slot];
        const struct ContentEntry *entry = &sDefaultContentEntry;

        memcpy(dst, &entry->data, sizeof(struct ContentData));
    }
    player.trainerId = gameName[1];
    memcpy(player.name, userName, 8);
    if (GetPlayerRank(&player, &rank, &total) == 0)
    {
        memcpy(gContentData[slot].text[0], sText_NoBattleRecord, sizeof(sText_NoBattleRecord));
    }
    else
    {
        SetNewsNumber3(0, 1, total);
        SetNewsNumber3(10, 1, rank);
    }
    GetTopRanking(top, 5);
    for (i = 0; i < 5; i++)
    {
        SetNewsName(4, i + 5, top[i].name);
        SetNewsNumber5(10, i + 5, top[i].id);
        SetNewsNumber3(16, i + 5, top[i].wins);
    }
    if (total <= 4)
    {
        n = 5 - total;
        for (i = 0; i < n; i++)
            memcpy(&gContentData[slot].text[9 - i][4], emptyRow, sizeof(emptyRow));
    }
}

void SetNewsNumber3(int x, int y, int value)
{
    char buf[8];
    int slot = GetCurrentSlot();
    int i;

    if (value > 999)
        value = 999;
    sprintf(buf, sFmt_3d, value);
    for (i = 0; i < 3; i++)
    {
        if (buf[i] == ' ')
            buf[i] = 0;
        else
            buf[i] += CHAR_0 - '0';
    }
    memcpy(&gContentData[slot].text[y][x], buf, 3);
}

void SetNewsNumber5(int x, int y, int value)
{
    char buf[8];
    int slot = GetCurrentSlot();
    int i;

    if (value > 99999)
        value = 99999;
    sprintf(buf, sFmt_05d, value);
    for (i = 0; i < 5; i++)
        buf[i] += CHAR_0 - '0';
    memcpy(&gContentData[slot].text[y][x], buf, 5);
}

void SetNewsName(int x, int y, const u8 *src)
{
    bool8 ended = FALSE;
    int slot = GetCurrentSlot();
    int i;

    for (i = 0; i < 5; i++)
    {
        if (src[i] == EOS)
            ended = TRUE;
        if (!ended)
            gContentData[slot].text[y][x + i] = src[i];
        else
            gContentData[slot].text[y][x + i] = 0;
    }
}

bool8 IsDateOnOrAfter(const u8 *date, const u8 *start)
{
    if (*(u32 *)start == 0)
        return TRUE;
    if (date[0] > start[0])
        return TRUE;
    if (date[0] < start[0])
        return FALSE;
    if (date[1] > start[1])
        return TRUE;
    if (date[1] < start[1])
        return FALSE;
    if (date[2] > start[2])
        return TRUE;
    if (date[2] < start[2])
        return FALSE;
    if (date[3] > start[3])
        return TRUE;
    if (date[3] < start[3])
        return FALSE;
    return TRUE;
}

bool8 IsDateOnOrBefore(const u8 *date, const u8 *end)
{
    s32 none = -1;

    if (*(s32 *)end == none)
        return TRUE;
    if (date[0] < end[0])
        return TRUE;
    if (date[0] > end[0])
        return FALSE;
    if (date[1] < end[1])
        return TRUE;
    if (date[1] > end[1])
        return FALSE;
    if (date[2] < end[2])
        return TRUE;
    if (date[2] > end[2])
        return FALSE;
    if (date[3] < end[3])
        return TRUE;
    if (date[3] > end[3])
        return FALSE;
    return TRUE;
}
