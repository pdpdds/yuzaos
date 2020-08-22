/* Includes */
#include <stdlib.h>
#include "./src/fdlibm.h"
/* Disable warning + pragma */
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable: 4164)
#pragma function(labs)
#endif

/* Absolutes the given integer value
* to a positive value */
MATH_API long labs(long j)
{
	return j < 0 ? -j : j;
}
