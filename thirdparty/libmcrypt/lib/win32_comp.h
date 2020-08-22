/* (c) D.Souflis dsouflis@acm.org */

#ifndef _WINSTUBDL_H
#define _WINSTUBDL_H

#ifdef WIN32
# define WIN32DLL_DEFINE __declspec( dllexport)
#else
# define WIN32DLL_DEFINE
#endif

#ifdef WIN32_DLOPEN
#include <windows.h>
#ifndef RTLD_LAZY
# define RTLD_LAZY 1
#endif
void *dlopen(const char *module, int unused);
void* dlsym(void* mo, const char *proc);
void dlclose(void* mo);
#endif

/* defined by ROB */
#define STDC_HEADERS 1
#define SIZEOF_UNSIGNED_INT 4
#define SIZEOF_UNSIGNED_LONG_INT 4
#define SIZEOF_UNSIGNED_SHORT_INT 2
#define SIZEOF_UNSIGNED_CHAR 1
#define HAVE_MEMMOVE 1
#define VERSION "2.5.8"
#define LIBDIR "C:\\windows\\system"

#endif
