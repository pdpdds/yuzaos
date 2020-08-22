#pragma once
#include "MultbootUtil.h"

bool Boot64BitMode(multiboot_info_t* pBootInfo, char* szKernelName);
bool Is64BitSwitchPossible();
bool DetectionCPUID();
bool IsLongModeCheckPossible();
bool IsLongModePossible();