/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the libintl for msvc.
 * No warranty is given. You can use this at your own risk.
 */
/*
  This file was created referring to Supplements for libintl on Windows.
  http://sourceforge.net/projects/libintl-windows/
*/

#ifndef LOADMSGCAT_WINDOWS_H
#define LOADMSGCAT_WINDOWS_H 1

#include <windows.h>
#include <io.h>

extern wchar_t *mbs_to_wcs_with_malloc(UINT acp, const char *buffer);

int
open_windows_utf8 (const char *pathname8, int flags)
{
  wchar_t *pathname16 = NULL;
  int result = 0;

  if (!pathname8)
    {
      errno = EFAULT;
      return -1;
    }

  pathname16 = mbs_to_wcs_with_malloc (CP_UTF8, pathname8);
  if (!pathname16)
    {
      return -1;
    }

  result = _wopen (pathname16, flags);
  free (pathname16);
  return result;
}

#undef open
#define open(name, flags) open_windows_utf8(name, flags)

#endif

