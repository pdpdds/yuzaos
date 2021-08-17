#include "libm.h"

extern float __math_xflowf(uint32_t sign, float y);

float __math_uflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p-95f);
}
