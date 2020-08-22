/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the libintl for msvc.
 * No warranty is given. You can use this at your own risk.
 */

#include <minwindef.h>
#include <wchar.h>
#include <winapi.h>
#include <memory.h>
#include <errno.h>
#include "config.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <debug.h>
#endif

/* http://msdn.microsoft.com/en-us/library/aa365247(VS.85).aspx */
#define WCHAR_LIBINTL_MAX_PATH 30000


/**
  convert to char from wchar_t
  Don't forget free() after use this.
*/
inline char *wcs_to_mbs_with_malloc(UINT acp, const wchar_t *wbuffer)
{
  unsigned int num = 0;
  char *buffer = NULL;

  if (wcslen (wbuffer) >= WCHAR_LIBINTL_MAX_PATH)
    {
      errno = ENAMETOOLONG;
      return NULL;
    }

  num = WideCharToMultiByte (acp, 0, wbuffer, -1, NULL, 0, NULL, NULL);
  if (num == 0)
    {
      errno = EACCES;
      return NULL;
    }

  if (num >= WCHAR_LIBINTL_MAX_PATH)
    {
      errno = ENAMETOOLONG;
      return NULL;
    }

  buffer = (char *)malloc (num * sizeof(char));
  if (!buffer)
    {
      errno = ENOMEM;
      return NULL;
    }

  if (0 == WideCharToMultiByte (acp, 0, wbuffer, -1, buffer, num, NULL, NULL))
    {
      free (buffer);
      errno = EACCES;
      return NULL;
    }

  errno = 0;
  return buffer;
}

/**
  convert to wchar_t from char
  Don't forget free() after use this.
*/
inline wchar_t *mbs_to_wcs_with_malloc(UINT acp, const char *buffer)
{
  unsigned int num = 0;
  wchar_t *wbuffer = NULL;

  num = MultiByteToWideChar (acp, 0, buffer, -1, NULL, 0);
  if (num == 0)
    {
      errno = EACCES;
      return NULL;
    }

  if (num >= WCHAR_LIBINTL_MAX_PATH)
    {
      errno = ENAMETOOLONG;
      return NULL;
    }

  wbuffer = (wchar_t *)malloc (num * sizeof(wchar_t));
  if (!buffer)
    {
      errno = ENOMEM;
      return NULL;
    }

  if (0 == MultiByteToWideChar (acp, 0, buffer, -1, wbuffer, num))
    {
      free (wbuffer);
      errno = EACCES;
      return NULL;
    }

  errno = 0;
  return wbuffer;
}

