#include <windef.h>
#include <math.h>

#if defined(MATH_WIN32)
#include <../libmath/libmath.h>
double acos(double x)
{
	return acos_impl(x);
}

float acosf(float x)
{
	return acosf_impl(x);
}

long double acosl(long double x)
{
	return acosl_impl(x);
}

double asin(double x)
{
	return asin_impl(x);
}

float asinf(float x)
{
	return asinf_impl(x);
}

long double asinl(long double x)
{
	return asinl_impl(x);
}

double atan(double x)
{
	return atan_impl(x);
}

float atanf(float x)
{
	return atanf_impl(x);
}

long double atanl(long double x)
{
	return atanl_impl(x);
}

double atan2(double y, double x)
{
	return atan2_impl(y, x);
}

float atan2f(float y, float x)
{
	return atan2f_impl(y, x);
}

long double atan2l(long double y, long double x)
{
	return atan2l_impl(y, x);
}

double ceil(double x)
{
	return ceil_impl(x);
}

float ceilf(float x)
{
	return ceilf_impl(x);
}

long double ceill(long double x)
{
	return ceill_impl(x);
}

double cos(double x)
{
	return cos_impl(x);
}

float cosf(float x)
{
	return cosf_impl(x);
}

long double cosl(long double x)
{
	return cosl_impl(x);
}

double cosh(double x)
{
	return cosh_impl(x);
}

float coshf(float x)
{
	return coshf_impl(x);
}

long double coshl(long double x)
{
	return coshl_impl(x);
}
double exp(double x)
{
	return exp_impl(x);
}

float expf(float x)
{
	return expf_impl(x);

}
long double expl(long double x)
{
	return expl_impl(x);

}
double fabs(double x)
{
	return fabs_impl(x);
}

float fabsf(float x)
{
	return fabsf_impl(x);
}

long double fabsl(long double x)
{
	return fabsl_impl(x);
}

double floor(double x)
{
	return floor_impl(x);
}
float floorf(float x)
{
	return floorf_impl(x);

}
long double floorl(long double x)
{
	return floorl_impl(x);
}

double fmod(double x, double y)
{
	return fmod_impl(x, y);
}

float fmodf(float x, float y)
{
	return fmodf_impl(x, y);
}

long double fmodl(long double x, long double y)
{
	return fmodl_impl(x, y);
}

double log(double x)
{
	return log_impl(x);
}

float logf(float x)
{
	return logf_impl(x);
}

long double logl(double x)
{
	return logl_impl(x);
}

double log10(double x)
{
	return log10_impl(x);
}

float log10f(float x)
{
	return log10f_impl(x);
}

long double log10l(double x)
{
	return log10l_impl(x);
}

double pow(double x, double y)
{
	return pow_impl(x, y);
}

float powf(float x, float y)
{
	return powf_impl(x, y);
}

long double powl(long double x, long double y)
{
	return powl_impl(x, y);
}

double tan(double x)
{
	return tan_impl(x);
}

float tanf(float x)
{
	return tanf_impl(x);
}

long double tanl(long double x)
{
	return tanl_impl(x);
}

double sin(double x)
{
	return sin_impl(x);
}

float sinf(float x)
{
	return sinf_impl(x);
}

long double sinl(long double x)
{
	return sinl_impl(x);
}

double sqrt(double x)
{
	return sqrt_impl(x);
}

float sqrtf(float x)
{
	return sqrtf_impl(x);
}
long double sqrtl(long double x)
{
	return sqrtl_impl(x);
}
double tanh(double x)
{
	return tanh_impl(x);
}
float tanhf(float x)
{
	return tanhf_impl(x);
}
long double tanhl(long double x)
{
	return tanhl_impl(x);
}


int abs(int n)
{
	return abs_impl(n);
}

long labs(long n)
{
	return labs_impl(n);
}
#else
#include <stddef.h>

#define DBL_EXPBITS 11
#define DBL_FRACHBITS   20
#define DBL_FRACLBITS   32

#define MIN( x, y )     ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )
#define MAX( x, y )     ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) )

#define RAND_MAX 0x7fff

#define M_E	2.71828182845904523536 //e	
#define M_LOG2E	 1.44269504088896340736 // log2(e)	
#define M_LOG10E	0.434294481903251827651 //	log10(e)
#define M_LN2		0.693147180559945309417 //ln(2)
#define M_LN10	2.30258509299404568402 //ln(10)	
#define M_PI  3.14159265358979323846 //	pi	
#define M_PI_2		1.57079632679489661923 //pi / 2
#define M_PI_4		0.785398163397448309616 //pi / 4
#define M_1_PI		0.318309886183790671538 //1 / pi
#define M_2_PI		0.636619772367581343076 //2 / pi
#define M_2_SQRTPI		1.12837916709551257390 //2 / sqrt(pi)
#define M_SQRT2		1.41421356237309504880 //sqrt(2)
#define M_SQRT1_2		0.707106781186547524401 //1 / sqrt(2)
#define PI		M_PI
#define PI2		M_PI_2


#define FLT_EPSILON	1.192092896e-07F
#define EPSILON FLT_EPSILON

//#define MATH_DLL 
/*#ifdef MATH_DLL
#define MATH_API __declspec(dllexport)
#else
#define MATH_API __declspec(dllimport)
#endif*/

struct ieee_double {
	/*#if _BYTE_ORDER == _BIG_ENDIAN
		unsigned int   dbl_sign : 1;
		unsigned int   dbl_exp : DBL_EXPBITS;
		unsigned int   dbl_frach : DBL_FRACHBITS;
		unsigned int   dbl_fracl : DBL_FRACLBITS;
	#else*/
	unsigned int   dbl_fracl : DBL_FRACLBITS;
	unsigned int   dbl_frach : DBL_FRACHBITS;
	unsigned int   dbl_exp : DBL_EXPBITS;
	unsigned int   dbl_sign : 1;
	//#endif

};

#ifndef _HUGE_ENUF
#define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL   ((double)INFINITY)
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
#define NAN        ((float)(INFINITY * 0.0F))

double frexp(double value, int* eptr)
{
	union {
		double v;
		struct ieee_double s;
	} u;

	if (value) {
		/*
		* Fractions in [0.5..1.0) have an exponent of 2^-1.
		* Leave Inf and NaN alone, however.
		* WHAT ABOUT DENORMS?
		*/
		u.v = value;
		if (u.s.dbl_exp != DBL_EXP_INFNAN) {
			*eptr = u.s.dbl_exp - (DBL_EXP_BIAS - 1);
			u.s.dbl_exp = DBL_EXP_BIAS - 1;
		}
		return (u.v);
	}
	else {
		*eptr = 0;
		return (0.0);
	}
}

float frexpf(float value, int* eptr)
{
	union {
		double v;
		struct ieee_double s;
	} u;

	if (value) {
		/*
		* Fractions in [0.5..1.0) have an exponent of 2^-1.
		* Leave Inf and NaN alone, however.
		* WHAT ABOUT DENORMS?
		*/
		u.v = value;
		if (u.s.dbl_exp != DBL_EXP_INFNAN) {
			*eptr = u.s.dbl_exp - (DBL_EXP_BIAS - 1);
			u.s.dbl_exp = DBL_EXP_BIAS - 1;
		}
		return (u.v);
	}
	else {
		*eptr = 0;
		return (0.0);
	}
}

int abs(int j)
{
	return(j < 0 ? -j : j);
}

/*double fabs(double j)
{
	return(j < 0 ? -j : j);
}*/

float fabsf(float j)
{
	return fabs(j);
}

double floor(double x)
{
	if (x >= 0)
	{
		return (int)x;
	}
	else
	{
		int y = (int)x;
		return ((float)y == x) ? y : y - 1;
	}
}

float floorf(float x)
{
	if (x >= 0)
	{
		return (int)x;
	}
	else
	{
		int y = (int)x;
		return ((float)y == x) ? y : y - 1;
	}
}

MATH_API float cosf(float x)
{
	return cos(x);
}

MATH_API float atanf(float x)
{
	return atan(x);
}




/* Function to calculate x raised to the power y

Time Complexity: O(n)
Space Complexity: O(1)
Algorithmic Paradigm: Divide and conquer.
*/
int power1(int x, unsigned int y)
{
	if (y == 0)
		return 1;
	else if ((y % 2) == 0)
		return power1(x, y / 2) * power1(x, y / 2);
	else
		return x * power1(x, y / 2) * power1(x, y / 2);

}

/* Function to calculate x raised to the power y in O(logn)
Time Complexity of optimized solution: O(logn)
*/
int power2(int x, unsigned int y)
{
	int temp;
	if (y == 0)
		return 1;

	temp = power2(x, y / 2);
	if ((y % 2) == 0)
		return temp * temp;
	else
		return x * temp * temp;
}

/* Extended version of power function that can work
for float x and negative y
*/
double pow(float x, int y)
{
	float temp;
	if (y == 0)
		return 1;
	temp = pow(x, y / 2);
	if ((y % 2) == 0) {
		return temp * temp;
	}
	else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

float powf(float x, float y)
{
	float temp;
	if (y == 0)
		return 1;
	temp = pow(x, (int)(y / 2));
	if (((int)y % 2) == 0) {
		return temp * temp;
	}
	else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

/* Extended version of power function that can work
for double x and negative y
*/
double powerd(double x, int y)
{
	double temp;
	if (y == 0)
		return 1;
	temp = powerd(x, y / 2);
	if ((y % 2) == 0) {
		return temp * temp;
	}
	else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

long fact(int n)
{
	int c;
	long result = 1;

	for (c = 1; c <= n; c++)
		result = result * c;

	return result;
}

double fmod(double a, double b)
{
	return (a - b * floor(a / b));
}

float fmodf(float a, float b)
{
	return fmod(a, b);
}

double powerOfTen(int num) {
	double rst = 1.0;
	if (num >= 0) {
		for (int i = 0; i < num; i++) {
			rst *= 10.0;
		}
	}
	else {
		for (int i = 0; i < (0 - num); i++) {
			rst *= 0.1;
		}
	}

	return rst;
}

double sqrt(double a)
{

	//find more detail of this method on wiki methods_of_computing_square_roots

	// Babylonian method cannot get exact zero but approximately value of the square_root

	double z = a;
	double rst = 0.0;
	int max = 8;	// to define maximum digit
	int i;
	double j = 1.0;
	for (i = max; i > 0; i--) {
		// value must be bigger then 0
		if (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
		{
			while (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
			{
				j++;
				if (j >= 10) break;

			}
			j--; //correct the extra value by minus one to j
			z -= ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a

			j = 1.0;


		}

	}

	for (i = 0; i >= 0 - max; i--) {
		if (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
		{
			while (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
			{
				j++;
				if (j >= 10) break;
			}
			j--;
			z -= ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a
			j = 1.0;
		}
	}
	// find the number on each digit
	return rst;
}

float sqrtf(float a)
{

	//find more detail of this method on wiki methods_of_computing_square_roots

	// Babylonian method cannot get exact zero but approximately value of the square_root

	float z = a;
	float rst = 0.0;
	int max = 8;	// to define maximum digit
	int i;
	float j = 1.0;
	for (i = max; i > 0; i--) {
		// value must be bigger then 0
		if (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
		{
			while (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
			{
				j++;
				if (j >= 10) break;

			}
			j--; //correct the extra value by minus one to j
			z -= ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a

			j = 1.0;


		}

	}

	for (i = 0; i >= 0 - max; i--) {
		if (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
		{
			while (z - ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)) >= 0)
			{
				j++;
				if (j >= 10) break;
			}
			j--;
			z -= ((2 * rst) + (j * powerOfTen(i)))*(j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a
			j = 1.0;
		}
	}
	// find the number on each digit
	return rst;
}

float roundf(float value)
{
	return floorf(value + 0.5f);
}

float fmaxf(float x, float y)
{
	if (x > y)
		return x;

	return y;
}
double fmax(double x, double y)
{
	if (x > y)
		return x;

	return y;
}
#endif

static unsigned long int next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
	next = seed;
}

static int _finite(double d)
{
#if ENDIAN == ENDIAN_LITTLE
	struct IEEEdp {
		unsigned manl : 32;
		unsigned manh : 20;
		unsigned exp : 11;
		unsigned sign : 1;
	} *ip;
#else
	struct IEEEdp {
		unsigned sign : 1;
		unsigned exp : 11;
		unsigned manh : 20;
		unsigned manl : 32;
	} *ip;
#endif

	ip = (struct IEEEdp*) & d;
	return (ip->exp != 0x7ff);
}

MATH_API int isinf(double d)
{
	return _finite(d);
}