#pragma once
#include "windef.h"
#include <BuildOption.h>

#if SKY_EMULATOR && !defined(SKY_LAUNCHER)
//#include "../yuza_core/SystemAPI.h"
#include <platformapi.h>
extern PlatformAPI g_platformAPI;
extern CRITICAL_SECTION g_interrupt_cs;
#endif

#ifdef __cplusplus
void *operator new(size_t size);
void *operator new[](size_t size);
void *operator new(size_t, void *p);
void *operator new[](size_t, void *p);

void operator delete(void *p);
void operator delete(void *p, size_t size);
void operator delete[](void *p);
void operator delete[](void *p, size_t size);
#endif

int __cdecl _purecall();

#ifdef  __cplusplus
extern "C" {
#endif
	void OutPortByte(ushort port, uchar value);
	void OutPortWord(ushort port, ushort value);
	void OutPortDWord(ushort port, unsigned int value);
	uchar InPortByte(ushort port);
	ushort InPortWord(ushort port);
	long InPortDWord(unsigned int port);

	int DisableInterrupts();
	void RestoreInterrupts(const int flags);
	void EnableInterrupts();

	void EnableIrq(int);
	void DisableIrq(int);
	
	extern unsigned char _BitScanReverse(unsigned long * _Index, unsigned long _Mask);
	extern unsigned char _BitScanForward(unsigned long * _Index, unsigned long _Mask);

#ifdef  __cplusplus
}


#endif

