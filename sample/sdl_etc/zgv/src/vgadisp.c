/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * vgadisp.c - VGA-specific display routines.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>		/* for pow() */
//#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
//#include <sys/file.h>
//#include <sys/time.h>
#include "zgv_io.h"
#include "zgv.h"
#include "magic.h"
#include "readgif.h"
#include "readjpeg.h"
#include "readpnm.h"
#include "readbmp.h"
#include "readpng.h"
#include "readtga.h"
#include "readpcx.h"
#include "readxvpic.h"
#include "readmrf.h"
#include "readxbm.h"
#include "readxpm.h"
#include "readpcd.h"
#include "readtiff.h"
#include "readprf.h"
#include "readnbkey.h"
#include "helppage.h"
#include "3deffects.h"
#include "rc_config.h"
#include "rcfile.h"
#include "mousecur.h"
#include "rbmenu.h"
#include "modesel.h"
#include "vgadisp.h"


/* zgv.c initialises these */
int curvgamode;
int zoom,virtual;
int vkludge;
int pixelsize;		/* in bytes - 1 for 8-bit, 3 for 24-bit */
int pic_incr;		/* after view, 0=stay, -1=prev, 1=next */

/* for right-buttom menu */
static int rb_ignore_first_left_click=0;
static unsigned char *rb_save=NULL;
static int rb_save_width,rb_save_height;

char *viewerhelp[]=
  {
  "? (question mark)\\this help page",
  "/\\show video mode selection help",
  " ",
  "v\\toggle smoothing in virtual/zoom modes",
  "comma/dot\\contrast down/up",
  "less-than/greater-than\\brightness down/up",
  "semicolon\\reset contrast and brightness",
  "z\\toggle zoomed mode",
  "r\\rotate clockwise 90 degrees",
  "m\\mirror",
  "f\\flip",
  "d\\double size of picture",
  "D\\halve size of picture",
  "Esc or x\\exit the viewer",
  " ",
  "hjkl or qaop keys scroll around the picture",
  "HJKL or QAOP (also the cursor keys) scroll in bigger steps",
  ""	/* end */
  };

char *viewermodeshelp[]=
  {
  "Generic VGA modes:",
  "5 - 320x200x8\\6 - 320x240x8\\",
  "7 - 320x400x8\\8 - 360x480x8\\0 - 640x480x4",
  " ",
  "SVGA modes (try these first):",
  "F1 - 640x480x8\\F2 - 800x600x8\\F3 - 1024x768x8",
  "F4 - 1280x1024x8\\F5 - 320x200x15\\F6 - 320x200x16",
  "F7 - 320x200x24\\F8 - 640x480x15\\F9 - 640x480x16",
  "F10 - 640x480x24\\SF1 - 800x600x15\\SF2 - 800x600x16",
  "SF3 - 800x600x24\\SF4 - 1024x768x15\\SF5 - 1024x768x16",
  "SF6 - 1024x768x24\\SF7 - 1280x1024x15\\SF8 - 1280x1024x16",
  "Tab-F1 - 1280x1024x24\\Tab-F2 - 1152x864x8\\Tab-F3 - 1152x864x15",
  "Tab-F4 - 1152x864x16\\Tab-F5 - 1152x864x24\\Tab-F6 - 1600x1200x8",
  "Tab-F7 - 1600x1200x15\\Tab-F8 - 1600x1200x16\\Tab-F9 - 1600x1200x24",
  " ",
  "Or to browse modes:\\[ - lower-res mode\\] - higher-res mode",
  ""	/* end */
  };


struct rbm_data_tag viewer_menu_data[]=
  {
  /* 31 chars max for label text */
  {1, "Exit viewer",		RK_ESC		},
  
  /* stuff in draw_rb_menu which sets the active field of these
   * mode buttons assumes they are in *exactly* this order/position.
   * So don't change anything before the "Normal" button without
   * thinking twice... or more. :-)
   *
   * modes less than 480 pixels high are omitted here since
   * the mouse menu is disabled on those.
   */
  {1, "360x480x8",		'8'		},
  {1, "640x480x4",		'0'		},
  {1, "640x480x8",		RK_F1		},
  {1, "800x600x8",		RK_F2		},
  {1, "1024x768x8",		RK_F3		},
  {1, "640x480x15",		RK_F8		},
  {1, "640x480x16",		RK_F9		},
  {1, "640x480x24",		RK_F10		},
  {1, "800x600x15",		RK_SHIFT_F1	},
  {1, "800x600x16",		RK_SHIFT_F2	},
  {1, "800x600x24",		RK_SHIFT_F3	},
  {1, "1024x768x15",		RK_SHIFT_F4	},
  {1, "1024x768x16",		RK_SHIFT_F5	},
  {1, "1024x768x24",		RK_SHIFT_F6	},
  {1, "Next file",		RK_ENTER	},
  {1, " ...tag first",		' '		},
  /* no gap thing here, due to split between columns */
  
  {1, "Normal size",		'n'		},
  {1, "Zoom on/off",		'z'		},
  {1, " ...smooth on/off",	'v'		},
  {1, "Scale up 1",		's'		},
  {1, "Scale down 1",		'S'		},
  {1, "Scale up x2",		'd'		},
  {1, "Scale down x2",		'D'		},
  {1, " ...smooth on/off",	'i'		},
  {1, "Normal orient'n",	128+'n'		},
  {1, "Save orient'n",		128+'s'		},
  {1, "Use last orient'n",	128+'o'		},
  {1, "Rotate c/w",		'r'		},
  {1, "Rotate anti-c/w",	'R'		},
  {1, "Mirror",			'm'		},
  {1, "Flip",			'f'		},
  /* this next one seems bizarre, but it's to the right of "next file" */
  {1, "Previous file",		127		},
  {1, "4-bit grey/colour",	'c'		},
  
  {0,"",0}
  };


unsigned char vkcache[32768],vkcache_valid[32768];


#define MOVSTP 10    /* for q,a,o,p panning */
#define BIGSTP 100   /* for Q,A,O,P panning (i.e. with shift) */

#define FIX_TO_EIGHT_BIT	1
#define FIX_TO_HIGHCOLOUR	2

byte *theimage;
int width,height,numcols;
byte *image_palette;

double contrast=1.0;  /* note that double contrast is in fact 2 :-) */
int brightness=0;
double picgamma=1.0;
double initial_picgamma=1.0;	/* value set by `4' */
int scaling=1,interp=0,inextpix=1;
int scrnwide,scrnhigh,scrnpixelsize,scrncols;

int palr[256],palg[256],palb[256];
byte palr64[256],palg64[256],palb64[256], palt[256];
int palrgb[768];
unsigned char pal32_no_bc[768];

unsigned char dither16_greylookup[256];
unsigned char dither16_rgb[768];

int first_repeat;
int loading_file_type;
int saved_px,saved_py;		/* for persistance in cfg.repeat_timer mode */

/* grey values for rgb - NTSC uses these (adding up to 1000) */
static int grey_red=299,grey_green=587,grey_blue=114;

int sgres;

static int override_zoom_clear=0;


/* Scary orientation stuff
 * -----------------------
 *
 * There are eight possible orientations (0 is the original image):
 *                             _____     _____ 
 *    _______     _______     |    a|   |    b|
 *   |a      |   |b      |    |     |   |     |
 *   |   0   |   |   1   |    |  4  |   |  5  |
 *   |______b|   |______a|    |b____|   |a____|
 *    _______     _______      _____     _____ 
 *   |      b|   |      a|    |b    |   |a    |
 *   |   2   |   |   3   |    |     |   |     |
 *   |a______|   |b______|    |  6  |   |  7  |
 *                            |____a|   |____b|
 *
 * That gives us these changes in orientation state for each of the
 * orientation-changing operations (rotate, mirror, flip):
 *
 * 		rot-cw	rot-acw	mirror	flip
 * 0 to...	4	5	3	2
 * 1 to...	5	4	2	3
 * 2 to...	7	6	1	0
 * 3 to...	6	7	0	1
 * 4 to...	1	0	7	6
 * 5 to...	0	1	6	7
 * 6 to...	2	3	5	4
 * 7 to...	3	2	4	5
 */

int orient_state_rot_cw[8] ={4,5,7,6,1,0,2,3};
int orient_state_rot_acw[8]={5,4,6,7,0,1,3,2};
int orient_state_mirror[8] ={3,2,1,0,7,6,5,4};
int orient_state_flip[8]   ={2,3,0,1,6,7,4,5};

int orient_override=0;		/* override orientation (used by Alt-s) */
int orient_override_state=0;


#define GET32BITCOLOUR(r,g,b) (b|(g<<8)|(r<<16))


/* indirection needed to allow high-colour brightness/contrast */
typedef void (*eventuallyfunc)(byte *,int,int,int);
eventuallyfunc eventuallydrawscansegment=NULL;
int doing_hicol_bc=0;	/* non-zero if doing hi-col b/c in current redraw */


/* prototypes */
int readpicture(char *giffn,hffunc howfarfunc,int show_dont_tell,
	int quick,int *real_width,int *real_height);
int has_highcolour_mode(void);
void fix_to_similar_mode(int modetype);
void aborted_file_cleanup(void);
int is_this_file_jpeg(void);
void makerealpal(void);
void filterpal(byte *palette);
int dimmer(int a);
int contrastup(int cp);
int apply_gamma(int val);
int get_mode_width(int vm);
int get_mode_height(int vm);
int get_mode_pixelbytes(int vm);
int get_mode_numcols(int vm);
void setmode_or_clear(int newmode);
void init_vkludge_cache(void);
void graphicson(void);
void graphicsoff(void);
int mode_is_usable_now(int modenum);
int showgif(char *filename,byte *palette);
static void draw_rb_menu(void);
static void undraw_rb_menu(void);
static int rb_menu_event(int *keyp);
void setpalvec(int start,int num,int *pal);
void redrawgif(int px, int py, int npx, int npy);
void eventuallydrawscansegment_without_bc(byte *ptr, int x, int y, int len);
void eventuallydrawscansegment_with_bc(byte *ptr, int x, int y, int len);
void dither16scansegment(unsigned char *ptr,int x,int y,int len);
int getvpix(int px, int py, int x, int y);
void drawzoomedgif(void);
void fx_mirror(void);
void fx_flip(void);
void fx_rot(void);
int closest(int r, int g, int b);
void samecentre(int *ppx, int *ppy, int newscale, int oldpx, int oldpy, int oldscale);
void show_dimensions(int px, int py, int scaling);
void orient_change_state(int from,int to,int clear_if_rot);
int animate_gif(int orient_state);



/* to read a picture only, set show_dont_tell=0.
 * If you're only reading the picture, not showing it, then:
 * 1. picture is returned in global vars. 'theimage' and 'image_palette'
 * 2. you have to set pixelsize=whatever and restore it afterwards.
 * 3. you have to free() both theimage and image_palette when finished.
 *
 * The `quick' arg sets whether or not to ask the jpeg decoder for a
 * rough image. This is useful when doing thumbnails, where we don't
 * need the accuracy, but you shouldn't use it elsewhere.
 *
 * `real_width' and `real_height' give the true width/height of the
 * image (useful if using `quick', and a smaller image was returned).
 * Set them to NULL if you don't care.
 */
int readpicture(char *giffn,hffunc howfarfunc,int show_dont_tell,
	int quick,int *real_width,int *real_height)
{
int result=0;
PICINFO ginfo;
byte *palette=NULL;
int realset=0;

first_repeat=1;
gif_delaycount=0;

do
  {
  ginfo.numcols=256;	/* only changes for GIF files */
  theimage=NULL;
  if(!first_repeat) howfarfunc=NULL;
  
  switch(loading_file_type=magic_ident(giffn))
    {
    case _IS_GIF:
      /* part of the New Strategy (tm); always use 8-bit for GIFs.
       * fix the mode to a similar 8-bit one if possible.
       */
      if(show_dont_tell) fix_to_similar_mode(FIX_TO_EIGHT_BIT);
      result=read_gif_file(giffn,howfarfunc,&theimage,&palette,
      			&pixelsize,&ginfo);
      height=ginfo.height; width=ginfo.width;
      break;
    
    case _IS_JPEG:
      /* don't *always* use 24-bit for JPEGs... check for 15/16/24-bit
       * video modes before doing so, as well as the config.
       */
      if(show_dont_tell)
        {
        pixelsize=has_highcolour_mode()?3:1;
        if(pixelsize==3 && cfg.jpeg24bit==0)
          pixelsize=1;
        }
  
      theimage=NULL;
      result=read_JPEG_file(giffn,howfarfunc,&palette,
      		quick,real_width,real_height);
      realset=1;
      
      /* if error, then palette has already been nuked, so only
       * need to deal with the image.
       */
      if(theimage!=NULL && result!=_PIC_OK) { free(theimage); theimage=NULL; }
      
      break;
      
    case _IS_PNG:
      if(show_dont_tell)
        {
        pixelsize=has_highcolour_mode()?3:1;
        if(pixelsize==3 && cfg.jpeg24bit==0)
          pixelsize=1;
        }
  
      theimage=NULL;
      result=read_png_file(giffn,howfarfunc,&palette);
      
      /* if error, then palette has already been nuked, so only
       * need to deal with the image.
       */
      if(theimage!=NULL && result!=_PIC_OK)
        {
        if(cfg.errignore)
          result=_PIC_OK;
        else
          { free(theimage); theimage=NULL; }
        }
        
      break;
      
    case _IS_PNM: case _IS_BMP: case _IS_TGA: case _IS_PCX:
    case _IS_XVPIC: case _IS_MRF: case _IS_PCD:
    case _IS_XBM: case _IS_XPM: case _IS_TIFF:
    case _IS_PRF:
      if(show_dont_tell) pixelsize=has_highcolour_mode()?3:1;
      switch(loading_file_type)
        {
        case _IS_PNM:
          result=read_pnm_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
        case _IS_TIFF:
          result=read_tiff_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
        case _IS_BMP:
          result=read_bmp_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
        case _IS_TGA:
          result=read_tga_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
	case _IS_PCX:
          result=read_pcx_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
	case _IS_XVPIC:
          result=read_xvpic(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
	case _IS_MRF:
          result=read_mrf_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
        case _IS_PRF:
          result=read_prf_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
	case _IS_XBM:
          result=read_xbm_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
#ifdef PCD_SUPPORT
        case _IS_PCD:
          result=read_pcd_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
#endif
        case _IS_XPM:
          result=read_xpm_file(giffn,howfarfunc,&theimage,&palette,&pixelsize,
                  &ginfo); break;
        }
      
      width=ginfo.width;
      height=ginfo.height;
      if(result!=_PIC_OK)
        {
        gif_delaycount=0;	/* ignore any animation stuff */
        
        if(theimage!=NULL) { free(theimage); theimage=NULL; }
        if(palette!=NULL)  { free(palette);  palette=NULL; }
        }
      else
        if(loading_file_type==_IS_TGA && tga_need_flip)
          fx_flip();	/* right way up! */
      break;

    
    /* if they voted for "none of the above"... */
    default:
      return(_PICERR_BADMAGIC);
    }
  
  
  if(!realset)
    {
    if(real_width) *real_width=width;
    if(real_height) *real_height=height;
    realset=1;
    }
  
  if(show_dont_tell)
    {
    if(pixelsize==3)
      fix_to_similar_mode(FIX_TO_HIGHCOLOUR);
    else
      fix_to_similar_mode(FIX_TO_EIGHT_BIT);
  
    if(result==_PIC_OK)
      {
      numcols=ginfo.numcols;
      sgres=showgif(giffn,palette);
      free(theimage);
      free(palette);
      }
    }
  else
    image_palette=palette;
  }
while(show_dont_tell && cfg.repeat_timer && result==_PIC_OK && sgres==0);

return(result);
}


/* modes aren't counted if they're locked out */
int has_highcolour_mode()
{
struct modedesc_tag *md_ptr;

if(cfg.hicolmodes) return(1);	/* forces positive response */

for(md_ptr=modedesc;md_ptr->mode;md_ptr++)
  if(md_ptr->bitspp>=15 &&
     vga_hasmode(md_ptr->mode) && cfg.mode_allowed[md_ptr->mode])
    return(1);

return(0);
}


/* since we've already tested if it's possible, this can't fail */
void fix_to_similar_mode(int modetype)
{
int f,newmode,numpixels,pixdiff,newnp,newdiff,mode_ok;
vga_modeinfo *vminfo;
struct modedesc_tag *md_ptr;

/* here's the plan;
 * - we try to keep the number of pixels as close as possible:
 *   as this will probably be quite noticeable, *this takes priority*.
 * - we then try to match any 8-bit modes for EIGHT_BIT, or match
 *   24/16/15-bit modes (in that order) for HIGHCOLOUR.
 * - of course, the mode has to be either 8 or 15/16/24-bit as reqd.
 *
 * no support in this for 640x480x4 (16-colour) - because that's
 * a really weird special-case mode, you have to explicitly select it
 * if you want it. However, it sticks in 640x480x4 if selected and
 * modetype is FIX_TO_EIGHT_BIT, because it'd be totally annoying
 * otherwise.
 */

if(curvgamode==G640x480x16 && modetype==FIX_TO_EIGHT_BIT) return;

newmode=-1; pixdiff=(1<<30);
numpixels=get_mode_width(curvgamode)*get_mode_height(curvgamode);

/* account for effect of 'virtual' */
if(curvgamode==G320x400x256 || curvgamode==G360x480x256)
  numpixels*=2;

for(md_ptr=modedesc;md_ptr->mode;md_ptr++)
  {
  f=md_ptr->mode;
  if(!vga_hasmode(f) || cfg.mode_allowed[f]==0 ||
     (vminfo=vga_getmodeinfo(f))==NULL)
    continue;
  
  mode_ok=0;
  if(modetype==FIX_TO_EIGHT_BIT)
    {
    if(vminfo->colors==256) mode_ok=1;
    }
  else
    {
    if(vminfo->colors>256) mode_ok=1;
    }
  
  if(mode_ok==0) continue;
  
  newnp=(vminfo->width)*(vminfo->height);
  if(f==G320x400x256 || f==G360x480x256)
    newnp*=2;   /* account for effect of 'virtual' */
  newdiff=numpixels-newnp;
  if(newdiff<0) newdiff=(-newdiff);
  
  if(newdiff<=pixdiff)
    {
    newmode=f;
    pixdiff=newdiff;
    }
  }

/* we rely on the 15/16/32/24-bit mode number ordering to sort that
 * out for us (hence the `<=' above).
 */
curvgamode=newmode;
virtual=(curvgamode==G320x400x256 || curvgamode==G360x480x256);
}


int mode_ok_for_auto_switch(struct modedesc_tag *md_ptr)
{
static int has640x480=-1;
int f=md_ptr->mode;

if(has640x480==-1)
  has640x480=(vga_hasmode(G640x480x256) && cfg.mode_allowed[G640x480x256]);

if(!vga_hasmode(f) || cfg.mode_allowed[f]==0)
  return(0);

/* don't use 320x200 modes (screwy aspect ratio). */
if(md_ptr->width==320 && md_ptr->height==200)
  return(0);
  
/* don't use 640x480x4 (too odd), or 320x400x8 (aspect ratio again). */
if(f==G640x480x16 || f==G320x400x256)
  return(0);

/* skip 360x480x8 if a 640x480x8 mode exists. */
if(f==G360x480x256 && has640x480)
  return(0);

/* obviously, has to be a compatible depth. */
if((pixelsize==1 && md_ptr->bitspp!=8) ||
   (pixelsize==3 && md_ptr->bitspp==8))
  return(0);

/* otherwise ok */
return(1);
}


void do_auto_mode_fit(void)
{
struct modedesc_tag *md_ptr;
int w,h;
int newmode=-1,newmode_w=(1<<30),newmode_h=(1<<30);
int bigmode=-1,bigmode_w=0,bigmode_h=0;

if(!cfg.automodefit) return;

for(md_ptr=modedesc;md_ptr->mode;md_ptr++)
  {
  if(!mode_ok_for_auto_switch(md_ptr))
    continue;
  
  w=md_ptr->width*(1+(md_ptr->mode==G360x480x256));
  h=md_ptr->height;
  
  /* has to be >= pic size, but <= current best-fit mode. */
  if(w+cfg.deltamodefit>=width && h+cfg.deltamodefit>=height)
    if(w<=newmode_w && h<=newmode_h)
      newmode=md_ptr->mode,newmode_w=w,newmode_h=h;
  
  /* also track biggest mode as backup in case no mode fits it all. */
  if(w>=bigmode_w && h>=bigmode_h)
    bigmode=md_ptr->mode,bigmode_w=w,bigmode_h=h;
  }

if(newmode==-1)
  {
  if(bigmode==-1)
    newmode=curvgamode;		/* should be a can't-happen though */
  else
    newmode=bigmode;
  }

curvgamode=newmode;
}


/* arg is non-zero if want bigger mode, or zero if want smaller.
 * returns 0 if no mode fits.
 */
int change_mode_size(int bigger)
{
struct modedesc_tag *md_ptr;
int w,h;
int newmode=-1,diff,mindiff=(1<<30);

for(md_ptr=modedesc;md_ptr->mode;md_ptr++)
  {
  if(!mode_ok_for_auto_switch(md_ptr))
    continue;
  
  w=md_ptr->width*(1+(md_ptr->mode==G360x480x256));
  h=md_ptr->height;
  
  if(bigger)
    {
    diff=(w-scrnwide)*(h-scrnhigh);
    /* the <= in the diff bit gives us the best depth available */
    if(w>scrnwide && h>scrnhigh && diff<=mindiff)
      newmode=md_ptr->mode,mindiff=diff;
    }
  else
    {
    /* smaller */
    diff=(scrnwide-w)*(scrnhigh-h);
    if(w<scrnwide && h<scrnhigh && diff<=mindiff)
      newmode=md_ptr->mode,mindiff=diff;
    }
  }

if(newmode==-1)
  return(0);

curvgamode=newmode;
return(1);
}


void aborted_file_cleanup()
{
switch(loading_file_type)
  {
  case _IS_GIF:
    aborted_file_gif_cleanup();
    break;
  case _IS_JPEG:
    aborted_file_jpeg_cleanup();
    break;
  case _IS_PNM:
    aborted_file_pnm_cleanup();
    break;
  case _IS_TIFF:
    aborted_file_tiff_cleanup();
    break;
  case _IS_BMP:
    aborted_file_bmp_cleanup();
    break;
  case _IS_TGA:
    aborted_file_tga_cleanup();
    break;
  case _IS_PNG:
    aborted_file_png_cleanup();
    break;
  case _IS_XVPIC:
    printf("aborted xvpic load: can't happen!\n");
    exit(1);
  case _IS_PCX:
    aborted_file_pcx_cleanup();
    break;
  case _IS_MRF:
    aborted_file_mrf_cleanup();
    break;
  case _IS_PRF:
    aborted_file_prf_cleanup();
    break;
  case _IS_XBM:
    aborted_file_xbm_cleanup();
    break;
#ifdef PCD_SUPPORT
  case _IS_PCD:
    aborted_file_pcd_cleanup();
    break;
#endif
  case _IS_XPM:
    aborted_file_xpm_cleanup();
    break;
  }
}


void makerealpal()
{
int f;
int r,rlo,g,glo,b,blo;
int gval;

for(f=0;f<256;f++)
  {
  r=palr[f]>>2; rlo=palr[f]&3;
  g=palg[f]>>2; glo=palg[f]&3;
  b=palb[f]>>2; blo=palb[f]&3;

  if(cfg.fakecols)
    {
    gval=(rlo*grey_red+glo*grey_green+blo*grey_blue)/4;
    /* so gval is sub-value, 0<=gval<1000. (really <750... :-( )
     * possible emulated sub-values are...
     * 0, 114 (b), 299 (r), 413 (r+b), 587 (g), 701 (g+b), 886 (r+g)
     */
  
    if(gval>=886)
      r++,g++;
    else if(gval>=701)
      g++,b++;
    else if(gval>=587)
      g++;
    else if(gval>=413)
      r++,b++;
    else if(gval>=299)
      r++;
    else if(gval>=114)
      b++;

    if(r>63) r=63; if(g>63) g=63; if(b>63) b=63;
    }

  palrgb[f*3  ]=palr64[f]=r;
  palrgb[f*3+1]=palg64[f]=g;
  palrgb[f*3+2]=palb64[f]=b;
  }
}


void filterpal(byte *palette)
{
int f;

for(f=0;f<256;f++)   /* don't *really* need to know number of colours */
  {
  /* we also abuse this to get a brightness/contrast-ified index
   * to use for high-colour mode b/c. :-) (Hence palt[].)
   */
  /* XXX it's debatable where gamma should be applied... */
  if(cfg.bc_order_rev)
    {
    palr[f]=contrastup(dimmer(apply_gamma(palette[    f])));
    palg[f]=contrastup(dimmer(apply_gamma(palette[256+f])));
    palb[f]=contrastup(dimmer(apply_gamma(palette[512+f])));
    palt[f]=contrastup(dimmer(apply_gamma(f)));
    }
  else
    {
    palr[f]=dimmer(contrastup(apply_gamma(palette[    f])));
    palg[f]=dimmer(contrastup(apply_gamma(palette[256+f])));
    palb[f]=dimmer(contrastup(apply_gamma(palette[512+f])));
    palt[f]=dimmer(contrastup(apply_gamma(f)));
    }
  }

makerealpal();
}


int dimmer(int a)
{
a+=brightness;
if(a<0) a=0;
if(a>255) a=255;
return(a);
}


int contrastup(int cp)
{
float g;

g=(float)(cp);
g=128.+(g-128.)*contrast;
if(g<0.) g=0.;
if(g>255.) g=255.;
return((int)g);
}


int apply_gamma(int val)
{
int d16col=(curvgamode==G640x480x16 && cfg.viewer16col);

/* the ==1.0 is a bit bogus, but likely to work if you've not
 * messed about with it too heavily - certainly it'll work if you
 * never changed it. :-)
 */
if(picgamma==1.0 && !d16col)
  return(val);

/* use 2.2 as a base, assuming most people will have gamma 2.2 monitors
 * (the 8-colour dither is inherently linear gamma in all cases).
 */
if(d16col)
  return((int)(pow(val/255., 2.2/picgamma)*255.+0.5));

return((int)(pow(val/255., 1./picgamma)*255.+0.5));
}


int get_mode_width(int vm)
{
vga_modeinfo *vmi;

vmi=vga_getmodeinfo(vm);
return(vmi->width);
}

int get_mode_height(int vm)
{
vga_modeinfo *vmi;

vmi=vga_getmodeinfo(vm);
return(vmi->height);
}

int get_mode_pixelbytes(int vm)
{
vga_modeinfo *vmi;

vmi=vga_getmodeinfo(vm);
return(vmi->bytesperpixel);
}

int get_mode_numcols(int vm)
{
vga_modeinfo *vmi;

vmi=vga_getmodeinfo(vm);
return(vmi->colors);
}


void do_16col_palette()
{
int paltmp[16*3],f,r,g,b;

if(cfg.viewer16col)
  {
  /* colour */
  for(f=r=0;r<2;r++)
    for(g=0;g<2;g++)
      for(b=0;b<2;b++,f+=3)
        paltmp[f]=r*63,paltmp[f+1]=g*63,paltmp[f+2]=b*63;
  /* FWIW, zero colours 8..15. It may seem wasteful not using these,
   * but there's no way I can see to use them as part of an ordered
   * dither which doesn't look awful (and error-diffused dithering
   * is too slow).
   */
  for(;f<16*3;f+=3)
    paltmp[f]=paltmp[f+1]=paltmp[f+2]=0;
  
  /* actually, in that case, best to allocate some for right-button
   * menu colours!
   */
  f=8;
  paltmp[f*3  ]=cfg.light.r;
  paltmp[f*3+1]=cfg.light.g;
  paltmp[f*3+2]=cfg.light.b;
  f++;
  paltmp[f*3  ]=cfg.medium.r;
  paltmp[f*3+1]=cfg.medium.g;
  paltmp[f*3+2]=cfg.medium.b;
  f++;
  paltmp[f*3  ]=cfg.dark.r;
  paltmp[f*3+1]=cfg.dark.g;
  paltmp[f*3+2]=cfg.dark.b;
  f++;
  paltmp[f*3  ]=cfg.black.r;
  paltmp[f*3+1]=cfg.black.g;
  paltmp[f*3+2]=cfg.black.b;
  }
else
  {
  /* 16-grey palette */
  for(f=0;f<16;f++)
    paltmp[f*3]=paltmp[f*3+1]=paltmp[f*3+2]=(f*63)/15;
  }

vga_setpalvec(0,16,paltmp);
}


/* set new mode if needed, else just clear screen. */
void setmode_or_clear(int newmode)
{
if(vga_getcurrentmode()!=newmode)
  vga_setmode(newmode); 
else
  vga_clear();
}


void init_vkludge_cache(void)
{
int x;

/* invalidate any cached colours */
memset(vkcache_valid,0,sizeof(vkcache_valid));

/* the actual palette colours are fairly likely to crop up :-),
 * so index them automatically.
 */
for(x=0;x<numcols;x++)
  closest(pal32_no_bc[x*3],pal32_no_bc[x*3+1],pal32_no_bc[x*3+2]);
}


void graphicson(void)
{
msgbox_draw_ok=0;

vga_lockvc();
if((vga_hasmode(curvgamode))&&(cfg.mode_allowed[curvgamode]))
  setmode_or_clear(curvgamode);
else
  {
  /* we haven't? Aaargh!!! Ok, use 640x480 or 360x480 instead. */
  if((vga_hasmode(G640x480x256))&&(cfg.mode_allowed[G640x480x256]))
    setmode_or_clear(curvgamode=G640x480x256);
  else
    {
    if((vga_hasmode(G360x480x256))&&(cfg.mode_allowed[G360x480x256]))
      setmode_or_clear(curvgamode=G360x480x256);
    else
      /* *must* have 320x200 (see rcfile.c for more info) */
      setmode_or_clear(curvgamode=G320x200x256);
    }
  }

virtual=0;
if(curvgamode==G320x400x256 || curvgamode==G360x480x256)
  virtual=1;

if(curvgamode==G640x480x16)
  do_16col_palette();

if(pixelsize!=1) gl_setcontextvga(curvgamode);

scrnwide=get_mode_width(curvgamode);
scrnhigh=get_mode_height(curvgamode);
scrnpixelsize=get_mode_pixelbytes(curvgamode);
scrncols=get_mode_numcols(curvgamode);

if(virtual)
  scrnwide*=2;
if(pixelsize==1)
  setpalvec(0,256,palrgb);

vga_unlockvc();
}


void graphicsoff()
{
/* we let zgv.c deal with this now, so this is just a placeholder */
}


/* tells us if mode is ok to use in current circumstances. */
int mode_is_usable_now(int modenum)
{
if(vga_hasmode(modenum) && cfg.mode_allowed[modenum] &&
   ((pixelsize==1 && get_mode_numcols(modenum)<=256) ||
    (pixelsize==3 && get_mode_numcols(modenum)> 256)))
  return(1);
return(0);
}


int allow_zoom(void)
{
return(zoom && (!cfg.zoom_reduce_only || width>scrnwide || height>scrnhigh));
}


void swap_black_to_bg(byte *palette)
{
unsigned char *ptr,minidx=0;
int mindist=(1<<30),dist;
int f,r,g,b;

if(pixelsize!=1 || palette[0]+palette[256]+palette[512]==0)
  return;

for(f=1;f<256;f++)
  {
  /* we can afford to be fairly approximate about this */
  dist=palette[f]+palette[256+f]+palette[512+f];
  if(dist<mindist)
    {
    mindist=dist;
    minidx=f;
    if(dist==0)
      break;
    }
  }

if(minidx==0) return;	/* sanity check - should be impossible */

/* do the swap */
ptr=theimage;
for(f=0;f<width*height;f++,ptr++)
  {
  if(*ptr==minidx)
    *ptr=0;
  else
    {
    if(*ptr==0)
      *ptr=minidx;
    }
  }

r=palette[minidx];
g=palette[minidx+256];
b=palette[minidx+512];
palette[minidx]=palette[0];
palette[minidx+256]=palette[256];
palette[minidx+512]=palette[512];
palette[0]=r;
palette[256]=g;
palette[512]=b;
}



/* returns 1 if ESC or 'x' was pressed, 0 if because of timeout when
 * using cfg.repeat_timer option.
 *
 * caller must restore mouse pos this saves, in appropriate mode.
 */
int showgif(char *filename,byte *palette)
{
int opx,opy,px,py,quitshow,key,redraw;
int npx=0,npy=0;    /* equivalent to px,py in scaling>1 mode */
int f;
int mode_prefix=0;
int rb_menu_mode=0,scrolling_with_mouse=0;
int mx=0,my=0,cx=0,cy=0,dx,dy;		/* mouse scroll saved/center/delta */
int slideshow_pause=0;
static int orient_lastpicexit_state=0;	/* last picture orientation state */
int orient_current_state;		/* current picture orientation state */
int vkcache_reinit=0;
int doing_auto_animate=0;

wait_for_foreground();

/* make sure we have a decent saved mouse position (initial position,
 * or (more usually) from selector).
 */
save_mouse_pos();
if(has_mouse) mx=mouse_getx(),my=mouse_gety();

orient_current_state=0;		/* picture starts off at normal orientation */

/* if image has less than 256 colours, we fill in the 64 greys so that
 * using the vkludge on, say, a mono file will look much better.
 */
if(numcols<256)
  {
  for(f=numcols;f<numcols+64;f++)
    palette[f]=palette[256+f]=palette[512+f]=((f-numcols)<<2);
  /* get rid of any junk entries which vkludge etc. might try and use */
  for(;f<256;f++)
    palette[f]=palette[256+f]=palette[512+f]=0;
  }

/* try for black background for 8-bit images if asked to */
if(pixelsize==1 && cfg.black_bg)
  swap_black_to_bg(palette);

/* put 5-bit version of palette[] in pal32_no_bc. This is in a different
 * format, as required by closest() which uses it.
 */
for(f=0;f<256;f++)
  {
  pal32_no_bc[f*3  ]=(palette[f]>>3);
  pal32_no_bc[f*3+1]=(palette[f+256]>>3);
  pal32_no_bc[f*3+2]=(palette[f+512]>>3);
  }

/* init vkludge cache for this picture. Doesn't need updating for
 * brightness/contrast changes, and is filled in as needed.
 */
if(pixelsize==1)
  init_vkludge_cache();

pic_incr=0;
zgv_io_timer_flag=0;
if(cfg.repeat_timer<0)
  cfg.repeat_timer=0;
if(cfg.repeat_timer && !tagview_mode)
  zgv_io_timer_start(100*cfg.repeat_timer);

if(tagview_mode)
  zgv_io_timer_start(100*cfg.tag_timer);

quitshow=0;
filterpal(palette);
if(!cfg.repeat_timer || (cfg.repeat_timer && first_repeat))
  {
  px=py=0;
  if(cfg.automodefit)
    do_auto_mode_fit();
  graphicson();
  first_repeat=0;
  if(cfg.revert)
    {
    scaling=1;
    interp=0;
    }
  if(!cfg.revert_orient && !orient_override)
    {
    orient_change_state(orient_current_state,orient_lastpicexit_state,0);
    orient_current_state=orient_lastpicexit_state;
    }
  }
else
  if(cfg.repeat_timer)
    {
    px=saved_px;
    py=saved_py;
    }

/* an orientation override always happens, even for repeat pics.
 * (Not least because it seems like the sort of thing which would be
 * dead useful for them. :-))
 */
if(orient_override)
  {
  orient_change_state(orient_current_state,orient_override_state,0);
  orient_current_state=orient_override_state;
  }

if(cfg.auto_animate && gif_delaycount>=2 && pixelsize==1)
  doing_auto_animate=1;
else
  redrawgif(px,py,npx,npy);

while(!quitshow && (zgv_io_timer_flag!=1 || rb_menu_mode || slideshow_pause))
  {
  if(doing_auto_animate)
    key='e',quitshow=1;		/* do animation only, then exit */
  else
    {
    if(rb_menu_mode) mousecur_on();
    key=wait_for_keys_or_mouse(zgv_ttyfd);
    if(rb_menu_mode) mousecur_off();
    }
  
  opx=px; opy=py; redraw=0;
  
  /* convert xzgv-ish keypresses */
  if(cfg.xzgvkeys)
    {
    switch(key)
      {
      /* omissions/problems:
       * - ctrl-cursors aren't distinguishable, so we can't support those.
       * - ^q would break slideshow pausing, which IMHO isn't worth it.
       *   (I've also omitted q for some sort of consistency with that.)
       * - tab would break selecting some modes, also not worth it.
       */
      case 'b':		key=127; break;
      case ' ':		key=RK_ENTER; break;
      case RK_CTRLSPACE: key=' '; break;
      case 'W'-0x40:	key=RK_ESC; break;
      case 'N':		key=128+'n'; break;
      }
    }

  if(rb_menu_mode)
    {
    /* we deal with keys ourselves... */
    if(key==RK_ESC || key=='x')
      {
      rb_menu_mode=0;
      undraw_rb_menu();
      continue;
      }
    
    /* but mouse dealt with by this. if not clicked an entry, skip
     * rest of loop.
     */
    if(rb_menu_event(&key))	/* returns non-zero if should stay in menu */
      continue;
    
    /* otherwise restore saved area of screen, and do action in (faked) key */
    rb_menu_mode=0;
    undraw_rb_menu();
    mx=mouse_getx();
    my=mouse_gety();
    goto parsekey;	/* skip normal mouse stuff just in case */
    }
  
  if(has_mouse)
    {
    int mbuttons=mouse_getbutton();
    int mleft_start=is_start_click_left();
    /* important to read both, even if not using both */
    int mleft=is_end_click_left(),mright=is_end_click_right();
    
    /* if right button released, get the right-button menu. */
    if(mright)
      {
      /* only allow it if >=480 pixels high! */
      if(scrnhigh>=480)
        {
        rb_menu_mode=1;
	if(scrolling_with_mouse)
	  {
	  mouse_setposition(mx,my);
	  scrolling_with_mouse=0;
	  }
        draw_rb_menu();
        /* if left mouse button down, ignore first end-left-click. */
        rb_ignore_first_left_click=((mbuttons&MOUSE_LEFTBUTTON)?1:0);
        
        /* skip rest of main loop except this. */
        continue;
        }
      }

    if(scrolling_with_mouse)
      {
      /* there may be subpixel motion in the low resolution modes,
       * especially if combined with another event(s), so the mouse
       * should be recentered on real position changes only.
       */
      dx=cx-mouse_getx();
      dy=cy-mouse_gety();
      if(dx || dy)
        {
        px-=dx;
        py-=dy;
        mouse_setposition(cx,cy);
	}
      }
    else
      {
      /* if left mouse button pressed, handle this event's movement
       * and prepare for next event(s).
       */
      if(mleft_start && !allow_zoom())
        {
        scrolling_with_mouse=1;
        px+=mx-mouse_getx();
        py+=my-mouse_gety();
        mx=mouse_getx();
        my=mouse_gety();
        cx=scrnwide/2;
        cy=scrnhigh/2;
        mouse_setposition(cx,cy);
        }
      }

    /* if left button not down, don't move with mouse any more.
     * this is after the rest to make sure we don't miss any
     * scrolling motions if scrolling_with_mouse is currently set.
     */
    if(!(mbuttons&MOUSE_LEFTBUTTON))
      {
      if(scrolling_with_mouse)
        {
        mouse_setposition(mx,my);
        scrolling_with_mouse=0;
        }
      else
        {
        mx=mouse_getx();
        my=mouse_gety();
        }
      }

    mleft=0;	/* kludge to keep gcc -Wall happy */
    }
  
  parsekey:
  /* avoid problems with mode or scaling change etc. */
  if(key && scrolling_with_mouse)
    {
    mouse_setposition(mx,my);
    scrolling_with_mouse=0;
    }
  
  /* mode selection - note that F11=shift-F1 and F12=shift-F2 */
  if(key=='0' || key=='^' || (key>='5' && key<='8') ||
     (key>=RK_F1 && key<=RK_F10) || (key>=RK_SHIFT_F1 && key<=RK_SHIFT_F10))
    {
    struct modedesc_tag *md_ptr;
    int mode=0,fallback_mode=0;
    
    for(md_ptr=modedesc;md_ptr->mode;md_ptr++)
      {
      if(mode_prefix!=md_ptr->is_tab_key) continue;
      
      if(key==md_ptr->viewer_key && mode_is_usable_now(md_ptr->mode))
        {
        if(md_ptr->bytespp==4)	/* if 32-bit */
          fallback_mode=md_ptr->mode;
        else
          mode=md_ptr->mode;
        }
      }
    
    mode_prefix=0;
    
    /* if mode is no good, try the fallback (if any) */
    if(!mode && fallback_mode)
      mode=fallback_mode;
    
    /* if we've got something valid, use it. */
    if(mode)
      {
      int oldmode=curvgamode;

      curvgamode=mode;
      if(curvgamode==G640x480x16 || oldmode==G640x480x16)
        {
        /* when switching to/from 640x480x4, need filterpal()
         * in case of implicit gamma change (see apply_gamma()).
         * Note that this is the only graphicson() call that needs
         * this, as the 640x480x4 mode can't be switched to
         * in any other way than specifically selecting it.
         */
        filterpal(palette);
        }
      
      graphicson();
      redraw=1;
      }
    }
  else	/* if not a mode-selecting key... */
    {
    if(key) mode_prefix=0;
    
    switch(key)
      {
#ifdef PCD_SUPPORT
      case 128+'1': case 128+'2': case 128+'3':
      case 128+'4': case 128+'5':		/* Alt1 - Alt5 */
        cfg.pcdres=key-128-'0';
	quitshow=2;
	pic_incr=PIC_INCR_RELOAD_KLUDGE;
        break;
#endif
      case 9:		/* tab */
        mode_prefix=1;
        break;
      case 'F':
        cfg.dither_hicol=!cfg.dither_hicol;
        redraw=1;
        break;
      case 'G':
        cfg.fakecols=!cfg.fakecols;
        if(pixelsize==1)
          {
          filterpal(palette),setpalvec(0,256,palrgb);
          if(curvgamode==G640x480x16) redraw=1;
#ifndef BACKEND_SVGALIB
          redraw=1;
#endif
          }
        break;
      
      /* the way pause/resume slideshow works is quite simple -
       * pausing overrides the zgv_io_timer_flag var set when the timer
       * times out, and resuming disables that override.
       */
      case 'S'-0x40:	/* pause slideshow */
        if(tagview_mode)
          slideshow_pause=1;
        break;
      case 'Q'-0x40:	/* resume slideshow */
        if(tagview_mode)
          slideshow_pause=0;
        break;
      
      case 's': case 'd':
        if(allow_zoom())
          vga_clear();	/* cls if zoomed, in case x2 is smaller! */
        zoom=0;
        if(scaling<512)
          {
          int oldscale=scaling;
          
          scaling+=(key=='d')?scaling:1;
          if(scaling>512) scaling=512;
          samecentre(&px,&py,scaling,px,py,oldscale);
          redraw=1; /* no cls reqd. - must be bigger */
          }
        break;
      case 'S': case 'D':
        if(scaling>1 || allow_zoom())
          {
          int oldscale=scaling;
          
          scaling-=(key=='D')?scaling/2:1;
          if(scaling<1) scaling=1;
          samecentre(&px,&py,scaling,px,py,oldscale);
          redraw=1;
          if(width*scaling<scrnwide || height*scaling<scrnhigh) vga_clear();
          }
        zoom=0;
        break;
      case 'n':
        if(allow_zoom() || scaling>1)
          {
          samecentre(&px,&py,1,px,py,scaling);
          vga_clear(); 
          }
        scaling=1; zoom=0;
        redraw=1; interp=0; inextpix=1;
        break;
      case RK_HOME: case 'A'-0x40:
        px=py=0; break;
      case RK_END:  case 'E'-0x40:
        px=py=1<<30; break;
      case RK_PAGE_UP: case 'U'-0x40:
        py-=scrnhigh*9/10; break;
      case RK_PAGE_DOWN: case 'V'-0x40:
        py+=scrnhigh*9/10; break;
      case '-':
        px-=scrnwide*9/10; break;
      case '=':
        px+=scrnwide*9/10; break;
      case 'v': vkludge=((vkludge==1)?0:1); redraw=1; graphicson(); break;
      case ',': case '.': case '<': case '>': case 128+',': case 128+'.':
      case 'B': case '*': case ';':
      case '1': case '2': case '3': case '4':
        switch(key)
          {
          case ',':	contrast-=0.05; break;
          case '.':	contrast+=0.05; break;
          case '<':	brightness-=10; break;
          case '>':	brightness+=10; break;
          case 128+',':	picgamma/=1.05; break;
          case 128+'.': picgamma*=1.05; break;
          case 'B':	cfg.bc_order_rev=!cfg.bc_order_rev; break;
          case '1':	picgamma=1.0; break;
          case '2':	picgamma=2.2; break;
          case '3':	picgamma=(1.0/2.2); break;
          case '4':	picgamma=cfg.initial_picgamma; break;
          default: 	brightness=0; contrast=1.0; break;
          }
        filterpal(palette);	/* make palr/g/b/t */
        if(scrnpixelsize>1)
          redraw=1;			/* redraw if non-palette-based... */
        else
          setpalvec(0,256,palrgb);	/* else change onscreen palette */
        /* 640x480 16-colour needs *both*! */
        if(curvgamode==G640x480x16) redraw=1;
#ifndef BACKEND_SVGALIB
        /* we assume the same for non-svgalib backends, too */
        redraw=1;
#endif
        /* any palette change will invalidate the vkludge cache.
         * Set flag to say we should reinit the cache when we next redraw.
         */
        vkcache_reinit=1;
        break;
      case ':':
        file_details(filename,
                     width,(gif_delaycount<2)?height:height/gif_delaycount,
                     &redraw);
        if(redraw)
          graphicson();
        break;
      case 'q': case 'k': py-=MOVSTP; break;
      case 'a': case 'j': py+=MOVSTP; break;
      case 'o': case 'h': px-=MOVSTP; break;
      case 'p': case 'l': px+=MOVSTP; break;
      case 'Q': case 'K': case RK_CURSOR_UP:    py-=BIGSTP; break;
      case 'A': case 'J': case RK_CURSOR_DOWN:  py+=BIGSTP; break;
      case 'O': case 'H': case RK_CURSOR_LEFT:  px-=BIGSTP; break;
      case 'P': case 'L': case RK_CURSOR_RIGHT: px+=BIGSTP; break;
      case 'e':		/* animate */
        {
        int ret;
        
        if(cfg.repeat_timer || tagview_mode)
          zgv_io_timer_stop();		/* cancel timeout for this image */
        
        ret=animate_gif(orient_current_state);

        if(cfg.repeat_timer || tagview_mode)
          zgv_io_timer_flag=0;			/* clear it in case */
        
        if(doing_auto_animate)	/* no need for redraw */
          {
          switch(ret)	/* change file if requested */
            {
            case -1:	goto do_prev_file;
            case 1:	goto do_next_file;
            case 2:	goto do_next_and_tag;
            }

          break;	/* else just exit switch(key) */
          }
        
        /* need clear as (if zoomed) full pic will take less space,
         * or (if rotated) pic effectively has different shape.
         */
        vga_clear();
        redraw=1;
        break;
        }
      case 'm':		/* mirror */
        fx_mirror();
        orient_current_state=orient_state_mirror[orient_current_state];
        px=py=0; redraw=1;
        break;
      case 'f':		/* flip */
        fx_flip();
        orient_current_state=orient_state_flip[orient_current_state];
        px=py=0; redraw=1;
        break;
      case 'r':		/* rotate clockwise */
        fx_rot(); vga_clear();
        orient_current_state=orient_state_rot_cw[orient_current_state];
        px=py=0; redraw=1;
        break;
      case 'R':		/* rotate anti-clockwise */
        fx_rot(); fx_flip(); fx_mirror(); vga_clear();
        orient_current_state=orient_state_rot_acw[orient_current_state];
        px=py=0; redraw=1;
        break;
      case 128+'n':	/* restore original orientation (state 0) */
        orient_change_state(orient_current_state,0,1);
        orient_current_state=0;
        px=py=0; redraw=1;
        break;
      case 128+'o':	/* re-use previous pic's orientation */
        orient_change_state(orient_current_state,orient_lastpicexit_state,1);
        orient_current_state=orient_lastpicexit_state;
        px=py=0; redraw=1;
        break;
      case 128+'s':	/* save current orientation, overriding until Esc */
        orient_override_state=orient_current_state;
        orient_override=1;
        break;
      case 128+'r':	/* reduce only */
        cfg.zoom_reduce_only=(!cfg.zoom_reduce_only);
        scaling=redraw=1; px=py=0;
        vga_clear();
        break;
      case 'z':
        zoom=(!zoom); scaling=redraw=1; px=py=0; vga_clear(); break;
      case 'Z':
        cfg.automodefit=(!cfg.automodefit);
        if(cfg.automodefit)
          do_auto_mode_fit();
        scaling=redraw=1; px=py=0;
        graphicson();
        break;
      case '[':
        if(change_mode_size(0))
          redraw=1,graphicson();
        break;
      case ']':
        if(change_mode_size(1))
          redraw=1,graphicson();
        break;
      case 'i':
        interp=(!interp); redraw=1; break;
      case '!':
        inextpix=(inextpix==1)?2:1;
        redraw=1;
        break;
      case '?':
        /* showhelp restores mouse pos */
        showhelp(zgv_ttyfd,"- KEYS FOR VIEWER -",viewerhelp);
        save_mouse_pos();
        graphicson(); redraw=1;
        break;
      case '/':
        /* showhelp restores mouse pos */
        showhelp(zgv_ttyfd,"- SELECTING VIDEO MODES -",viewermodeshelp);
        save_mouse_pos();
        /* falls through to `refresh screen' */
      case 12: case 18:     /* 12,18 = Ctrl-L, Ctrl-R */
        graphicson(); redraw=1; break;
      case 'N'-0x40:
        pic_incr=1; quitshow=1; break;
      case 'P'-0x40:
        pic_incr=-1; quitshow=1; break;
      case RK_ENTER:
      do_next_file:
	pic_incr=1; quitshow=2; break;
      case ' ':
      do_next_and_tag:
	pic_incr=1; quitshow=3; break;
      case 127: case 8:
      do_prev_file:
	pic_incr=-1; quitshow=2; break;
      case 'c': case 'C':
        if(curvgamode==G640x480x16)
          {
          cfg.viewer16col=!cfg.viewer16col;
          filterpal(palette);	/* implicit gamma change (see apply_gamma()) */
          graphicson();		/* palette has to change */
          redraw=1;
          }
        break;
      case 128+'c':	/* alt-c */
        if(curvgamode==G640x480x16 && cfg.viewer16col)
          {
          cfg.fastdither16col=!cfg.fastdither16col;
          redraw=1;
          }
        break;
      case RK_ESC: case 'x':
        quitshow=1;
      }
    }

  /* if an auto-animate finished, exit */
  if(doing_auto_animate)
    break;
      
  if(!allow_zoom())
    {
    int swidth=width*scaling,sheight=height*scaling;
    
    if(sheight<=scrnhigh)
      py=0;
    else
      if(sheight-py<scrnhigh) py=sheight-scrnhigh;
    if(swidth<=scrnwide)
      px=0;
    else
      if(swidth-px<scrnwide) px=swidth-scrnwide;
    if(px<0) px=0;
    if(py<0) py=0;
    if(scaling>1)
      {
      npx=px/scaling;
      npy=py/scaling;
      }
    }
  else
    px=py=npx=npy=0;
    
  if(redraw || opx!=px || opy!=py)
    {
    if(vkludge && vkcache_reinit)
      init_vkludge_cache();
    vkcache_reinit=0;
    redrawgif(px,py,npx,npy);
    }
  }

graphicsoff();

/* make sure we ignore any previous GIF animation stuff in future */
gif_delaycount=0;

if(cfg.selecting)
  show_dimensions(px,py,scaling);

if(cfg.repeat_timer || tagview_mode)
  {
  zgv_io_timer_stop();
  saved_px=px;
  saved_py=py;
  if(quitshow==1) tagview_mode=0;
  }

/* save orientation state as last-picture-exit state */
orient_lastpicexit_state=orient_current_state;

/* caller must do mouse restore */

return(quitshow);
}


/* save old contents of area to put right-button menu on, and draw it. */
static void draw_rb_menu()
{
int light,medium,dark,black;		/* for menu */
int mwhite,mblack;			/* for mouse pointer */
int f,y;

rbm_xysize(viewer_menu_data,&rb_save_width,&rb_save_height);
if(scrnpixelsize<1) scrnpixelsize=1;	/* sanity check */

if((rb_save=malloc(rb_save_width*rb_save_height*scrnpixelsize))==NULL) return;

switch(curvgamode)
  {
  /* for 16-colour and 8-bit generic-VGA, don't use vgagl.
   * though I have many modes listed here, in truth it's only
   * 640x480x4 and 360x480x8 this can possibly be called for.
   * (NB: can't use scrnwide here, as that's twice phys. width in 360x480.)
   */
  case G640x480x16:
  case G320x200x256: case G320x240x256:
  case G320x400x256: case G360x480x256:
    for(y=0;y<rb_save_height;y++)
      vga_getscansegment(rb_save+rb_save_width*y,
      		vga_getxdim()-rb_save_width,y,rb_save_width);
    break;
  
  /* otherwise use vgagl */
  default:
    /* may not be current context if 8-bit mode, so make sure. */
    gl_setcontextvga(curvgamode);
    
    gl_getbox(scrnwide-rb_save_width,0,rb_save_width,rb_save_height,rb_save);
  }

/* grey out (make non-active) any ones we shouldn't allow.
 * this is nasty, but decl of viewer_menu_data notes that there's
 * a nasty thing here, so it should be ok.
 */

f=1;
viewer_menu_data[f++].active=mode_is_usable_now(G360x480x256);
viewer_menu_data[f++].active=mode_is_usable_now(G640x480x16);
viewer_menu_data[f++].active=mode_is_usable_now(G640x480x256);
viewer_menu_data[f++].active=mode_is_usable_now(G800x600x256);
viewer_menu_data[f++].active=mode_is_usable_now(G1024x768x256);
viewer_menu_data[f++].active=mode_is_usable_now(G640x480x32K);
viewer_menu_data[f++].active=mode_is_usable_now(G640x480x64K);
viewer_menu_data[f++].active=
  mode_is_usable_now(G640x480x16M) || mode_is_usable_now(G640x480x16M32);
viewer_menu_data[f++].active=mode_is_usable_now(G800x600x32K);
viewer_menu_data[f++].active=mode_is_usable_now(G800x600x64K);
viewer_menu_data[f++].active=
  mode_is_usable_now(G800x600x16M) || mode_is_usable_now(G800x600x16M32);
viewer_menu_data[f++].active=mode_is_usable_now(G1024x768x32K);
viewer_menu_data[f++].active=mode_is_usable_now(G1024x768x64K);
viewer_menu_data[f++].active=
  mode_is_usable_now(G1024x768x16M) || mode_is_usable_now(G1024x768x16M32);

rbm_set_active_flag(viewer_menu_data,"grey/colour",(curvgamode==G640x480x16));

/* make some colours available if needed for light/medium/dark/black
 * colours. Here's the way this works:
 * - for 15/16/24-bit modes, just use the colours.
 * - for 8-bit, find closest matches and force those to the right
 *   colour *temporarily*, i.e. while rb menu is onscreen. That means
 *   we'll need a palette `redraw' (setpalvec) when screen is restored.
 * - for 640x480 4-bit, we kludge it a bit (see below).
 */
switch(scrncols)
  {
  case 16:	/* 640x480 16-colour */
    if(cfg.viewer16col)
      {
      /* if in colour, they were reserved for us out of the 2nd 8 colours */
      light=8; medium=9; dark=10; black=11;
      mwhite=light; mblack=black;
      }
    else
      {
      /* otherwise, we use some fixed greys which match the default
       * zgv setup fairly closely.
       */
      light=9; medium=7; dark=5; black=0;
      mwhite=light; mblack=1;
      }
    break;
  
  case 256:	/* 8-bit */
    /* find and fix the colours */
    rbm_find_and_fix_ui_cols(&light,&medium,&dark,&black,&mblack,
                             palr64,palg64,palb64);
    mwhite=light;
    break;
  
  default:	/* 15/16/24-bit */
    light =((cfg.light.r<<18)|(cfg.light.g<<10)|(cfg.light.b<<2));
    medium=((cfg.medium.r<<18)|(cfg.medium.g<<10)|(cfg.medium.b<<2));
    dark  =((cfg.dark.r<<18)|(cfg.dark.g<<10)|(cfg.dark.b<<2));
    black =((cfg.black.r<<18)|(cfg.black.g<<10)|(cfg.black.b<<2));
    /* mblack/mwhite should be in screen format */
    mblack=1;	/* black musn't be 0, unfortunately */
    switch(scrncols)
      {
      case 32768:
        mwhite=GET15BITCOLOUR(4*cfg.light.r,4*cfg.light.g,4*cfg.light.b);
        break;
      case 65536:
        mwhite=GET16BITCOLOUR(4*cfg.light.r,4*cfg.light.g,4*cfg.light.b);
        break;
      default:
        mwhite=light;	/* 24-bit is easy :-) */
      }
    break;
  }

mousecur_init(mblack,mwhite);

/* restore old mouse pos. */
restore_mouse_pos();

/* now draw the thing */
rbm_draw(viewer_menu_data,light,medium,dark,black);
}


/* restore old contents of area with right-button menu on. */
static void undraw_rb_menu()
{
int y;

/* save current mouse pos. */
save_mouse_pos();

if(rb_save==NULL) return;	/* ran out of memory, can't do much! */

switch(curvgamode)
  {
  /* again, not vgagl, and again, can't use scrnwide */
  case G640x480x16:
  case G320x200x256: case G320x240x256:
  case G320x400x256: case G360x480x256:
    for(y=0;y<rb_save_height;y++)
      vga_drawscansegment(rb_save+rb_save_width*y,
      		vga_getxdim()-rb_save_width,y,rb_save_width);
    break;
  
  /* vgagl */
  default:
    gl_putbox(scrnwide-rb_save_width,0,rb_save_width,rb_save_height,rb_save);
  }

if(scrncols==256)
  setpalvec(0,256,palrgb);	/* restore palette */

free(rb_save);
}


/* possibly have a mouse event to deal with for right-button menu.
 * uses pointer to key to fake keys to do stuff, and returns
 * 1 if we should stay in rb menu mode, else 0.
 */
static int rb_menu_event(int *keyp)
{
/* important to read both, even if not using both */
int mleft=is_end_click_left(),mright=is_end_click_right();
int key;

if(!mleft)
  return(1);

if(rb_ignore_first_left_click)
  {
  rb_ignore_first_left_click=0;
  return(1);
  }

key=mright;	/* a kludge to keep gcc -Wall quiet */

/* get faked key for viewer_menu_data, or zero if none */
*keyp=0;
key=rbm_mousepos_to_key(viewer_menu_data,mouse_getx(),mouse_gety());
if(key!=-1)	/* -1 means quit menu with no key */
  *keyp=key;

return((key==0));	/* 1 if didn't match any, else 0 */
}



void setpalvec(int start,int num,int *pal)
{
/* only relevant for 8-bit modes */
if(pixelsize==1)
  {
  if(curvgamode!=G640x480x16)
    vga_setpalvec(start,num,pal);
  else
    {
    int f,c;
    unsigned char *dptr=dither16_rgb+start*3;
    int *sptr=pal+start*3;
    
    for(f=start;f<start+num;f++)
      {
      if(cfg.viewer16col)
        {
        /* used to adjust contrast here, but that's now done
         * more accurately using gamma in apply_gamma().
         */
        *dptr++=*sptr++;
        *dptr++=*sptr++;
        *dptr++=*sptr++;
        }
      else
        {
        c=(pal[f*3]*grey_red+pal[f*3+1]*grey_green+
           pal[f*3+2]*grey_blue)/1000;
        if(c>63) c=63;
        dither16_greylookup[f]=c;
        }
      }
    }
  }
}


/* this routine is getting ridiculous */
void redrawgif(int px,int py,int npx,int npy)
{
int x,y,xdim;
int x_add,y_add;
byte *realline,*ptr;

/* set line-draw routine depending on whether we need to do
 * 15/16/24-bit brightness/contrast or not.
 * `doing_hicol_bc' just saves us doing this test elsewhere.
 */
eventuallydrawscansegment=eventuallydrawscansegment_without_bc;
doing_hicol_bc=0;
if(scrnpixelsize>1 && (brightness!=0 || contrast!=1.0 || picgamma!=1.0))
  {
  eventuallydrawscansegment=eventuallydrawscansegment_with_bc;
  doing_hicol_bc=1;
  }

if(allow_zoom())
  drawzoomedgif();
else
  {
  int swidth=width*scaling,sheight=height*scaling;
  int linelen=(swidth>scrnwide?scrnwide:swidth);
  
  /* draw non-zoomed pic */
  x_add=y_add=0;	/* these control the centering */
  if(cfg.centreflag)
    {
    if(swidth<scrnwide)
      x_add=(scrnwide-swidth)>>1;
    
    if(sheight<scrnhigh)
      y_add=(scrnhigh-sheight)>>1;
      
    if(virtual) x_add>>=1;
    }
  
  if(swidth-px<scrnwide) xdim=swidth-px; else xdim=scrnwide;
  if((py>=sheight)||(px>=swidth)) return;
  
  /* hopefully the following is fairly quick as fewer ifs... ? */
  if(virtual && scaling==1)		/* 320x400 or 360x480 */
    {
    if((realline=calloc(1,scrnwide))==NULL) return;
    for(y=0;(y<sheight-py)&&(y<scrnhigh);y++)
      {
      for(x=0;x<xdim;x++)
        *(realline+(x>>1))=getvpix(px,py,x,y);
      vga_drawscansegment(realline,x_add,y+y_add,xdim>>1);
      }
    free(realline);
    }
  else if(scaling>1)
    {
    /* Better grab your Joo Janta 200 Super-Chromatic Peril Sensitive
     * Sunglasses (both pairs) for this next bit...
     */
    int cdown=-1,i,pxx,pyy,pyym;
    int a1,a2,a3,a4,in_rg,in_dn,in_dr;
    int wp=width*pixelsize,sci=scaling*inextpix;
    int scaleincr=0,subpix_xpos,subpix_ypos,sxmulsiz,symulsiz,simulsiz=0;
    int sisize=0,sis2=0;
    unsigned char *ptr1,*ptr2,*ptr3,*ptr4,*ptr2_end,*ptr4_end;
    unsigned char *src,*dst;
    
    if((realline=calloc(pixelsize,scrnwide))==NULL) return;
    
    if(interp)
      {
      sisize=0;
      while(sisize<256) sisize+=scaling;
      scaleincr=sisize/scaling;
      simulsiz=scaleincr*sisize;
      sis2=sisize*sisize;
      }
    
    for(y=0,pyy=py;(y<sheight-py)&&(y<scrnhigh);y++,pyy++)
      {
      /* this is horribly slow... :-( */
      if(cdown<=0 || interp)
        {
        src=theimage+pixelsize*(pyy/scaling)*width;
        dst=realline;
        if(pixelsize==1)
          if(virtual)
            for(x=0;(x<swidth-px)&&(x<scrnwide);x+=2)
              *dst++=*(src+(px+x)/scaling);
          else
            for(x=0;(x<swidth-px)&&(x<scrnwide);x++)
              *dst++=*(src+(px+x)/scaling);
        else if(interp==0)
          /* normal */
          for(x=0;(x<swidth-px)&&(x<scrnwide);x++)
            {
            ptr=src+((px+x)/scaling)*pixelsize;
            *dst++=*ptr++; *dst++=*ptr++; *dst++=*ptr;
            }
        else
          {
          /* interpolated */
          
          /* This has been hacked into unreadability in an attempt to get it
           * as fast as possible.
           * It's still really slow. :-(
           */

          in_rg=inextpix*3;
          in_dn=inextpix*wp;
          in_dr=in_dn+in_rg;
          pyym=pyy%scaling;
          subpix_ypos=(pyy%scaling)*scaleincr;
          subpix_xpos=(px%scaling)*scaleincr;  /* yes px not pxx */
          
          ptr1=ptr3=src+(px/scaling)*3;
          ptr2=ptr4=ptr1+in_rg;
          ptr2_end=ptr4_end=src+in_dn-3;
          if(pyy<sheight-sci)
            {
            ptr3=ptr1+in_dn;
            ptr4=ptr1+in_dr;
            ptr4_end+=in_dn;
            }
          
          symulsiz=sisize*subpix_ypos;
          sxmulsiz=sisize*subpix_xpos;
          
          for(x=0,pxx=px;x<swidth-px && x<scrnwide;x++,pxx++)
            {
            a3=symulsiz-(a4=subpix_xpos*subpix_ypos);
            a2=sxmulsiz-a4;
            a1=sis2-sxmulsiz-symulsiz+a4;

            for(i=0;i<3;i++)
              *dst++=(ptr1[i]*a1+ptr2[i]*a2+
                      ptr3[i]*a3+ptr4[i]*a4)/sis2;
              
            subpix_xpos+=scaleincr;
            sxmulsiz+=simulsiz;
            if(subpix_xpos>=sisize)
              {
              subpix_xpos=sxmulsiz=0;
              ptr1+=3; ptr3+=3;
              if(ptr2<ptr2_end)
                ptr2+=3;
              if(ptr4<ptr4_end)
                ptr4+=3;
              }
            }
          }
              
        cdown=(cdown==-1)?(scaling-(py%scaling)):scaling;
        }
      
      cdown--;
      
      if(scrnpixelsize==3 && !doing_hicol_bc)
        gl_putbox(x_add,y+y_add,linelen,1,realline);
      else
        if(virtual)
          vga_drawscansegment(realline,x_add,y+y_add,linelen>>1);
        else
          eventuallydrawscansegment(realline,x_add,y+y_add,linelen*pixelsize);
      }
    free(realline);
    }
  else		/* if not scaled and not zoomed... */
    {
    switch(scrnpixelsize)
      {
      case 3:
#ifdef BACKEND_SVGALIB
        if(!doing_hicol_bc)
          {
          gl_putboxpart(x_add,y_add,(scrnwide<width)?scrnwide:width,
	  	(scrnhigh<height)?scrnhigh:height,width,height,theimage,px,py);
          break;
          }
#endif
        /* otherwise, FALLS THROUGH */
      
      default:
        /* for 320x200 (I think), 320x240, 320x400, 360x480 we need the
         * length to be a multiple of 8. :-( We allocate an image-width
         * line (blanked), and display via that.
         * (320x400/360x480 were dealt with earlier though; can only
         * be 320x200/320x240 if it gets here.)
         *
         * This isn't needed for svgalib >=1.3.1, but I'm leaving it in
         * for `if it ain't broke don't fix it' reasons. :-)
         */
        if(curvgamode==G320x200x256 || curvgamode==G320x240x256)
          {
          static unsigned char hack[320];
          
          memset(hack,0,320);
          for(y=0;(y<height-py)&&(y<scrnhigh);y++)
            {
            /* assume pixelsize==1, because it must be */
            memcpy(hack+x_add,theimage+((py+y)*width+px),xdim);
            vga_drawscansegment(hack,0,y+y_add,scrnwide);
            }
          }
        else
          for(y=0;(y<height-py)&&(y<scrnhigh);y++)
            eventuallydrawscansegment(theimage+pixelsize*((py+y)*width+px),
            	x_add,y+y_add,xdim*pixelsize);
        break;
      }
    }
  }
}


static int inline get15bitcolour_dithered(int x,int y,int r,int g,int b)
{
/* ordered dither, based on that used for 16-colour mode */
static unsigned char ditherdata[12]=
  {
  /* red */
  2,6,
  8,4,
  
  /* green - mirrored */
  6,2,
  4,8,
  
  /* blue - flipped */
  8,4,
  2,6,
  };

unsigned char *dithptr=ditherdata+(y&1)*2+(x&1);
int br,bg,bb;
  
br=(r&~7)+8*((r&7)>=*dithptr);
bg=(g&~7)+8*((g&7)>=dithptr[4]);
bb=(b&~7)+8*((b&7)>=dithptr[8]);
if(br>255) br=255;
if(bg>255) bg=255;
if(bb>255) bb=255;

return(GET15BITCOLOUR(br,bg,bb));
}


static int inline get16bitcolour_dithered(int x,int y,int r,int g,int b)
{
/* ordered dither, based on that used for 16-colour mode */
static unsigned char ditherdata[12]=
  {
  /* red */
  2,6,
  8,4,
  
  /* green - mirrored */
  3,1,
  2,4,
  
  /* blue - flipped */
  8,4,
  2,6,
  };

unsigned char *dithptr=ditherdata+(y&1)*2+(x&1);
int br,bg,bb;
  
br=(r&~7)+8*((r&7)>=*dithptr);
bg=(g&~3)+4*((g&3)>=dithptr[4]);
bb=(b&~7)+8*((b&7)>=dithptr[8]);
if(br>255) br=255;
if(bg>255) bg=255;
if(bb>255) bb=255;

return(GET16BITCOLOUR(br,bg,bb));
}


#define MODE_WIDTH_BUF_SIZE	16384

/* this variant should never be called with scrnpixelsize==3. */
void eventuallydrawscansegment_without_bc(byte *ptr,int x,int y,int len)
{
static unsigned short buf[MODE_WIDTH_BUF_SIZE];

switch(scrnpixelsize)
  {
  case 4:	/* 32-bit */
    {
    unsigned long *bufref=(unsigned long *)buf;
    unsigned char *ptrend=ptr+len;
    int i=0;
    
    for(;ptr<ptrend;ptr+=3,i++)
      *bufref++=GET32BITCOLOUR(ptr[2],ptr[1],*ptr);
    gl_putbox(x,y,i,1,buf);
    }
    break;
    
  case 3:	/* 24-bit, on non-svgalib backends */
    gl_putbox(x,y,len/3,1,ptr);
    break;
    
  case 2:	/* 15/16-bit */
    {
    unsigned short *bufref=buf;
    unsigned char *ptrend=ptr+len;
    int xx;
    
    if(scrncols==32768)
      {
      if(cfg.dither_hicol)
        for(xx=x;ptr<ptrend;ptr+=3,xx++)
          *bufref++=get15bitcolour_dithered(xx,y,ptr[2],ptr[1],*ptr);
      else
        for(;ptr<ptrend;ptr+=3)
          *bufref++=GET15BITCOLOUR(ptr[2],ptr[1],*ptr);
      }
    else
      {
      if(cfg.dither_hicol)
        for(xx=x;ptr<ptrend;ptr+=3,xx++)
          *bufref++=get16bitcolour_dithered(xx,y,ptr[2],ptr[1],*ptr);
      else
        for(;ptr<ptrend;ptr+=3)
          *bufref++=GET16BITCOLOUR(ptr[2],ptr[1],*ptr);
      }
    
    gl_putbox(x,y,bufref-buf,1,buf);
    }
    break;
    
  default:	/* 8-bit */
    if(curvgamode==G640x480x16)
      dither16scansegment(ptr,x,y,len);
    else
      vga_drawscansegment(ptr,x,y,len);
  }
}


void eventuallydrawscansegment_with_bc(byte *ptr,int x,int y,int len)
{
static unsigned short buf[MODE_WIDTH_BUF_SIZE];
unsigned char *ptrend=ptr+len;

/* if we get here, scrnpixelsize must be 2, 3, or 4. */

switch(scrnpixelsize)
  {
  case 4:	/* 32-bit */
    {
    unsigned long *bufref=(unsigned long *)buf;
    unsigned char *ptrend=ptr+len;
    int i=0;
    
    for(;ptr<ptrend;ptr+=3,i++)
      *bufref++=GET32BITCOLOUR(palt[ptr[2]],palt[ptr[1]],palt[*ptr]);
    gl_putbox(x,y,i,1,buf);
    }
    break;
  
  case 3:	/* 24-bit */
    {
    unsigned char *bufref=(unsigned char *)buf;	/* no kludges here, senor */
    
    while(ptr<ptrend)
      *bufref++=palt[*ptr++];
    
    gl_putbox(x,y,(bufref-(unsigned char *)buf)/3,1,buf);
    break;
    }
  
  case 2:	/* 15/16-bit */
    {
    unsigned short *bufref=buf;
    int xx;
    
    if(scrncols==32768)
      {
      if(cfg.dither_hicol)
        for(xx=x;ptr<ptrend;ptr+=3,xx++)
          *bufref++=get15bitcolour_dithered(xx,y,palt[ptr[2]],palt[ptr[1]],palt[*ptr]);
      else
        for(;ptr<ptrend;ptr+=3)
          *bufref++=GET15BITCOLOUR(palt[ptr[2]],palt[ptr[1]],palt[*ptr]);
      }
    else
      {
      if(cfg.dither_hicol)
        for(xx=x;ptr<ptrend;ptr+=3,xx++)
          *bufref++=get16bitcolour_dithered(xx,y,palt[ptr[2]],palt[ptr[1]],palt[*ptr]);
      else
        for(;ptr<ptrend;ptr+=3)
          *bufref++=GET16BITCOLOUR(palt[ptr[2]],palt[ptr[1]],palt[*ptr]);
      }
    
    gl_putbox(x,y,bufref-buf,1,buf);
    break;
    }
  
  default:
	printf("not high-colour in ..._with_bc, can't happen!\n");
    exit(1);
  }
}


/* only called for 640x480x4-bit */
void dither16scansegment(unsigned char *ptr,int x,int y,int len)
{
static unsigned char scanline[640];
unsigned char *lptr=scanline;
int f;

if(!cfg.viewer16col)
  {
  /* greyscale, but dithered between the scales;
   * dithering slows it down more than a direct mapping would,
   * but it just looks too nasty without it.
   */
  static unsigned char ditherdata[4]=
    {
    1,4,
    3,2
    };
  unsigned char c,*dithptr=ditherdata+(y&1)*2+(x&1);
  int xm=2-(x&1);
  
  for(f=0;f<len;f++)
    {
    if((c=dither16_greylookup[ptr[f]])>=60)
      c=60;	/* saves needing a range check after :-) */
    
    *lptr++=((c>>2)+((c&3)>=*dithptr++));
    if(!(--xm))
      dithptr-=(xm=2);
    }
  }
else
  {
  /* colour */
  if(cfg.fastdither16col)
    {
    /* ordered dither */
    static unsigned char ditherdata[48]=
      {
      /* red */
      1*4-2,13*4-2, 4*4-2,16*4-2,
      9*4-2, 5*4-2,12*4-2, 8*4-2,
      3*4-2,15*4-2, 2*4-2,14*4-2,
      7*4-2,10*4-2, 6*4-2,11*4-2,
  
      /* green - mirrored */
      16*4-2, 4*4-2,13*4-2, 1*4-2,
       8*4-2,12*4-2, 5*4-2, 9*4-2,
      14*4-2, 2*4-2,15*4-2, 3*4-2,
      11*4-2, 6*4-2,10*4-2, 7*4-2,
  
      /* blue - flipped */
      7*4-2,10*4-2, 6*4-2,11*4-2,
      3*4-2,15*4-2, 2*4-2,14*4-2,
      9*4-2, 5*4-2,12*4-2, 8*4-2,
      1*4-2,13*4-2, 4*4-2,16*4-2
      };
    
    unsigned char *dithptr=ditherdata+(y&3)*4+(x&3);
    unsigned char *palptr;
    int xm=4-(x&3);
  
    for(f=0;f<len;f++)
      {
      palptr=dither16_rgb+ptr[f]*3;
      *lptr++=(((*palptr>=*dithptr)*4)|
               ((palptr[1]>=dithptr[16])*2)|
               (palptr[2]>=dithptr[32]));
      dithptr++;
      if(!(--xm))
        dithptr-=(xm=4);
      }
    }
  else
    {
    /* error-diffused dither, based on the one in readpnm.c */
    static int evenerr[(640+10)*3],odderr[(640+10)*3];  /* 640x480-only */
    static int oldline=(1<<30);
    unsigned char *theline=ptr;
    int width=len;
    int xx,lx;
    int c0,c1,c2,times2;
    int terr0,terr1,terr2,actual0,actual1,actual2;
    int start,addon,r,g,b,c;
    int *thiserr;
    int *nexterr;

    /* we assume going backwards = new set of lines being drawn */
    if(y<oldline)
      {
      memset(evenerr,0,sizeof(evenerr));
      memset(odderr,0,sizeof(odderr));
      }
    oldline=y;
  
    if((y&1)==0)
      {start=0; addon=1;
      thiserr=evenerr+3; nexterr=odderr+width*3;}
    else
      {start=width-1; addon=-1;
      thiserr=odderr+3; nexterr=evenerr+width*3;}
    nexterr[0]=nexterr[1]=nexterr[2]=0;
    xx=start;
    for(lx=0;lx<width;lx++)
      {
      c=theline[xx];
      b=palb[c];
      g=palg[c];
      r=palr[c];

      terr0=r+((thiserr[0]+8)>>4);
      terr1=g+((thiserr[1]+8)>>4);
      terr2=b+((thiserr[2]+8)>>4);
      
      actual0=(terr0>=128)?255:0;
      actual1=(terr1>=128)?255:0;
      actual2=(terr2>=128)?255:0;
      c=(((actual0&1)<<2)|((actual1&1)<<1)|(actual2&1));
      
      c0=terr0-actual0;
      c1=terr1-actual1;
      c2=terr2-actual2;

      times2=(c0<<1);
      nexterr[-3] =c0; c0+=times2;
      nexterr[ 3]+=c0; c0+=times2;
      nexterr[ 0]+=c0; c0+=times2;
      thiserr[ 3]+=c0;

      times2=(c1<<1);
      nexterr[-2] =c1; c1+=times2;
      nexterr[ 4]+=c1; c1+=times2;
      nexterr[ 1]+=c1; c1+=times2;
      thiserr[ 4]+=c1;

      times2=(c2<<1);
      nexterr[-1] =c2; c2+=times2;
      nexterr[ 5]+=c2; c2+=times2;
      nexterr[ 2]+=c2; c2+=times2;
      thiserr[ 5]+=c2;

      scanline[xx]=c;

      thiserr+=3;
      nexterr-=3;
      xx+=addon;
      }
    }
  }

vga_drawscansegment(scanline,x,y,len);
}


int getvpix(int px,int py,int x,int y)
{
int p1,p2;

if((vkludge)&&(pixelsize==1))
  {
  p1=*(theimage+(py+y)*width+px+x);
  if(px+x+1>=width) return(p1);
  p1*=3;
  p2=*(theimage+(py+y)*width+px+x+1)*3;
  return(closest((pal32_no_bc[p1  ]+pal32_no_bc[p2  ])>>1,
                 (pal32_no_bc[p1+1]+pal32_no_bc[p2+1])>>1,
                 (pal32_no_bc[p1+2]+pal32_no_bc[p2+2])>>1));
  }
else
  return(*(theimage+pixelsize*((py+y)*width+px+x)));
}


/* this routine is nasty writ big, but about as quick as I can manage */
void drawzoomedgif()
{
int a,b,x,yp,yw;
long y,sw,sh,lastyp;
int xoff,yoff;
int x_add,y_add;	/* for centering */
int pixwide,pixhigh;
int bigimage;
byte *rline,*sptr,*dptr;
int tmp1,tmp2,tr,tg,tb,tn;

if((rline=calloc(scrnwide*pixelsize,1))==NULL) return;

/* try landscapey */
sw=scrnwide; sh=(scrnwide*height)/width;
if(sh>scrnhigh)
  /* no, oh well portraity then */
  sh=scrnhigh,sw=(scrnhigh*width)/height;

/* so now our zoomed image will be sw x sh */

/* fix centering if needed */
x_add=y_add=0;
if(cfg.centreflag)
  {
  if(sw<scrnwide)
    x_add=((scrnwide-sw)>>1);
  
  if(sh<scrnhigh)
    y_add=((scrnhigh-sh)>>1);
  
  if(virtual) x_add>>=1;
  }

bigimage=(width>sw)?1:0;   /* 1 if image has been reduced, 0 if made bigger */
if(bigimage)
  /* it's been reduced - easy, just make 'em fit in less space */
  {
  if(virtual) sw>>=1;
  lastyp=-1;
  if(!override_zoom_clear)	/* for animate_gif() */
    vga_clear();
  pixhigh=(int)(((float)height)/((float)sh)+0.5);
  pixwide=(int)(((float)width)/((float)sw)+0.5);
  pixhigh++;
  pixwide++;
  for(y=0;y<height;y++)
    {
    yp=(y*sh)/height;
    if(yp!=lastyp)
      {
      yw=y*width;
      if(vkludge)
        {
        /* we try to resample it a bit */
        for(x=0;x<width;x++,yw++)
          {
          tr=tg=tb=tn=0;
          for(b=0;(b<pixhigh)&&(y+b<height);b++)
            for(a=0;(a<pixwide)&&(x+a<width);a++)
              {
              if(scrncols<=256)
                {
                tmp2=*(theimage+yw+a+b*width)*3;
                tr+=pal32_no_bc[tmp2];
                tg+=pal32_no_bc[tmp2+1];
                tb+=pal32_no_bc[tmp2+2];
                }
              else
                {
                tb+=*(theimage+3*(yw+a+b*width));
                tg+=*(theimage+3*(yw+a+b*width)+1);
                tr+=*(theimage+3*(yw+a+b*width)+2);
                }  
              tn++;
              }
          tr/=tn; tg/=tn; tb/=tn;
          if(scrncols<=256)
            rline[(x*sw)/width]=closest(tr,tg,tb);
          else
            {
            tmp1=(x*sw)/width*pixelsize;
            rline[tmp1]=tb;
            rline[tmp1+1]=tg;
            rline[tmp1+2]=tr;
            }
          }
        }
      else	/* not vkludge... */
        for(x=0;x<width;x++,yw++)
          {
          if(scrncols<=256)
            rline[(x*sw)/width]=*(theimage+yw);
          else
            memcpy(rline+(x*sw)/width*3,theimage+yw*3,3);
          }
      eventuallydrawscansegment(rline,x_add,yp+y_add,sw*pixelsize);
      lastyp=yp;
      }
    }
  free(rline);
  }
else
  {
  /* well, we need to fill in the gaps because it's been made bigger.
   * However, it makes more sense to look at it the other way around.
   * So we don't just make small pixels bigger; instead, we go through
   * each of the pixels in the scaled-up image and copy an appropriate
   * pixel from the original (smaller) image. This is a lot simpler
   * and cleaner than the more obvious approach - and I speak from
   * experience. :-)
   */
  if(virtual)
    sw>>=1;
  for(y=0;y<sh;y++)
    {
    yoff=(y*height)/sh; 
    if(yoff>=height) yoff=height-1;
    dptr=rline;
   
    for(x=0;x<sw;x++)
      {
      xoff=(x*width)/sw;
      if(xoff>=width) xoff=width-1;
      
      if(scrncols<=256)
        /* image must be 8-bit */
        *dptr++=*(theimage+yoff*width+xoff);
      else
        {
        sptr=theimage+3*(yoff*width+xoff);
        *dptr++=*sptr; *dptr++=sptr[1]; *dptr++=sptr[2];
        }
      }
    
    /* draw the line */
    eventuallydrawscansegment(rline,x_add,y+y_add,sw*pixelsize);
    }
  free(rline);
  }
}


void fx_mirror()
{
unsigned char *src,*dst;
byte *tmp;
int x,y;

tmp=malloc(width*pixelsize);
if(tmp==NULL) return;

if(pixelsize==1)
  {
  for(y=0;y<height;y++)
    {
    dst=tmp;
    src=theimage+y*width+width-1;
    for(x=0;x<width;x++)
      *dst++=*src--;
    memcpy(src+1,tmp,width);
    }
  }
else
  {
  /* we know pixelsize must be 3 then, so take advantage of that */
  
  for(y=0;y<height;y++)
    {
    dst=tmp;
    src=theimage+3*(y*width+width-1);
    for(x=0;x<width;x++)
      {
      *dst++=*src++;
      *dst++=*src++;
      *dst++=*src;
      src-=5;
      }
    memcpy(src+3,tmp,width*3);
    }
  }

free(tmp);
}


void fx_flip()
{
unsigned char *tmp,*ptr1,*ptr2;
int y,y2,wp=width*pixelsize;

tmp=malloc(wp);
if(tmp==NULL) return;

ptr1=theimage;
ptr2=theimage+(height-1)*wp;

for(y=0,y2=height-1;y<height/2;y++,y2--,ptr1+=wp,ptr2-=wp)
  {
  memcpy(tmp,ptr1,wp);
  memcpy(ptr1,ptr2,wp);
  memcpy(ptr2,tmp,wp);
  }

free(tmp);
}


/* NB: you must do a vga_clear() after this and before redisplaying
 * the picture, otherwise the old one is left onscreen!
 */
void fx_rot()
{
unsigned char *tmp,*tmp2=NULL,*ptr1,*ptr2=NULL;
int x,y,y2,wp=width*pixelsize,hp=height*pixelsize;
int blockup=4;

tmp=malloc(wp*height);
if(pixelsize==3) tmp2=malloc(wp*blockup);
if(tmp==NULL || (pixelsize==3 && tmp2==NULL)) return;

if(pixelsize==1)
  for(y=0,y2=hp-1;y<height;y++,y2--)
    {
    ptr1=tmp+y2; ptr2=theimage+wp*y;
    for(x=0;x<width;x++,ptr1+=hp)
      *ptr1=*ptr2++;
    }
else
  {
  int hpm3=hp-3;
  
  for(y=0,y2=height-1;y<height;y++,y2--)
    {
    ptr1=tmp+y2*3;
    if(y%blockup==0)
      {
      memcpy(tmp2,theimage+wp*y,wp*((y2<blockup)?(y2+1):blockup));
      ptr2=tmp2;
      }
    for(x=0;x<width;x++,ptr1+=hpm3)
      {
      *ptr1++=*ptr2++;
      *ptr1++=*ptr2++;
      *ptr1++=*ptr2++;
      }
    }
  }
  
x=height;y=width;
width=x;height=y;

if(pixelsize==3) free(tmp2);
free(theimage);
theimage=tmp;
}


/* rgb values must be 0..31; 6 bits of the rgb count in VGA 8-bit modes,
 * so this does effectively lose a bit of detail, but since we're
 * picking from a restricted set anyway it's probably lost in the noise.
 */
int closest(int r,int g,int b)
{
int idx;
unsigned char *ptr,distnum;
int xr,xg,xb,dist,distquan,f,checknumcols;

idx=((b<<10)|(g<<5)|r);

if(vkcache_valid[idx])
  return(vkcache[idx]);

distnum=0;
distquan=(1<<30);

/* if numcols=256 we do 0-255, otherwise 0-numcols+63 */
checknumcols=((numcols==256)?256:numcols+64);

for(ptr=pal32_no_bc,f=0;f<checknumcols;f++)
  {
  xr=(r-*ptr++);
  xg=(g-*ptr++);
  xb=(b-*ptr++);
  if((dist=xr*xr+xg*xg+xb*xb)<distquan)
    {
    distnum=f;
    distquan=dist;
    if(dist==0) break;  /* premature exit if it can't get any better */
    }
  }

vkcache_valid[idx]=1;
return((int)(vkcache[idx]=distnum));
}


void samecentre(int *ppx,int *ppy,int newscale,
                int oldpx,int oldpy,int oldscale)
{
int xa,ya,sw,sh;

/* even if the centre flag is off, we still need to do this */
xa=ya=0;
sw=oldscale*width;
sh=oldscale*height;
if(sw<scrnwide) xa=(scrnwide-sw)>>1;
if(sh<scrnhigh) ya=(scrnhigh-sh)>>1;  

/* finds centre of old screen, and makes it centre of new one */
*ppx=(oldpx-xa+(scrnwide>>1))*newscale/oldscale;
*ppy=(oldpy-ya+(scrnhigh>>1))*newscale/oldscale;

xa=ya=0;
sw=newscale*width;
sh=newscale*height;
if(sw<scrnwide) xa=(scrnwide-sw)>>1;
if(sh<scrnhigh) ya=(scrnhigh-sh)>>1;  

*ppx-=(scrnwide>>1)+xa;
*ppy-=(scrnhigh>>1)+ya;
}


/* this gets run on picture exit when cfg.selecting is true.
 * the output is designed to be put straight into a pnmcut command-line.
 */
void show_dimensions(int px,int py,int scaling)
{
int x,y,w,h;

x=px/scaling; y=py/scaling;
if(width*scaling<scrnwide)  w=width;  else w=scrnwide/scaling;
if(height*scaling<scrnhigh) h=height; else h=scrnhigh/scaling;
printf("%4d %4d %4d %4d\n",x,y,w,h);
}


/* change from one orientation state to another.
 * See the big comment about this earlier on (near the start of the file)
 * if you feel a `!?' coming on.
 *
 * clear_if_rot is non-zero if it should do a vga_clear() if it needs
 * to do a rotate. See the comment above fx_rot()'s def for why.
 */
void orient_change_state(int from,int to,int clear_if_rot)
{
/* the basic idea is this:
 *
 * - if from and to are equal, return.
 * - if a single flip/mirror/rot will do it, use that.
 * - otherwise, try a rotate if we know it's needed (see below).
 * - then see if a flip/mirror does the trick.
 * - if not, it must need flip *and* mirror.
 */
int state=from;

if(from==to) return;

/* try a one-step route. */
if(orient_state_flip[state]==to)	{ fx_flip(); return; }
if(orient_state_mirror[state]==to)	{ fx_mirror(); return; }
if(orient_state_rot_cw[state]==to)
  {
  fx_rot();
  if(clear_if_rot) vga_clear();
  return;
  }

/* nope, ok then, things get complicated.
 * we can get any required rotate out of the way -
 * if it's switched from portrait to landscape or vice versa, we must
 * need one. That's if it's gone from 0..3 to 4..7 or 4..7 to 3..0.
 */
if((from<4 && to>=4) || (from>=4 && to<4))
  {
  fx_rot();
  if(clear_if_rot) vga_clear();
  state=orient_state_rot_cw[state];
  }

/* now try a flip/mirror. */
if(orient_state_flip[state]==to)	{ fx_flip(); return; }
if(orient_state_mirror[state]==to)	{ fx_mirror(); return; }

/* no? Well it must need both then. */
fx_flip();
fx_mirror();

/* sanity check */
if(orient_state_mirror[orient_state_flip[state]]!=to)
	printf("can't happen - orient_change_state(%d,%d) failed!\n",
  	from,to);
}


/* returns:
 * -1 if we want to go to previous image,
 *  1 if we want to go to next image,
 *  2 if we want to tag this image and go to next,
 * else 0.
 * (non-zero returns only meaningful when auto-animating)
 */
int animate_gif(int orient_state)
{
static int orient_to_movetype[8]={0,1,1,0,3,2,3,2};	/* see below */
int frame,delay,key,f;
int framewidth,frameheight;
int oldwidth,oldheight;
int paused=0;
int done=0;
unsigned char *oldimage;
unsigned char *dupimage=NULL;
int num_images=gif_delaycount;
int movetype;
int offset,startoffset;
unsigned int tagtimeout=(tagview_mode?cfg.tag_timer*100:0);
unsigned int hundredths=0;
int ret=0;

if(num_images<2 || pixelsize!=1)
  return(0);

/* the direction and axis of movement along the image between
 * frames depends on the orientation. The movetypes are:
 *
 * 0 - top-to-bottom, height=real height/num images
 * 1 - bottom-to-top, ditto
 * 2 - left-to-right, width=real width/num images
 * 3 - right-to-left, ditto
 *
 * (We only need a lookup table due to my slightly screwy orientation
 * state numbering scheme. :-))
 */
movetype=orient_to_movetype[orient_state];

/* a rotated image also needs a buffer frame we can copy partial image
 * lines to temporarily.
 */
if(movetype<2)
  framewidth=width,frameheight=height/num_images;
else
  {
  frameheight=height,framewidth=width/num_images;
  if((dupimage=malloc(height*(width/num_images)))==NULL)
    return(0);
  }

vga_clear();

/* don't clear screen if in zoom mode */
override_zoom_clear=1;

frame=0;
startoffset=0;
if((movetype&1)) startoffset=num_images-1;
offset=startoffset;

while(!done)
  {
  delay=gif_delay[frame];
  if(delay<=0) delay=1;		/* stop it going too fast */
  
  oldimage=theimage; oldwidth=width; oldheight=height;

  if(movetype<2)
    theimage+=offset*width*frameheight;
  else
    {
    /* copy the frame to the frame-sized buffer */
    theimage+=offset*framewidth;
    for(f=0;f<height;f++)
      {
      if(f) theimage+=width;
      memcpy(dupimage+f*framewidth,theimage,framewidth);
      }
    
    theimage=dupimage;
    }
  
  width=framewidth;
  height=frameheight;
  
  redrawgif(0,0,0,0);		/* XXX not great to have to force top-left */

  theimage=oldimage; width=oldwidth; height=oldheight;

  zgv_io_timer_start(delay);

  while(!done && (!zgv_io_timer_flag || paused))
    {
    key=wait_for_keys_or_mouse(zgv_ttyfd);
    
    /* convert xzgv-ish keypresses */
    if(cfg.xzgvkeys)
      switch(key)
        {
        case 'b':	key=127; break;
        case ' ':	key=RK_ENTER; break;
        case RK_CTRLSPACE: key=' '; break;
        }

    if(has_mouse)
      {
      if(is_end_click_left())
        key='p';
      if(is_end_click_right())
        key=RK_ESC;
      }

    if(key=='n')
      {
      zgv_io_timer_stop();
      break;		/* hence not in the switch() below */
      }
    
    switch(key)
      {
      case 'p':		paused=!paused; break;
      case RK_ESC: case 'x': done=1; break;		/* exit */
      case RK_ENTER:	done=1; ret=1; break;		/* next image */
      case ' ':		done=1; ret=2; break;		/* tag & next */
      case 127: case 8:	done=1; ret=-1; break;		/* prev image */
      }
    }
  
  hundredths+=delay;
  if(tagtimeout && hundredths>=tagtimeout)
    done=1,ret=1;
  
  /* next frame */
  frame++;
  offset+=(movetype&1)?-1:1;
  if(frame>=num_images)
    {
    frame=0;
    offset=startoffset;
    }
  }

override_zoom_clear=0;

zgv_io_timer_stop();

if(dupimage)
  free(dupimage);

return(ret);
}
