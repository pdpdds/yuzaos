#if _MSC_VER > 1000
#pragma once
#endif

#ifndef FLOAT_H
#define FLOAT_H

#define FLT_RADIX     2
#define FLT_ROUNDS    1
#define FLT_DIG       6
#define FLT_EPSILON   1.192092896e-07F
#define FLT_MANT_DIG  24
#define FLT_MAX       3.402823466e+38F
#define FLT_MAX_EXP   38
#define FLT_MIN       1.175494351e-38F
#define FLT_MIN_EXP   (-37)

#define DBL_DIG        15
#define DBL_EPSILON    2.2204460492503131e-016
#define DBL_MANT_DIG   53
#define DBL_MAX_10_EXP  308                     // max decimal exponent
#define DBL_MAX        1.7976931348623158e+308
#define DBL_MAX_EXP    1024
#define DBL_MIN        2.2250738585072014e-308
#define DBL_MIN_EXP    (-307)

#define LDBL_MANT_DIG 53
#define _LDBL_EQ_DBL
#define LDBL_DIG         DBL_DIG                 // # of decimal digits of precision
#define LDBL_EPSILON     DBL_EPSILON             // smallest such that 1.0+LDBL_EPSILON != 1.0
#define LDBL_HAS_SUBNORM DBL_HAS_SUBNORM         // type does support subnormal numbers
#define LDBL_MAX         DBL_MAX                 // max value
#define LDBL_MAX_10_EXP  DBL_MAX_10_EXP          // max decimal exponent
#define LDBL_MAX_EXP     DBL_MAX_EXP             // max binary exponent
#define LDBL_MIN         DBL_MIN                 // min normalized positive value
#define LDBL_MIN_10_EXP  DBL_MIN_10_EXP          // min decimal exponent
#define LDBL_MIN_EXP     DBL_MIN_EXP             // min binary exponent
#define _LDBL_RADIX      _DBL_RADIX              // exponent radix
#define LDBL_TRUE_MIN    DBL_TRUE_MIN            // min positive value

#define	FLT_MIN_10_EXP -37
#define	DBL_MIN_10_EXP  -307

#define FLT_MAX_10_EXP 38




#ifdef  __cplusplus
extern "C" {
#endif

void _fpreset();

#ifdef  __cplusplus
}
#endif

#endif

