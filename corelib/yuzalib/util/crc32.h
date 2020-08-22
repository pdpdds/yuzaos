#ifndef __CRC32_H__
#define __CRC32_H__

/* Includes
 * - Library */
#include "windef.h"
#include "stdint.h"

// Standard CRC-32 polynomial
#define POLYNOMIAL              0x04c11db7L

/* Crc32GenerateTable
 * Generates a dynamic crc-32 table. */

void Crc32GenerateTable(void);

/* Crc32Generate
 * Generates an crc-32 checksum from the given accumulator and
 * the given data. */

uint32_t Crc32Generate(
    uint32_t CrcAccumulator, 
    uint8_t *DataPointer, 
    size_t DataSize);

#endif //!__MCORE_CRC32_H__
