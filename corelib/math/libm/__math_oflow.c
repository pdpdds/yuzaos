#include "libm.h"

extern double __math_xflow(uint32_t sign, double y);

double __math_oflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p769);
}
