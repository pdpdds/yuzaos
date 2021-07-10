#pragma once
#include <stdarg.h>
#include <memory.h>
#include <FileService.h>
#include <sprintf.h>
#include <errno.h>
#include <getenv.h>

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
	CRT_API int vprintf(const char *format, va_list ap);
	CRT_API int vsprintf(char *str, const char *format, va_list ap);
	//int vasprintf(char **ret, const char *format, va_list ap);
	CRT_API long strtol(const char* nptr, char** endptr, int base);
	CRT_API int atoi(const char * str);
	CRT_API double atof(char *p);
	CRT_API long int atol(const char* nptr);
	CRT_API int atob(int *vp, char *p, int base);
	CRT_API int sscanf(const char *, const char *, ...);
	CRT_API int vsscanf(const char* buf, const char* fmt, va_list args);
	CRT_API unsigned long atoul(const char *num);

	CRT_API unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
	CRT_API long simple_strtol(const char *cp, char **endp, unsigned int base);
	CRT_API unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base);
	CRT_API long long simple_strtoll(const char *cp, char **endp, unsigned int base);
	CRT_API void printf(const char* fmt, ...);
	CRT_API unsigned long strtoul(const char* nptr, char** endptr, int base);

	CRT_API void abort();

	char* tmpnam(char* s);
	FILE* tmpfile(void);

#ifdef  __cplusplus
}
#endif



// Function pointer typedef for less typing //
typedef void(__cdecl* _PVFV)(void);

int __cdecl atexit(_PVFV fn);

#ifdef  __cplusplus
extern "C" {
#else
#endif
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
extern int errno;
#ifdef  __cplusplus
}
#endif
