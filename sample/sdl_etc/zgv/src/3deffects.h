/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * 3deffects.h - prototypes for 3deffects.c
 */

/* type of msgbox */
#define MSGBOXTYPE_OK       1        /* will always cause return of 1 */
#define MSGBOXTYPE_YESNO    2        /* causes return of 1=yes, 0=no */
#define MSGBOXTYPE_OKCANCEL 3        /* similar to YESNO - NYI */
#define MSGBOXTYPE_FILEDETAILS 4

extern int msgbox_draw_ok;

extern void drawtext3d(int x,int y,int s,char *str,
                       int isout,int light,int dark,int txt);
extern void undrawtext3d(int x,int y,int s,char *str);
extern void draw3dbox(int x1,int y1,int x2,int y2,int depth,
                      int isout,int light,int dark);
extern void undraw3dbox(int x1,int y1,int x2,int y2,int depth);
extern void drawbutton(int x1,int y1,int x2,int y2,char *str,int centred,
                       int light,int dark,int black,int txt);
extern int msgbox_did_restore(void);
extern int msgbox(int ttyfd,char *message,int replytype,
                  int light,int dark,int txt);
extern char *cm_getline(int ttyfd,char *prompt,
	int light,int dark,int txt,int med);
