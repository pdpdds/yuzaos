/* zgv 5.8 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2004 Russell Marks. See README for license details.
 *
 * rc_config.h - Defined! the zgv_config structure.
 *                       photographic evidence on page 2. :-)
 */

struct cfgrgb_tag { int r,g,b; };

struct zgv_config
  {
  int videomode,zoom,zoom_reduce_only,vkludge,jpeg24bit,jpegspeed;
  int jpegindexstyle;  /* 0=v. fast, 1=fast, 2=normal, 3=slow (as in <=v2.7) */
  int centreflag,blockcursor,thicktext;
  int betterpgm;  /* grinds PGM to 24-bit internally allowing 256 shades */
  int hicolmodes;	/* force zgv to think card has high-colour modes */
  int nodelprompt,nodelprompt_tagged,perfectindex;
  int xvpic_index;
  int onefile_progress,cleartext;
  int force16fs;
  int repeat_timer,tag_timer;
  int revert,revert_orient,fakecols,selecting;
  int fs16col;
  int viewer16col,fastdither16col;
  int echotagged;
  int forgetoldpos;
  int linetext;
  int smallfstext;
  int writefile;
  int slowupdate;
  int shuffleslideshow;
  int scrollbar;
  int mousekludge;
  int mousescale;
  int stupid_gnu_verbosity;
  int fs_startmode;
  int bc_order_rev;
  int xzgvkeys;
  int automodefit;
  int deltamodefit;
  double initial_picgamma;
  int fsmagic;
  int black_bg;
  int auto_animate;
  int fullscreen;
  int dither_hicol;
  
  int brightness;
  double contrast;
  
  struct cfgrgb_tag black,dark,medium,light,marked;
  int mode_allowed[256];
  int showxvpicdir;

  int loop;		/* loop forever in slideshow */
  int svgalib_mouse;	/* whether we should try using mouse or not */
  
  int errignore;

  int pcdres;
  };
