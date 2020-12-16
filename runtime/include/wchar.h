#pragma once
#include <windef.h>
#include <stdint.h>

#if !defined(__cplusplus)
typedef short wchar_t;
#endif

#ifdef DLL_WSTRING_EXPORT
#define WSTRING_API __declspec(dllexport) 
#else
#define WSTRING_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		wchar_t __cp;
		uint16_t __class;
		wchar_t __uc;
		wchar_t __lc;
	} __wchar_info_t;


#define MB_CUR_MAX 128

	WSTRING_API double wcstod(const wchar_t* __restrict nptr, wchar_t** __restrict endptr);
	WSTRING_API size_t
		wcsftime(wchar_t* __restrict wcs, size_t maxsize,
			const wchar_t* __restrict format, const struct tm* __restrict timeptr);
	WSTRING_API long wcstol(const wchar_t* __restrict nptr, wchar_t** __restrict endptr, int base);
	WSTRING_API unsigned long wcstoul(const wchar_t* __restrict nptr, wchar_t** __restrict endptr, int base);
	WSTRING_API size_t wcsxfrm(wchar_t* __restrict a, const wchar_t* __restrict b, size_t n);
	WSTRING_API int wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t n);
	WSTRING_API size_t wcscspn(const wchar_t* s, const wchar_t* set);
	WSTRING_API size_t wcsspn(const wchar_t* s, const wchar_t* set);
	WSTRING_API int wcscoll(const wchar_t* a, const wchar_t* b);
	WSTRING_API int wcsicmp(const wchar_t* cs, const wchar_t* ct);
	WSTRING_API size_t wcsnlen(const wchar_t* s, size_t maxlen);
	WSTRING_API wchar_t* wcsrchr(const wchar_t* s, wchar_t c);
	WSTRING_API wchar_t* wcsstr(const wchar_t* __restrict big, const wchar_t* __restrict little);
	WSTRING_API wchar_t* wcsncat(wchar_t* __restrict s1, const wchar_t* __restrict s2, size_t n);
	WSTRING_API wchar_t *wcscat(wchar_t*__restrict dest, const wchar_t*__restrict src);
	WSTRING_API wchar_t* wcschr(const wchar_t* str, wchar_t c);
	WSTRING_API int wcscmp(const wchar_t *str1, const wchar_t *str2);
	WSTRING_API wchar_t *wcscpy(wchar_t *__restrict strDestination, const wchar_t *__restrict strSource);
	WSTRING_API size_t wcslen(const wchar_t *str);
	WSTRING_API int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
	WSTRING_API wchar_t* wcspbrk(const wchar_t *s1, const wchar_t *s2);
	WSTRING_API size_t wcstombs(char *mbstr, const wchar_t *wcstr, size_t count);	
	WSTRING_API size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
	WSTRING_API int _wcsnicmp(const wchar_t *s1, const wchar_t *s2, size_t n);		
	WSTRING_API wchar_t *wmemcpy(wchar_t *__restrict d, const wchar_t *__restrict s, size_t n);
	WSTRING_API size_t wcslcpy(wchar_t *dst, const wchar_t *src, size_t siz);
	WSTRING_API wchar_t* wcsncpy(wchar_t* __restrict s1, const wchar_t* __restrict s2, size_t n);
	//WSTRING_API size_t wcsrtombs(char* __restrict dst, const wchar_t** __restrict src, size_t len, mbstate_t* __restrict ps);
	

#ifdef __cplusplus
}
#endif