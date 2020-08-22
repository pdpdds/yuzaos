#include <stringdef.h>

//extern "C" {

	int AtomicAdd(volatile int *var, int val)
	{
#ifdef HAVE_MSC_ATOMICS
		return _InterlockedExchangeAdd((long*)var, val);
#endif
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, var
			mov edx, val
			loop_point :
			mov eax, [edi]
			mov ecx, eax
			add ecx, edx
			lock cmpxchg[edi], ecx
			jnz loop_point

			mov oldVal, eax
			mov dummy, ecx
		}
		return oldVal;
	}

	int AtomicAnd(volatile int *var, int val)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, var
			mov edx, val
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				and ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	int AtomicOr(volatile int *var, int val)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, var
			mov edx, val
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				or ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	LONG AtomicInterlockedAdd(LONG volatile *Addend, LONG Value)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, Addend
			mov edx, Value
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				add ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	LONG AtomicInterlockedAnd(LONG volatile *Destination, LONG Value)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, Destination
			mov edx, Value
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				and ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	LONG AtomicInterlockedOr(LONG volatile *Destination, LONG Value)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, Destination
			mov edx, Value
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				or ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	LONG AtomicInterlockedCompareExchange(LONG volatile *Destination, LONG ExChange, LONG Comperand)
	{
		int success;
		__asm
		{
			mov eax, ExChange
			mov ecx, Comperand
			mov edi, Destination
			lock cmpxchg[edi], ecx
			sete al
			and eax, ExChange
			mov success, eax

		}
		return success;
	}

	LONG AtomicInterlockedDecrement(LONG volatile *Addend)
	{
		int one = -1;
		__asm
		{
			mov eax, Addend
			mov ebx, one
			lock xadd[eax], ebx
		}

		return *Addend;
	}

	LONG AtomicInterlockedIncrement(LONG volatile *Addend)
	{
		int one = 1;
		__asm
		{
			mov eax, Addend
			mov ebx, one
			lock xadd[eax], ebx
		}

		return *Addend;
	}

	LONG AtomicInterlockedXor(LONG volatile *Destination, LONG Value)
	{
		int oldVal;
		int dummy;
		__asm
		{
			mov edi, Destination
			mov edx, Value
			loop_point :
			mov eax, [edi]
				mov ecx, eax
				xor ecx, edx
				lock cmpxchg[edi], ecx
				jnz loop_point

				mov oldVal, eax
				mov dummy, ecx
		}
		return oldVal;
	}

	LONG  AtomicInterlockedExchange(LPLONG volatile Target, LONG Value)
	{
		LONG ReturnValue;

		__asm
		{
			push ebx;	
			pop edi
			mov ebx, Value;
			mov edi, Target

			lock xchg [edi], ebx
			mov ReturnValue, ebx
			pop edi
			pop ebx
				
		}
		return ReturnValue;
	}
//}

