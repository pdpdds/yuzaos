/* Message catalogs for internationalization.
   Copyright (C) 1995-1997, 2000-2012 Free Software Foundation, Inc.

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

/* Support for format strings with positions in *printf(), following the
   POSIX/XSI specification.
   Note: These replacements for the *printf() functions are visible only
   in source files that #include <libintl.h> or #include "gettext.h".
   Packages that use *printf() in source files that don't refer to _()
   or gettext() but for which the format string could be the return value
   of _() or gettext() need to add this #include.  Oh well.  */

#ifndef _LIBINTL_PRINTF_H
#define _LIBINTL_PRINTF_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>

/* Get va_list.  */
#if (defined __STDC__ && __STDC__) || defined __cplusplus || defined _MSC_VER
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#if !(defined fprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef fprintf
#define fprintf libintl_fprintf
extern int fprintf (FILE *, const char *, ...);
#endif
#if !(defined vfprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef vfprintf
#define vfprintf libintl_vfprintf
extern int vfprintf (FILE *, const char *, va_list);
#endif

#if !(defined printf && defined _GL_STDIO_H) /* don't override gnulib */
#undef printf
#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__ || defined __MINGW32__
/* Don't break __attribute__((format(printf,M,N))).
   This redefinition is only possible because the libc in NetBSD, Cygwin,
   mingw does not have a function __printf__.
   Alternatively, we could have done this redirection only when compiling with
   __GNUC__, together with a symbol redirection:
       extern int printf (const char *, ...)
              __asm__ (#__USER_LABEL_PREFIX__ "libintl_printf");
   But doing it now would introduce a binary incompatibility with already
   distributed versions of libintl on these systems.  */
# define libintl_printf __printf__
#endif
#define printf libintl_printf
extern int printf (const char *, ...);
#endif
#if !(defined vprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef vprintf
#define vprintf libintl_vprintf
extern int vprintf (const char *, va_list);
#endif

#if !(defined sprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef sprintf
#define sprintf libintl_sprintf
extern int sprintf (char *, const char *, ...);
#endif
#if !(defined vsprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef vsprintf
#define vsprintf libintl_vsprintf
extern int vsprintf (char *, const char *, va_list);
#endif

#if !(defined snprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef snprintf
#define snprintf libintl_snprintf
extern int snprintf (char *, size_t, const char *, ...);
#endif
#if !(defined vsnprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef vsnprintf
#define vsnprintf libintl_vsnprintf
extern int vsnprintf (char *, size_t, const char *, va_list);
#endif

#if !(defined asprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef asprintf
#define asprintf libintl_asprintf
extern int asprintf (char **, const char *, ...);
#endif
#if !(defined vasprintf && defined _GL_STDIO_H) /* don't override gnulib */
#undef vasprintf
#define vasprintf libintl_vasprintf
extern int vasprintf (char **, const char *, va_list);
#endif

#undef fwprintf
#define fwprintf libintl_fwprintf
extern int fwprintf (FILE *, const wchar_t *, ...);
#undef vfwprintf
#define vfwprintf libintl_vfwprintf
extern int vfwprintf (FILE *, const wchar_t *, va_list);

#undef wprintf
#define wprintf libintl_wprintf
extern int wprintf (const wchar_t *, ...);
#undef vwprintf
#define vwprintf libintl_vwprintf
extern int vwprintf (const wchar_t *, va_list);

#undef swprintf
#define swprintf libintl_swprintf
extern int swprintf (wchar_t *, size_t, const wchar_t *, ...);
#undef vswprintf
#define vswprintf libintl_vswprintf
extern int vswprintf (wchar_t *, size_t, const wchar_t *, va_list);

#ifdef __cplusplus
}
#endif

#endif /* _LIBINTL_PRINTF_H */
