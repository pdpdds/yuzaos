#pragma once
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif
	size_t  strnlen(const char* str, size_t  maxlen);
	CRT_API float strtof(const char* string, char** endPtr);
	CRT_API char* strcpy(char* s1, const char* s2);
	CRT_API size_t strlen(const char* str);
	CRT_API int strcmp(const char* str1, const char* str2);
	CRT_API int strncmp(const char* s1, const char* s2, size_t n);
	CRT_API int strncasecmp(const char* s1, const char* s2, size_t n);
	CRT_API int strcasecmp(const char* s1, const char* s2);
	CRT_API char* strstr(const char* in, const char* str);
	CRT_API char* strupr(char* str);
	CRT_API char* strcat(char* dest, const char* src);
	CRT_API char* strncat(char* destination, const char* source, size_t num);
	CRT_API char* strchr(const char* str, int character);
	CRT_API int stricmp(const char* s1, const char* s2);
	CRT_API size_t strlcpy(char* dst, const char* src, size_t maxlen);
	CRT_API size_t strlcat(char* dst, const char* src, size_t maxlen);
	CRT_API char* strpbrk(const char* strSrc, const char* str);
	CRT_API char* strtok(char* s1, const char* delimit);
	CRT_API int strspn(const char* strSrc, const char* str);
	CRT_API char* strncpy(char* string1, const char* string2, size_t count);
	CRT_API int strnicmp(const char* String1, const char* String2, unsigned int Len);
	CRT_API char* strnchr(const char* str, char c, size_t count);
	CRT_API const char* strrchr(const char* String, char const Character);
	CRT_API size_t strxfrm(char* dst, char* src, size_t n);
	CRT_API int strcoll(const char* s1, const char* s2);
	CRT_API double strtod(const char* string, char** endPtr);
	CRT_API size_t strcspn(const char* s1, const char* s2);
	CRT_API unsigned long strtoul(const char* nptr, char** endptr, int base);
	//CRT_API size_t strftime(char* __restrict s,
		//size_t maxsize, const char* __restrict format,
		//const struct tm* __restrict tim_p);

	CRT_API void ftoa_fixed(char* buffer, double value);
	CRT_API void ftoa_sci(char* buffer, double value);
	CRT_API char* strichr(char* p, int c);
	CRT_API int vsnprintf(char* out, size_t size, const char* format, va_list args);
	CRT_API int snprintf(char* out, int size, const char* fmt, ...);
	//CRT_API char* strndup(const char* s, size_t n);
	CRT_API char* strdup(const char* s);
	CRT_API char* strndup(const char* s, size_t n);
	CRT_API int strtoimax(const char* nptr, char** endptr, int base);

	CRT_API __int64 strtoi64(const char* nptr, char** endptr, int 	base);
	CRT_API char* strtok_r(char* s, const char* delim, char** save_ptr);

	CRT_API int stricmp(const char* s1, const char* s2);
	CRT_API int strnicmp(const char* s1, const char* s2, size_t n);

	CRT_API char* strsep(char** stringp, const char* delim);
	CRT_API unsigned __int64 _strtoui64(const char* nptr, char** endptr, int base);
	CRT_API __int64 _atoi64(const char* nptr);

	CRT_API char* strlwr(char* s);
	CRT_API long strtol(const char* nptr, char** endptr, int base);
	//void printf(const char* fmt, ...);

	CRT_API void splitpath(const char* path, char* drv, char* dir, char* name, char* ext);

#ifdef  __cplusplus
}
#endif