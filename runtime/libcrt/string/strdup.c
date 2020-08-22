#include "strdup.h"
#include "memory.h"
#include "string.h"

extern  void* malloc(u32int sz);

char*	strndup(const char *s, size_t n)
{
char *result;
result = (char*)malloc(n + 1);
memcpy(result, s, n + 1);
result[n] = 0;
return result;
}

char*	strdup(const char *s)
{
	char *result;
	int len = strlen(s);
	result = (char*)malloc(len + 1);
	memcpy(result, s, len + 1);
	return result;
}