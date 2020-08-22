#pragma once
#include <windef.h>

#if  defined(DLL_CRT_EXPORT)
#define CRT_API __declspec(dllexport) 
#elif defined(DLL_CRT_IMPORT) 
#define CRT_API __declspec(dllimport)
#else
#define CRT_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif
	CRT_API void* memcpy(void *dest, const void *src, size_t count);
	CRT_API void* memset(void *dest, char val, size_t count);
	CRT_API unsigned short* memsetw(unsigned short *dest, unsigned short val, size_t count);
	CRT_API int memcmp(const void *s1, const void *s2, size_t n);
	CRT_API void* memmove(void *dest, void *src, size_t n);
	CRT_API void* memchr(const void *s, int c, size_t n);
	CRT_API void* malloc(size_t size);
	CRT_API void* malloc_aligned(size_t size, size_t alignment);
	CRT_API void* realloc(void * ptr, size_t size);
	CRT_API void* calloc(size_t nmemb, size_t size);
	CRT_API void free(void *p);
#ifdef  __cplusplus
}
#endif
