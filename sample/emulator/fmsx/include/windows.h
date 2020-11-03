//	Win.H:  Window header file

//	Flags
#define dialog 1
#define textbox 2

typedef struct {
	int16_t winflags;
	int16_t winx, winx8, winy;
	int16_t winxl, winxl16, winyl, winyl16;
	int16_t winh, winh16, winv, winv16;
	vptype border;
	vptype inside;
	vptype topleft;
	} wintype;

extern void defwin (wintype *win,int16_t x8,int16_t y,int16_t xl16,int16_t yl16,int16_t h16,int16_t v16,int16_t flags);
extern void undrawwin (wintype *win);
extern void drawwin (wintype *win);
extern void wprint (vptype *vp,int16_t x,int16_t y,int16_t font,char *text);
extern int16_t wgetkey (vptype *vp,int16_t x,int16_t y,int16_t font);
extern void wprintc (vptype *vp,int16_t y,int16_t font,char *text);
extern void winput (vptype *vp,int16_t x,int16_t y,int16_t font,char *text,int16_t maxlen);
extern void initvp (vptype *vp,int16_t bkgnd);
extern void clearvp (vptype *vp);
extern void fontcolor (vptype *vp,int16_t hi,int16_t back);

extern void titlewin (wintype *win,char *text,int16_t flg);