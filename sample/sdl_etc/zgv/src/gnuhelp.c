/* zgv 5.5 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * gnuhelp.c - the texts `required' in interactive programs by the GPL.
 *		(In case it's not clear, I don't like this `requirement'.
 *		This file implements it in a painfully obnoxious way to
 *		take the piss.)
 */

/* #ifdef DIATRIBE
 *
 * I put `requirement' in quotes as it's not really a requirement.
 * While the "How to Apply These Terms to Your New Programs" appendix
 * instructs you to do it in typical RMS my-way-is-best fashion ;-),
 * the last sentence of paragraph 2c clearly shows that it is NOT
 * an cast-iron requirement.
 *
 * "c) If the modified program normally reads commands interactively
 *  when run, you must cause it, when started running for such
 *  interactive use in the most ordinary way, to print or display an
 *  announcement including an appropriate copyright notice and a
 *  notice that there is no warranty (or else, saying that you provide
 *  a warranty) and that users may redistribute the program under
 *  these conditions, and telling the user how to view a copy of this
 *  License.  (Exception: if the Program itself is interactive but
 *  does not normally print such an announcement, your work based on
 *  the Program is not required to print an announcement.)"
 *
 * So if you *write* a program you don't have to do it, but once a
 * program *has* such an announcement the copyright owner is the
 * only person who can remove it (it can't be removed in derivative
 * works).
 *
 * Are we being sufficiently stupid yet?
 *
 * #endif
 */

#include <stdio.h>
#include "zgv.h"
#include "helppage.h"
#include "gnuhelp.h"

/* from the GPL:
"If the program is interactive, make it output a short notice like this
when it starts in an interactive mode:

    Gnomovision version 69, Copyright (C) 19yy name of author
    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details."
*/

char *gnu_init_page[]=
  {
  " ",
  " ",
  "zgv version " ZGV_VER ", Copyright (C) 1993-2001 Russell Marks",
  "zgv comes with ABSOLUTELY NO WARRANTY; for details",
  "press W in the file selector. This is free software, and you are",
  "welcome to redistribute it under certain conditions; see the",
  "COPYING file for details.",
  /* I'm buggered if I'm going to put the whole bloody GPL in zgv
   * even as a piss-take, ta very much...
   */
  " ",
  "(To disable this stupid message, put gnulitically-correct off",
  "in a zgv config file.)",
  ""	/* end */
  };

char *gnu_warranty_page[]=
  {
  "zgv " ZGV_VER ", Copyright (C) 1993-2001 Russell Marks",
  " ",
  "This program is free software; you can redistribute it and/or",
  "modify it under the terms of the GNU General Public License as",
  "published by the Free Software Foundation; either version 2",
  "of the License, or (at your option) any later version.",
  " ",
  "This program is distributed in the hope that it will be useful,",
  "but WITHOUT ANY WARRANTY; without even the implied",
  "warranty of MERCHANTABILITY or FITNESS FOR A",
  "PARTICULAR PURPOSE. See the GNU General Public License",
  "for more details.",
  " ",
  "You should have received a copy of the GNU General Public",
  "License along with this program; if not, write to the Free",
  "Software Foundation, Inc., 675 Mass Ave, Cambridge,",
  "MA 02139, USA.",
  ""	/* end */
  };


/* THESE SHOULD NEVER EVER BE CALLED UNLESS gnulitically_correct IS SET.
 * (For the sake of everyone's sanity...)
 */

void gnu_init_help(int ttyfd)
{
showhelp(ttyfd,"- AM I STUPID AND ANNOYING YET? -",gnu_init_page);
}

void gnu_warranty_help(int ttyfd)
{
showhelp(ttyfd,"- NO WABBITS^H^H^H^H^HRRANTY -",gnu_warranty_page);
}
