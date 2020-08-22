/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the libintl for msvc.
 * No warranty is given. You can use this at your own risk.
 */

#include <minwindef.h>
#include <wchar.h>
#include <winapi.h>
#include <memory.h>
#include "config.h"
#include "../intl/gettextP.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <debug.h>
#endif

extern DLL_EXPORTED
const wchar_t *libintl_dcwgettext (const char *domainname, const wchar_t *wmsgid, int category);

/**
   This function is wchar_t version of gettext().
   Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
DLL_EXPORTED
const wchar_t *libintl_wgettext (const wchar_t *wmsgid)
{
  return libintl_dcwgettext (NULL, wmsgid, LC_MESSAGES);
}
