/*
 *
 */

#pragma once

#ifndef _GETTEXT_H_
#define _GETTEXT_H_

#include <windows.h>
#include <stdio.h>
/* #include <tchar.h> */
/* #include <shlwapi.h> */

#include "libintl.h"
#if defined(UNICODE) || defined(_UNICODE)
#ifdef _DEBUG
#pragma comment(lib, "libintlud.lib")
#else
#pragma comment(lib, "libintlu.lib")
#endif
#include "wlibintl.h"
#else
#ifdef _DEBUG
#pragma comment(lib, "libintld.lib")
#else
#pragma comment(lib, "libintl.lib")
#endif
#endif

#if defined(UNICODE) || defined(_UNICODE)
/* if variable string, use _tgettext instead of _(). */
#define _(text) wgettext(L##text)
#define _tgettext wgettext
#define _tdgettext dwgettext
#define _tdcgettext dcwgettext
#define _tngettext nwgettext
#define _tdngettext dnwgettext
#define _tdcngettext dcnwgettext

#define _tbindtextdomain wbindtextdomain
#define _tbind_textdomain_codeset wbind_textdomain_codeset

#define _ttextdomain wtextdomain

#define libintl_tfreeres libintl_wfreeres
#else /* MBCS */
#define _(text) gettext(text)
#define _tgettext gettext
#define _tdgettext dgettext
#define _tdcgettext dcgettext
#define _tngettext ngettext
#define _tdngettext dngettext
#define _tdcngettext dcngettext

#define _tbindtextdomain bindtextdomain
#define _tbind_textdomain_codeset wbind_textdomain_codeset

#define _ttextdomain textdomain

#define libintl_tfreeres libintl_freeres
#endif

#endif /* _GETTEXT_H_ */
