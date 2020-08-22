#pragma once
#include "windef.h"

#ifdef _MSC_VER
#define interrupt __declspec (naked)
#else
#define interrupt
#endif

#define far
#define near

#pragma pack (push, 1)
typedef struct registers
{
	u32int ds, es, fs, gs;                  // 데이터 세그먼트 셀렉터
	u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // PUSHAD
	u32int eip, cs, eflags, useresp, ss; //프로세스에 의해 자동적으로 푸쉬되는 데이터
} registers_t;
#pragma pack (pop)
