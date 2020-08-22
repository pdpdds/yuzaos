//! required by MSVC++ runtime for floating point operations (Must be 1)
int _fltused = 1;

void __declspec(naked) _allshr()
{
	/* *INDENT-OFF* */
	__asm {
		cmp         cl, 40h
		jae         RETZERO
		cmp         cl, 20h
		jae         MORE32
		shrd        eax, edx, cl
		sar         edx, cl
		ret
		MORE32 :
		mov         eax, edx
			xor         edx, edx
			and         cl, 1Fh
			sar         eax, cl
			ret
			RETZERO :
		xor         eax, eax
			xor         edx, edx
			ret
	}
	/* *INDENT-ON* */
}

