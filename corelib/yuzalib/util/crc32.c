#include "crc32.h"

// Static storage for the crc-table
static uint32_t CrcTable[256] = { 0 };

/* Crc32GenerateTable
 * Generates a dynamic crc-32 table. */
void
Crc32GenerateTable(void)
{
    register uint32_t CrcAccumulator;
    register int i, j;

    for (i=0;  i < 256; i++) {
        CrcAccumulator = ((uint32_t) i << 24);
        for (j = 0;  j < 8;  j++) {
            if (CrcAccumulator & 0x80000000L) {
                CrcAccumulator = (CrcAccumulator << 1) ^ POLYNOMIAL;
            }
            else {
                CrcAccumulator = (CrcAccumulator << 1);
            }
        }
        CrcTable[i] = CrcAccumulator;
    }
}

/* Crc32Generate
 * Generates an crc-32 checksum from the given accumulator and
 * the given data. */
uint32_t
Crc32Generate(
    uint32_t CrcAccumulator, 
    uint8_t *DataPointer, 
    size_t DataSize)
{
    register size_t i, j;

    for (j = 0; j < DataSize; j++) {
        i = ((int) (CrcAccumulator >> 24) ^ *DataPointer++) & 0xFF;
        CrcAccumulator = (CrcAccumulator << 8) ^ CrcTable[i];
    }
    CrcAccumulator = ~CrcAccumulator;
    return CrcAccumulator;
}
