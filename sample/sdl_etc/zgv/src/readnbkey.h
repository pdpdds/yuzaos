/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readnbkey.h - #defines for the readnbkey() routine's return values.
 */


extern int readnbkey(int ttyfd);
extern void click_update(void);
extern int wait_for_keys_or_mouse(int fd);
extern int mousecur_wait_for_keys_or_mouse(int fd);
extern void save_mouse_pos(void);
extern void restore_mouse_pos(void);
extern void restore_mouse_pos_with_size(int width,int height);
extern int is_end_click_left(void);
extern int is_end_click_right(void);
extern int is_start_click_left(void);
extern int is_start_click_right(void);

#define RK_NO_KEY         0

#define RK_F10            0x10A  
#define RK_F11            0x10B  /* NB: same as RK_SHIFT_F1 */
#define RK_F12            0x10C  /* NB: same as RK_SHIFT_F2 */
#define RK_F1             0x101
#define RK_F2             0x102
#define RK_F3             0x103
#define RK_F4             0x104
#define RK_F5             0x105
#define RK_F6             0x106
#define RK_F7             0x107
#define RK_F8             0x108
#define RK_F9             0x109 
#define RK_SHIFT_F10      0x114
#define RK_SHIFT_F1       0x10B
#define RK_SHIFT_F2       0x10C
#define RK_SHIFT_F3       0x10D
#define RK_SHIFT_F4       0x10E
#define RK_SHIFT_F5       0x10F
#define RK_SHIFT_F6       0x110
#define RK_SHIFT_F7       0x111
#define RK_SHIFT_F8       0x112
#define RK_SHIFT_F9       0x113

/* NB: shift F11 and F12 are the same as normal F11 and F12, which in turn are
 * the same as shift F1 and F2.
 */


#define RK_CURSOR_LEFT    0x200
#define RK_CURSOR_DOWN    0x201
#define RK_CURSOR_UP      0x202
#define RK_CURSOR_RIGHT   0x203
#define RK_HOME           0x204
#define RK_END            0x205
#define RK_PAGE_UP        0x206
#define RK_PAGE_DOWN      0x207
#define RK_INSERT         0x208
#define RK_DELETE         0x209

#define RK_ENTER          10
#define RK_ESC            27
#define RK_TAB            9

/* this needs to be out of the way, but still fit in a byte */
#define RK_CTRLSPACE      0x80
