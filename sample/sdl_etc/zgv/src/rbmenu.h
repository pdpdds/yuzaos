/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * rbmenu.h - defines for rbmenu.c
 */

struct rbm_data_tag
  {
  char active;
  char label[32];	/* so keep 'em short! */
  int key;		/* must be int - see readnbkey.h */
  };

#define RBM_ENTRY_HEIGHT	27
#define RBM_ENTRY_WIDTH		138
#define RBM_LEFT_XSKIP		8
#define RBM_RIGHT_XSKIP		RBM_LEFT_XSKIP
#define RBM_TOP_YSKIP		8
#define RBM_BOT_YSKIP		RBM_TOP_YSKIP

/* could have variable vertical size easily enough, but I thought
 * keeping the arrangement of buttons the same in all modes would be best.
 * Still, the variable version is here in the #if 0 bit.
 */
#if 0
#define RBM_HEIGHT		vga_getydim()
#else
#define RBM_HEIGHT		480
#endif

#define RBM_MAXENTRIES_Y \
	((RBM_HEIGHT-RBM_TOP_YSKIP-RBM_BOT_YSKIP)/RBM_ENTRY_HEIGHT)


extern void rbm_xysize(struct rbm_data_tag menu_data[],int *wp,int *hp);
extern void rbm_draw(struct rbm_data_tag menu_data[],
                     int light,int medium,int dark,int black);
extern int rbm_mousepos_to_key(struct rbm_data_tag menu_data[],int mx,int my);
extern void rbm_set_active_flag(struct rbm_data_tag menu_data[],
                                char *substr,int active);
extern void rbm_find_and_fix_ui_cols(int *lightp,int *mediump,
                                     int *darkp,int *blackp,
                                     int *mblackp,
                                     unsigned char *palr64,
                                     unsigned char *palg64,
                                     unsigned char *palb64);
