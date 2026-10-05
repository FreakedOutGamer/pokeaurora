#ifndef GUARD_CONTENT_H
#define GUARD_CONTENT_H

// The Gen 3 character set (charmap.txt)
#define CHAR_SPACE 0x00
#define CHAR_0     0xA1
#define EOS        0xFF

// WONDER NEWS, as pokeemerald's struct WonderNews
struct ContentData
{
    u16 id;
    u8 sendType;
    u8 bgType;
    u8 title[40];
    u8 text[10][40];
};

// Which games get the news: the questionnaire answers they must have given.
// 0xFFFF matches anything.
struct ContentHeader
{
    u16 questionnaire[4];
};

struct ContentEntry
{
    u8 start[4];
    u8 end[4];
    struct ContentHeader header;
    struct ContentData data;
};

struct ContentSlot
{
    struct ContentHeader header;
    const struct ContentData *data;
};

// The ranking the default news shows. The ranking functions are stubs, so the
// news says the player has no battle record.
struct RankQuery
{
    u16 trainerId;
    u8 name[8];
};

struct RankEntry
{
    u16 id;
    u8 name[10];
    u16 wins;
    u16 unused;
};

extern struct ContentSlot gContentSlots[4][16];
extern struct ContentData gContentData[4];
extern const struct ContentEntry sContentEntries[];
extern const struct ContentEntry sDefaultContentEntry;
extern const u8 sText_EmptyRankingRow[15];
extern const u8 sText_NoBattleRecord[2][40];
extern const char sFmt_3d[];
extern const char sFmt_05d[];

bool8 IsDateOnOrAfter(const u8 *date, const u8 *start);
bool8 IsDateOnOrBefore(const u8 *date, const u8 *end);

void UpdateCurrentNews(const u8 *date);
void GetCurrentSlotNews(struct ContentSlot **out);
void BuildRankingNews(const u16 *gameName, const u8 *userName);
void SetNewsNumber3(int x, int y, int value);
void SetNewsNumber5(int x, int y, int value);
void SetNewsName(int x, int y, const u8 *src);
bool32 GetPlayerRank(const struct RankQuery *player, int *rank, int *total);
void GetTopRanking(struct RankEntry *entries, int count);

#endif // GUARD_CONTENT_H
