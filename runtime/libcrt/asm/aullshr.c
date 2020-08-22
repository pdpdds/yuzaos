#include "windef.h"
#include <stdint.h>

//! my implimentation of _aullshr
uint64_t _declspec (naked) _aullshr() {

	_asm {
		//! only handle 64bit shifts or more
		cmp     cl, 64
		jae     invalid

		//! handle shifts between 0 and 31 bits
		cmp     cl, 32
		jae     more32
		shrd    eax, edx, cl
		shr     edx, cl
		ret

		//! handle shifts of 32-63 bits
		more32 :
		mov     eax, edx
			xor edx, edx
			and cl, 31
			shr     eax, cl
			ret

			//! invalid number (its less then 32bits), return 0
			invalid :
		xor eax, eax
			xor edx, edx
			ret
	}
}