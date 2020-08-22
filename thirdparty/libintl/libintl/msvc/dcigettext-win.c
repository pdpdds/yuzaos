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

extern char *wcs_to_mbs_with_malloc(UINT acp, const wchar_t *wbuffer);
extern void libintl_wtextdomain_freeres ();

/**
   This function is wchar_t version of dcigettext().
   Look up MSGID in the DOMAINNAME message catalog for the current
   CATEGORY locale and, if PLURAL is nonzero, search over string
   depending on the plural form determined by N.  */
DLL_EXPORTED
const wchar_t *libintl_dciwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid1, const wchar_t *wmsgid2,
	    int plural, unsigned long int n, int category)
{
  char *domainname = NULL;
  char *msgid1 = NULL;
  char *msgid2 = NULL;
  char *translated = NULL;
  const wchar_t *retval = NULL;

  if (wmsgid1 == NULL)
    {
      goto end;
    }

  if (wdomainname != NULL)
    {
      domainname = wcs_to_mbs_with_malloc(CP_UTF8, wdomainname);
      if (domainname == NULL)
        {
          goto end;
        }
    }

  msgid1 = wcs_to_mbs_with_malloc(CP_UTF8, wmsgid1);
  if (msgid1 == NULL)
    {
      goto end;
    }

  if (wmsgid2 != NULL)
    {
      msgid2 = wcs_to_mbs_with_malloc(CP_UTF8, wmsgid2);
      if (msgid2 == NULL)
        {
          goto end;
        }
    }

  translated = libintl_dcigettext(domainname, msgid1, msgid2, plural, n, category);
  if (translated == msgid1)
    {
      retval = wmsgid1;
      goto end;
    }
  if (translated == msgid2)
    {
      retval = wmsgid2;
      goto end;
    }

  retval = (const wchar_t *) translated;

end:
  free (domainname);
  free (msgid1);
  free (msgid2);
  return retval;
}

/**
   If you use wgettext, call this instead of libintl_freeres().
   If we want to free all resources we have to do some work at
   program's end.
 */
DLL_EXPORTED
void
libintl_wfreeres ()
{
  libintl_freeres();
  libintl_wtextdomain_freeres();
}
