//	GR.H:  Header of data types for shapes

#include "../port.h"

void setcolor (int16_t c, int16_t n1, int16_t n2, int16_t n3);

#define byte unsigned char
#define x_modes 5
#define shm_maxtbls 64

extern int16_t shm_want[shm_maxtbls];
extern char *shm_tbladdr[shm_maxtbls];
extern int16_t shm_tbllen[shm_maxtbls];
extern int16_t shm_flags[shm_maxtbls];

#define shm_fontf 	1
#define shm_xxxxx 	2
#define shm_blflag 	4

//	Shape Manager Stuff

#define st_byte		0
#define st_plain		1
#define st_rle			2

#define numcolors (1<<(numcolorbits))
#define colormask (numcolors-1)
#define shsize_cga(xl,yl) ((yl)*((xl+3)/4))
#define shsize_ega(xl,yl) (4*(yl)*((xl+7)/8))
#define shsize_vga(xl,yl) (4*yl*((xl+3)/4))

//	Graphics Driver Header

typedef struct {
	int16_t vpx, vpy;							// Absolute start wrt Screen
	int16_t vpxl, vpyl;						// Length
	int16_t vpox, vpoy;						// Origin Offset
	int16_t vphi, vpback;
	} vptype;

#define varincrx -16304

extern byte x_ourmode;
extern vptype mainvp;
extern uint16_t cmtab [4][256];
extern int16_t pagemode, pageshow, pagedraw;
extern int16_t showofs, drawofs, pagelen;
extern char pixvalue;

#define x_cga			0
#define x_cgagrey  	1
#define x_ega			2
#define x_egagrey		3
#define x_vga			4
#define x_mcga			6
#define x_modetype (x_ourmode & 0xfe)

extern void drawshape (vptype *vp, int16_t n, int16_t x, int16_t y);
//	n<0 = erase shape (-n)
extern void clrvp (vptype *vp,byte col);
extern void scrollvp (vptype *vp,int16_t xd,int16_t yd);
extern void scroll (vptype *vp,int16_t x0,int16_t y0,int16_t x1,int16_t y1,int16_t xd,int16_t yd);
extern void plot (vptype *vp,int16_t x,int16_t y,int16_t color);
extern void gr_init (void);
extern int16_t  gr_config (void);
extern void gr_exit (void);
extern void fntcolor (int16_t hi,int16_t lo,int16_t back);
extern void waitsafe (void);						// Wait until safe retrace
extern void setpagemode (int16_t mode);
extern void setpages (void);
extern void pageflip (void);
extern void fadein (void);
extern void fadeout (void);
extern void clrpal (void);

typedef unsigned char p_rec[0x300];
extern void vga_setpal (void);
extern p_rec vgapal;

//	Shape Manager SHM.C

extern void shm_init (char *fname);
extern void shm_do (void);
extern void shm_exit (void);

//	Grl : Low-level graphics

//void ldrawsh_cga (vptype *vp, int16_t draw_x, int16_t draw_y, int16_t sh_xlb, int16_t sh_yl,
//   char far *shape, int16_t cmtable);
//void ldrawsh_ega (vptype *vp, int16_t draw_x, int16_t draw_y, int16_t sh_xlb, int16_t sh_yl,
//   char far *shape, int16_t cmtable);
void ldrawsh_vga (vptype *vp, int16_t draw_x, int16_t draw_y, int16_t sh_xlb, int16_t sh_yl,
   char far *shape, int16_t cmtable);
//void lcopypage(void);

void pixaddr_cga (int16_t x, int16_t y, char **vidbuf, unsigned char *bitc);
void pixaddr_ega (int16_t x, int16_t y, char **vidbuf, unsigned char *bitc);
void pixaddr_vga (int16_t x, int16_t y, char **vidbuf, unsigned char *bitc);
