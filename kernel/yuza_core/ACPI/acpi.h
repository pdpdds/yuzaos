// ------------------------------------------------------------------------------------------------
// acpi/acpi.h
// ------------------------------------------------------------------------------------------------

#pragma once

#include <windef.h>
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define MAX_CPU_COUNT 16

extern uint g_acpiCpuCount;
extern u8 g_acpiCpuIds[MAX_CPU_COUNT];

void AcpiInit();
uint AcpiRemapIrq(uint irq);
