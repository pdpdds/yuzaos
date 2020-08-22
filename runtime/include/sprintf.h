#pragma once
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif
	CRT_API int sprintf(char* s, const char* format, ...);
	CRT_API int sprintf_f(char* s, const char* format, ...);
	//int snprintf(char* str, size_t size, const char* format, ...);
	CRT_API void itoa(unsigned i, unsigned base, char* buf);
	//void itoa_s(int i, unsigned base, char* buf);
	CRT_API void itoa_s(unsigned int i, unsigned base, char* buf);
	CRT_API int strcmp(const char* string1, const char* string2);
	CRT_API char* _i64toa(long long value, char* str, int radix);
	CRT_API void qsort(void* aa, size_t n, size_t _es, int(*cmp)(const void*, const void*));
#ifdef  __cplusplus
}
#endif