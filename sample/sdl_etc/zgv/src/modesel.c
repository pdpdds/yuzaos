/* zgv 5.6 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2002 Russell Marks. See README for license details.
 *
 * modesel.c - mode-selection array (and check routine for it).
 *		To add a video mode to zgv, just add an entry here.
 */

#include <stdio.h>
#include "zgv_io.h"
#include "readnbkey.h"
#include "modesel.h"


/* The modes listed must be in order of bit depth (vgadisp.c's
 * fix_to_similar_mode() depends on this), except that 32-bit modes
 * come before 24-bit ones (more on this below).
 * They don't have to be in any order within a given bit depth though.
 *
 * See modesel.h for what the fields here mean.
 */
struct modedesc_tag modedesc[]=
  {
  /* generic VGA modes */
  { G640x480x16,	640,480,4,1,	RK_NO_KEY,	0,'0' },
  { G320x200x256,	320,200,8,1,	RK_NO_KEY,	0,'5' },
  { G320x240x256,	320,240,8,1,	RK_NO_KEY,	0,'6' },
  { G320x400x256,	320,400,8,1,	RK_NO_KEY,	0,'7' },
  { G360x480x256,	360,480,8,1,	RK_NO_KEY,	0,'8' },
  
  /* SVGA 8-bit modes */
  { G640x480x256,	640,480,8,1,	RK_F1,		0,RK_F1 },
  { G800x600x256,	800,600,8,1,	RK_F2,		0,RK_F2 },
  { G1024x768x256,	1024,768,8,1,	RK_F3,		0,RK_F3 },
  { G1280x1024x256,	1280,1024,8,1,	RK_F4,		0,RK_F4 },
  { G1152x864x256,	1152,864,8,1,	RK_NO_KEY,	1,RK_F2 },
  { G1600x1200x256,	1600,1200,8,1,	RK_NO_KEY,	1,RK_F6 },
  
  /* SVGA 15-bit modes */
  { G320x200x32K,	320,200,15,2,	RK_NO_KEY,	0,RK_F5 },
  { G320x240x32K,	320,240,15,2,	RK_NO_KEY,	0,RK_NO_KEY },
  { G640x480x32K,	640,480,15,2,	RK_NO_KEY,	0,RK_F8 },
  { G800x600x32K,	800,600,15,2,	RK_NO_KEY,	0,RK_SHIFT_F1 },
  { G1024x768x32K,	1024,768,15,2,	RK_NO_KEY,	0,RK_SHIFT_F4 },
  { G1280x1024x32K,	1280,1024,15,2,	RK_NO_KEY,	0,RK_SHIFT_F7 },
  { G1152x864x32K,	1152,864,15,2,	RK_NO_KEY,	1,RK_F3 },
  { G1600x1200x32K,	1600,1200,15,2,	RK_NO_KEY,	1,RK_F7 },
  
  /* SVGA 16-bit modes */
  { G320x200x64K,	320,200,16,2,	RK_NO_KEY,	0,RK_F6 },
  { G320x240x64K,	320,240,16,2,	RK_NO_KEY,	0,RK_NO_KEY },
  { G640x480x64K,	640,480,16,2,	RK_NO_KEY,	0,RK_F9 },
  { G800x600x64K,	800,600,16,2,	RK_NO_KEY,	0,RK_SHIFT_F2 },
  { G1024x768x64K,	1024,768,16,2,	RK_NO_KEY,	0,RK_SHIFT_F5 },
  { G1280x1024x64K,	1280,1024,16,2,	RK_NO_KEY,	0,RK_SHIFT_F8 },
  { G1152x864x64K,	1152,864,16,2,	RK_NO_KEY,	1,RK_F4 },
  { G1600x1200x64K,	1600,1200,16,2,	RK_NO_KEY,	1,RK_F8 },
  
  /* SVGA 32-bit modes */
  /* Since zgv treats these as 24-bit (well, what else could it do?)
   * the bitspp field is *24*, not 32. However, bytespp is 4,
   * as that indicates the size of a pixel in video memory.
   * Also, the viewer keys must be the same as those for the 24-bit
   * variants.
   */
  { G320x200x16M32,	320,200,24,4,	RK_NO_KEY,	0,RK_F7 },
  { G320x240x16M32,	320,240,24,4,	RK_NO_KEY,	0,'^' },
  { G640x480x16M32,	640,480,24,4,	RK_NO_KEY,	0,RK_F10 },
  { G800x600x16M32,	800,600,24,4,	RK_NO_KEY,	0,RK_SHIFT_F3 },
  { G1024x768x16M32,	1024,768,24,4,	RK_NO_KEY,	0,RK_SHIFT_F6 },
  { G1280x1024x16M32,	1280,1024,24,4,	RK_NO_KEY,	1,RK_F1 },
  { G1152x864x16M32,	1152,864,24,4,	RK_NO_KEY,	1,RK_F5 },
  { G1600x1200x16M32,	1600,1200,24,4,	RK_NO_KEY,	1,RK_F9 },
  
  /* SVGA 24-bit modes */
  /* These should come after the 32-bit ones, so that these
   * are effectively preferred. (The assumption here is that 24-bit
   * modes are faster than 32-bit ones, and therefore preferable.
   * I welcome feedback on whether this is the Right Thing or not.)
   */
  { G320x200x16M,	320,200,24,3,	RK_NO_KEY,	0,RK_F7 },
  { G320x240x16M,	320,240,24,3,	RK_NO_KEY,	0,'^' },
  { G640x480x16M,	640,480,24,3,	RK_NO_KEY,	0,RK_F10 },
  { G800x600x16M,	800,600,24,3,	RK_NO_KEY,	0,RK_SHIFT_F3 },
  { G1024x768x16M,	1024,768,24,3,	RK_NO_KEY,	0,RK_SHIFT_F6 },
  { G1280x1024x16M,	1280,1024,24,3,	RK_NO_KEY,	1,RK_F1 },
  { G1152x864x16M,	1152,864,24,3,	RK_NO_KEY,	1,RK_F5 },
  { G1600x1200x16M,	1600,1200,24,3,	RK_NO_KEY,	1,RK_F9 },
  
  /* end */
  { 0,			0,0,0,0,	0,		0,0 }
  };


static void cma_abort(struct modedesc_tag *md_ptr,char *complaint)
{
	printf("zgv: modesel.c error (%dx%dx%d), %s.\n",
	md_ptr->width,md_ptr->height,(md_ptr->bytespp==4)?32:md_ptr->bitspp,
	complaint);
	printf(
  "    (If you haven't modified modesel.c, getting such an error most likely\n"
  "    means you need a more recent version of svgalib to run zgv.)\n");

	//20200102
	for (;;);
	//exit(1);
}


void check_modedesc_array(void)
{
int curbits,fs_key,viewer_key,is_tab_key;
struct modedesc_tag *ptr,*ptr2;
vga_modeinfo *vmi;

/* check they're in bit depth order */
for(ptr=modedesc,curbits=0;ptr->mode;ptr++)
  {
  if(ptr->bitspp<curbits)
    cma_abort(ptr,"modes not in bitspp order");
  else
    curbits=ptr->bitspp;
  }

/* check width/height/bitspp/bytespp match svgalib.
 * They're only in the struct as a sanity check really
 * (to give us some (semi-)independent corroboration of the mode's
 * nature, i.e. something more than just the mode number).
 */
for(ptr=modedesc;ptr->mode;ptr++)
  {
  if(ptr->mode<1 || ptr->mode>GLASTMODE)
    cma_abort(ptr,"mode out of range");
  if((vmi=vga_getmodeinfo(ptr->mode))==NULL)
    cma_abort(ptr,"vga_getmodeinfo() failed");
  if(ptr->width!=vmi->width || ptr->height!=vmi->height)
    cma_abort(ptr,"width/height inconsistent");
  if((1<<ptr->bitspp)!=vmi->colors)
    cma_abort(ptr,"bitspp inconsistent");
  
  /* all generic VGA modes except 320x200 are known to have funny
   * bytesperpixel values, skip this check for them.
   */
  if(ptr->mode!=G640x480x16  && ptr->mode!=G320x240x256 &&
     ptr->mode!=G320x400x256 && ptr->mode!=G360x480x256 &&
     ptr->bytespp!=vmi->bytesperpixel)
    cma_abort(ptr,"bytespp inconsistent");
  }

/* check keys used for modes don't clash with each other
 * (we ignore 32-bit modes for this check).
 */
for(ptr=modedesc;ptr->mode;ptr++)
  {
  fs_key=ptr->fs_key;
  viewer_key=ptr->viewer_key;
  is_tab_key=ptr->is_tab_key;
  
  if(ptr->bytespp!=4)
    for(ptr2=modedesc;ptr2->mode;ptr2++)
      /* Lispers should feel right at home with this one :-) */
      if(ptr!=ptr2 && ptr2->bytespp!=4 &&
         ((fs_key!=RK_NO_KEY && fs_key==ptr2->fs_key) ||
          (viewer_key!=RK_NO_KEY &&
           viewer_key==ptr2->viewer_key &&
           is_tab_key==ptr2->is_tab_key)))
        cma_abort(ptr,"key clashes");
  }

/* check all 32-bit ones' keys match those for any matching 24-bit modes.
 * Note that it is not an error to have a 32-bit mode with no matching
 * 24-bit mode; but *if* there is a matching one, it must use the same
 * (viewer) key.
 */
for(ptr=modedesc;ptr->mode;ptr++)
  {
  if(ptr->bytespp==4)
    for(ptr2=modedesc;ptr2->mode;ptr2++)
      if(ptr->width==ptr2->width && ptr->height==ptr2->height &&
         ptr2->bytespp==3 && (ptr->viewer_key!=ptr2->viewer_key ||
         		      ptr->is_tab_key!=ptr2->is_tab_key))
        cma_abort(ptr,"must match 24-bit key");
  }

/* check no >8-bit modes have a filesel key. */
for(ptr=modedesc;ptr->mode;ptr++)
  if(ptr->fs_key!=RK_NO_KEY && ptr->bitspp>8)
    cma_abort(ptr,"bad bitspp for fs mode");

/* check keys are 5..9 or RK_F1..RK_F10 or RK_SHIFT_F1..RK_SHIFT_F8
 * (only up to shift-F8 as only `F1'..`F20' seem to reliably generate
 * known strings) or 0 or ^.
 */
#define TEST_KEY(x) \
  if(x!=RK_NO_KEY && \
     (x<'5' || x>'9') && \
     (x<RK_F1 || x>RK_F10) && \
     (x<RK_SHIFT_F1 || x>RK_SHIFT_F8) && \
     (x!='0') && \
     (x!='^')) \
    cma_abort(ptr,"key not a permitted mode-selecting key")

for(ptr=modedesc;ptr->mode;ptr++)
  {
  TEST_KEY(ptr->fs_key);
  TEST_KEY(ptr->viewer_key);
  }
}
