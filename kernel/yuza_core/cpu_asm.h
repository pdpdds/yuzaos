// 
// Copyright 1998-2012 Jeff Bush
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef _CPU_ASM_H
#define _CPU_ASM_H

#include "ktypes.h"
#include "x86.h"
#include <BuildOption.h>
#include <memory.h>

void read_tsc(int *cl_Low, int *cl_High);

typedef int (*CallHook)(...);

/*
inline void write_io_8(int value, int port)
{
	asm volatile("outb %%al, %%dx" : : "a" (value), "d" (port));
}

inline void write_io_16(int value, int port)
{
	asm volatile("outw %%ax, %%dx" : : "a" (value), "d" (port));
}

inline void write_io_32(int value, int port)
{
	asm volatile("outl %%eax, %%dx" : : "a" (value), "d" (port));
}

inline unsigned int read_io_8(unsigned int port)
{
	unsigned char retval;
	asm volatile("inb %%dx, %%al" : "=a" (retval) : "d" (port));
	return retval;
}

inline unsigned int read_io_16(unsigned int port)
{
	unsigned short retval;
	asm volatile("inw %%dx, %%ax" : "=a" (retval) : "d" (port));
	return retval;
}

inline unsigned int read_io_32(unsigned int port)
{
	unsigned int retval;
	asm volatile("inl %%dx, %%eax" : "=a" (retval) : "d" (port));
	return retval;
}


inline void LoadIdt(const IdtEntry base[], unsigned int limit)
{
	struct desc {
		unsigned short limit;
		const IdtEntry *base;
	} PACKED;
	desc d;
	d.base = base;
	d.limit = limit;
	asm("lidt (%0)" : : "r" (&d));
}

inline void LoadGdt(const GdtEntry base[], unsigned int limit)
{
	struct desc {
		unsigned short limit;
		const GdtEntry *base;
	} PACKED;
	desc d;
	d.base = base;
	d.limit = limit;
	asm("lgdt (%0);"
		"movw $0x10, %%ax;"
		"movw %%ax, %%ds;"
		"movw %%ax, %%es;"
		"movw %%ax, %%gs;"
		"movw %%ax, %%fs;"
		"movw %%ax, %%ss;"
		"movw $0x28, %%ax;"
		"ltr %%ax;"
		: : "r" (&d) : "eax");
}


	
	*/
extern "C" {
	unsigned int GetDR6();
	bigtime_t SystemTime();
	int InvokeSystemCall(const CallHook *func, int stackData[], int stackSize);
	int CopyUserInternal(void *dest, const void *src, unsigned int size, unsigned int *handler);
	void ContextSwitch(unsigned int *oldEsp, unsigned int newEsp, unsigned int pdbr);
	void SwitchToUserMode(unsigned int _start, unsigned int user_stack) NORETURN;

	void write_io_str_16(int port, short buf[], int count);
	void read_io_str_16(int port, short buf[], int count);
	//void SetWatchpoint(unsigned int va);

	int AtomicAdd(volatile int *var, int val);
	int AtomicAnd(volatile int *var, int val);
	int AtomicOr(volatile int *var, int val);
};

/// Set the physical address of the current page directory
inline void SetCurrentPageDir(unsigned int addr)
{
#if !SKY_EMULATOR
	_asm
	{
		mov	eax, [addr]
		mov	cr3, eax
	}
#endif
}

/// Return the physical address of the current page directory
unsigned int GetCurrentPageDir();

/// Invalidate Translation Lookaside Buffer for a specific virtual address
/// This removes any cached page mappings for this address.  It must be called
/// whenever the mapping for a virtual address is changed.
inline void InvalidateTLB(unsigned int va)
{
#if !SKY_EMULATOR
	__asm
	{
		invlpg	va
	}
#endif
}

/// Returns the virtual address that was being accessed when the page fault occured
inline unsigned int GetFaultAddress()
{
	unsigned int retval;
	__asm
	{
		push eax
		mov eax, cr2
		mov retval, eax
		pop eax
	}

	return retval;
}

inline void LoadIdt(const IdtEntry base[], unsigned int limit)
{

}

#pragma pack( push, 1 )
struct desc {
	unsigned short limit;
	const GdtEntry* base;
} PACKED;
#pragma pack( pop )

inline void LoadGdt(const GdtEntry base[], unsigned int limit)
{
	
	desc d;
	d.base = base;
	d.limit = limit;

	_asm
	{
		lgdt[d]
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov gs, ax
		mov fs, ax
		mov ss, ax
		mov ax, 0x28
		ltr ax
	}
}

inline void Halt()
{
#if SKY_EMULATOR
	__asm pause
#else
	__asm hlt
#endif
}

inline void ClearPage(void *va)
{
#if SKY_EMULATOR
	memset(va, 0, PAGE_SIZE);
	return;
#endif

	int dummy0, dummy1;
	__asm
	{
		mov eax, 0
		mov ecx, (PAGE_SIZE / 4)
		mov edi, va
		rep stos		
		mov dummy0, ecx
		mov dummy1, edi		
	}	
}

union int32to64
{
	__int64 i64;
	int i32[2];
};



inline int64 rdtsc()
{
	int32to64 a;
	read_tsc(&a.i32[0], &a.i32[1]);

	return a.i64;
}

inline bool cmpxchg32(volatile int *var, int oldValue, int newValue)
{
	int success;
	__asm
	{
		mov eax, oldValue
		mov ecx, newValue
		mov edi, var
		lock cmpxchg [edi], ecx 
		sete al 
		and eax, 1			
		mov success, eax

	}
	return success;
}

inline void CopyPageInternal(void *dest, const void *src)
{
	int dummy0, dummy1, dummy2;
	__asm
	{
		mov eax, 0
		mov ecx, (PAGE_SIZE / 4)
		mov edi, dest
		mov esi, src
		rep movs		

		mov dummy0, ecx
		mov dummy1, edi
		mov dummy2, esi
	}			
}

inline void SaveFp(FpState* state)
{
	
	__asm
	{
		mov eax, state
		fnsave [eax]
	}
}

inline void RestoreFp(const FpState* state)
{
	__asm
	{
		mov eax, state
		frstor [eax]
	}
	 
}

inline void ClearTrapOnFp()
{
#if !SKY_EMULATOR
	__asm clts
#endif
}

inline void SetTrapOnFp()
{
#if !SKY_EMULATOR
	__asm
	{
		mov eax, cr0
		or eax, 8
		mov cr0, eax
	}
#endif
}

inline bool _get_interrupt_state()
{
#if SKY_EMULATOR
	return false;
#endif

	unsigned int result;
	__asm
	{
		pushf 
		pop result
	}

	return (result & (1 << 9)) != 0;
}



/*inline int64 rdtsc()
{
	unsigned int high, low;
	_asm("rdtsc" : "=a" (low), "=d" (high));
	return (int64)high << 32 | low;

	return 0;
}*/

/*inline int AtomicAdd(volatile int *var, int val)
{
int oldVal = *var;
*var = *var + val;
return oldVal;
}*/




#endif

