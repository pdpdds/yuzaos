#pragma once
#include "MultbootUtil.h"

bool ValidatePEImage64(void* image);
uint32_t FindKernel64Entry(const char* szFileName, char* buf, uint32_t& imageBase);
bool IsKernel64(multiboot_info_t* pInfo, const char* szFileName);