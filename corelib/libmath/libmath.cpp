#include <windef.h>
#include "libmath.h"

extern "C"
{
	double acos(double x);
	double asin(double x);
	double atan(double x);
	double atan2(double y, double x);
	double ceil(double x);
	double cos(double x);
	
	double cosh(double x);
	double exp(double x);
	double fabs(double x);

	double floor(double x);
	double fmod(double x, double y);
	double log(double x);
	double log10(double x);
	double pow(double x, double y);
	double tan(double x);
	double sin(double x);
	double sqrt(double x);
	double tanh(double x);

	int abs(int n);
	long labs(long n);

}

double acos_impl(double x)
{
	return acos(x);
}

float acosf_impl(float x)
{
	return acos(x);
}

long double acosl_impl(long double x)
{
	return acos(x);
}

double asin_impl(double x)
{
	return asin(x);
}

float asinf_impl(float x)
{
	return asin(x);
}

long double asinl_impl(long double x)
{
	return asin(x);
}

double atan_impl(double x)
{
	return atan(x);
}

float atanf_impl(float x)
{
	return atan(x);
}

long double atanl_impl(long double x)
{
	return atan(x);
}

double atan2_impl(double y, double x)
{
	return atan2(y, x);
}

float atan2f_impl(float y, float x)
{
	return atan2(y, x);
}

long double atan2l_impl(long double y, long double x)
{
	return atan2(y, x);
}

double ceil_impl(double x)
{
	return ceil(x);
}

float ceilf_impl(float x)
{
	return ceil(x);
}

long double ceill_impl(long double x)
{
	return ceil(x);
}

double cos_impl(double x)
{
	return cos(x);
}

float cosf_impl(float x)
{
	return cos(x);
}

long double cosl_impl(long double x)
{
	return cos(x);
}

double cosh_impl(double x)
{
	return cosh(x);
}

float coshf_impl(float x)
{
	return cosh(x);
}

long double coshl_impl(long double x)
{
	return cosh(x);
}
double exp_impl(double x)
{
	return exp(x);
}

float expf_impl(float x)
{
	return exp(x);

}
long double expl_impl(long double x)
{
	return exp(x);

}
double fabs_impl(double x)
{
	return fabs(x);
}

float fabsf_impl(float x)
{
	return fabs(x);
}

long double fabsl_impl(long double x)
{
	return fabs(x);
}

double floor_impl(double x)
{
	return floor(x);
}
float floorf_impl(float x)
{
	return floor(x);

}
long double floorl_impl(long double x)
{
	return floor(x);
}

double fmod_impl(double x, double y)
{
	return fmod(x, y);
}

float fmodf_impl(float x, float y)
{
	return fmod(x, y);
}

long double fmodl_impl(long double x, long double y)
{
	return fmod(x, y);
}

double log_impl(double x)
{
	return log(x);
}

float logf_impl(float x)
{
	return log(x);
}

long double logl_impl(double x)
{
	return log(x);
}

double log10_impl(double x)
{
	return log10(x);
}

float log10f_impl(float x)
{
	return log10(x);
}

long double log10l_impl(double x)
{
	return log10(x);
}

double pow_impl(double x, double y)
{
	return pow(x, y);
}

float powf_impl(float x, float y)
{
	return pow(x, y);
}

long double powl_impl(long double x, long double y)
{
	return pow(x, y);
}

double tan_impl(double x)
{
	return tan(x);
}

float tanf_impl(float x)
{
	return tan(x);
}

long double tanl_impl(long double x)
{
	return tan(x);
}

double sin_impl(double x)
{
	return sin(x);
}

float sinf_impl(float x)
{
	return sin(x);
}

long double sinl_impl(long double x)
{
	return sin(x);
}

double sqrt_impl(double x)
{
	return sqrt(x);
}

float sqrtf_impl(float x)
{
	return sqrt(x);
}
long double sqrtl_impl(long double x)
{
	return sqrt(x);
}
double tanh_impl(double x)
{
	return tanh(x);
}
float tanhf_impl(float x)
{
	return tanh(x);
}
long double tanhl_impl(long double x)
{
	return tanh(x);
}

int abs_impl(int n)
{
	return abs(n);
}

long labs_impl(long n)
{
	return labs(n);
}