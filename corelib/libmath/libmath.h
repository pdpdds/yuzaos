#pragma once

#ifdef LIB_MATH_DLL
#define LIB_MATH_API __declspec(dllexport)
#else
#define LIB_MATH_API __declspec(dllimport)
#endif


#ifdef __cplusplus

extern "C" {
#endif
	LIB_MATH_API double acos_impl(double x);
	LIB_MATH_API float acosf_impl(float x);
	LIB_MATH_API long double acosl_impl(long double x);
	 
	LIB_MATH_API double asin_impl(double x);
	LIB_MATH_API float asinf_impl(float x);
	LIB_MATH_API long double asinl_impl(long double x);
	 
	LIB_MATH_API double atan_impl(double x);
	LIB_MATH_API float atanf_impl(float x);
	LIB_MATH_API long double atanl_impl(long double x);
	
	LIB_MATH_API double atan2_impl(double y, double x);
	LIB_MATH_API float atan2f_impl(float y, float x);
	LIB_MATH_API long double atan2l_impl(long double y, long double x);
	
	LIB_MATH_API double ceil_impl(double x);
	LIB_MATH_API float ceilf_impl(float x);
	LIB_MATH_API long double ceill_impl(long double x);
	
	LIB_MATH_API double cos_impl(double x);
	LIB_MATH_API float cosf_impl(float x);
	LIB_MATH_API long double cosl_impl(long double x);
	
	LIB_MATH_API double cosh_impl(double x);
	LIB_MATH_API float coshf_impl(float x);
	LIB_MATH_API long double coshl_impl(long double x);
	
	LIB_MATH_API double exp_impl(double x);
	LIB_MATH_API float expf_impl(float x);
	LIB_MATH_API long double expl_impl(long double x);
	
	LIB_MATH_API double fabs_impl(double x);
	LIB_MATH_API float fabsf_impl(float x);
	LIB_MATH_API long double fabsl_impl(long double x);
	
	LIB_MATH_API double floor_impl(double x);
	LIB_MATH_API float floorf_impl(float x);
	LIB_MATH_API long double floorl_impl(long double x);
	
	LIB_MATH_API double fmod_impl(double x, double y);
	LIB_MATH_API float fmodf_impl(float x, float y);
	LIB_MATH_API long double fmodl_impl(long double x, long double y);
	
	LIB_MATH_API double log_impl(double x);
	LIB_MATH_API float logf_impl(float x);
	LIB_MATH_API long double logl_impl(double x);
	LIB_MATH_API double log10_impl(double x);
	LIB_MATH_API float log10f_impl(float x);
	LIB_MATH_API long double log10l_impl(double x);
	
	LIB_MATH_API double pow_impl(double x, double y);
	LIB_MATH_API float powf_impl(float x, float y);
	LIB_MATH_API long double powl_impl(long double x, long double y);
	
	LIB_MATH_API double tan_impl(double x);
	LIB_MATH_API float tanf_impl(float x);
	LIB_MATH_API long double tanl_impl(long double x);
	
	LIB_MATH_API double sin_impl(double x);
	LIB_MATH_API float sinf_impl(float x);
	LIB_MATH_API long double sinl_impl(long double x);
	
	LIB_MATH_API double sqrt_impl(double x);
	LIB_MATH_API float sqrtf_impl(float x);
	LIB_MATH_API long double sqrtl_impl(long double x);
	
	LIB_MATH_API double tanh_impl(double x);
	LIB_MATH_API float tanhf_impl(float x);
	LIB_MATH_API long double tanhl_impl(long double x);

	LIB_MATH_API int abs_impl(int n);
	LIB_MATH_API long labs_impl(long n);

#ifdef __cplusplus
}
#endif