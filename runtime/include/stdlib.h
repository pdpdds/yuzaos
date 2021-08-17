#pragma once
#include <memory.h>
#include <string.h>

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



#ifdef __cplusplus
extern "C" {
#endif
    div_t div(int num, int denom);
	int exit(int errorCode);
#ifdef __cplusplus
}

// Sizes for buffers used by the _makepath() and _splitpath() functions.
// note that the sizes include space for 0-terminator
#define _MAX_PATH   260 // max. length of full pathname
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component
#endif