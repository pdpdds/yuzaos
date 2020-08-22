/* grep.h - interface to grep driver for searching subroutines.
   Copyright (C) 1992, 1998, 2001, 2007, 2009 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 6) || __STRICT_ANSI__
# define __attribute__(x)
#endif

/* We build specialized legacy "egrep" and "fgrep" programs.
   No program adjusts its behavior according to its argv[0].
   No scripts are provided as an alternative.  Distributors
   are free to do otherwise, but it is their burden to do so.  */
#if !defined(GREP_PROGRAM) && !defined(EGREP_PROGRAM) && !defined(FGREP_PROGRAM)
# define GREP_PROGRAM
#endif

/* The two functions each matcher provides.  */
#define COMPILE_RET void
#define COMPILE_ARGS \
  (char const *pattern, size_t size)
#define EXECUTE_RET size_t
#define EXECUTE_ARGS \
  (char const *buf, size_t size, size_t *match_size, char const *start_ptr)
  /* start_ptr == NULL means the caller is not looking for an exact match.  */

#ifdef GREP_PROGRAM
/* Function definitions.  */
# define COMPILE_FCT(f) static COMPILE_RET f COMPILE_ARGS
# define EXECUTE_FCT(f) static EXECUTE_RET f EXECUTE_ARGS
/* Function pointer types.  */
typedef COMPILE_RET (*compile_fp_t) PARAMS (COMPILE_ARGS);
typedef EXECUTE_RET (*execute_fp_t) PARAMS (EXECUTE_ARGS);

/* grep.c expects the matchers vector to be terminated
   by an entry with a NULL compile, and to contain at least
   an entry named "default". */
extern struct matcher
{
  char name[8];
  compile_fp_t compile;
  execute_fp_t execute;
} const matchers[];
#else /* !GREP_PROGRAM */
/* Straight functions for specialized "egrep" and "fgrep" programs.  */
/* Function definitions.  */
# define COMPILE_FCT(f) COMPILE_RET compile COMPILE_ARGS
# define EXECUTE_FCT(f) EXECUTE_RET execute EXECUTE_ARGS
/* Function prototypes.  */
extern COMPILE_RET compile PARAMS (COMPILE_ARGS);
extern EXECUTE_RET execute PARAMS (EXECUTE_ARGS);
#endif /* GREP_PROGRAM */

/* The following flags are exported from grep for the matchers
   to look at. */
extern int match_icase;		/* -i */
extern int match_words;		/* -w */
extern int match_lines;		/* -x */
extern unsigned char eolbyte;	/* -z */
