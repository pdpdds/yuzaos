// ------------------------------------------------------------------------------------------------
// intr/ioapic.h
// ------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

extern u8 *g_ioApicAddr;

void IoApicInit();
void IoApicSetEntry(u8 *base, u8 index, u64 data);

static inline void MmioWrite32(void* p, u32 data)
{
	*(volatile u32*)(p) = data;
}

static inline u32 MmioRead32(void* p)
{
	return *(volatile u32*)(p);
}
