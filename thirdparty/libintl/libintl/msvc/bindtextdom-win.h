/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the libintl for msvc.
 * No warranty is given. You can use this at your own risk.
 */
/*
  This file was created referring to Supplements for libintl on Windows.
  http://sourceforge.net/projects/libintl-windows/
*/

#ifndef BINDTEXTDOM_WIN_H
#define BINDTEXTDOM_WIN_H 1

#include <minwindef.h>

extern char *wcs_to_mbs_with_malloc(UINT acp, const wchar_t *wbuffer);
extern wchar_t *mbs_to_wcs_with_malloc(UINT acp, const char *buffer);

/**
   This function is wchar_t version of bindtextdomain().
   Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
DLL_EXPORTED
wchar_t *
libintl_wbindtextdomain (const wchar_t *wdomainname, const wchar_t *wdirname)
{
  char *domainname = NULL;
  char *dirname = NULL;
  char *dirname_bak = NULL;

  domainname = wcs_to_mbs_with_malloc (CP_UTF8, wdomainname);
  if (!domainname)
    {
      return NULL;
    }

  dirname = wcs_to_mbs_with_malloc (CP_UTF8, wdirname);
  if (!dirname)
    {
      free (domainname);
      return NULL;
    }

  dirname_bak = dirname;

  set_binding_values (domainname, (const char**)&dirname, NULL);
  if (!dirname)
    {
      free (dirname_bak);
      free (domainname);
      return NULL;
    }

  free (dirname_bak);
  free (domainname);

  return (wchar_t *) wdirname;
}

/**
   This function is wchar_t version of bind_textdomain_codeset().
   Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
DLL_EXPORTED
wchar_t *
libintl_wbind_textdomain_codeset (const wchar_t *wdomainname, const wchar_t *wcodeset)
{
  char *domainname = NULL;
  char *codeset = NULL;
  char *codeset_bak = NULL;

  domainname = wcs_to_mbs_with_malloc (CP_UTF8, wdomainname);
  if (!domainname)
    {
      return NULL;
    }

  codeset = wcs_to_mbs_with_malloc (CP_UTF8, wcodeset);
  if (!codeset)
    {
      free (domainname);
      return NULL;
    }

  codeset_bak = codeset;

  set_binding_values (domainname, NULL, (const char**)&codeset);
  if (!codeset)
    {
      free (codeset_bak);
      free (domainname);
      return NULL;
    }

  free (codeset_bak);
  free (domainname);

  return (wchar_t *) wcodeset;
}

#endif /* BINDTEXTDOM_WIN_H */

