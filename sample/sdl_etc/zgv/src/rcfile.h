/* zgv 5.2 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * rcfile.h - protos for rcfile.c
 */

extern struct zgv_config cfg;

/* rcfile.c */
extern void getconfig(void);
extern void fixconfig(void);
extern int parsecommandline(int argc,char **argv);
extern int modematch(int x, int y, int bpp);
