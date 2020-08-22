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

static wchar_t *libintl_current_wdomainname = NULL;

extern char *wcs_to_mbs_with_malloc(UINT acp, const wchar_t *wbuffer);
extern wchar_t *mbs_to_wcs_with_malloc(UINT acp, const char *buffer);

/**
   This function is wchar_t version of textdomain().
   Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
DLL_EXPORTED
wchar_t *
libintl_wtextdomain (const wchar_t *wdomainname)
{
  char *domainname = NULL;
  char *new_domainname = NULL;
  wchar_t *new_wdomainname = NULL;

  domainname = wcs_to_mbs_with_malloc (CP_UTF8, wdomainname);
  if (!domainname)
    {
      return NULL;
    }

  new_domainname = libintl_textdomain (domainname);
  if (!new_domainname)
    {
      free (domainname);
      return NULL;
    }
  if (new_domainname == domainname)
    {
      free (domainname);
      return (wchar_t *) wdomainname;
    }

  new_wdomainname = mbs_to_wcs_with_malloc (CP_UTF8, domainname);
  if (!new_wdomainname)
    {
      free (domainname);
      return NULL;
    }
  free (libintl_current_wdomainname);

  libintl_current_wdomainname = new_wdomainname;

  free (domainname);
  return (wchar_t *) libintl_current_wdomainname;
}

/**
   free resource at program's end.
 */
void
libintl_wtextdomain_freeres ()
{
  free (libintl_current_wdomainname);
}

