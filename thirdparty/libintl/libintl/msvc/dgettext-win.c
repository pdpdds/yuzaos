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
   This function is wchar_t version of dgettext().
   Look up MSGID in the DOMAINNAME message catalog of the current
   LC_MESSAGES locale.  */
DLL_EXPORTED
const wchar_t *libintl_dwgettext (const char *domainname, const wchar_t *wmsgid)
{
  return libintl_dcwgettext (domainname, wmsgid, LC_MESSAGES);
}
