/* Message catalogs for internationalization. "wchar_t" support.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef WLIBINTL_H
#define WLIBINTL_H	1

#ifdef __cplusplus
extern "C"
{
#endif

/* wchar_t support */
#ifdef _INTL_REDIRECT_INLINE
#define WCHAR_LIBINTL_STATIC_INLINE static inline
#elif defined(_MSC_VER)
#define WCHAR_LIBINTL_STATIC_INLINE static _inline
#else
#define WCHAR_LIBINTL_STATIC_INLINE static
#endif

extern const wchar_t *libintl_wgettext (const wchar_t *wmsgid);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *wgettext (const wchar_t *wmsgid)
{
  return libintl_wgettext (wmsgid);
}

extern const wchar_t *libintl_dwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *dwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid)
{
  return libintl_dwgettext (wdomainname, wmsgid);
}

extern const wchar_t *libintl_dcwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid, int category);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *dcwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid, int category)
{
  return libintl_dcwgettext (wdomainname, wmsgid, category);
}

extern const wchar_t *libintl_nwgettext (const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *nwgettext (const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n)
{
  return libintl_nwgettext (wmsgid1, wmsgid2, n);
}

extern const wchar_t *libintl_dnwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *dnwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n)
{
  return libintl_dnwgettext (wdomainname, wmsgid1, wmsgid2, n);
}

extern const wchar_t *libintl_dcnwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n, int category);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *dcnwgettext (const wchar_t *wdomainname, const wchar_t *wmsgid1, const wchar_t *wmsgid2, unsigned long int n, int category)
{
  return libintl_dcnwgettext (wdomainname, wmsgid1, wmsgid2, n, category);
}

extern const wchar_t *libintl_wbindtextdomain (const wchar_t *wdomainname, const wchar_t *wdirname);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *wbindtextdomain (const wchar_t *wdomainname, const wchar_t *wdirname)
{
  return libintl_wbindtextdomain (wdomainname, wdirname);
}

extern const wchar_t *libintl_wbind_textdomain_codeset (const wchar_t *wdomainname, const wchar_t *wcodeset);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *wbind_textdomain_codeset (const wchar_t *wdomainname, const wchar_t *wcodeset)
{
  return libintl_wbind_textdomain_codeset (wdomainname, wcodeset);
}

extern const wchar_t *libintl_wtextdomain (const wchar_t *wdomainname);
WCHAR_LIBINTL_STATIC_INLINE 
const wchar_t *wtextdomain (const wchar_t *wdomainname)
{
  return libintl_wtextdomain (wdomainname);
}

extern void libintl_wfreeres ();

#undef WCHAR_LIBINTL_STATIC_INLINE 

#ifdef __cplusplus
}
#endif

#endif /* wlibintl.h */

