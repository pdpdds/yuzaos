#pragma once
#include "stdint.h"

#ifdef  __cplusplus
extern "C" {
#endif

uint16_t swap_uint16(uint16_t val);

//! Byte swap short
int16_t swap_int16(int16_t val);

//! Byte swap unsigned int
uint32_t swap_uint32(uint32_t val);

//! Byte swap int
int32_t swap_int32(int32_t val);

int64_t swap_int64(int64_t val);

uint64_t swap_uint64(uint64_t val);

#ifdef  __cplusplus
}
#endif

