/*
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004,
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

   This file includes the setup for the file access block for VMS.
   Written by Phillip C. Brisco 8/98.
 */

#include <rms.h>
#include <ssdef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <starlet.h>

#if defined(VMS) && defined(__DECC) /* need function prototype */
# if (__DECC_VER<50790004)           /* have an own one         */
char *alloca(unsigned int);
# else
#  define alloca __ALLOCA
# endif
#endif


struct FAB fab;
struct NAM nam;

int length_of_fna_buffer;
int fab_stat;
char expanded_name[NAM$C_MAXRSS];
char fna_buffer[NAM$C_MAXRSS];
char result_name[NAM$C_MAXRSS];
char final_name[NAM$C_MAXRSS];
int max_file_path_size = NAM$C_MAXRSS;
char *arr_ptr[32767];
