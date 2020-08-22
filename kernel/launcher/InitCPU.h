#pragma once
#include <MultBootUtil.h>

bool InitCPU();
//물리/가상 메모리 매니저를 초기화한다.
bool InitPhysicalMemorySystem(multiboot_info_t* pBootInfo);
bool InitVirtualMemorySystem(int maskPageCount);