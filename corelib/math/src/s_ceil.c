
/* @(#)s_ceil.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * ceil(x)
 * Return x rounded toward -inf to integral value
 * Method:
 *	Bit twiddling.
 * Exception:
 *	Inexact flag raised if x not equal to ceil(x).
 */

#include "fdlibm.h"

#ifdef __STDC__
static const double huge = 1.0e300;
#else
static double huge = 1.0e300;
#endif

//Fix it
double ceil(double x)
{
	long r;
	r = x;
	if (r < 0)
		return r;
	else
		return (r + ((r < x) ? 1 : 0));
}

MATH_API float ceilf(float x)
{

	long r;
	r = x;
	if (r < 0)
		return r;
	else
		return (r + ((r < x) ? 1 : 0));
}