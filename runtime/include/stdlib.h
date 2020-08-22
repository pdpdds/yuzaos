#pragma once
#include <memory.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct _div_t
{
    int quot;
    int rem;
} div_t;

typedef struct _ldiv_t
{
    long quot;
    long rem;
} ldiv_t;

div_t div(int num, int denom);

#ifdef __cplusplus
extern "C" {
#endif
	int exit(int errorCode);
#ifdef __cplusplus
}
#endif