#pragma once

#ifdef MATH_DLL
#define MATH_API __declspec(dllexport)
#else
#define MATH_API __declspec(dllimport)
#endif

#define MIN( x, y )     ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )
#define MAX( x, y )     ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) )

#ifndef M_PI
#define M_PI  3.14159265358979323846264f  // from CRC
#endif

// Maximum value that can be returned by the rand function:
#define RAND_MAX 0x7fff

#ifndef _HUGE_ENUF
#define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL   ((double)INFINITY)
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
#define NAN        ((float)(INFINITY * 0.0F))

#ifdef __cplusplus

extern "C" {
#endif
	MATH_API double acos(double x);
	MATH_API float acosf(float x);
	MATH_API long double acosl(long double x);

	MATH_API double sinh(double x);
	MATH_API float sinhf(float x);
	MATH_API double asin(double x);
	MATH_API float asinf(float x);
	MATH_API long double asinl(long double x);

	MATH_API double atan(double x);
	MATH_API float atanf(float x);
	MATH_API long double atanl(long double x);

	MATH_API double atan2(double y, double x);
	MATH_API float atan2f(float y, float x);
	MATH_API long double atan2l(long double y, long double x);

	MATH_API double ceil(double x);
	MATH_API float ceilf(float x);
	MATH_API long double ceill(long double x);

	MATH_API double cos(double x);
	MATH_API float cosf(float x);
	MATH_API long double cosl(long double x);

	MATH_API double cosh(double x);
	MATH_API float coshf(float x);
	MATH_API long double coshl(long double x);

	MATH_API double exp(double x);
	MATH_API float expf(float x);
	MATH_API long double expl(long double x);

	MATH_API double fabs(double x);
	MATH_API float fabsf(float x);
	MATH_API long double fabsl(long double x);

	MATH_API double floor(double x);
	MATH_API float floorf(float x);
	MATH_API long double floorl(long double x);

	MATH_API double fmod(double x, double y);
	MATH_API float fmodf(float x,float y);
	MATH_API long double fmodl(long double x,long double y);

	MATH_API double log(double x);
	MATH_API float logf(float x);
	MATH_API long double logl(double x);
	
	MATH_API double log10(double x);
	MATH_API float log10f(float x);
	MATH_API long double log10l(double x);

	MATH_API double pow(double x, double y);
	MATH_API float powf(float x, float y);
	MATH_API long double powl(long double x, long double y);

	MATH_API double tan(double x);
	MATH_API float tanf(float x);
	MATH_API long double tanl(long double x);

	MATH_API double sin(double x);
	MATH_API float sinf(float x);
	MATH_API long double sinl(long double x);

	MATH_API double sqrt(double x);
	MATH_API float sqrtf(float x);
	MATH_API long double sqrtl(long double x);

	MATH_API double tanh(double x);
	MATH_API float tanhf(float x);
	MATH_API long double tanhl(long double x);

	MATH_API int abs(int n);
	MATH_API long labs(long n);

	MATH_API double modf(double value, double* iptr);
	MATH_API float modff(float value, float* iptr);
	MATH_API long double modfl(long double value, long double* iptr);

	MATH_API int rand(void);
	MATH_API void srand(unsigned int seed);

	MATH_API float log2f(float x);
	MATH_API  double ldexp(double value, int exp);
	MATH_API double frexp(double value, int* eptr);

	MATH_API float roundf(float value);
	MATH_API float fmaxf(float a, float y);
	MATH_API double fmax(double x, double y);

	MATH_API int isnan(double);
	MATH_API int isinf(double);

	MATH_API int isfinite(double x);

	MATH_API double hypot(double x, double y);

	MATH_API double rint(double x);

	MATH_API float       nanf(const char* arg);
	MATH_API double      nan(const char* arg);
	MATH_API long double nanl(const char* arg);
	

#ifdef __cplusplus
}
#endif