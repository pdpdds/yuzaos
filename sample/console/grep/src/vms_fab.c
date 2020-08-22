     /*
   Copyright (C) 1992, 1997, 1998, 1999, 2000, 2001, 2002, 2004,
   2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

        <vms_fab>

        This macro sets up the file access block and name block for VMS.
        It also does the initial parsing of the input string (resolving
        wildcards,
        if any) and finds all files matching the input pattern.
        The address of the first matching pattern is returned.

        Written by Phillip C. Brisco 8/98.
      */
#include "vms_fab.h"

void
vms_fab (int * argp, char **argvp[])
{
  extern int optind;
  int optout;

  fab = cc$rms_fab;
  nam = cc$rms_nam;

  optout = 0;
  strcpy (fna_buffer, *argvp[optind]);
  length_of_fna_buffer = NAM$C_MAXRSS;

  fab.fab$b_bid = FAB$C_BID;
  fab.fab$b_bln = FAB$C_BLN;
  fab.fab$l_fop = FAB$M_NAM;
  fab.fab$l_nam = &nam;
  fab.fab$l_fna = (char *)&fna_buffer;
  fab.fab$b_fns = length_of_fna_buffer;

  nam.nam$b_bid = NAM$C_BID;
  nam.nam$b_bln = NAM$C_BLN;
  nam.nam$l_esa = (char *)&expanded_name;
  nam.nam$b_ess = NAM$C_MAXRSS;
  nam.nam$l_rsa = (char *)&result_name;
  nam.nam$b_rss = NAM$C_MAXRSS;

  fab_stat = sys$parse (&fab);
  fab_stat = sys$search (&fab);

  if (fab_stat != 65537)
    {
      fprintf (stderr, "No Matches found.\n");
      exit (0);
    }

  /*
     While we find matching patterns, continue searching for more.
   */
  while (fab_stat == 65537)
    {
      /*
         Allocate memory for the filename
       */
      arr_ptr[optout] = alloca (max_file_path_size + 1);

      strcpy (arr_ptr[optout], result_name);

      /*
         If we don't tack on a null character at the end of the
         filename,
         we can get partial data which is still there from the last
         sys$search command.
       */
      arr_ptr[optout][nam.nam$b_dev +
		      nam.nam$b_dir +
		      nam.nam$b_name +
		      nam.nam$b_type +
		      nam.nam$b_ver] = '\0';

      fab_stat = sys$search (&fab);
      optout++;
    }

  optout--;

  /* Return a pointer to the beginning of memory that has the expanded
     filenames.
   */
  *argp = optout;
  *argvp = arr_ptr;

}
