/*
 * This source code is public domain.
 *
 * Authors: Rani Assaf <rani@magic.metawire.com>,
 *          Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
 */

#ifndef _STDAFX_H_
#define _STDAFX_H_
#include <minwindef.h>
/* Autoconf detection of stdint/inttypes */
#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
# include "config.h"
# define CONFIG_H_INCLUDED 1
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

/* disable AGC and FILESAVE for all targets for uniformity. */
#define NO_AGC
#define MODPLUG_NO_FILESAVE

#ifdef _WIN32

#ifdef MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4514)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include "string.h"

#define srandom(_seed)  srand(_seed)
#define random()        rand()
#define sleep(_ms)      Sleep(_ms)

inline void ProcessPlugins(int n) {}

#undef strcasecmp
#undef strncasecmp
#define strcasecmp(a,b)     _stricmp(a,b)
#define strncasecmp(a,b,c)  _strnicmp(a,b,c)

#define HAVE_SINF 1

/*#ifndef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif*/

#else

#include "windef.h"
#include "stdint.h"
#include "memory.h"
//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#define strcasecmp(a,b)     stricmp(a,b)
#define strncasecmp(a,b,c)  strnicmp(a,b,c)

/*#ifndef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif*/

#define srandom(_seed)  srand(_seed)
#define random()        rand()
#define sleep(_ms)      ksleep(_ms)

inline LONG MulDiv (long a, long b, long c)
{
/*if (!c) return 0;*/
  return ((uint64_t) a * (uint64_t) b ) / c;
}

#define LPCTSTR LPCSTR
#define lstrcpyn strncpy
#define lstrcpy strcpy
#define lstrcmp strcmp
#define wsprintf sprintf

#define WAVE_FORMAT_PCM 1

#define  GHND   0
#define GlobalFreePtr(p) free((void *)(p))
inline int8_t * GlobalAllocPtr(unsigned int, size_t size)
{
  int8_t * p = (int8_t *) malloc(size);

  if (p != NULL) memset(p, 0, size);
  return p;
}

inline void ProcessPlugins(int n) {}

#ifndef FALSE
#define FALSE	false
#endif

#ifndef TRUE
#define TRUE	true
#endif

#endif /* _WIN32 */

#if defined(_WIN32) || defined(__CYGWIN__) || defined(SKYOS32)
# if defined(MODPLUG_BUILD) && defined(DLL_EXPORT)	/* building libmodplug as a dll for windows */
#   define MODPLUG_EXPORT __declspec(dllexport)
# elif defined(MODPLUG_BUILD) || defined(MODPLUG_STATIC)	/* building or using static libmodplug for windows */
#   define MODPLUG_EXPORT
# else
#   define MODPLUG_EXPORT __declspec(dllimport)			/* using libmodplug dll for windows */
# endif
#elif defined(MODPLUG_BUILD) && defined(SYM_VISIBILITY)
#   define MODPLUG_EXPORT __attribute__((visibility("default")))
#else
#define MODPLUG_EXPORT
#endif

#endif
