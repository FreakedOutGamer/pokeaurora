#include "global.h"
#include "aurora.h"
#include "link.h"

// The EWRAM variables are initialized (to zero), so they are in .data, which
// AgbMain copies from the ROM to EWRAM.
union SaveBuffer gSaveBuffer = {0};

void InitSave(void)
{
    memset(sSectorDirty, 0, sizeof(sSectorDirty));
    sSaveState = 0;
    SetCurrentSlot(-1);
    if (LoadSaveHeader())
        LoadSaveSectors();
}

bool8 LoadSaveHeader(void)
{
    int i;
    struct SaveHeader *header = &gSaveBuffer.header;

    for (i = 0; i < 2; i++)
    {
        ReadFlash(i * 16, 0, &gSaveBuffer, SECTOR_SIZE);
        if (memcmp(header, &sSaveMagic, 4) == 0
         && CalcSectorChecksum(0) == header->checksum)
            return TRUE;
    }
    ClearSaveHeader();
    return FALSE;
}

void ClearSaveHeader(void)
{
    struct SaveHeader *header = &gSaveBuffer.header;

    DmaFill32(3, 0, header, SECTOR_SIZE);
    header->magic = sSaveMagic;
    header->terminalId = 1;
}

void LoadSaveSectors(void)
{
    int i, j;
    struct SaveHeader *header = &gSaveBuffer.header;

    for (i = 1; i < 16; i++)
    {
        for (j = 0; j < 2; j++)
        {
            int sector = i + ((header->bank[i] ^ j) & 1) * 16;

            ReadFlash(sector, 0, gSaveBuffer.sectors[i], SECTOR_SIZE);
            if (CalcSectorChecksum(i) == header->checksums[sector])
            {
                header->bank[i] = (j ^ header->bank[i]) & 1;
                break;
            }
            if (j == 1)
            {
                DmaFill32(3, 0, gSaveBuffer.sectors[i], SECTOR_SIZE);
                header->bank[i] = 0;
            }
        }
    }
}

u32 CalcSectorChecksum(int sector)
{
    int n = SECTOR_SIZE / 4;
    u32 sum = 0;
    int i;

    if (sector == 0)
        n--;
    for (i = 0; i < n; i++)
        sum += ((u32 *)gSaveBuffer.sectors[sector])[i];
    return sum;
}

s8 SaveStep(void)
{
    int i;
    int sector;
    struct SaveHeader *header = &gSaveBuffer.header;

    if (sSaveState == 0)
    {
        for (i = 0; i < 16; i++)
        {
            if (sSectorDirty[i])
                break;
            if (i == 15)
                return FALSE;
        }
    }
    if (sSaveState == 0)
    {
        for (i = 1; i < 16; i++)
        {
            if (sSectorDirty[i])
            {
                header->bank[i] ^= 1;
                sector = i + header->bank[i] * 16;
                header->checksums[sector] = CalcSectorChecksum(i);
            }
        }
        header->checksum = CalcSectorChecksum(0);
        ProgramFlashSectorAndVerify(0, &gSaveBuffer);
        sSectorDirty[0] = 0;
        sSaveState = 1;
        return TRUE;
    }
    for (i = 1; i < 16; i++)
    {
        if (sSectorDirty[i])
        {
            sector = i + header->bank[i] * 16;
            ProgramFlashSectorAndVerify(sector, gSaveBuffer.sectors[i]);
            sSectorDirty[i] = 0;
            return TRUE;
        }
    }
    ProgramFlashSectorAndVerify(16, &gSaveBuffer);
    sSaveState = 0;
    return FALSE;
}
void SaveAll(void)
{
    int i, j;
    struct SaveHeader *header = &gSaveBuffer.header;
    u8 *buffer;

    for (i = 0; i < 16; i++)
    {
        if (sSectorDirty[i])
            break;
        if (i == 15)
            return;
    }
    for (i = 1; i < 16; i++)
    {
        if (sSectorDirty[i])
        {
            u32 checksum;

            header->bank[i] = 0;
            checksum = CalcSectorChecksum(i);
            for (j = 0; j < 2; j++)
            {
                header->checksums[i + j * 16] = checksum;
                ProgramFlashSectorAndVerify(i + j * 16, gSaveBuffer.sectors[i]);
            }
            sSectorDirty[i] = 0;
        }
    }
    header->checksum = CalcSectorChecksum(0);
    ProgramFlashSectorAndVerify(0, (buffer = (u8 *)&gSaveBuffer));
    ProgramFlashSectorAndVerify(16, buffer);
    sSectorDirty[0] = 0;
}

bool8 IsSaving(void)
{
    int i;

    if (sSaveState)
        return TRUE;
    for (i = 0; i < 16; i++)
    {
        if (sSectorDirty[i])
            return TRUE;
    }
    return FALSE;
}

s8 GetSaveState(void)
{
    return sSaveState;
}

u32 GetSaveUnk94(void)
{
    return gSaveBuffer.header.unk94;
}

void IncrementSaveCounter(int i)
{
    gSaveBuffer.header.counters[i]++;
    sSectorDirty[0] = 1;
}

void IncrementDistributionCount(void)
{
    gSaveBuffer.header.distributionCount++;
    sSectorDirty[0] = 1;
}

void *GetSaveDataStart(void)
{
    return gSaveBuffer.sectors[1];
}

u32 GetTerminalId(void)
{
    return gSaveBuffer.header.terminalId;
}

void SetTerminalId(u16 value)
{
    gSaveBuffer.header.terminalId = value;
    sSectorDirty[0] = 1;
    SaveAll();
}

bool32 ReadSaveBlock(u32 offset, void *dest, u32 size)
{
    u8 *block;

    if (IsSaveBlockRangeValid(offset, size) && (block = (u8 *)GetSaveBlock() + offset) != NULL)
    {
        memcpy(dest, block, size);
        return FALSE;
    }
    return TRUE;
}

bool32 WriteSaveBlock(u32 offset, const void *src, u32 size)
{
    u8 *block;
    u32 start;
    int i, last;

    if (sSaveState || !IsSaveBlockRangeValid(offset, size) || (block = (u8 *)GetSaveBlock() + offset) == NULL)
        return TRUE;
    memcpy(block, src, size);
    start = block - (u8 *)&gSaveBuffer;
    last = (start + size - 1) / SECTOR_SIZE;
    for (i = start / SECTOR_SIZE; i <= last; i++)
        sSectorDirty[i] = 1;
    return FALSE;
}

bool32 ClearSaveBlock(void)
{
    u8 *block = GetSaveBlock();
    u32 id = GetCurrentSlotSerialNo();
    u32 size;
    u32 offset;
    int i, last;

    if (sSaveState)
        return TRUE;
    for (i = 0; i < 16; i++)
    {
        if (id == sSaveBlockIds[i])
        {
            size = sSaveBlockSizes[i];
            break;
        }
    }
    memset(block, 0, size);
    offset = block - (u8 *)&gSaveBuffer;
    last = (offset + size - 1) / SECTOR_SIZE;
    for (i = offset / SECTOR_SIZE; i <= last; i++)
        sSectorDirty[i] = 1;
    return FALSE;
}

bool8 IsSaveBlockRangeValid(u32 offset, u32 size)
{
    u32 id = GetCurrentSlotSerialNo();
    int i;

    for (i = 0; i < 16; i++)
    {
        if (id == sSaveBlockIds[i] && offset + size <= sSaveBlockSizes[i])
            return TRUE;
    }
    return FALSE;
}

void *GetSaveBlock(void)
{
    u32 *block = (u32 *)gSaveBuffer.sectors[1];
    u32 id = GetCurrentSlotSerialNo();
    int i;

    for (i = 0; i < 16; i++)
    {
        if (id == sSaveBlockIds[i])
            return block;
        block += sSaveBlockSizes[i] / 4;
    }
    return NULL;
}
