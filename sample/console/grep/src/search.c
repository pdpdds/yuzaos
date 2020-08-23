/* search.c - searching subroutines using dfa, kwset and regex for grep.
   Copyright 1992, 1998, 2000, 2007, 2009 Free Software Foundation, Inc.

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

/* Written August 1992 by Mike Haertel. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>

#include "mbsupport.h"
#ifdef MBS_SUPPORT
/* We can handle multibyte strings. */
# include <wchar.h>
# include <wctype.h>
#endif

#include "system.h"
#include "grep.h"
#ifndef FGREP_PROGRAM
# include <regex.h>
# include "dfa.h"
#endif
#include "kwset.h"
#include "error.h"
#include "xalloc.h"
#ifdef HAVE_LIBPCRE
# include <pcre.h>
#endif

#define NCHAR (UCHAR_MAX + 1)

/* For -w, we also consider _ to be word constituent.  */
#define WCHAR(C) (ISALNUM(C) || (C) == '_')

/* KWset compiled pattern.  For Ecompile and Gcompile, we compile
   a list of strings, at least one of which is known to occur in
   any string matching the regexp. */
static kwset_t kwset;

static void
kwsinit (void)
{
  static char trans[NCHAR];
  int i;

  if (match_icase)
    for (i = 0; i < NCHAR; ++i)
      trans[i] = TOLOWER (i);

  if (!(kwset = kwsalloc (match_icase ? trans : (char *) 0)))
    error (2, 0, _("memory exhausted"));
}

#ifndef FGREP_PROGRAM
/* DFA compiled regexp. */
static struct dfa dfa;

/* The Regex compiled patterns.  */
static struct patterns
{
  /* Regex compiled regexp. */
  struct re_pattern_buffer regexbuf;
  struct re_registers regs; /* This is here on account of a BRAIN-DEAD
			       Q@#%!# library interface in regex.c.  */
} patterns0;

struct patterns *patterns;
size_t pcount;

void
dfaerror (char const *mesg)
{
  error (2, 0, mesg);
}

/* Number of compiled fixed strings known to exactly match the regexp.
   If kwsexec returns < kwset_exact_matches, then we don't need to
   call the regexp matcher at all. */
static int kwset_exact_matches;

/* If the DFA turns out to have some set of fixed strings one of
   which must occur in the match, then we build a kwset matcher
   to find those strings, and thus quickly filter out impossible
   matches. */
static void
kwsmusts (void)
{
  struct dfamust const *dm;
  char const *err;

  if (dfa.musts)
    {
      kwsinit ();
      /* First, we compile in the substrings known to be exact
	 matches.  The kwset matcher will return the index
	 of the matching string that it chooses. */
      for (dm = dfa.musts; dm; dm = dm->next)
	{
	  if (!dm->exact)
	    continue;
	  ++kwset_exact_matches;
	  if ((err = kwsincr (kwset, dm->must, strlen (dm->must))) != 0)
	    error (2, 0, err);
	}
      /* Now, we compile the substrings that will require
	 the use of the regexp matcher.  */
      for (dm = dfa.musts; dm; dm = dm->next)
	{
	  if (dm->exact)
	    continue;
	  if ((err = kwsincr (kwset, dm->must, strlen (dm->must))) != 0)
	    error (2, 0, err);
	}
      if ((err = kwsprep (kwset)) != 0)
	error (2, 0, err);
    }
}
#endif /* !FGREP_PROGRAM */

#ifdef MBS_SUPPORT
/* This function allocate the array which correspond to "buf".
   Then this check multibyte string and mark on the positions which
   are not single byte character nor the first byte of a multibyte
   character.  Caller must free the array.  */
static char*
check_multibyte_string(char const *buf, size_t size)
{
  char *mb_properties = xmalloc(size);
  mbstate_t cur_state;
  wchar_t wc;
  int i;

  memset(&cur_state, 0, sizeof(mbstate_t));
  memset(mb_properties, 0, sizeof(char)*size);

  for (i = 0; i < size ;)
    {
      size_t mbclen;
      mbclen = mbrtowc(&wc, buf + i, size - i, &cur_state);

      if (mbclen == (size_t) -1 || mbclen == (size_t) -2 || mbclen == 0)
	{
	  /* An invalid sequence, or a truncated multibyte character.
	     We treat it as a single byte character.  */
	  mbclen = 1;
	}
      else if (match_icase)
	{
	  if (iswupper((wint_t)wc))
	    {
	      wc = towlower((wint_t)wc);
	      wcrtomb(buf + i, wc, &cur_state);
	    }
	}
      mb_properties[i] = mbclen;
      i += mbclen;
    }

  return mb_properties;
}
#endif /* MBS_SUPPORT */

#if defined(GREP_PROGRAM) || defined(EGREP_PROGRAM)
#ifdef EGREP_PROGRAM
COMPILE_FCT(Ecompile)
{
  reg_syntax_t syntax_bits = RE_SYNTAX_POSIX_EGREP;
#else
/* No __VA_ARGS__ in C89.  So we have to do it this way.  */
static COMPILE_RET
GEAcompile (char const *pattern, size_t size, reg_syntax_t syntax_bits)
{
#endif /* EGREP_PROGRAM */
  const char *err;
  const char *sep;
  size_t total = size;
  char const *motif = pattern;

#if 0
  if (match_icase)
    syntax_bits |= RE_ICASE;
#endif
  re_set_syntax (syntax_bits);
  dfasyntax (syntax_bits, match_icase, eolbyte);

  /* For GNU regex compiler we have to pass the patterns separately to detect
     errors like "[\nallo\n]\n".  The patterns here are "[", "allo" and "]"
     GNU regex should have raise a syntax error.  The same for backref, where
     the backref should have been local to each pattern.  */
  do
    {
      size_t len;
      sep = memchr (motif, '\n', total);
      if (sep)
	{
	  len = sep - motif;
	  sep++;
	  total -= (len + 1);
	}
      else
	{
	  len = total;
	  total = 0;
	}

      patterns = realloc (patterns, (pcount + 1) * sizeof (*patterns));
      if (patterns == NULL)
	error (2, errno, _("memory exhausted"));
      patterns[pcount] = patterns0;

      if ((err = re_compile_pattern (motif, len,
				    &(patterns[pcount].regexbuf))) != 0)
	error (2, 0, err);
      pcount++;

      motif = sep;
    } while (sep && total != 0);

  /* In the match_words and match_lines cases, we use a different pattern
     for the DFA matcher that will quickly throw out cases that won't work.
     Then if DFA succeeds we do some hairy stuff using the regex matcher
     to decide whether the match should really count. */
  if (match_words || match_lines)
    {
      static char const line_beg_no_bk[] = "^(";
      static char const line_end_no_bk[] = ")$";
      static char const word_beg_no_bk[] = "(^|[^[:alnum:]_])(";
      static char const word_end_no_bk[] = ")([^[:alnum:]_]|$)";
#ifdef EGREP_PROGRAM
# define IF_BK(x, y) (y)
      char *n = xmalloc (sizeof word_beg_no_bk - 1 + size + sizeof word_end_no_bk);
#else
      static char const line_beg_bk[] = "^\\(";
      static char const line_end_bk[] = "\\)$";
      static char const word_beg_bk[] = "\\(^\\|[^[:alnum:]_]\\)\\(";
      static char const word_end_bk[] = "\\)\\([^[:alnum:]_]\\|$\\)";
      int bk = !(syntax_bits & RE_NO_BK_PARENS);
# define IF_BK(x, y) ((bk) ? (x) : (y))
      char *n = xmalloc (sizeof word_beg_bk - 1 + size + sizeof word_end_bk);
#endif /* EGREP_PROGRAM */

      strcpy (n, match_lines ? IF_BK(line_beg_bk, line_beg_no_bk)
			     : IF_BK(word_beg_bk, word_beg_no_bk));
      total = strlen(n);
      memcpy (n + total, pattern, size);
      total += size;
      strcpy (n + total, match_lines ? IF_BK(line_end_bk, line_end_no_bk)
				     : IF_BK(word_end_bk, word_end_no_bk));
      total += strlen (n + total);
      pattern = motif = n;
      size = total;
    }
  else
    motif = NULL;

  dfacomp (pattern, size, &dfa, 1);
  kwsmusts ();

  if (motif)
    free((char *) motif);
}

#ifndef EGREP_PROGRAM
COMPILE_FCT(Gcompile)
{
  return GEAcompile (pattern, size,
		     RE_SYNTAX_GREP | RE_HAT_LISTS_NOT_NEWLINE);
}

COMPILE_FCT(Acompile)
{
  return GEAcompile (pattern, size, RE_SYNTAX_AWK);
}

COMPILE_FCT(Ecompile)
{
  return GEAcompile (pattern, size, RE_SYNTAX_POSIX_EGREP);
}
#endif /* !EGREP_PROGRAM */

EXECUTE_FCT(EGexecute)
{
  register char const *buflim, *beg, *end, *match, *best_match;
  char eol = eolbyte;
  int backref, start, len, best_len;
  struct kwsmatch kwsm;
  size_t i, ret_val;
#ifdef MBS_SUPPORT
  char *mb_properties = NULL;
  if (MB_CUR_MAX > 1)
    {
      if (match_icase)
        {
          char *case_buf = xmalloc(size);
          memcpy(case_buf, buf, size);
	  if (start_ptr)
	    start_ptr = case_buf + (start_ptr - buf);
          buf = case_buf;
        }
      if (kwset)
        mb_properties = check_multibyte_string(buf, size);
    }
#endif /* MBS_SUPPORT */

  buflim = buf + size;

  for (beg = end = buf; end < buflim; beg = end)
    {
      if (!start_ptr)
	{
	  /* We don't care about an exact match.  */
	  if (kwset)
	    {
	      /* Find a possible match using the KWset matcher. */
	      size_t offset = kwsexec (kwset, beg, buflim - beg, &kwsm);
	      if (offset == (size_t) -1)
		goto failure;
	      beg += offset;
	      /* Narrow down to the line containing the candidate, and
		 run it through DFA. */
	      end = memchr(beg, eol, buflim - beg);
	      end++;
#ifdef MBS_SUPPORT
	      if (MB_CUR_MAX > 1 && mb_properties[beg - buf] == 0)
		continue;
#endif
	      while (beg > buf && beg[-1] != eol)
		--beg;
	      if (kwsm.index < kwset_exact_matches)
		goto success;
	      if (dfaexec (&dfa, beg, end - beg, &backref) == (size_t) -1)
		continue;
	    }
	  else
	    {
	      /* No good fixed strings; start with DFA. */
	      size_t offset = dfaexec (&dfa, beg, buflim - beg, &backref);
	      if (offset == (size_t) -1)
		break;
	      /* Narrow down to the line we've found. */
	      beg += offset;
	      end = memchr (beg, eol, buflim - beg);
	      end++;
	      while (beg > buf && beg[-1] != eol)
		--beg;
	    }
	  /* Successful, no backreferences encountered! */
	  if (!backref)
	    goto success;
	}
      else
	{
	  /* We are looking for the leftmost (then longest) exact match.
	     We will go through the outer loop only once.  */
	  beg = start_ptr;
	  end = buflim;
	}

      /* If we've made it to this point, this means DFA has seen
	 a probable match, and we need to run it through Regex. */
      best_match = end;
      best_len = 0;
      for (i = 0; i < pcount; i++)
	{
	  patterns[i].regexbuf.not_eol = 0;
	  if (0 <= (start = re_search (&(patterns[i].regexbuf),
				       buf, end - buf - 1,
				       beg - buf, end - beg - 1,
				       &(patterns[i].regs))))
	    {
	      len = patterns[i].regs.end[0] - start;
	      match = buf + start;
	      if (match > best_match)
		continue;
	      if (start_ptr && !match_words)
		goto assess_pattern_match;
	      if ((!match_lines && !match_words)
		  || (match_lines && len == end - beg - 1))
		{
		  match = beg;
		  len = end - beg;
		  goto assess_pattern_match;
		}
	      /* If -w, check if the match aligns with word boundaries.
		 We do this iteratively because:
		 (a) the line may contain more than one occurence of the
		 pattern, and
		 (b) Several alternatives in the pattern might be valid at a
		 given point, and we may need to consider a shorter one to
		 find a word boundary.  */
	      if (match_words)
		while (match <= best_match)
		  {
		    if ((match == buf || !WCHAR ((unsigned char) match[-1]))
			&& (len == end - beg - 1
			    || !WCHAR ((unsigned char) match[len])))
		      goto assess_pattern_match;
		    if (len > 0)
		      {
			/* Try a shorter length anchored at the same place. */
			--len;
			patterns[i].regexbuf.not_eol = 1;
			len = re_match (&(patterns[i].regexbuf),
					buf, match + len - beg, match - buf,
					&(patterns[i].regs));
		      }
		    if (len <= 0)
		      {
			/* Try looking further on. */
			if (match == end - 1)
			  break;
			match++;
			patterns[i].regexbuf.not_eol = 0;
			start = re_search (&(patterns[i].regexbuf),
					   buf, end - buf - 1,
					   match - buf, end - match - 1,
					   &(patterns[i].regs));
			if (start < 0)
			  break;
			len = patterns[i].regs.end[0] - start;
			match = buf + start;
		      }
		  } /* while (match <= best_match) */
	      continue;
	    assess_pattern_match:
	      if (!start_ptr)
		{
		  /* Good enough for a non-exact match.
		     No need to look at further patterns, if any.  */
		  beg = match;
		  goto success_in_len;
		}
	      if (match < best_match || (match == best_match && len > best_len))
		{
		  /* Best exact match:  leftmost, then longest.  */
		  best_match = match;
		  best_len = len;
		}
	    } /* if re_search >= 0 */
	} /* for Regex patterns.  */
	if (best_match < end)
	  {
	    /* We have found an exact match.  We were just
	       waiting for the best one (leftmost then longest).  */
	    beg = best_match;
	    len = best_len;
	    goto success_in_len;
	  }
    } /* for (beg = end ..) */

 failure:
  ret_val = -1;
  goto out;

 success:
  len = end - beg;
 success_in_len:
  *match_size = len;
  ret_val = beg - buf;
 out:
#ifdef MBS_SUPPORT
  if (MB_CUR_MAX > 1)
    {
      if (match_icase)
        free((char*)buf);
      if (mb_properties)
        free(mb_properties);
    }
#endif /* MBS_SUPPORT */
  return ret_val;
}
#endif /* defined(GREP_PROGRAM) || defined(EGREP_PROGRAM) */

#if defined(GREP_PROGRAM) || defined(FGREP_PROGRAM)
COMPILE_FCT(Fcompile)
{
  char const *beg, *lim, *err;

  kwsinit ();
  beg = pattern;
  do
    {
      for (lim = beg; lim < pattern + size && *lim != '\n'; ++lim)
	;
      if ((err = kwsincr (kwset, beg, lim - beg)) != 0)
	error (2, 0, err);
      if (lim < pattern + size)
	++lim;
      beg = lim;
    }
  while (beg < pattern + size);

  if ((err = kwsprep (kwset)) != 0)
    error (2, 0, err);
}

EXECUTE_FCT(Fexecute)
{
  register char const *beg, *try, *end;
  register size_t len;
  char eol = eolbyte;
  struct kwsmatch kwsmatch;
  size_t ret_val;
#ifdef MBS_SUPPORT
  char *mb_properties = NULL;
  if (MB_CUR_MAX > 1)
    {
      if (match_icase)
        {
          char *case_buf = xmalloc(size);
          memcpy(case_buf, buf, size);
	  if (start_ptr)
	    start_ptr = case_buf + (start_ptr - buf);
          buf = case_buf;
        }
      mb_properties = check_multibyte_string(buf, size);
    }
#endif /* MBS_SUPPORT */

  for (beg = start_ptr ? start_ptr : buf; beg <= buf + size; beg++)
    {
      size_t offset = kwsexec (kwset, beg, buf + size - beg, &kwsmatch);
      if (offset == (size_t) -1)
	goto failure;
#ifdef MBS_SUPPORT
      if (MB_CUR_MAX > 1 && mb_properties[offset+beg-buf] == 0)
	continue; /* It is a part of multibyte character.  */
#endif /* MBS_SUPPORT */
      beg += offset;
      len = kwsmatch.size[0];
      if (start_ptr && !match_words)
	goto success_in_beg_and_len;
      if (match_lines)
	{
	  if (beg > buf && beg[-1] != eol)
	    continue;
	  if (beg + len < buf + size && beg[len] != eol)
	    continue;
	  goto success;
	}
      else if (match_words)
	for (try = beg; len; )
	  {
	    if (try > buf && WCHAR((unsigned char) try[-1]))
	      break;
	    if (try + len < buf + size && WCHAR((unsigned char) try[len]))
	      {
		offset = kwsexec (kwset, beg, --len, &kwsmatch);
		if (offset == (size_t) -1)
		  break;
		try = beg + offset;
		len = kwsmatch.size[0];
	      }
	    else if (!start_ptr)
	      goto success;
	    else
	      goto success_in_beg_and_len;
	  } /* for (try) */
      else
	goto success;
    } /* for (beg in buf) */

 failure:
  ret_val = -1;
  goto out;

 success:
  end = memchr (beg + len, eol, (buf + size) - (beg + len));
  end++;
  while (buf < beg && beg[-1] != eol)
    --beg;
  len = end - beg;
 success_in_beg_and_len:
  *match_size = len;
  ret_val = beg - buf;
 out:
#ifdef MBS_SUPPORT
  if (MB_CUR_MAX > 1)
    {
      if (match_icase)
        free((char*)buf);
      if (mb_properties)
        free(mb_properties);
    }
#endif /* MBS_SUPPORT */
  return ret_val;
}
#endif /* defined(GREP_PROGRAM) || defined(FGREP_PROGRAM) */

#ifdef GREP_PROGRAM
#if HAVE_LIBPCRE
/* Compiled internal form of a Perl regular expression.  */
static pcre *cre;

/* Additional information about the pattern.  */
static pcre_extra *extra;
#endif

COMPILE_FCT(Pcompile)
{
#if !HAVE_LIBPCRE
  error (2, 0, "%s", _("Support for the -P option is not compiled into this --disable-perl-regexp binary"));
#else
  int e;
  char const *ep;
  char *re = xmalloc (4 * size + 7);
  int flags = PCRE_MULTILINE | (match_icase ? PCRE_CASELESS : 0);
  char const *patlim = pattern + size;
  char *n = re;
  char const *p;
  char const *pnul;

  /* FIXME: Remove these restrictions.  */
  if (eolbyte != '\n')
    error (2, 0, _("The -P and -z options cannot be combined"));
  if (memchr(pattern, '\n', size))
    error (2, 0, _("The -P option only supports a single pattern"));

  *n = '\0';
  if (match_lines)
    strcpy (n, "^(");
  if (match_words)
    strcpy (n, "\\b(");
  n += strlen (n);

  /* The PCRE interface doesn't allow NUL bytes in the pattern, so
     replace each NUL byte in the pattern with the four characters
     "\000", removing a preceding backslash if there are an odd
     number of backslashes before the NUL.

     FIXME: This method does not work with some multibyte character
     encodings, notably Shift-JIS, where a multibyte character can end
     in a backslash byte.  */
  for (p = pattern; (pnul = memchr (p, '\0', patlim - p)); p = pnul + 1)
    {
      memcpy (n, p, pnul - p);
      n += pnul - p;
      for (p = pnul; pattern < p && p[-1] == '\\'; p--)
	continue;
      n -= (pnul - p) & 1;
      strcpy (n, "\\000");
      n += 4;
    }

  memcpy (n, p, patlim - p);
  n += patlim - p;
  *n = '\0';
  if (match_words)
    strcpy (n, ")\\b");
  if (match_lines)
    strcpy (n, ")$");

  cre = pcre_compile (re, flags, &ep, &e, pcre_maketables ());
  if (!cre)
    error (2, 0, ep);

  extra = pcre_study (cre, 0, &ep);
  if (ep)
    error (2, 0, ep);

  free (re);
#endif
}

EXECUTE_FCT(Pexecute)
{
#if !HAVE_LIBPCRE
  abort ();
  return -1;
#else
  /* This array must have at least two elements; everything after that
     is just for performance improvement in pcre_exec.  */
  int sub[300];

  int e = pcre_exec (cre, extra, buf, size,
		     start_ptr ? (start_ptr - buf) : 0, 0,
		     sub, sizeof sub / sizeof *sub);

  if (e <= 0)
    {
      switch (e)
	{
	case PCRE_ERROR_NOMATCH:
	  return -1;

	case PCRE_ERROR_NOMEMORY:
	  error (2, 0, _("Memory exhausted"));

	default:
	  abort ();
	}
    }
  else
    {
      /* Narrow down to the line we've found.  */
      char const *beg = buf + sub[0];
      char const *end = buf + sub[1];
      char const *buflim = buf + size;
      char eol = eolbyte;
      if (!start_ptr)
	{
	  /* FIXME: The case when '\n' is not found indicates a bug:
	     Since grep is line oriented, the match should never contain
	     a newline, so there _must_ be a newline following.
	   */
	  if (!(end = memchr (end, eol, buflim - end)))
	    end = buflim;
	  else
	    end++;
	  while (buf < beg && beg[-1] != eol)
	    --beg;
	}

      *match_size = end - beg;
      return beg - buf;
    }
#endif
}

struct matcher const matchers[] = {
  { "default", Gcompile, EGexecute },
  { "grep",    Gcompile, EGexecute },
  { "egrep",   Ecompile, EGexecute },
  { "awk",     Acompile, EGexecute },
  { "fgrep",   Fcompile, Fexecute },
  { "perl",    Pcompile, Pexecute },
  { "", 0, 0 },
};
#endif /* GREP_PROGRAM */
