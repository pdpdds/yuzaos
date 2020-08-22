#pragma once
#include <MultbootUtil.h>

bool ValidatePEImage(void* image);
bool FindModuleEntry(const char* szFileName, char* buf, KernelInfo* pInfo);