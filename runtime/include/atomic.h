#pragma once
#include "windef.h"
#include <winnt.h>

#ifdef  __cplusplus
extern "C" {
#endif

	int AtomicAdd(volatile int *var, int val);
	int AtomicAnd(volatile int *var, int val);
	int AtomicOr(volatile int *var, int val);

	LONG AtomicInterlockedAdd(LONG volatile *Addend, LONG Value);
	LONG AtomicInterlockedAnd(LONG volatile *Destination, LONG Value);
	LONG AtomicInterlockedCompareExchange(LONG volatile *Destination, LONG ExChange, LONG Comperand);
	LONG AtomicInterlockedDecrement(LONG volatile *Addend);
	LONG AtomicInterlockedIncrement(LONG volatile *Addend);
	LONG AtomicInterlockedOr(LONG volatile *Destination, LONG Value);
	LONG AtomicInterlockedXor(LONG volatile *Destination, LONG Value);
	LONG AtomicInterlockedExchange(LPLONG volatile Target, LONG Value);

	//Interlocked...
	extern LONG _InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand);
	extern LONG _InterlockedIncrement(LONG volatile* Addend);
	extern LONG _InterlockedDecrement(LONG volatile* Addend);
	extern LONG _InterlockedExchange(LONG volatile* Target, LONG Value);
	extern PVOID _InterlockedExchangePointer(PVOID volatile* Target, PVOID Value);
	extern PVOID _InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID Exchange, PVOID Comperand);
	extern LONG _InterlockedExchangeAdd(LONG volatile* Addend, LONG Value);
	extern LONG _InterlockedAnd(LONG volatile* Destination, LONG Value);
	extern LONG _InterlockedOr(LONG volatile* Destination, LONG Value);
	extern LONG _InterlockedXor(LONG volatile* Destination, LONG Value);

	extern void _ReadWriteBarrier(void);

	extern unsigned char _BitScanReverse(unsigned long * _Index, unsigned long _Mask);
	extern unsigned char _BitScanForward(unsigned long * _Index, unsigned long _Mask);

#ifdef  __cplusplus
}


#endif

