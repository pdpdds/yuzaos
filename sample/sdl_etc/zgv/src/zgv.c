/* zgv 5.8 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2003 Russell Marks.
 *
 * zgv.c - This provides the zgv file selector, and interfaces to the
 *         vga display routines (vgadisp.c)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <minwindef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <setjmp.h>
 //#include <pwd.h>
 //#include <grp.h>
 //#include <sys/param.h>
 //#include <sys/types.h>
 //#include <sys/stat.h>
 //#include <sys/file.h>
 //#include <sys/ioctl.h>
#include <errno.h>
#include "zgv_io.h"
#include "zgv.h"
#include <math.h>
#include <systemcall_impl.h>
#include <getenv.h>
/* these must come after zgv.h */
#ifdef OSTYPE_LINUX
//#include <sys/vt.h>
#endif
#ifdef OSTYPE_FREEBSD
#include <sys/consio.h>
#endif
#include "readgif.h"
#include "vgadisp.h"
#include "readnbkey.h"
#include "font.h"
#include "3deffects.h"
#include "helppage.h"
#include "rc_config.h"
#include "rcfile.h"
#include "readpnm.h"	/* for the dithering - should be moved! */
#include "resizepic.h"
#include "copymove.h"
#include "mousecur.h"
#include "scrollbar.h"
#include "rbmenu.h"
#include "gnuhelp.h"
#include "modesel.h"
#include "magic.h"

#define MAXPATHLEN 256

int stat_(char const* const fileName, struct stat* fno)
{
    memset(fno, 0, sizeof(struct stat));
    int result = fstat(fileName, fno);

    if (result == 0)
        return 0;

    return -1;
}

void putchar(char c)
{
    printf("%c", c);
}

void usleep(int time)
{
    Syscall_Sleep(time / 1000);
}

#define HOWFAR_LOADING_MSG	"Reading - please wait"

/* text used when copying >=2 files */
#define HOWFAR_COPYING_MSG	"Copying files - please wait"

/* text used when moving >=2 files */
#define HOWFAR_MOVING_MSG	"Moving files - please wait"

/* and deleting >=2 */
#define HOWFAR_DELETING_MSG	"Deleting files - please wait"

char* zgvhelp[] =
{
" ",
"? (question mark)\\this help page",
" ",
"up (also k or q)\\move file selection cursor up",
"down (also j or a)\\move file selection cursor down",
"left (also h or o)\\move file selection cursor left",
"right (also l or p)\\move file selection cursor right",
" ",
"Enter\\display file or change directory",
" ",
"v\\visual selection mode on/off",
"u\\create/update thumbnails in visual mode",
"Ctrl-R\\update directory listing / redraw screen",
"D or Delete\\delete file",
" ",
"Esc or x\\exit zgv",
""
};

struct rbm_data_tag filesel_menu_data[] =
{
    /* 31 chars max for label text */
    {1, "Quit zgv",		RK_ESC		},

    /* stuff in draw_rb_menu which sets the active field of these
     * mode buttons assumes they are in *exactly* this order/position.
     * So don't change anything before the "Tag all" button without
     * thinking twice... or more. :-)
     */
    {0,"-",0},
    {1, "640x480x8",		RK_F1		},
    {1, "800x600x8",		RK_F2		},
    {1, "1024x768x8",		RK_F3		},
    {1, "1280x1024x8",		RK_F4		},
    {0,"-",0},
    {1, "Tag all",		'T'		},
    {1, "Untag all",		'N'		},
    {0,"-",0},
    {1, "Tag",			't'		},
    {1, "Untag",			'n'		},
    {1, "Toggle tag",		' '		},
    {0,"-",0},
    {1, "Copy files",		'C'		},
    {1, "Move files",		'M'		},
    {1, "Delete file",		'D'		},

    {1, "Slideshow",		RK_TAB		},
    {1, " ...looping on/off",	'L'		},
    {1, " ...shuffle on/off",	'S'		},
    {0,"-",0},
    {1, "Rescan dir",		'R' - 0x40	},
    {0,"-",0},
    {1, "Show file info",		':'		},
    {1, "Thumbnails on/off",	'v'		},
    {1, " ...grey/colour",	'c'		},
    {0,"-",0},
    {1, "Thumbnail update",	'u'		},
    {1, " ...for directories",	'd'		},
    {0,"-",0},
    {1, "Sort by name",		128 + 'n'		},
    {1, " ...extension",		128 + 'e'		},
    {1, " ...file size",		128 + 's'		},
    {1, " ...date/time",		128 + 'd'		},

    {0,"",0}
};



/* from 18-bit RGB (as used by VGA palette) to 3:3:2 palette index */
#define MAKE332COL(r,g,b) (((r)>>3)*32+((g)>>3)*4+((b)>>4))

/* colour indicies used for filenames-only selector */
#define LIGHT		2
#define DARK		1
#define BLACK		15
#define MIDGREY		3
#define MARKEDCOL	14

/* these relate to the size of each file's entry ("bar") onscreen */
#define GDFYBIT			18
#define BARWIDTH		126
#define BAR_RESTRICT_WIDTH	BARWIDTH-4

#define DIR_OF_XPOS	(100)
#define DIR_OF_YPOS	(12)
#define DIR_OF_XSIZ	(fs_scrnwide-DIR_OF_XPOS-10)
#define COLS		(fs_scrnwide-5)
#define ROWS		(fs_scrnhigh-10-yofs)
#define XSIZ		(COLS/BARWIDTH)
#define YSIZ		(ROWS/barheight)
#define XSTARTPOS	(4+(COLS%BARWIDTH)/2)
#define XENDPOS		(XSTARTPOS+COLS)

#define fwinxpos(f) (XSTARTPOS+(((f)-1)/YSIZ)*BARWIDTH)
#define fwinypos(f) (yofs+(ROWS%barheight)/2-cfg.scrollbar*9+ \
			barheight*(((f)-1)%YSIZ))

/* maximum no. of `past positions' in dirs to save.
 * if it runs out of space the oldest entries are lost.
 */
#define MPPOSSIZ	256

 /* number of greyscales *less one* used in selector when in 640x480x16 mode */
#define GREY_MAXVAL	10


struct gifdir_tag
{
    char name[256];
    char isdir;            /* 0=file, 1=dir. */
    char xvw, xvh;		/* xvpic width and height, zero if none */
    char marked;
    off_t size;
    time_t mtime;
    int extofs;
};

int gifdirsiz;

struct gifdir_tag* gifdir = NULL;
int gifdir_byte_size = 64 * sizeof(struct gifdir_tag);
int gifdir_byte_incr = 32 * sizeof(struct gifdir_tag);

enum sorttypes
{
    sort_name, sort_ext, sort_size, sort_mtime
};

/* this has to be global so we can use it in gcompare() */
enum sorttypes filesel_sorttype = sort_name;


int* slideshow_idx = NULL, slideshow_idx_size;

int zgv_ttyfd = -1, howfar_upto;
jmp_buf setjmpbuf;   /* in case someone aborts decompressing a file */
static int xv332_how_far_xpos, xv332_how_far_ypos;
static int one_file_only = 0;
int original_vt, separate_vt = 0;
int zgv_vt;
int tagview_mode = 0;
int slider_drag = 0;				/* for scrollbar/mouse */

/* for right-buttom menu */
static int rb_ignore_first_left_click = 0;
static unsigned char* rb_save = NULL;
static int rb_save_width, rb_save_height;

int idx_light, idx_medium, idx_dark, idx_black, idx_marked;
int idx_blacknz; 	/* idx_blacknz is always black, and always non-zero */
            /* (it's needed for mouse cursor and buttons) */
int updating_index = 0;

int gdfsiz = 3;		/* size of filename text in selector */
int gdfofs = 0;

int barheight, yofs;	/* height of cursor, and offset of selection bit */

/* XXX this sgres stuff is bogus in the extreme */
extern int sgres;

int has_mouse = 0;

int current_colour = 0;	/* see zgv.h */

struct pastpos_tag
{
    int dev, inode, curent, startfrom;
} pastpos[MPPOSSIZ];

/* jpeg/png error messages are written into this */
char jpeg_png_errmsg[JPEG_PNG_ERRMSG_SIZE];

int fs_vgamode = G640x480x256;	/* current video mode for selector */
int fs_scrnwide, fs_scrnhigh;	/* width/height of selector mode */

/* stuff for checking old directories (to avoid symlink loops in
 * recursive update).
 */
struct olddir_tag
{
    dev_t device;
    ino_t inode;
};

static struct olddir_tag* olddirs = NULL;
static int olddir_byte_size = 64 * sizeof(struct olddir_tag);
static int olddir_byte_incr = 32 * sizeof(struct olddir_tag);
static int num_olddirs = 0;



/* prototypes */
int main(int argc, char* argv[]);
void write_gifdir_name(int entry, char* str);
//void my_mouse_init(void);
void openstdin_nonblocking(void);
void writeppm(void);
void load_one_file(char* filename);
void copyfromconfig(void);
void mainloop(void);
void fix_startfrom(int curent, int* startfromp);
int rename_file(int curent, int startfrom);
static void draw_rb_menu(void);
static void undraw_rb_menu(void);
static int rb_menu_event(int* keyp);
void new_pastpos(int curent, int startfrom);
void get_pastpos(int* curentp, int* startfromp);
int load_single_file(int curent, int do_howfar);
int load_tagged_files(void);
void delete_gifdir_element(int n);
void copymovedel_file_or_tagged_files(int curent, int remove_from_array,
    int no_dir_prompt,
    int (*func_ptr)(char*, char*),
    char* msg_action, char* msg_acting,
    char* msg_acted, char* msg_howfar);
int goto_named_dir(void);
int permissiondenied(char* fname);
void redrawall(int curent, int startfrom);
void inithowfar(char* msg);
void showhowfar(int sofar, int total);
void smallhowfar(int sofar, int total);
void showbar(int entnum, int selected, int startfrom);
int centreseltxt(int x, int fsiz, char* str);
void showgifdir(int startfrom, int unshow, int drawdirmsg, int do_one_only);
int ispicture(char* filename);
void readgifdir(int graphics_ok);
void sort_files(void);
int gcompare(void* gn1, void* gn2);
void prettyfile(char* buf, struct gifdir_tag* gifdptr, int bufsize);
void screenon(void);
void screenoff(void);
void cleartext(void);
void showerrmessage(int errnumber);
int delete_file(char* filename, int doprompt, int report_error);
int delete_file_simple(char* filename, char* junk);
void xv332_how_far(int sofar, int total);
void clear_xvpic(int xpos, int ypos);
int makexv332(char* filename, char* xvpicfn, unsigned int howfar);
int makedirxv332(char* filename, char* xvpicfn, unsigned int howfar);
void update_xvpics(int do_dirs_instead);
void recursive_update(void);
int fixvt(void);
void switchback(void);
void greyfix332(unsigned char** image, int w, int h, int xw7, int w8);
void wait_for_foreground(void);
void ctrlc(int foo);
int make_slideshow_idx_array(void);
void shuffle_slideshow_idx(void);
void gifdir_init(void);
int gifdir_resize_if_needed(int newent);



int main(int argc, char* argv[])
{
    int argsleft;

#ifdef BACKEND_SVGALIB
    if (fixvt() == 0)
    {
        fprintf(stderr, "zgv: not running on console and no free VTs found.\n");
        exit(1);
    }
#endif

    atexit(switchback);		/* this gets called after svgalib stuff */

    /* this gets called after svgalib's ^C handler, on ^C */
    //signal(SIGINT, ctrlc);

    vga_disabledriverreport();    /* quieten svgalib */
    vga_init();
    /* root permissions should now have been ditched */

    /* check modesel.c's mode array is valid & consistent (and quit if not) */
    check_modedesc_array();

    /*  Temporary workaround for svgalib security problem  --kvajk  */
    //{int fd = 3; while (!fcntl(fd, F_SETFD, 1)) fd++; }

    gifdir_init();		/* allocate initial memory for gifdir[] */

    srand(time(NULL));	/* in case we use shuffleslideshow */

    pixelsize = 1;
    getconfig();				/* see rcfile.c...   */
    argsleft = parsecommandline(argc, argv);	/* ...for these two. */
    fixconfig();		/* deal with any bogus config settings */

    copyfromconfig();

    //my_mouse_init();

    cleartext();

    /* do one-file-only if have one arg. */
    /* (does a chdir and carries on if it's a dir.) */
    if (argsleft == 1)
        load_one_file(argv[argc - 1]);

    /* do slideshow if more than one arg */
    /* XXX this should be shoved into a separate routine sometime */
    if (argsleft > 1)
    {
        int i, entnum;
        struct stat buf;
        for (i = argc - argsleft, entnum = 0; i <= argc - 1; i++)
            if (stat_(argv[i], &buf) != -1 && !S_ISDIR(buf.st_mode))
            {
                entnum++;
                if (!gifdir_resize_if_needed(entnum))
                    fprintf(stderr, "zgv: out of memory\n"), exit(1);
                write_gifdir_name(entnum, argv[i]);
                gifdir[entnum].marked = 1;
                gifdir[entnum].isdir = 0;
            }
        gifdirsiz = entnum;
        one_file_only = 1;
        openstdin_nonblocking();
        screenon();
        load_tagged_files();
        screenoff();
        exit(0);
    }

    /* normal zgv startup */
    cfg.errignore = 0;	/* don't ignore errs if using this way */
    openstdin_nonblocking();
    screenon();
    mainloop();
    screenoff();

    if (cfg.echotagged)
    {
        int f;

        for (f = 1; f <= gifdirsiz; f++)
            if (gifdir[f].marked)
                printf("%s\n", gifdir[f].name);
    }

    exit(0);
}


/* write str to gifdir[entry].name checking buffer size */
void write_gifdir_name(int entry, char* str)
{
    strncpy(gifdir[entry].name, str, sizeof(gifdir[entry].name) - 1);
    gifdir[entry].name[sizeof(gifdir[entry].name) - 1] = 0;
}



void openstdin_nonblocking()
{
#ifdef BACKEND_SVGALIB
    zgv_ttyfd = fileno(stdin);
    fcntl(zgv_ttyfd, F_SETFL, O_NONBLOCK);
#endif
}


/* write PPM of image to stdout (for `-w') */
void writeppm()
{
    int c, x, y;

    printf("P6\n%d %d\n255\n", width, height);

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            if (pixelsize == 1)
            {
                /* lookup index in palette */
                c = theimage[y * width + x];
                putchar(image_palette[0 + c]);
                putchar(image_palette[256 + c]);
                putchar(image_palette[512 + c]);
            }
            else
            {
                /* just copy */
                putchar(theimage[(y * width + x) * 3 + 2]);
                putchar(theimage[(y * width + x) * 3 + 1]);
                putchar(theimage[(y * width + x) * 3]);
            }
}


void load_one_file(char* filename)
{
    static hffunc hf;
    struct stat sbuf;

    /* before we load it, see if it's a directory. If so, we run zgv with
     * much the same result as typing '( cd whatever;zgv )'.
     */

    if (stat_(filename, &sbuf) != -1 && S_ISDIR(sbuf.st_mode))
    {
        chdir(filename);
        return;
    }

    if (cfg.writefile) cfg.onefile_progress = 0;

    openstdin_nonblocking();	/* we hadn't done this yet */
    one_file_only = 1;
    if (cfg.onefile_progress)
    {
        screenon();
        inithowfar(HOWFAR_LOADING_MSG);
        hf = showhowfar;
    }
    else
    {
        hf = NULL;
        if (!cfg.writefile) fprintf(stderr, "Loading...");
    }

    if (cfg.onefile_progress && hf != NULL) vga_runinbackground(1);

    if (cfg.writefile) pixelsize = 3;	/* 24-bit output preferred */

    /* save context for possible abort (if using showhowfar) */
    if (setjmp(setjmpbuf))
    {
        /* if we get here, someone aborted loading a file. */
        wait_for_foreground();
        screenoff();
        exit(0);
    }

    if (readpicture(filename, hf, !cfg.writefile, 0, NULL, NULL) != _PIC_OK)
    {
        if (cfg.onefile_progress) screenoff();
        fprintf(stderr, "\rzgv: error loading file.\n");
        exit(1);
    }
    else
    {
        if (hf == NULL && !cfg.writefile)
            fprintf(stderr, "\r          \r");
    }

    /* restore_mouse_pos is unimportant for a one-file situation, so we skip it. */

    if (cfg.writefile)
    {
        writeppm();
        /* pointless really, but might as well... */
        free(theimage);
        free(image_palette);
    }
    else
    {
        wait_for_foreground();
        screenoff();
    }
    exit(0);
}


void copyfromconfig()
{
    curvgamode = cfg.videomode;
    zoom = cfg.zoom;
    vkludge = cfg.vkludge;
    brightness = cfg.brightness;
    contrast = cfg.contrast;
    picgamma = cfg.initial_picgamma;
    virtual = (curvgamode == G320x400x256 || curvgamode == G360x480x256);

    fs_vgamode = cfg.fs_startmode;
    
#ifndef BACKEND_SVGALIB

    zgv_io_fixfsmode(&fs_vgamode);

#else	/* BACKEND_SVGALIB */

    /* if we don't have or don't allow the mode, use 640x480x8. */
    if (!vga_hasmode(fs_vgamode) || !cfg.mode_allowed[fs_vgamode])
        fs_vgamode = G640x480x256;

    /* now make it 640x480x4 (which also locks it in that mode)
     * if force16fs, or if 640x480x8 and we don't have it (or don't allow it).
     */
    if (cfg.force16fs || (fs_vgamode == G640x480x256 &&
        (!vga_hasmode(fs_vgamode) || !cfg.mode_allowed[fs_vgamode])))
        fs_vgamode = G640x480x16;

#endif	/* BACKEND_SVGALIB */
}


/* make `rw-r--r--'-style permission string from mode. */
char* make_perms_string(int mode)
{
    static char buf[10];
    int f, shift, submode;
    char* execptr;

    strcpy(buf, "---------");

    for (f = 0, shift = 6; f < 3; f++, shift -= 3)
    {
        /* first do basic `rwx' bit. */
        submode = ((mode >> shift) & 7);
        if (submode & 4) buf[f * 3 + 0] = 'r';
        if (submode & 2) buf[f * 3 + 1] = 'w';
        if (submode & 1) buf[f * 3 + 2] = 'x';

        execptr = buf + f * 3 + 2;

        /* apply any setuid/setgid/sticky bits */
        switch (f)
        {
        case 0: if (mode & 04000) *execptr = ((*execptr == 'x') ? 's' : 'S'); break;
        case 1: if (mode & 02000) *execptr = ((*execptr == 'x') ? 's' : 'S'); break;
        case 2: if (mode & 01000) *execptr = ((*execptr == 'x') ? 't' : 'T'); break;
        }
    }

    return(buf);
}


/* this does the stat() itself, as it may be called from vgadisp.c.
 * width/height are zero if we should read the thumbnail to get them.
 * if need_redraw_ptr is non-NULL, it gets set to 1 when a screen redraw
 * is needed on return (only possible when called from the viewer).
 */
void file_details(char* filename, int w, int h, int* need_redraw_ptr)
{
    static char buf[2048];
    struct tm* ctime;
    struct stat sbuf;
    //struct passwd* pwptr = NULL;
    //struct group* grptr = NULL;
    int gotdim = 1, dim_from_pic = 1;

    if (need_redraw_ptr) *need_redraw_ptr = 0;

    if (stat_(filename, &sbuf) == -1)
        return;

    /* this should be a can't happen, I think... */
    if ((ctime = localtime(&sbuf.st_mtime)) == NULL)
        return;

    /* try reading dimensions from thumbnail if needed */
    if (!w || !h)
    {
        FILE* tn;
        char* ptr;
        dim_from_pic = 0;

        if ((ptr = strrchr(filename, '/')) == NULL)
            snprintf(buf, sizeof(buf), ".xvpics/%s", filename);
        else
        {
            buf[sizeof(buf) - 1] = 0;
            strncpy(buf, filename, sizeof(buf) - 1);
            snprintf(buf + strlen(buf), sizeof(buf) - 1 - strlen(buf),
                ".xvpics/%s", ptr + 1);
        }

        gotdim = 0;
        if ((tn = fopen(buf, "rb")) != NULL)
        {
            fgets(buf, sizeof(buf), tn);	/* lose first line */
            fgets(buf, sizeof(buf), tn);	/* this may be "#IMGINFO:123x456 <type>" */
            while (buf[0] == '#')
            {
                if (sscanf(buf, "#IMGINFO:%dx%d", &w, &h) == 2)
                {
                    gotdim = 1;
                    break;
                }
                /* otherwise try another comment line */
                fgets(buf, sizeof(buf), tn);
            }
            fclose(tn);
        }
    }

    if (!gotdim)
        w = h = 0;

    //pwptr = getpwuid(sbuf.st_uid);
    //grptr = getgrgid(sbuf.st_gid);

    /* given the 7 field values, msgbox() does most of the work. */
    snprintf(buf, sizeof(buf),
        "%s\n"
        "%dk\n"
        "%d-%02d-%02d  %02d:%02d\n"
        "%s  (%o)\n"
        "%s\n"
        "%s\n"
        "%c%d x %d\n",
        filename,
        (int)(sbuf.st_size + 1023) / 1024,
        1900 + ctime->tm_year, ctime->tm_mon + 1, ctime->tm_mday,
        ctime->tm_hour, ctime->tm_min,
        make_perms_string(sbuf.st_mode & 07777), sbuf.st_mode & 07777,
        //pwptr ? pwptr->pw_name : "unknown",
        //grptr ? grptr->gr_name : "unknown",
        "unknown",
        "unknown",
        dim_from_pic ? 'p' : 't',	/* bit of a kludge :-/ */
        w, h);

    msgbox(zgv_ttyfd, buf, MSGBOXTYPE_FILEDETAILS, idx_light, idx_dark, idx_black);

    if (need_redraw_ptr) *need_redraw_ptr = !msgbox_did_restore();
}


void file_count(void)
{
    char buf[128];
    int f, tagged = 0, files = 0;

    for (f = 1; f <= gifdirsiz; f++)
    {
        if (!gifdir[f].isdir) files++;
        if (gifdir[f].marked) tagged++;
    }

    if (!files)
    {
        msgbox(zgv_ttyfd, "No files", MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return;
    }

    if (tagged)
        snprintf(buf, sizeof(buf),
            "%d file%s (%d tagged)",
            files, files > 1 ? "s" : "", tagged);
    else
        snprintf(buf, sizeof(buf),
            "%d file%s (none tagged)",
            files, files > 1 ? "s" : "");

    msgbox(zgv_ttyfd, buf, MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
}


int tagged_count(void)
{
    int f, tagged = 0;

    for (f = 1; f <= gifdirsiz; f++)
        if (gifdir[f].marked) tagged++;

    return(tagged);
}


void mainloop()
{
    int quit, key, curent; /* quit, key pressed, current entry */
    int oldent, startfrom, oldstart, f, markchange;
    int rb_menu_mode = 0;
    int pending_mouse_viewsel = 0;
    int tmpkey;

    /* blank out past-positions array */
    for (f = 0; f < MPPOSSIZ; f++)
        pastpos[f].dev = -1, pastpos[f].inode = -1;

    quit = 0; curent = 1; oldent = 1;
    startfrom = 1;

    readgifdir(1);
    showgifdir(startfrom, 0, 1, 0);
    showbar(curent, 1, startfrom);

    while (!quit)
    {
        oldent = curent;
        markchange = 0;
        key = mousecur_wait_for_keys_or_mouse(zgv_ttyfd);

        /* convert xzgv-ish keypresses */
        if (cfg.xzgvkeys)
        {
            switch (key)
            {
            case ' ':		key = RK_ENTER; break;
            case '-':		key = 'n'; break;
            case '=':		key = 't'; break;
            case '-' + 128:	key = 'N'; break;
            case '=' + 128:	key = 'T'; break;
            case 'N' - 0x40:	key = 'R'; break;
            case 'D' - 0x40:	key = RK_DELETE; break;
            case 'Q' - 0x40:	key = RK_ESC; break;
            case 'q':		key = RK_ESC; break;
            }
        }

        if (rb_menu_mode)
        {
            /* we deal with keys ourselves... */
            if (key == RK_ESC || key == 'x')
            {
                rb_menu_mode = 0;
                undraw_rb_menu();
                continue;
            }

            /* but mouse dealt with by this. if not clicked an entry, skip
             * rest of loop.
             */
            if (rb_menu_event(&key))	/* returns non-zero if should stay in menu */
                continue;

            /* otherwise restore saved area of screen, and do action in (faked) key */
            rb_menu_mode = 0;
            undraw_rb_menu();
            goto parsekey;	/* skip normal mouse stuff just in case */
        }

        /* grok mouse movements and buttons */
        if (has_mouse)
        {
            int mx = mouse_getx(), my = mouse_gety();
            int mbuttons = mouse_getbutton();
            int mleft_start = is_start_click_left();
            int mleft = is_end_click_left(), mright = is_end_click_right();

            if (slider_drag)
            {
                /* the usual way of dragging a slider is (I think) that the (in this
                 * case) horiz. pos. of the slider and the mouse pointer should (where
                 * `physically' possible) stay constant relative to each other.
                 * But the way zgv does it is to treat the mouse pointer's position
                 * on the scrollbar as a place to move the file cursor to, despite
                 * the relative position. This is a little odd, but I quite like it. :-)
                 *
                 * Anyway, the calculations are similar to those used by scrollbar.c
                 * to draw the slider, so we let that take care of it...
                 */
                curent = scrollbar_conv_drag_to_curent(mx, gifdirsiz);
            }
            else	/* not dragging slider */
            {
                /* we're only interested in the mouse if:
                 * - have released right button
                 * - pending_mouse_viewsel is set and have released left
                 * - we're on the scrollbar and have pressed the left button
                 * - we're on the file area and have pressed left
                 *		(sets pending_mouse_viewsel)
                 * - we're on the current-directory bit and have released left
                 */

                 /* if right button released, get the right-button menu. */
                if (mright)
                {
                    rb_menu_mode = 1;
                    pending_mouse_viewsel = 0;	/* cancel any pending view */
                    draw_rb_menu();
                    /* if left mouse button is depressed now (poor thing :-))
                     * we should set flag to ignore first end-left-click.
                     */
                    rb_ignore_first_left_click = ((mbuttons & MOUSE_LEFTBUTTON) ? 1 : 0);
                    continue;	/* skip rest of main loop */
                }

                /* this comes early (before other mleft-testing stuff)
                 * so we can be sure that the end click doesn't do something
                 * else as well.
                 */
                if (mleft && pending_mouse_viewsel)
                {
                    pending_mouse_viewsel = 0;
                    key = RK_ENTER;	/* fake a key to view current */
                    mleft = 0;	/* make sure it doesn't match stuff below :-) */
                }

                if (cfg.scrollbar)
                {
                    /* check for scrollbar. This is actually a bit complicated
                     * button-wise due to dragging etc., so we check for mouse being
                     * in it first.
                     * my is tested first as that's more likely to fail quickly if not
                     * in scrollbar area.
                     */
                    if (my >= SCRLBAR_YPOS && my < SCRLBAR_YPOS + SCRLBAR_HEIGHT &&
                        mx >= SCRLBAR_XPOS && mx < SCRLBAR_XPOS + SCRLBAR_WIDTH)
                    {
                        int slid_xpos = scrollbar_slider_xpos();
                        int slid_width = scrollbar_slider_width();

                        /* so we're here then. where we are determines which button
                         * states we care about. here's the list:
                         * - for arrows, end of left click (XXX this isn't ideal!)
                         * - for scrollbar but not slider, end of left click (ditto)
                         * - for scrollbar slider, start of left click puts us
                         *   in slider_drag mode, end of left click leaves it.
                         *   (end of click isn't tested here though; it's tested above by
                         *   seeing if button is currently not pressed, not quite the
                         *   same thing)
                         */
                         /* now then, check buttons depending on horizontal position. */
                        if (mx < SCRLBAR_MAIN_XPOS || mx >= SCRLBAR_RIGHTARROW_XPOS)
                        {
                            /* an arrow */
                            if (mleft)
                            {
                                if (mx < SCRLBAR_MAIN_XPOS)
                                    curent = startfrom, curent -= YSIZ;
                                else
                                    curent = startfrom + XSIZ * YSIZ - 1, curent += YSIZ;
                            }
                            if (curent < 1) curent = 1;
                            if (curent > gifdirsiz) curent = gifdirsiz;
                        }
                        else
                            if (slid_xpos != -1)	/* sanity check */
                            {
                                if (mx < SCRLBAR_MAIN_XPOS + slid_xpos ||
                                    mx >= SCRLBAR_MAIN_XPOS + slid_xpos + slid_width)
                                {
                                    /* non-slider */
                                    if (mleft)
                                    {
                                        if (mx < SCRLBAR_MAIN_XPOS + slid_xpos)
                                            curent = startfrom, curent -= XSIZ * YSIZ;
                                        else
                                            curent = startfrom + XSIZ * YSIZ - 1, curent += XSIZ * YSIZ;
                                    }
                                    if (curent < 1) curent = 1;
                                    if (curent > gifdirsiz) curent = gifdirsiz;
                                }
                                else
                                {
                                    /* slider */
                                    /* anyone who tries to visit parallel universes using this
                                     * does so at their own risk :-) */
                                    if (mleft_start)
                                        slider_drag = 1;
                                }
                            }
                    }
                }	/* end of if(cfg.scrollbar) */

              /* check for on-file-area-and-started-button-press */
                if (mleft_start)
                    if (mx >= XSTARTPOS && mx < XSTARTPOS + XSIZ * BARWIDTH &&
                        my >= fwinypos(1) && my < fwinypos(1) + YSIZ * barheight)
                    {
                        /* ok, on the file area and pressed button.
                         * find the entry matching the mouse position.
                         */
                        int tmpcurent = startfrom + YSIZ * ((mx - XSTARTPOS) / BARWIDTH) +
                            (my - fwinypos(1)) / barheight;

                        /* however, only *do* anything if there's a file/dir there :-) */
                        if (tmpcurent <= gifdirsiz)
                        {
                            /* update cursor position */
                            curent = tmpcurent;
                            if (curent != oldent)
                            {
                                showbar(oldent, 0, startfrom);
                                showbar(curent, 1, startfrom);
                            }
                            oldent = curent;	/* stop it being redrawn again */
                            pending_mouse_viewsel = 1;	/* view it when we let go */
                        }
                    }

                /* check for on-current-dir-bit-and-released-left */
                if (mleft)
                {
                    if (mx >= DIR_OF_XPOS && mx < DIR_OF_XPOS + DIR_OF_XSIZ &&
                        my >= DIR_OF_YPOS && my < DIR_OF_YPOS + 20)
                        /* pretty easy, fake a `G'. */
                        key = 'G';
                }
            }

            /* if left button isn't pressed currently, can't be dragging slider.
             * tested here (at end of mouse bits) to make sure we don't miss any
             * of the dragging.
             */
            if (!(mbuttons & MOUSE_LEFTBUTTON))
                slider_drag = 0;
        }		/* end of if(has_mouse) */

      /* now the good old keyboard stuff */
    parsekey:
        if (key) slider_drag = pending_mouse_viewsel = 0;

        /* disable any picture orientation override.
         * *slight* overkill to do it each time round the loop :-),
         * but it can't hurt.
         */
        orient_override = 0;

        if (fs_vgamode != G640x480x16 &&
            ((key >= RK_F1 && key <= RK_F10) || (key >= RK_SHIFT_F1 && key <= RK_SHIFT_F10)))
        {
            struct modedesc_tag* md_ptr;
            int oldmode = fs_vgamode;

            fs_vgamode = 0;
            for (md_ptr = modedesc; md_ptr->mode; md_ptr++)
            {
                if (key == md_ptr->fs_key && md_ptr->bytespp == 1)
                {
                    fs_vgamode = md_ptr->mode;
                    break;
                }
            }

            if (!fs_vgamode ||
                !vga_hasmode(fs_vgamode) || !cfg.mode_allowed[fs_vgamode])
                fs_vgamode = oldmode;
            else
                if (fs_vgamode != oldmode)
                {
                    /* we have to redraw a little unusually to
                     * make sure startfrom is sane before drawing.
                     * save/restore_mouse_pos effectively rescale the mouse pos.
                     */
                    save_mouse_pos();
                    screenon();
                    restore_mouse_pos();
                    fix_startfrom(curent, &startfrom);
                    oldstart = startfrom;	/* since we're redrawing here */
                    showgifdir(startfrom, 0, 1, 0);
                    showbar(curent, 1, startfrom);
                }
        }
        else /* if not selecting a mode... */
            switch (key)
            {
            case 128 + 'n':	/* sort by name */
            case 128 + 'e':	/* sort by `extension' */
            case 128 + 's':	/* sort by size */
            case 128 + 't':
            case 128 + 'd':	/* sort by mtime (time/date) */
                switch (key)
                {
                case 128 + 'n':	filesel_sorttype = sort_name; break;
                case 128 + 'e':	filesel_sorttype = sort_ext; break;
                case 128 + 's':	filesel_sorttype = sort_size; break;
                case 128 + 't':
                case 128 + 'd':	filesel_sorttype = sort_mtime; break;
                }
                sort_files();
                redrawall(curent, startfrom);
                break;
            case 'u': case 128 + 'u':
                /* ignore it in an xvpics dir */
            {
                char cdir[MAXPATHLEN + 1];

                *cdir = 0;
                getcwd(cdir, MAXPATHLEN);
                if (strstr(cdir, "/.xvpics") == NULL && cfg.xvpic_index)
                {
                    updating_index = 1;
                    if (key == 'u')
                        update_xvpics(0);
                    else
                    {
                        /* recursive update can take ages; make sure they know.
                         * (This is possibly annoying, but I still think it's a good idea.)
                         */
                        if (!msgbox(zgv_ttyfd,
                            "Recursive updates can take quite some time; are you sure?",
                            MSGBOXTYPE_YESNO, idx_light, idx_dark, idx_black))
                        {
                            updating_index = 0;
                            break;	/* skip redraw too */
                        }

                        recursive_update();

                        /* restore old state */
                        readgifdir(1);
                        if (curent > gifdirsiz || startfrom > gifdirsiz) curent = startfrom = 1;
                        oldent = curent;
                        fix_startfrom(curent, &startfrom);
                        oldstart = startfrom;	/* since we're redrawing below */
                    }
                    updating_index = 0;
                    redrawall(curent, startfrom);
                }
            }
            break;
            case 'd':
                if (cfg.xvpic_index)
                {
                    updating_index = 1;
                    update_xvpics(1);
                    updating_index = 0;
                    redrawall(curent, startfrom);
                }
                break;
            case 'v':
                cfg.xvpic_index = !cfg.xvpic_index;
                redrawall(curent, startfrom);
                break;
            case 'c':
                if (fs_vgamode != G640x480x16) break;
                cfg.fs16col = !cfg.fs16col;
                redrawall(curent, startfrom);
                break;
            case 's':
                cfg.scrollbar = !cfg.scrollbar;
                redrawall(curent, startfrom);
                break;
            case '?':
                save_mouse_pos();		/* showhelp restores this save */
                showhelp(zgv_ttyfd, "- KEYS FOR FILE SELECTOR -", zgvhelp);
                redrawall(curent, startfrom);
                break;
            case '\'': case 'g':
                do
                    tmpkey = mousecur_wait_for_keys_or_mouse(zgv_ttyfd);
                while (tmpkey == RK_NO_KEY);
                if (tmpkey >= 33 && tmpkey <= 126)
                {
                    int nofiles = 1, found = 0;

                    /* go to first file (not dir) which starts with that char.
                     * if there isn't one, go to first which starts with a later
                     * char.
                     * also, if there aren't any files (just dirs) don't move;
                     * otherwise, if there are no files with 1st char >=tmpkey,
                     * go to last file.
                     *
                     * nicest way to do this would be a binary search, but that
                     * would complicate matters; a linear search may be crude
                     * but it's still blindingly fast. And at least a linear one
                     * gives *predictably* useless results when not using
                     * sort-by-name. :-)
                     */
                    for (f = 1; f <= gifdirsiz; f++)
                        if (!gifdir[f].isdir)
                        {
                            nofiles = 0;
                            if (gifdir[f].name[0] >= tmpkey)
                            {
                                curent = f;
                                found = 1;
                                break;
                            }
                        }
                    /* if didn't find one >=tmpkey but there *are* files in the
                     * dir, go to the last one.
                     */
                    if (!found && !nofiles)
                        curent = gifdirsiz;
                }
                break;
            case RK_HOME: case 'A' - 0x40: /* home or ^a */
                curent = 1;
                break;
            case RK_END:  case 'E' - 0x40: /* end  or ^e */
                curent = gifdirsiz;
                break;
            case RK_PAGE_UP: case 'U' - 0x40: /* pgup or ^u */
                curent -= YSIZ * (XSIZ - 1);
                if (curent < 1) curent = 1;
                break;
            case RK_PAGE_DOWN: case 'V' - 0x40: /* pgdn or ^v */
                curent += YSIZ * (XSIZ - 1);
                if (curent > gifdirsiz) curent = gifdirsiz;
                break;
            case 'K':	/* kill mouse */
                if (!has_mouse)
                    msgbox(zgv_ttyfd, "No mouse enabled!",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                else
                    if (msgbox(zgv_ttyfd, "Really disable mouse for rest of session?",
                        MSGBOXTYPE_YESNO, idx_light, idx_dark, idx_black))
                    {
                        mouse_close();
                        has_mouse = 0;
                    }
                break;
            case 128 + 'f':	/* Alt-f, file (and tagged file) count */
                file_count();
                break;
            case 'D':		/* shift-D deletes tagged files (current if none) */
            case RK_DELETE:	/* `Del' deletes a single file */
                if (key == 'D')
                {
                    /* delete tagged files. We always prompt for this (unless
                     * overridden by hand), even if no files are tagged, to
                     * make it clear just what shift-D is doing.
                     */
                    if (cfg.nodelprompt_tagged ||
                        msgbox(zgv_ttyfd, "Really delete ALL tagged files?",
                            MSGBOXTYPE_YESNO, idx_light, idx_dark, idx_black))
                    {
                        copymovedel_file_or_tagged_files(curent, 1, 1, delete_file_simple,
                            "delete", "deleting", "deleted",
                            HOWFAR_DELETING_MSG);
                        goto do_redraw;
                    }
                }
                else
                {
                    if (gifdir[curent].isdir) break;

                    /* delete file at cursor */
                    if (delete_file(gifdir[curent].name, !cfg.nodelprompt, 1))
                    {
                        delete_gifdir_element(curent);
                        goto do_redraw;
                    }
                }
                break;
            case 128 + 'r':	/* Alt-r, rename file */
            case 'R':		/* now also on R :-) */
                if (rename_file(curent, startfrom))
                {
                    sort_files();	/* order may have changed */
                    goto do_redraw;
                }
                break;
            case 'C':	/* copy tagged files, or current file if none */
                copymovedel_file_or_tagged_files(curent, 0, 0, copyfile,
                    "copy", "copying", "copied",
                    HOWFAR_COPYING_MSG);
                /* the redraw is needed as we may have used a `percentage' bar. */
                /* (XXX may no longer be needed?) */
                redrawall(curent, startfrom);
                break;
            case 'M':	/* move tagged files, or current file if none */
                copymovedel_file_or_tagged_files(curent, 1, 0, movefile,
                    "move", "moving", "moved",
                    HOWFAR_MOVING_MSG);
                /* need redraw, and will probably have fewer files so fix things */
                goto do_redraw;
                break;
            case 18: case 12:   /* ^R and ^L */
                readgifdir(1);
            do_redraw:
                if (curent > gifdirsiz) curent = gifdirsiz;
                oldent = curent;
                fix_startfrom(curent, &startfrom);
                oldstart = startfrom;	/* since we're redrawing here */
                redrawall(curent, startfrom);
                break;
            case 'G':	/* goto new directory (by entering name) */
              /* we don't do any pastpos stuff here, as it's a direct
               * jump and it doesn't seem to make sense to me to
               * remember old positions etc. in such a case.
               */
                if (goto_named_dir())
                {
                    oldent = curent = startfrom = 1;
                    readgifdir(1);
                    redrawall(curent, startfrom);
                }
                break;
            case 't':	/* tag current file and move on */
                if (gifdir[curent].isdir == 0)
                {
                    gifdir[curent].marked = 1;
                    markchange = 1;
                }
                if (curent < gifdirsiz) curent++;
                break;
            case 'n':	/* untag current file and move on */
                if (gifdir[curent].isdir == 0)
                {
                    gifdir[curent].marked = 0;
                    markchange = 1;
                }
                if (curent < gifdirsiz) curent++;
                break;
            case ' ':   /* toggle tag/untag flag of current file and move on */
                if (gifdir[curent].isdir == 0)
                {
                    gifdir[curent].marked = 1 - gifdir[curent].marked;
                    markchange = 1;
                }
                if (curent < gifdirsiz) curent++;
                break;
            case 128 + 'm':	/* toggle find-by-magic-number */
                cfg.fsmagic = !cfg.fsmagic;
                if (cfg.fsmagic)
                    msgbox(zgv_ttyfd, "Find files by magic number enabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                else
                    msgbox(zgv_ttyfd, "Find files by magic number disabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                /* and obviously we want a reread/redraw after... */
                readgifdir(1);
                goto do_redraw;
                break;
            case 'L':	/* toggle slideshow looping */
                cfg.loop = !cfg.loop;
                if (cfg.loop)
                    msgbox(zgv_ttyfd, "Loop in slideshow now enabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                else
                    msgbox(zgv_ttyfd, "Looping in slideshow now disabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                break;
            case 'S':	/* toggle slideshow shuffle */
                cfg.shuffleslideshow = !cfg.shuffleslideshow;
                if (cfg.shuffleslideshow)
                    msgbox(zgv_ttyfd, "Slideshow shuffling now enabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                else
                    msgbox(zgv_ttyfd, "Slideshow shuffling now disabled",
                        MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                break;
            case 'T':	/* tag all files */
                for (f = 1; f <= gifdirsiz; f++) if (gifdir[f].isdir == 0) gifdir[f].marked = 1;
                showgifdir(startfrom, 0, 0, 0);
                break;
            case 'N':	/* untag all files */
                for (f = 1; f <= gifdirsiz; f++) if (gifdir[f].isdir == 0) gifdir[f].marked = 0;
                showgifdir(startfrom, 0, 0, 0);
                break;
            case 'q': case 'k': case 'P' - 0x40: case RK_CURSOR_UP:
                if (curent > 1) curent--; break;
            case 'a': case 'j': case 'N' - 0x40: case RK_CURSOR_DOWN:
                if (curent < gifdirsiz) curent++; break;
            case 'o': case 'h': case 'B' - 0x40: case RK_CURSOR_LEFT:
                curent -= YSIZ;
                if (curent < 1) curent = 1;
                break;
            case 'p': case 'l': case 'F' - 0x40: case RK_CURSOR_RIGHT:
                curent += YSIZ;
                if (curent > gifdirsiz) curent = gifdirsiz;
                break;
            case RK_ENTER:
                /* uhhhhh my head hurts */

                /* we do our own mouse pos save, in case showgif() isn't run */
                save_mouse_pos();

                sgres = 0;

                do
                {
                    if (permissiondenied(gifdir[curent].name))
                    {
                        msgbox(zgv_ttyfd, "Permission denied or file not found",
                            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                        pic_incr = 0;	/* stop here *even if ^p or ^n used* */
                        if (!msgbox_draw_ok)
                            redrawall(curent, startfrom);
                        break;
                    }

                    /* this is only in the loop so that it's after the permissiondenied
                     * stuff. It's unsightly having it here, but is otherwise ok.
                     */
                    if (gifdir[curent].isdir)
                    {
                        new_pastpos(curent, startfrom);
                        showbar(curent, 0, startfrom);
                        showgifdir(startfrom, 1, 1, 0);
                        chdir(gifdir[curent].name);
                        readgifdir(1);
                        get_pastpos(&curent, &startfrom);
                        /* if these are off the end, it's clearly out of date (dir
                         * must have changed) so might as well just start at top.
                         */
                        if (curent > gifdirsiz || startfrom > gifdirsiz) curent = startfrom = 1;
                        oldent = curent;
                        showgifdir(startfrom, 0, 1, 0);
                        showbar(curent, 1, startfrom);
                        break;
                    }

                    tagview_mode = 0;
                    if (sgres >= 2)
                    {
                        if (load_single_file(curent, 0) == 0) pic_incr = 0;
                    }
                    else
                        if (load_single_file(curent, 1) == 0) pic_incr = 0;

                    if (sgres == 3) gifdir[curent].marked = 1;

                    if (pic_incr != PIC_INCR_RELOAD_KLUDGE)
                        curent += pic_incr;
                    if (curent > gifdirsiz)
                        curent--, pic_incr = 0;
                    if (curent < 1 || gifdir[curent].isdir)
                        curent++, pic_incr = 0;

                    oldstart = startfrom;
                    fix_startfrom(curent, &startfrom);

                    if (sgres < 2 || pic_incr == 0)
                    {
                        oldent = -1;
                        restore_mouse_pos_with_size(fs_scrnwide, fs_scrnhigh);
                        redrawall(curent, startfrom);
                    }
                    else
                    {
                        /* if we're *not* redrawing the selector, restore given
                         * current (viewer) screen's size.
                         */
                        restore_mouse_pos();
                    }
                } while (pic_incr);

                /* make sure we fix mouse pos for selector screen */
                restore_mouse_pos_with_size(fs_scrnwide, fs_scrnhigh);
                break;

            case '\t':		/* tab */
                if (load_tagged_files())	/* rets non-zero if needs redraw */
                    redrawall(curent, startfrom);
                break;

            case ':':		/* give full filename, for you nethack fans :-) */
            case ';':		/* (also allow semicolon, might as well) */
                file_details(gifdir[curent].name, 0, 0, NULL);
                break;

            case 'W':
                if (cfg.stupid_gnu_verbosity)
                {
                    gnu_warranty_help(zgv_ttyfd);
                    redrawall(curent, startfrom);
                }
                break;

            case 'x': case RK_ESC:
                quit = 1;
            } /* end of switch(key) */

        oldstart = startfrom;
        fix_startfrom(curent, &startfrom);
        if (startfrom != oldstart)
        {
            showbar(oldent, 0, oldstart);
            showgifdir(oldstart, 1, 0, 0);
            showgifdir(startfrom, 0, 0, 0);
            showbar(curent, 1, startfrom);
        }
        else
            if (oldent != -1 && (curent != oldent || markchange))
            {
                showbar(oldent, 0, startfrom);
                showbar(curent, 1, startfrom);
            }
    }
}


void fix_startfrom(int curent, int* startfromp)
{
    int startfrom = *startfromp;

    while (curent < startfrom)
        startfrom -= YSIZ;
    while (fwinxpos(curent - startfrom + 1) + BARWIDTH > XENDPOS)
        startfrom += YSIZ;
    if (startfrom < 1) startfrom = 1;

    *startfromp = startfrom;
}


/* returns non-zero if we need to update the screen */
int rename_file(int curent, int startfrom)
{
    static char prompt[256];
    char* dest;
    struct stat sbuf;
    char* tn_src, * tn_dst;

    snprintf(prompt, sizeof(prompt), "Rename %s to what?", gifdir[curent].name);
    dest = cm_getline(zgv_ttyfd, prompt, idx_light, idx_dark, idx_black, idx_medium);

    if (dest == NULL || *dest == 0) return(0);

    /* refuse anything with path elements in */
    if (strchr(dest, '/'))
    {
        msgbox(zgv_ttyfd, "File must remain in current directory",
            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return(0);
    }

    /* refuse the renaming if it would blast an existing file */
    if (stat_(dest, &sbuf) != -1)
    {
        msgbox(zgv_ttyfd, "File already exists",
            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return(0);
    }

    if (rename(gifdir[curent].name, dest) == -1)
    {
        msgbox(zgv_ttyfd, "Couldn't rename file",
            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return(0);
    }

    /* if it worked, rename the entry in gifdir[] and try renaming any
     * thumbnail. (XXX doesn't yet try to rename any ~/.xvpics/_foo_bar/baz one.)
     */

    tn_src = tn_dst = NULL;

    /* ".xvpics/" is 8 chars */
    if ((tn_src = malloc(8 + strlen(gifdir[curent].name) + 1)) == NULL ||
        (tn_dst = malloc(8 + strlen(dest) + 1)) == NULL)
    {
        write_gifdir_name(curent, dest);	/* do the name copy */
        if (tn_src) free(tn_src);
        return(1);	/* not a big deal failing to rename thumbnail */
    }

    strcpy(tn_src, ".xvpics/");
    strcat(tn_src, gifdir[curent].name);
    strcpy(tn_dst, ".xvpics/");
    strcat(tn_dst, dest);

    rename(tn_src, tn_dst);		/* don't much care if it works or not */

    free(tn_dst);
    free(tn_src);

    write_gifdir_name(curent, dest);		/* finally, do the name copy */

    return(1);
}


/* save old contents of area to put right-button menu on, and draw it. */
static void draw_rb_menu()
{
    int fs256 = (fs_vgamode != G640x480x16);
    int f;

    rbm_xysize(filesel_menu_data, &rb_save_width, &rb_save_height);
    if ((rb_save = malloc(rb_save_width * rb_save_height)) == NULL) return;

    if (fs256)
    {
        /* use vgagl */
        gl_setcontextvga(fs_vgamode);
        gl_getbox(fs_scrnwide - rb_save_width, 0,
            rb_save_width, rb_save_height, rb_save);
    }
    else
    {
        /* 16-colour, use vga_getscansegment */
        unsigned char* ptr = rb_save;
        int y;

        for (y = 0; y < rb_save_height; y++, ptr += rb_save_width)
            vga_getscansegment(ptr, 640 - rb_save_width, y, rb_save_width);
    }

    /* grey out (make non-active) any ones we shouldn't allow.
     * this is nasty, but decl of filesel_menu_data notes that there's
     * a nasty thing here, so it should be ok.
     */
    for (f = 2; f < 6; f++) filesel_menu_data[f].active = 0;

    if (fs_vgamode != G640x480x16)
    {
        f = 2;
        filesel_menu_data[f++].active =
            (vga_hasmode(G640x480x256) && cfg.mode_allowed[G640x480x256]);
        filesel_menu_data[f++].active =
            (vga_hasmode(G800x600x256) && cfg.mode_allowed[G800x600x256]);
        filesel_menu_data[f++].active =
            (vga_hasmode(G1024x768x256) && cfg.mode_allowed[G1024x768x256]);
        filesel_menu_data[f++].active =
            (vga_hasmode(G1280x1024x256) && cfg.mode_allowed[G1280x1024x256]);
    }

    /* only other one which matters is grey/colour, which only means
     * anything when using 16-col file selector with thumbnails on.
     */
    rbm_set_active_flag(filesel_menu_data, "grey/colour",
        (!fs256 && cfg.xvpic_index));

    /* now draw the thing */
    rbm_draw(filesel_menu_data, idx_light, idx_medium, idx_dark, idx_black);
}


/* restore old contents of area with right-button menu on. */
static void undraw_rb_menu()
{
    if (rb_save == NULL) return;	/* ran out of memory, can't do much! */

    if (fs_vgamode != G640x480x16)
    {
        /* use vgagl again */
        gl_putbox(fs_scrnwide - rb_save_width, 0,
            rb_save_width, rb_save_height, rb_save);
    }
    else
    {
        /* 16-colour */
        unsigned char* ptr = rb_save;
        int y;

        for (y = 0; y < rb_save_height; y++, ptr += rb_save_width)
            vga_drawscansegment(ptr, 640 - rb_save_width, y, rb_save_width);
    }

    free(rb_save);
}


/* possibly have a mouse event to deal with for right-button menu.
 * uses pointer to key to fake keys to do stuff, and returns
 * 1 if we should stay in rb menu mode, else 0.
 */
static int rb_menu_event(int* keyp)
{
    /* important to read both, even if not using both */
    int mleft = is_end_click_left(), mright = is_end_click_right();
    int key;

    if (!mleft)
        return(1);

    if (rb_ignore_first_left_click)
    {
        rb_ignore_first_left_click = 0;
        return(1);
    }

    key = mright;	/* a kludge to keep gcc -Wall quiet */

    /* get faked key for filesel_menu_data, or zero if none */
    *keyp = 0;
    key = rbm_mousepos_to_key(filesel_menu_data, mouse_getx(), mouse_gety());
    if (key != -1)	/* -1 means quit menu with no key */
        *keyp = key;

    return((key == 0));	/* 1 if didn't match any, else 0 */
}



/* add new pastpos[0], shifting down all the rest of the entries. */
void new_pastpos(int curent, int startfrom)
{
    struct stat sbuf;
    int f;

    if (cfg.forgetoldpos) return;

    for (f = MPPOSSIZ - 1; f > 0; f--)
    {
        pastpos[f].dev = pastpos[f - 1].dev;
        pastpos[f].inode = pastpos[f - 1].inode;
        pastpos[f].curent = pastpos[f - 1].curent;
        pastpos[f].startfrom = pastpos[f - 1].startfrom;
    }

    if (stat_(".", &sbuf) == -1) return;

    pastpos[0].dev = sbuf.st_dev;
    pastpos[0].inode = sbuf.st_ino;
    pastpos[0].curent = curent;
    pastpos[0].startfrom = startfrom;
}


/* return curent from pastpos[] entry matching current directory,
 * or if none match return 1.
 */
void get_pastpos(int* curentp, int* startfromp)
{
    struct stat sbuf;
    int f;

    *curentp = *startfromp = 1;

    if (cfg.forgetoldpos || stat_(".", &sbuf) == -1) return;

    for (f = 0; f < MPPOSSIZ; f++)
        if (pastpos[f].inode == sbuf.st_ino && pastpos[f].dev == sbuf.st_dev)
        {
            *curentp = pastpos[f].curent;
            *startfromp = pastpos[f].startfrom;
            return;
        }
}


/* loads/views one file. caller must do the restore_mouse_pos. */
int load_single_file(int curent, int do_howfar)
{
    int tmp;

    if (do_howfar)
        inithowfar(HOWFAR_LOADING_MSG);

    vga_runinbackground(1);

    /* save context for possible abort */
    if (setjmp(setjmpbuf))
    {
        /* if we get here, someone aborted loading a file. */
        if (!tagview_mode || (tagview_mode && do_howfar))
        {
            wait_for_foreground();
            msgbox(zgv_ttyfd, "File view aborted", MSGBOXTYPE_OK,
                idx_light, idx_dark, idx_black);
        }
        return(0);
    }
    else
    {
        if ((tmp = readpicture(gifdir[curent].name,
            do_howfar ? showhowfar : smallhowfar, 1, 0, NULL, NULL)) != _PIC_OK)
        {
            wait_for_foreground();
            if (tagview_mode) screenon();
            showerrmessage(tmp);
            return(0);
        }
    }

    return(1);
}


/* returns non-zero if we need to redraw the selector */
int load_tagged_files(void)
{
    /* are f and dohf *really* endangered by the possible longjmp back to
     * load_single_file()? Still, playing it safe...
     */
    static int f, dohf;
    int t, ent;

    for (f = 1, t = 0; f <= gifdirsiz; f++)
        if (gifdir[f].marked) t++;

    if (t == 0)
    {
        msgbox(zgv_ttyfd, "No files tagged", MSGBOXTYPE_OK,
            idx_light, idx_dark, idx_black);
        return(0);
    }

    /* vgadisp.c sets tagview_mode==0 if esc is pressed */
    tagview_mode = 1;
    dohf = 1;

    /* we do our own mouse pos save, in case showgif() isn't run */
    save_mouse_pos();

    do
    {
        if (!make_slideshow_idx_array())
        {
            /* this will only happen if memory is VERY tight, as it's quite
             * a small array...
             */
            msgbox(zgv_ttyfd, "Out of memory",
                MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
            return(!msgbox_draw_ok);
        }

        if (cfg.shuffleslideshow)
            shuffle_slideshow_idx();

        for (f = 0; f < slideshow_idx_size && tagview_mode; f++)
        {
            ent = slideshow_idx[f];
            if (permissiondenied(gifdir[ent].name))
            {
                msgbox(zgv_ttyfd, "Permission denied or file not found",
                    MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
                free(slideshow_idx);
                return(!msgbox_draw_ok);
            }

            if (load_single_file(ent, dohf) == 0) break;
            dohf = 0;

            /* restore mouse pos ready for it to be saved again */
            restore_mouse_pos();
        }

        free(slideshow_idx);
    } while (cfg.loop && sgres != 1);

    /* make sure we fix mouse pos for selector screen */
    restore_mouse_pos_with_size(fs_scrnwide, fs_scrnhigh);

    return(1);	/* need redraw */
}


void delete_gifdir_element(int n)
{
    if (n<1 || n>gifdirsiz) return;

    /* if not last, need to move further elements up */
    if (n < gifdirsiz)
        memmove(gifdir + n, gifdir + n + 1,
            sizeof(struct gifdir_tag) * (gifdirsiz - n));

    gifdirsiz--;
}


void copymovedel_file_or_tagged_files(int curent, int remove_from_array,
    int no_dir_prompt,
    int (*func_ptr)(char*, char*),
    char* msg_action, char* msg_acting,
    char* msg_acted, char* msg_howfar)
{
    static char buf[256];	/* must be static to survive longjmp */
    static int fakemark, fakegone;	/* same here */
    static char* msg_action_sav;	/* and here, I think */
    /* gcc -Wall wants the next two non-auto, too, but in reality they don't
     * actually have to be.
     */
    static int t;
    static char* dest;
    int f, done;
    struct stat sbuf;

    msg_action_sav = msg_action;
    fakemark = fakegone = 0;
    dest = NULL;

    t = tagged_count();

    if (t == 0 && gifdir[curent].isdir)
    {
        msgbox(zgv_ttyfd, "No files tagged and cursor not on a file", MSGBOXTYPE_OK,
            idx_light, idx_dark, idx_black);
        return;
    }

    if (!no_dir_prompt)
    {
        snprintf(buf, sizeof(buf), "Directory to %s to (or Esc to abort)", msg_action);
        dest = cm_getline(zgv_ttyfd, buf, idx_light, idx_dark, idx_black, idx_medium);

        if (dest == NULL || *dest == 0) return;

        if (stat_(dest, &sbuf) == -1)
        {
            /* it would be nicer to say `dir doesn't exist' or something,
             * but this is the only reasonably safe bet :-)
             */
            msgbox(zgv_ttyfd, "Error - directory cannot be accessed",
                MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
            return;
        }

        if (!S_ISDIR(sbuf.st_mode))
        {
            msgbox(zgv_ttyfd, "Error - destination is not a directory",
                MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
            return;
        }

        /* the dir might yet not be writable, but we let the copy/movefile routine
         * find that out.
         */
    }

    if (t == 0)
    {
        t = 1;
        gifdir[curent].marked = 1;
        fakemark = 1;
    }

    if (t > 1) inithowfar(msg_howfar);

    /* save context for possible abort */
    if (setjmp(setjmpbuf))
    {
        /* if we get here, someone aborted copying/moving/deleting a file. */
        if (fakemark && !fakegone) gifdir[curent].marked = 0;
        snprintf(buf, sizeof(buf), "File %s aborted", msg_action_sav);
        msgbox(zgv_ttyfd, buf, MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return;
    }

    for (done = 0, f = 1; f <= gifdirsiz; f++)
    {
        if (gifdir[f].marked == 0) continue;

        if (!(*func_ptr)(gifdir[f].name, dest))
        {
            snprintf(buf, sizeof(buf), "Error %s %s", msg_acting, gifdir[f].name);
            msgbox(zgv_ttyfd, buf, MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);

            if (fakemark && !fakegone) gifdir[curent].marked = 0;
            return;
        }

        if (remove_from_array)
        {
            delete_gifdir_element(f);
            if (fakemark) fakegone = 1;
            f--;
        }

        done++;

        /* we do this such a bonehead way to defeat the %10 business in
         * showhowfar - optimal it ain't. :-/
         */
        if (t > 1) showhowfar(done * 10, t * 10);
    }

    if (fakemark && !fakegone) gifdir[curent].marked = 0;

    sprintf(buf, "File%s %s OK", (t > 1) ? "s" : "", msg_acted);
    msgbox(zgv_ttyfd, buf, MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
}


int goto_named_dir()
{
    char* dest;

    dest = cm_getline(zgv_ttyfd, "Go to which directory?",
        idx_light, idx_dark, idx_black, idx_medium);

    if (dest == NULL || *dest == 0) return(0);

    if (chdir(dest) < 0)
    {
        msgbox(zgv_ttyfd, "Error changing directory",
            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        return(0);
    }

    return(1);
}


/* this could also be file not found of course */
int permissiondenied(char* fname)
{
    FILE* junk;
    if ((junk = fopen(fname, "rb")) == NULL)
        return(1);
    fclose(junk);
    return(0);
}


void redrawall(int curent, int startfrom)
{
    screenon();

    showgifdir(startfrom, 0, 1, 0);
    showbar(curent, 1, startfrom);
}


static int howfar_wbar, howfar_hmid;

void inithowfar(char* msg)
{
    static char* last = NULL;
    int scrn_hmid;
    int f;

    if (!msg)
        msg = last;
    else
        last = msg;

    scrn_hmid = fs_scrnhigh / 2;
    vga_setcolor(idx_medium);
    for (f = scrn_hmid - 16; f <= scrn_hmid + 16; f++)
        vga_drawline(104, f, fs_scrnwide - 104, f);
    drawbutton(104, scrn_hmid - 16, fs_scrnwide - 104, scrn_hmid + 16,
        NULL, 0, idx_light, idx_dark, idx_blacknz, 0);
    howfar_upto = 0;
    if (msg)
    {
        vga_setcolor(idx_black);
        vgadrawtext((fs_scrnwide - vgatextsize(2, msg)) / 2, scrn_hmid - 5, 2, msg);
    }

    howfar_wbar = fs_scrnwide - 220;
    howfar_hmid = scrn_hmid;
}


/* call with sofar==-1 to re-init progress indicator; however,
 * if you do, the char * originally passed to inithowfar() *must*
 * have pointed to a statically allocated string.
 */
void showhowfar(int sofar, int total)
{
    int f, d;

    if (sofar == -1)
    {
        inithowfar(NULL);
        return;
    }

    if (sofar > total) sofar = total;

    /* test for abort and keep mouse pos up-to-date */
    smallhowfar(sofar, total);

    if (((sofar % 10) == 0) || (sofar == total))
    {
        d = (howfar_wbar * sofar) / total;
        if (d > howfar_upto)
        {
            vga_lockvc();
            if (!vga_oktowrite())
            {
                vga_unlockvc();
                return;
            }
            vga_setcolor(idx_light); /* we set this always in case of a VC switch */
            for (f = howfar_upto; f <= d; f++)
                vga_drawline(110 + f, howfar_hmid - 10, 110 + f, howfar_hmid + 10);
            vga_unlockvc();
            howfar_upto = f;
        }
    }
}


/* minimal 'how far' function that just tests for an abort and stops
 * mouse causing problems.
 */
void smallhowfar(int sofar, int total)
{
    if (sofar == -1)
        return;

    if (has_mouse) mouse_update(), click_update();

    /* we jump back to an abort message if Esc was pressed */
    if (cfg.repeat_timer)
        zgv_io_screen_update();	/* given that we're not calling readnbkey() */
    else
    {
        if (readnbkey(zgv_ttyfd) == RK_ESC)
        {
            /* these routines blast the malloc'ed stuff, which has *for sure*
             * been allocated by now, because we must already be reading the file
             * in for us to get here!
             */
            aborted_file_cleanup();
            longjmp(setjmpbuf, 1);
        }
    }
}


void showbar(int entnum, int selected, int startfrom)
{
    static char ctmp[256 + 2];	/* prettyfile() buffer */
    int xpos, ypos;
    int xt;

    xpos = fwinxpos(entnum - startfrom + 1);
    if ((xpos < 1) || (xpos + BARWIDTH > XENDPOS)) return;
    ypos = fwinypos(entnum - startfrom + 1);
    prettyfile(ctmp, &(gifdir[entnum]), sizeof(ctmp));

    set_max_text_width(BAR_RESTRICT_WIDTH);
    xt = cfg.xvpic_index ? centreseltxt(xpos, gdfsiz, ctmp) : xpos + 3;
    if (cfg.blockcursor)
    {
        /* block-style cursor - hopefully easier to read/see. */
        if (selected)
            draw3dbox(xpos - 2, ypos - 2, xpos + BARWIDTH + 1, ypos + barheight + 1, 4, 1,
                idx_dark, idx_dark);
        else
            undraw3dbox(xpos - 2, ypos - 2, xpos + BARWIDTH + 1, ypos + barheight + 1, 4);

        /* in case the last file was just marked/unmarked */
        vga_setcolor(gifdir[entnum].marked ? idx_marked : idx_black);
        vgadrawtext(xt, ypos + 3 + gdfofs, gdfsiz, ctmp);
    }
    else
    {
        if (selected)
        {
            draw3dbox(xpos, ypos, xpos + BARWIDTH - 1, ypos + barheight - 1, 1, 1,
                idx_light, idx_dark);
            drawtext3d(xt, ypos + 3 + gdfofs, gdfsiz, ctmp, 0, idx_light, idx_dark,
                gifdir[entnum].marked ? idx_marked : idx_black);
            /* box if cfg.xvpic_index and is an xvpic being used */
            if (cfg.xvpic_index && gifdir[entnum].xvw != 0)
                draw3dbox(xpos + (BARWIDTH - gifdir[entnum].xvw) / 2 - 2,
                    ypos + GDFYBIT + 39 - gifdir[entnum].xvh / 2 - 2,
                    xpos + (BARWIDTH - gifdir[entnum].xvw) / 2 + gifdir[entnum].xvw + 1,
                    ypos + GDFYBIT + 39 - gifdir[entnum].xvh / 2 + gifdir[entnum].xvh + 1,
                    1, 0, idx_light, idx_dark);
        }
        else
        {
            undraw3dbox(xpos, ypos, xpos + BARWIDTH - 1, ypos + barheight - 1, 1);
            undrawtext3d(xt, ypos + 3 + gdfofs, gdfsiz, ctmp);
            vga_setcolor(gifdir[entnum].marked ? idx_marked : idx_black);
            vgadrawtext(xt, ypos + 3 + gdfofs, gdfsiz, ctmp);
            /* undraw box if cfg.xvpic_index and is an xvpic being used */
            if (cfg.xvpic_index && gifdir[entnum].xvw != 0)
                undraw3dbox(xpos + (BARWIDTH - gifdir[entnum].xvw) / 2 - 2,
                    ypos + GDFYBIT + 39 - gifdir[entnum].xvh / 2 - 2,
                    xpos + (BARWIDTH - gifdir[entnum].xvw) / 2 + gifdir[entnum].xvw + 1,
                    ypos + GDFYBIT + 39 - gifdir[entnum].xvh / 2 + gifdir[entnum].xvh + 1,
                    1);
        }
    }

    set_max_text_width(NO_CLIP_FONT);
}


int centreseltxt(int x, int fsiz, char* str)
{
    int a;

    a = vgatextsize(fsiz, str);
    return(x + (BARWIDTH - a) / 2);
}


/* a de-hassled getcwd(). You need to free the memory after use, though.
 * (I could have used GNU's one, but I already had this from xzgv.)
 */
char* getcwd_allocated(void)
{
    int incr = 1024;
    int size = incr;
    char* buf = malloc(size);

    while (buf != NULL && getcwd(buf, size - 1) == NULL)
    {
        free(buf);
        size += incr;
        buf = malloc(size);
    }

    return(buf);
}


void showgifdir(int startfrom, int unshow, int drawdirmsg, int do_one_only)
{
    char cdir[MAXPATHLEN + 1], * ptr;
    static char ctmp[1024], ctmp2[1024];
    int f, ypos, xpos, w, h, y, xt;
    unsigned char* image;
    int xvpics_dir_exists = 0;
    int start, end;

    /* draw the bulk of the scrollbar. unshow code for this is
     * at bottom of routine so scrollbar isn't `empty' long when moving.
     */
    if (!unshow && cfg.scrollbar && !do_one_only)
        draw_scrollbar_main(startfrom, gifdirsiz, XSIZ * YSIZ);

    *cdir = 0;
    getcwd(cdir, MAXPATHLEN);

    if (drawdirmsg)
    {
        if (updating_index)
        {
            char* cptr = getcwd_allocated();

            snprintf(cdir, sizeof(cdir), "updating index%s%s",
                cptr ? " of " : "", cptr ? cptr : "");
            if (cptr)
                free(cptr);
        }

        vga_setcolor(unshow ? idx_medium : idx_black);
        set_max_text_width(DIR_OF_XSIZ);
        vgadrawtext(DIR_OF_XPOS, DIR_OF_YPOS, 3, cdir);
    }

    /* see if either xvpics dir exists. if not, we can speed things
     * up a little by not bothering to look for thumbnails at all (the
     * difference is usually small, but on a dos partition it's sometimes
     * *really* painful otherwise).
     *
     * There's a little bit of duplicated code though, so it's a bit
     * nasty and should be cleaned up sometime (XXX).
     */
    if (strstr(cdir, "/.xvpics") != NULL)
        xvpics_dir_exists = 1;
    else
    {
        struct stat sbuf;

        /* must be either a dir or symlink (symlink could be to a dir...) */
        if (stat_(".xvpics", &sbuf) != -1 && (S_ISDIR(sbuf.st_mode) ||
            S_ISLNK(sbuf.st_mode)))
            xvpics_dir_exists = 1;
        else
        {
            snprintf(ctmp2, sizeof(ctmp2),
                "%s/.xvpics/", getenv("HOME") ? getenv("HOME") : "");
            ptr = ctmp2 + strlen(ctmp2);
            getcwd(ptr, sizeof(ctmp2) - strlen(ctmp2) - 1);
            /* convert /foo/bar/baz to _foo_bar_baz */
            while ((ptr = strchr(ptr, '/')) != NULL) *ptr++ = '_';
            if (stat_(ctmp2, &sbuf) != -1 && (S_ISDIR(sbuf.st_mode) ||
                S_ISLNK(sbuf.st_mode)))
                xvpics_dir_exists = 1;
        }
    }

    if (do_one_only)
        start = end = do_one_only;
    else
        start = startfrom, end = gifdirsiz;

    for (f = start; f <= end; f++)
    {
        set_max_text_width(BAR_RESTRICT_WIDTH);
        xpos = fwinxpos(f - startfrom + 1);
        if (xpos + BARWIDTH > XENDPOS) break;
        ypos = fwinypos(f - startfrom + 1);
        prettyfile(ctmp, &(gifdir[f]), sizeof(ctmp));
        xt = cfg.xvpic_index ? centreseltxt(xpos, gdfsiz, ctmp) : xpos + 3;
        vga_setcolor(unshow ? idx_medium : (gifdir[f].marked ? idx_marked : idx_black));
        vgadrawtext(xt, ypos + 3 + gdfofs, gdfsiz, ctmp);
        if (cfg.linetext && cfg.thicktext && cfg.blockcursor)
            vgadrawtext(xt + 1, ypos + 3 + gdfofs, gdfsiz, ctmp);
        vga_setcolor(unshow ? idx_medium : idx_black);

        if (cfg.xvpic_index)
        {
            /* load and draw thumbnail file (or undraw it) */
            if (unshow)
            {
                image = malloc(96);
                if (image != NULL)
                {
                    memset(image, idx_medium, 96);
                    for (y = -2; y < 62; y++)
                        vga_drawscansegment(image,
                            xpos + (BARWIDTH - 80) / 2 - 2, ypos + y + GDFYBIT + 9, 96);
                    free(image);
                }
            }
            else
            {
                if (xvpics_dir_exists)
                {
                    /* if '/.xvpics' is in the (absolute) path somewhere, we're
                     * in a xvpics subdir. So look for the thumbnail here! :-)
                     */
                    if (strstr(cdir, "/.xvpics") != NULL)
                    {
                        strncpy(ctmp, gifdir[f].name, sizeof(ctmp) - 1);
                        ctmp[sizeof(ctmp) - 1] = 0;
                    }
                    else
                        snprintf(ctmp, sizeof(ctmp), ".xvpics/%s", gifdir[f].name);
                    /* -1 here buys space for the "/" below */
                    snprintf(ctmp2, sizeof(ctmp2) - 1,
                        "%s/.xvpics/", getenv("HOME") ? getenv("HOME") : "");
                    ptr = ctmp2 + strlen(ctmp2);
                    getcwd(ptr, sizeof(ctmp2) - strlen(ctmp2) - 1);
                    /* convert /foo/bar/baz to _foo_bar_baz */
                    while ((ptr = strchr(ptr, '/')) != NULL) *ptr++ = '_';
                    strcat(ctmp2, "/");
                    strncat(ctmp2, gifdir[f].name, sizeof(ctmp2) - strlen(ctmp2) - 1);
                }

                gifdir[f].xvw = gifdir[f].xvh = 0;

                if (xvpics_dir_exists && (read_xv332(ctmp, &image, &w, &h) == _PIC_OK ||
                    read_xv332(ctmp2, &image, &w, &h) == _PIC_OK))
                {
                    int xwant = xpos + (BARWIDTH - w) / 2, w8 = 0;

                    gifdir[f].xvw = w; gifdir[f].xvh = h;
                    if (fs_vgamode == G640x480x16)
                    {
                        w8 = ((w + 7) & ~7) + 8;
                        greyfix332(&image, w, h, xwant & 7, w8);
                        xwant &= ~7;
                    }

                    if (image != NULL)
                    {
                        for (y = 0; y < h; y++)
                            vga_drawscansegment(image + y * (fs_vgamode == G640x480x16 ? w8 : w), xwant,
                                ypos + y + GDFYBIT + 39 - h / 2, fs_vgamode == G640x480x16 ? w8 : w);
                        free(image);
                    }
                }
                else
                    if (gifdir[f].isdir)
                    {
                        /* 'folder' icon, as usual for these types of things */
                        vga_setcolor(unshow ? idx_medium : idx_black);
                        xt = xpos + (BARWIDTH - 80) / 2;
                        ypos += GDFYBIT + 9;

                        /* main bit */
                        vga_drawline(xt + 10, ypos + 50, xt + 70, ypos + 50);
                        vga_drawline(xt + 70, ypos + 50, xt + 70, ypos + 20);
                        vga_drawline(xt + 70, ypos + 20, xt + 65, ypos + 15);
                        vga_drawline(xt + 65, ypos + 15, xt + 15, ypos + 15);
                        vga_drawline(xt + 15, ypos + 15, xt + 10, ypos + 20);
                        vga_drawline(xt + 10, ypos + 20, xt + 10, ypos + 50);

                        /* top bit */
                        vga_drawline(xt + 15, ypos + 15, xt + 20, ypos + 10);
                        vga_drawline(xt + 20, ypos + 10, xt + 35, ypos + 10);
                        vga_drawline(xt + 35, ypos + 10, xt + 40, ypos + 15);
                        ypos -= GDFYBIT + 9;

                        gifdir[f].xvw = w = 80; gifdir[f].xvh = h = 60;
                    }
                    else
                    {
                        /* a default icon-type-thing here */
                        vga_setcolor(unshow ? idx_medium : idx_black);
                        xt = xpos + (BARWIDTH - 80) / 2;
                        ypos += GDFYBIT + 9;

                        /* main bit */
                        vga_drawline(xt + 20, ypos + 50, xt + 60, ypos + 50);
                        vga_drawline(xt + 60, ypos + 50, xt + 60, ypos + 20);
                        vga_drawline(xt + 60, ypos + 20, xt + 50, ypos + 10);
                        vga_drawline(xt + 50, ypos + 10, xt + 20, ypos + 10);
                        vga_drawline(xt + 20, ypos + 10, xt + 20, ypos + 50);

                        /* 'folded' bit */
                        vga_drawline(xt + 50, ypos + 10, xt + 50, ypos + 20);
                        vga_drawline(xt + 50, ypos + 20, xt + 60, ypos + 20);

                        ypos -= GDFYBIT + 9;
                        gifdir[f].xvw = w = 80; gifdir[f].xvh = h = 60;
                    }

                if (gifdir[f].xvw != 0)
                {
                    draw3dbox(xpos + (BARWIDTH - w) / 2 - 1, ypos + GDFYBIT + 38 - h / 2,
                        xpos + (BARWIDTH - w) / 2 + w, ypos + GDFYBIT + 39 - h / 2 + h, 1, 0,
                        idx_light, idx_dark);
                }
            }
        }	/* end of thumbnail stuff */
    }

    set_max_text_width(NO_CLIP_FONT);

    /* see top of routine for why this is here: */
    if (unshow && cfg.scrollbar && !do_one_only)
        undraw_scrollbar_slider();
}


void readgifdir(int graphics_ok)
{
    DIR* dirfile;
    struct dirent* anentry;
    struct stat buf;
    char cdir[MAXPATHLEN + 1], * ptr;
    int entnum, isdir;

    *cdir = 0;
    getcwd(cdir, MAXPATHLEN);

    gifdirsiz = 0;

    dirfile = opendir(".");
    if (dirfile == NULL)
    {
        /* if drawing stuff onscreen isn't allowed, just leave it */
        if (!graphics_ok)
            return;

        /* we get here if we can't read the dir.
         * Zgv tests we have permission to access a file or dir before
         * selecting it, so this can only happen if it was started on the dir
         * from the cmdline, or if the directory has changed somehow since we
         * last read it.
         * the first reaction is to try $HOME instead.
         * if *that* doesn't work, we cough and die, not unreasonably. :-)
         */
         /* be sure to mention what we're doing first... :-) */
        msgbox(zgv_ttyfd, "Directory unreadable - going to $HOME...",
            MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
        if (getenv("HOME") == NULL)
            goto badhome;
        chdir(getenv("HOME"));
        if ((dirfile = opendir(".")) == NULL)
        {
        badhome:
            screenoff();
            fprintf(stderr, "zgv: $HOME is unreadable. This is a Bad Thing. TTFN...\n");
            exit(1);
        }

        /* this screenon() could result in us clearing the screen twice overall
         * (if we did a ^R on a now-deleted dir, for example), but this isn't
         * the kind of thing which happens all the time, so I guess this is ok.
         */
        screenon();
    }

    entnum = 0;
    while ((anentry = readdir(dirfile)) != NULL)
    {
        if (!cfg.showxvpicdir || (cfg.showxvpicdir &&
            strcmp(anentry->d_name, ".xvpics") != 0))
            if (anentry->d_name[0] == '.' && anentry->d_name[1] != '.')
                continue;	/* skip (most) 'dot' files */
            /* no `.' ever, and no `..' if at root. */
        if (strcmp(anentry->d_name, ".") != 0 &&
            !(strcmp(cdir, "/") == 0 && strcmp(anentry->d_name, "..") == 0))
        {
            if ((stat_(anentry->d_name, &buf)) == -1)
            {
                buf.st_mode = 0;
                buf.st_size = 0;
                buf.st_mtime = 0;
            }
            isdir = S_ISDIR(buf.st_mode);
            /* directories, GIF/JPG/PBM/PGM/PPM tested here */
            if (isdir || ispicture(anentry->d_name))
            {
                entnum++;
                if (!gifdir_resize_if_needed(entnum))
                {
                    wait_for_foreground();
                    screenoff();
                    fprintf(stderr, "zgv: out of memory\n");
                    exit(1);
                }

                write_gifdir_name(entnum, anentry->d_name);

                /* can't use a pointer to the extension, as gifdir[] is
                 * resized using realloc() and the name may be moved around,
                 * so it has to be an offset.
                 */
                if ((ptr = strrchr(anentry->d_name, '.')) == NULL)
                    /* use the NUL, Luke */
                    gifdir[entnum].extofs = strlen(anentry->d_name);
                else
                    gifdir[entnum].extofs = ptr - anentry->d_name;

                gifdir[entnum].isdir = isdir;
                gifdir[entnum].size = buf.st_size;
                gifdir[entnum].mtime = buf.st_mtime;
                gifdir[entnum].marked = 0;
            }
        }
    }
    closedir(dirfile);
    gifdirsiz = entnum;
    sort_files();
    slider_drag = 0;	/* stop any current drag op, as gifdirsiz may have changed */
}


void sort_files(void)
{
    qsort(&(gifdir[1]), gifdirsiz, sizeof(struct gifdir_tag), (void*)gcompare);
}


int ispicture(char* filename)
{
    int l = strlen(filename);

    if (cfg.fsmagic)
        return(magic_ident(filename) != _IS_BAD);

    if (l <= 4) return(0);

    if ((!strcasecmp(filename + l - 4, ".gif")) ||
        (!strcasecmp(filename + l - 4, ".jpg")) ||
        (!strcasecmp(filename + l - 5, ".jpeg")) ||
        (!strcasecmp(filename + l - 4, ".png")) ||
        (!strcasecmp(filename + l - 4, ".mrf")) ||
        (!strcasecmp(filename + l - 4, ".prf")) ||
        (!strcasecmp(filename + l - 4, ".xbm")) ||
        (!strcasecmp(filename + l - 5, ".icon")) ||	/* presumably an XBM */
        (!strcasecmp(filename + l - 4, ".xpm")) ||
        (!strcasecmp(filename + l - 4, ".pbm")) ||
        (!strcasecmp(filename + l - 4, ".pgm")) ||
        (!strcasecmp(filename + l - 4, ".ppm")) ||
        (!strcasecmp(filename + l - 4, ".bmp")) ||
        (!strcasecmp(filename + l - 4, ".tga")) ||
        (!strcasecmp(filename + l - 4, ".pcx")) ||
#ifdef PCD_SUPPORT
        (!strcasecmp(filename + l - 4, ".pcd")) ||
#endif
        (!strcasecmp(filename + l - 4, ".tif")) ||
        (!strcasecmp(filename + l - 5, ".tiff")))
        return(1);
    else
        return(0);
}


int gcompare(void* gn1, void* gn2)
{
    struct gifdir_tag* g1 = (struct gifdir_tag*)gn1;
    struct gifdir_tag* g2 = (struct gifdir_tag*)gn2;

    /* directories always come first.
     * so, if comparing two files, use a normal comparison;
     * otherwise if it's two dirs, use a strcmp on the names;
     * otherwise it's one file and one dir, and the dir is always `less'.
     */
    if (g1->isdir && g2->isdir)
        return(strcmp(g1->name, g2->name));  /* both directories, use strcmp. */

    if (!g1->isdir && !g2->isdir)
    {
        /* both files, use normal comparison. */
        int ret = 0;

        switch (filesel_sorttype)
        {
        case sort_name:
            ret = strcmp(g1->name, g2->name);
            break;

        case sort_ext:
            ret = strcmp(g1->name + g1->extofs, g2->name + g2->extofs);
            break;

        case sort_size:
            if (g1->size < g2->size)
                ret = -1;
            else
                if (g1->size > g2->size)
                    ret = 1;
            break;

        case sort_mtime:
            if (g1->mtime < g2->mtime)
                ret = -1;
            else
                if (g1->mtime > g2->mtime)
                    ret = 1;
            break;
        }

        /* for all equal matches on primary key, use name as secondary */
        if (ret == 0)
            ret = strcmp(g1->name, g2->name);

        return(ret);
    }	/* end of if */

  /* otherwise, one or both are dirs. */

    if (g1->isdir) return(-1);	/* first one is dir */
    return(1);			/* else second one is dir */
}


void prettyfile(char* buf, struct gifdir_tag* gifdptr, int bufsize)
{
    if (gifdptr->isdir)
        snprintf(buf, bufsize, "(%s)", gifdptr->name);
    else
    {
        strncpy(buf, gifdptr->name, bufsize - 1);
        buf[bufsize - 1] = 0;
    }
}


void screenon(void)
{
    static int first_call = 1;
    int r, g, b, n;
    unsigned char* tmp;
    char* title;

    /* fs_vgamode is known to be valid at this point. */

    if (fs_vgamode != vga_getcurrentmode())
        vga_setmode(fs_vgamode);
    else
    {
        /* the palette-setting makes sure we don't have a possibly-crazy palette
         * for a short time while sorting things out.
         */
        vga_setpalette(0, 0, 0, 0);
        vga_clear();
    }

    fs_scrnwide = vga_getxdim();
    fs_scrnhigh = vga_getydim();

    msgbox_draw_ok = 1;
    gdfsiz = 3 - cfg.smallfstext;
    gdfofs = 4 * cfg.smallfstext;

    yofs = 50;

    if (cfg.xvpic_index)
    {
        if (fs_vgamode == G640x480x16)
        {
            /* if 16 colour mode... */
            if (cfg.fs16col)
                /* colour */
                for (r = 0; r < 2; r++)
                    for (g = 0; g < 2; g++)
                        for (b = 0; b < 2; b++)
                            vga_setpalette(r * 4 + g * 2 + b + 1, r * 63, g * 63, b * 63);
            else
                /* greyscale */
                for (b = 0; b <= GREY_MAXVAL; b++)
                    vga_setpalette(b, b * 63 / GREY_MAXVAL, b * 63 / GREY_MAXVAL, b * 63 / GREY_MAXVAL);

            idx_black = idx_blacknz = 14; idx_marked = 12;
            idx_dark = 13; idx_medium = 0; idx_light = 15;
        }
        else
        {
            /* if 256 colour mode... */
            /* construct 3:3:2 palette */
            for (r = n = 0; r < 8; r++)
                for (g = 0; g < 8; g++)
                    for (b = 0; b < 4; b++, n++)
                        vga_setpalette(n, r * 63 / 7, g * 63 / 7, b * 63 / 3);

            /* find approximations to file selector colours.
             * these are then blasted with the *real* file selector colours
             * unless cfg.perfectindex is set.
             */
            idx_medium = MAKE332COL(cfg.medium.r, cfg.medium.g, cfg.medium.b);
            idx_dark = MAKE332COL(cfg.dark.r, cfg.dark.g, cfg.dark.b);
            idx_light = MAKE332COL(cfg.light.r, cfg.light.g, cfg.light.b);
            idx_black = MAKE332COL(cfg.black.r, cfg.black.g, cfg.black.b);
            idx_marked = MAKE332COL(cfg.marked.r, cfg.marked.g, cfg.marked.b);
            idx_blacknz = 0; r = g = b = 0;
            while (idx_blacknz == 0)
                idx_blacknz = MAKE332COL(r++, g++, b++);
        }

        /* fix bar height */
        barheight = GDFYBIT + 6 + 70;
    }
    else
    {
        /* no-thumbnails selector */
        barheight = GDFYBIT + 6;
        idx_medium = MIDGREY; idx_dark = DARK; idx_light = LIGHT; idx_black = BLACK;
        idx_marked = MARKEDCOL;
        idx_blacknz = BLACK;
        if (fs_vgamode == G640x480x16) idx_medium = 0;
    }

    if (!(cfg.xvpic_index && cfg.perfectindex))
    {
        vga_setpalette(idx_medium, cfg.medium.r, cfg.medium.g, cfg.medium.b);
        vga_setpalette(idx_dark, cfg.dark.r, cfg.dark.g, cfg.dark.b);
        vga_setpalette(idx_light, cfg.light.r, cfg.light.g, cfg.light.b);
        vga_setpalette(idx_black, cfg.black.r, cfg.black.g, cfg.black.b);
        vga_setpalette(idx_marked, cfg.marked.r, cfg.marked.g, cfg.marked.b);
    }

    /* make sure mouse pointer is in the right colours,
     * and set mouse x/y range correctly for screen.
     */
    mousecur_init(idx_blacknz, idx_light);

    /* do GNU stupidity if gnulitically_correct is enabled.
     * This is done even if only viewing one file, for maximal annoyance.
     */
    if (first_call)
    {
        first_call = 0;
        if (cfg.stupid_gnu_verbosity)
        {
            gnu_init_help(zgv_ttyfd);
            screenon();
            return;
        }
    }

    if (one_file_only) return;

    if (idx_medium)
    {
        /* clear screen with `medium' (i.e. background) colour. */
        if ((tmp = malloc(fs_scrnwide)) != NULL)
        {
            memset(tmp, idx_medium, fs_scrnwide);
            for (n = 0; n < fs_scrnhigh; n++)
                vga_drawscanline(n, tmp);
            free(tmp);
        }
    }
#ifndef BACKEND_SVGALIB
    else
        vga_clear();	/* reflect zero-index palette change */
#endif

    draw3dbox(0, 0, fs_scrnwide - 1, 39, 1, 1, idx_light, idx_dark);
    draw3dbox(0, 40, fs_scrnwide - 1, fs_scrnhigh - 1, 1, 1, idx_light, idx_dark);

    /* draw complete scrollbar, but without any slider. */
    if (cfg.scrollbar)
        draw_scrollbar_empty();

    title = "zgv " ZGV_VER " - ";
    vga_setcolor(idx_black);
    vgadrawtext(DIR_OF_XPOS - vgatextsize(3, title), DIR_OF_YPOS, 3, title);
}


void screenoff(void)
{
    vga_setmode(TEXT);
    cleartext();
}


void cleartext()
{
    if (cfg.cleartext)
        fprintf(stderr, "\033[H\033[J");
}


/* this shows any error message from GIF reading.
 * Notice that JPEG errors have already been reported!
 */
void showerrmessage(int errnumber)
{
    char buf[256];

    if (updating_index) return;	/* ignore if updating thumbnail index */

    strcpy(buf, "Error reading file - ");
    switch (errnumber)
    {
    case _PICERR_NOFILE:
        strcat(buf, "file not found"); break;
    case _PICERR_NOMEM:
        strcat(buf, "out of memory"); break;
    case _PICERR_BADMAGIC:
        strcat(buf, "not a supported format or bad extension"); break;
    case _PICERR_NOCOLOURMAP:
        strcat(buf, "no global colour map"); break;
    case _PICERR_NOIMAGE:
        strcat(buf, "no image block found"); break;
    case _PICERR_UNSUPPORTED:
        strcat(buf, "unsupported image sub-type"); break;
    case _PICERR_CORRUPT:
        strcat(buf, "corrupt or short file"); break;
    case _PICERR_ISRLE:
        strcat(buf, "can't handle RLE BMP files"); break;
    case _PICERR_TOOMANYCOLS:
        strcat(buf, "too many colours"); break;
    case _PICERR_BADXCOL:
        strcat(buf, "bad X colour name"); break;
    case _PICERR_SEE_ERRMSG:
        /* this only happens with JPEGs/PNGs. */
        strncat(buf, jpeg_png_errmsg, sizeof(buf) - strlen(buf) - 1); break;
    default:
        strcat(buf, "unknown error (ulp!)");
    }
    msgbox(zgv_ttyfd, buf, MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
}


/* ok, a couple of people want move, copy, and (especially) delete,
 * and it sounds like a good idea, so here goes.
 *
 * delete is the easiest.
 * we also delete any matching thumbnail file in .xvpics, and
 * attempt to remove the .xvpics directory also - relying on the
 * OS to do the Right Thing if other thumbnail files remain
 * (which it does, of course).
 */
int delete_file(char* filename, int doprompt, int report_error)
{
    char buf[270];
    int retn = 1;

    if (doprompt)
    {
        snprintf(buf, sizeof(buf), "Really delete %s?", filename);
        retn = msgbox(zgv_ttyfd, buf, MSGBOXTYPE_YESNO, idx_light, idx_dark, idx_black);
    }

    if (retn == 1)
    {
        if (remove(filename) == -1)
        {
            if (report_error)
                msgbox(zgv_ttyfd, "Unable to delete file!", MSGBOXTYPE_OK,
                    idx_light, idx_dark, idx_black);
            return(0);
        }

        snprintf(buf, sizeof(buf), ".xvpics/%s", filename);
        remove(buf);		/* don't care if it fails */
        rmdir(".xvpics");	/* same here */
    }

    return(retn);
}


/* non-interactive version of delete_file() callable from copymovedel_*() */
int delete_file_simple(char* filename, char* junk)
{
    return(delete_file(filename, 0, 0));
}


void xv332_how_far(int sofar, int total)
{
    char tmp[128];
    int done;

    if (sofar == -1)
        return;

    if (sofar > total) sofar = total;

    done = sofar * 100 / total;

    vga_lockvc();
    if (vga_oktowrite() && ((done % 10) == 0 || sofar == total))
    {
        clear_xvpic(xv332_how_far_xpos, xv332_how_far_ypos);

        if (sofar != total)
        {
            vga_setcolor(idx_black);
            snprintf(tmp, sizeof(tmp), "Reading - %2d%%", done);
            vgadrawtext(xv332_how_far_xpos + (BARWIDTH - 70) / 2,
                xv332_how_far_ypos + GDFYBIT + 39 - 4, 2, tmp);
        }
    }
    vga_unlockvc();
}


void clear_xvpic(int xpos, int ypos)
{
    unsigned char tmp[96];
    int y;

    memset(tmp, idx_medium, 96);
    for (y = -3; y < 63; y++)
        vga_drawscansegment(tmp, xpos + (BARWIDTH - 80) / 2 - 3, ypos + y + GDFYBIT + 9, 96);
}


/* if howfar equals 0, no progress report is done.
 * if it is >0, then the high 16 bits are 'xpos' and low 16 are 'ypos'.
 */
int makexv332(char* filename, char* xvpicfn, unsigned int howfar)
{
    FILE* out;
    int tmp;
    int w, h, y;
    int realw, realh;
    unsigned char* smallpic;

    pixelsize = 1;		/* ouch */

    if (howfar)
    {
        /* given the way the progress reporting from readpicture() works,
         * I have to use some global variables here. :-(
         */
        xv332_how_far_xpos = howfar >> 16;
        xv332_how_far_ypos = howfar & 0xFFFF;
    }

    tmp = ((cfg.jpegindexstyle == 3) ? 0 : 1);
    if ((tmp = readpicture(filename, howfar ? xv332_how_far : NULL, 0,
        tmp, &realw, &realh)) != _PIC_OK)
        return(tmp);

    /* image is pointed to by theimage, image_palette */

    /* pretend an animated GIF is only as high as 1st image */
    if (gif_delaycount >= 2)
        height /= gif_delaycount, realh /= gif_delaycount;

    vga_lockvc();
    if (vga_oktowrite())
    {
        clear_xvpic(xv332_how_far_xpos, xv332_how_far_ypos);
        vga_setcolor(idx_black);
        vgadrawtext(xv332_how_far_xpos + (BARWIDTH - 62) / 2,
            xv332_how_far_ypos + GDFYBIT + 39 - 4, 2, "Resampling...");
    }
    vga_unlockvc();

    /* resize */
    w = 80; h = 60;
    smallpic = resizepic(theimage, image_palette + 512, image_palette + 256, image_palette,
        width, height, &w, &h);
    free(theimage); free(image_palette);	/* finished with these */

    vga_lockvc();
    if (vga_oktowrite())
    {
        clear_xvpic(xv332_how_far_xpos, xv332_how_far_ypos);
        vga_setcolor(idx_black);
        vgadrawtext(xv332_how_far_xpos + (BARWIDTH - 55) / 2,
            xv332_how_far_ypos + GDFYBIT + 39 - 4, 2, "Dithering...");
    }
    vga_unlockvc();

    /* dither */
    ditherinit(w);
    for (y = 0; y < h; y++)
        ditherline(smallpic + y * 80 * 3, y, w);
    ditherfinish();

    /* write */
    if ((out = fopen(xvpicfn, "wb")) == NULL)
        return(_PICERR_NOFILE);		/* well, kind of */

    fprintf(out, "P7 332\n");
    fprintf(out, "#IMGINFO:%dx%d RGB\n", realw, realh);
    fprintf(out, "#END_OF_COMMENTS\n");
    fprintf(out, "%d %d 255\n", w, h);

    for (y = 0; y < h; y++)
        fwrite(smallpic + y * 80 * 3, 1, w, out);

    fclose(out);
    free(smallpic);

    return(_PIC_OK);
}


/* hack of above routine to work on dirs.
 * `filename' is the name of the dir in this case, of course.
 */
int makedirxv332(char* filename, char* xvpicfn, unsigned int howfar)
{
    static char files[4][256];
    static struct { int x, y; } xypos[4] = { {0,0}, {1,0}, {0,1}, {1,1} };
    FILE* out;
    DIR* dirfile;
    int tmp;
    int w, h, y;
    unsigned char* smallpic, * totalpic;
    struct dirent* anentry;
    struct stat buf;
    int f, pos;
    char tmpstr[40];

    /* the dir. xvpics are a montage of the top 4 files of subdir. working out
     * which these are is even less fun than you'd think, as they aren't
     * necessarily in alphabetical order. rather than read them all in, qsort
     * that, and take the top 4, we instead keep a `top 4' table and do a kind
     * of incremental sort on that as we go along.
     */

    for (f = 0; f < 4; f++) files[f][0] = 0;

    dirfile = opendir(filename);
    if (dirfile == NULL) return(_PICERR_NOFILE);

    while ((anentry = readdir(dirfile)) != NULL)
    {
        if (anentry->d_name[0] == '.') continue;		/* skip `dot' files */
        if ((stat_(anentry->d_name, &buf)) == -1)
            buf.st_mode = 0;
        if (S_ISDIR(buf.st_mode)) continue;		/* ...and subdirs */
        if (!ispicture(anentry->d_name)) continue;	/* must be picture file */

        /* now check against existing files in table. start at the bottom
         * and see how high we get.
         */
        pos = -1;
        for (f = 3; f >= 0; f--)
            if (files[f][0] == 0 || strcmp(anentry->d_name, files[f]) < 0)
                pos = f;

        if (pos >= 0)
        {
            if (pos < 3)
                for (f = 3; f > pos; f--)
                    strcpy(files[f], files[f - 1]);

            strncpy(files[pos], anentry->d_name, sizeof(files[pos]) - 1);
            files[pos][sizeof(files[pos]) - 1] = 0;
        }
    }
    closedir(dirfile);

    if (files[0][0] + files[1][0] + files[2][0] + files[3][0] == 0)
        /* no files there, just leave it alone. */
        return(_PICERR_NOFILE);

    /* so we finally have the four filenames. */

    pixelsize = 1;		/* ouch */

    if (howfar)
    {
        /* given the way the progress reporting from readpicture() works,
         * I have to use some global variables here. :-(
         */
        xv332_how_far_xpos = howfar >> 16;
        xv332_how_far_ypos = howfar & 0xFFFF;
    }

    /* get total image */
    if ((totalpic = malloc(80 * 60)) == NULL) return(_PICERR_NOMEM);

    memset(totalpic, 0, 80 * 60);

    for (f = 0; f < 4; f++)
    {
        vga_lockvc();
        if (vga_oktowrite())
        {
            vga_setcolor(idx_black);
            sprintf(tmpstr, "Image %d/4...", f + 1);
            clear_xvpic(xv332_how_far_xpos, xv332_how_far_ypos);
            vgadrawtext(xv332_how_far_xpos + (BARWIDTH - 62) / 2,
                xv332_how_far_ypos + GDFYBIT + 39 - 4, 2, tmpstr);
        }
        vga_unlockvc();

        tmp = ((cfg.jpegindexstyle == 3) ? 0 : 1);
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s", filename, files[f]);
        if ((tmp = readpicture(tmpstr, NULL, 0, tmp, NULL, NULL)) != _PIC_OK)
        {
            if (f == 0)
            {
                free(totalpic);
                remove(xvpicfn);
                return(tmp);	/* if it's the first, abort... */
            }
            else
                continue;		/* else just skip it. */
        }

        /* image is pointed to by theimage, image_palette */

        /* pretend an animated GIF is only as high as 1st image */
        if (gif_delaycount >= 2) height /= gif_delaycount;

        /* resize */
        w = 40; h = 30;
        smallpic = resizepic(theimage, image_palette + 512, image_palette + 256,
            image_palette, width, height, &w, &h);
        free(theimage); free(image_palette);	/* finished with these */

        /* dither */
        ditherinit(w);
        for (y = 0; y < h; y++)
            ditherline(smallpic + y * 40 * 3, y, w);
        ditherfinish();

        for (y = 0; y < h; y++)
            memcpy(totalpic + 80 * 30 * xypos[f].y + 40 * xypos[f].x + y * 80, smallpic + y * 40 * 3, w);

        free(smallpic);
    }

    /* write */
    if ((out = fopen(xvpicfn, "wb")) == NULL)
        return(_PICERR_NOFILE);		/* well, kind of */

    fprintf(out, "P7 332\n");
    fprintf(out, "# dir thumbnail file produced by zgv %s\n", ZGV_VER);
    fprintf(out, "#END_OF_COMMENTS\n");
    fprintf(out, "80 60 255\n");

    fwrite(totalpic, 1, 80 * 60, out);

    fclose(out);

    free(totalpic);

    return(_PIC_OK);
}


/* common to update_xvpics() and recursive_update().
 * zero return means error - error strings for dialog are in errstring.
 * (not doing error dialogs here allows recursive update to ignore errors.)
 * zero return and errstring==NULL means they pressed Esc.
 * firstcall should be non-zero for first call of a recursive `run', zero
 *  otherwise.
 */
int update_xvpics_common(int do_dirs_instead, char** errstring,
    int firstcall, int is_recursive)
{
    static char buf[1024], buf2[1024], altdirbuf[1024];
    static int redraw_needed = 0;
    FILE* test;
    int f;
    int curent, oldent, lastchange_ent;
    int startfrom, oldstart, lastchange_start;
    struct stat realpic, xvpic, xvpic2, tmpsbuf;
    char* ptr;
    int r1 = 0, r2 = 0;
    int altdirexists = 0;

    *errstring = NULL;
    if (firstcall) redraw_needed = 0;

    /* for each picture in the current directory, we check to see if
     * a file exists in the .xvpics directory with the same filename.
     * If not, it is created. If a file does exist, we check the
     * modification times. If the picture file is newer than the index
     * file, a new one gets created. Hope that's clear now. :-)
     */

    if (is_recursive)
        redraw_needed = 1;	/* don't redraw until needed */
    else
    {
        vga_lockvc();
        if (vga_oktowrite())
        {
            if (redraw_needed)
                screenon();
            redrawall(1, 1);
            redraw_needed = 0;
        }
        vga_unlockvc();
    }

    curent = startfrom = lastchange_ent = lastchange_start = 1;


    /* see if the ~/.xvpics/foo_bar_baz dir exists. If not, we can
     * avoid having to test for it below, making for far fewer stat()s.
     * this also sets altdirbuf, so we don't have to work out the dir's
     * name all the time. :-)
     */

    snprintf(altdirbuf, sizeof(altdirbuf),
        "%s/.xvpics/", getenv("HOME") ? getenv("HOME") : "");
    ptr = altdirbuf + strlen(altdirbuf);
    getcwd(ptr, sizeof(altdirbuf) - strlen(altdirbuf) - 1);
    while ((ptr = strchr(ptr, '/')) != NULL) *ptr++ = '_';
    altdirexists = (stat_(altdirbuf, &tmpsbuf) != -1);


    for (f = 1; f <= gifdirsiz; f++)
    {
        /* with the do_dirs_instead mode, the first non-dir entry is a
         * sensible place to stop.
         */
        if (do_dirs_instead && gifdir[f].isdir == 0) break;

        if (gifdir[f].isdir == do_dirs_instead && stat_(gifdir[f].name, &realpic) != -1 &&
            gifdir[f].name[0] != '.')
        {
            /* test for normal .xvpics/wibble */
            snprintf(buf, sizeof(buf), ".xvpics/%s", gifdir[f].name);
            r1 = stat_(buf, &xvpic);

            /* and for ~/.xvpics/foo_bar_baz/wibble if needed */
            if (!altdirexists)
                r2 = -1;
            else
            {
                snprintf(buf2, sizeof(buf2), "%s/%s", altdirbuf, gifdir[f].name);
                r2 = stat_(buf2, &xvpic2);
            }

            if ((r1 == -1 && r2 == -1) ||
                (r1 != -1 && realpic.st_mtime > xvpic.st_mtime) ||
                (r2 != -1 && realpic.st_mtime > xvpic2.st_mtime) ||
                do_dirs_instead)
            {
                vga_lockvc();
                if (!vga_oktowrite())
                    redraw_needed = 1;
                else
                {
                    if (redraw_needed)
                    {
                        screenon();
                        showgifdir(startfrom, 0, 1, 0);
                        showbar(curent, 1, startfrom);
                        redraw_needed = 0;
                    }
                    else
                        if (!cfg.slowupdate)
                        {
                            showbar(lastchange_ent, 0, lastchange_start);
                            if (lastchange_start != startfrom)
                            {
                                showgifdir(lastchange_start, 1, 0, 0);
                                showgifdir(startfrom, 0, 0, 0);
                            }
                            showbar(curent, 1, startfrom);
                        }
                }
                vga_unlockvc();
                lastchange_ent = curent;
                lastchange_start = startfrom;

                /* this is pretty BFI and messy */

                if (mkdir(".xvpics") != -1 || errno == EEXIST || errno == EACCES)
                {
                    /* check if we can write to the file */
                    if ((test = fopen(buf, "wb")) != NULL)
                        fclose(test);
                    else
                        goto usehomedir;
                }
                else	/* if couldn't create/use .xvpics ... */
                {
                usehomedir:
                    /* couldn't create ./.xvpics, try ~/.xvpics */
                    snprintf(buf, sizeof(buf),
                        "%s/.xvpics", getenv("HOME") ? getenv("HOME") : "");
                    if (mkdir(buf) == -1 && errno != EEXIST && errno != EACCES)
                    {
                        *errstring = "Unable to create either .xvpics directory";
                        return(0);
                    }

                    /* also need to create ~/.xvpics/_foo_bar_baz */
                    if (mkdir(altdirbuf) == -1 && errno != EEXIST && errno != EACCES)
                    {
                        *errstring = "Unable to create ~/.xvpics/... directory";
                        return(0);
                    }

                    /* make sure filename for xvpic is in buf */
                    snprintf(buf, sizeof(buf), "%s/%s", altdirbuf, gifdir[f].name);
                }

                if (do_dirs_instead)
                    makedirxv332(gifdir[f].name, buf, (fwinxpos(f - startfrom + 1) << 16) |
                        fwinypos(f - startfrom + 1));
                else
                    makexv332(gifdir[f].name, buf, (fwinxpos(f - startfrom + 1) << 16) |
                        fwinypos(f - startfrom + 1));

                /* redraw without mode change */
                vga_lockvc();
                if (vga_oktowrite())
                {
                    if (redraw_needed)	/* if a full redraw is needed do it */
                    {
                        screenon();
                        showgifdir(startfrom, 0, 1, 0);
                        showbar(curent, 1, startfrom);
                        redraw_needed = 0;
                    }
                    else
                    {
                        showbar(curent, 0, startfrom);
                        showgifdir(startfrom, 1, 0, curent);
                        showgifdir(startfrom, 0, 0, curent);
                        showbar(curent, 1, startfrom);
                    }
                }
                else	/* if we're not on current vc */
                    redraw_needed = 1;
                vga_unlockvc();
            }
        }


        /* move down one if possible */
        oldent = curent; oldstart = startfrom;
        if (curent < gifdirsiz) curent++;

        /* move right if needed */
        while (fwinxpos(curent - startfrom + 1) + BARWIDTH > XENDPOS)
            startfrom += YSIZ;

        vga_lockvc();

        /* redraw */
        if (vga_oktowrite() && !redraw_needed && cfg.slowupdate)
        {
            if (startfrom != oldstart)
            {
                showbar(oldent, 0, oldstart);
                showgifdir(oldstart, 1, 1, 0);
                showgifdir(startfrom, 0, 1, 0);
                showbar(curent, 1, startfrom);
            }
            else
                if (curent != oldent)
                {
                    showbar(oldent, 0, startfrom);
                    showbar(curent, 1, startfrom);
                }
        }

        vga_unlockvc();

        /* check for Esc. There is no mouse equivalent as button presses
         * are *very* likely to be lost due to length between mouse_update
         * calls. (The call will read both the button press and release - and
         * we never get to see either.) The mouse (pointer) is difficult to cope
         * with when there's a lot of drawing going on anyway, so this probably
         * isn't much of a loss.
         */
        if (has_mouse) mouse_update(), click_update();
        if (readnbkey(zgv_ttyfd) == RK_ESC)
            return(0);
    }

    /* lose any pending left/right click (this is good enough) */
    is_end_click_left(); is_end_click_right();

    return(1);	/* `success' */
}


/* update indexes (xvpics) of current directory.
 * we draw the stuff as we go along, and allow Esc to abort
 * the process. Checking for Esc happens between creating xvpics.
 * the directory `.xvpics' is created if needed.
 */
void update_xvpics(int do_dirs_instead)
{
    char* errstring;
    int ret;

    vga_runinbackground(1);
    ret = update_xvpics_common(do_dirs_instead, &errstring, 1, 0);
    wait_for_foreground();

    if (ret)
        usleep(400000);	/* for effect :-) */
    else
    {
        if (errstring)
            msgbox(zgv_ttyfd, errstring,
                MSGBOXTYPE_OK, idx_light, idx_dark, idx_black);
    }
}


/* allocate some initial space for the olddirs[] array. */
void olddir_init()
{
    if (olddirs != NULL) return;	/* sanity check */

    /* if there's no memory, NULL is the indication and we do without;
     * so not checking the result is *intentional*, we check it later.
     */
    olddirs = malloc(olddir_byte_size);

    num_olddirs = 0;
}


/* make olddirs bigger if needed.
 * call this *before writing each new entry*.
 */
void olddir_resize_if_needed(int newent)
{
    if (!olddirs) return;

    /* this is absurdly conservative, just to be on the safe side :-) */
    if ((newent + 1) * sizeof(struct olddir_tag) >= olddir_byte_size)
    {
        struct olddir_tag* lastval = olddirs;

        olddir_byte_size += olddir_byte_incr;
        if ((olddirs = realloc(olddirs, olddir_byte_size)) == NULL)
        {
            free(lastval);	/* since realloc() doesn't */
            /* olddirs is already NULL */
            num_olddirs = 0;
        }
    }
}


void olddir_uninit(void)
{
    if (!olddirs) return;

    free(olddirs);
    olddirs = NULL;
    num_olddirs = 0;
}


/* *firstp is non-zero if we've done no update_xvpics_common() calls,
 * else zero.
 */
int recursive_update_internal(char* dirname, int* firstp)
{
    DIR* dir;
    struct dirent* dent;
    struct stat sbuf;
    int ent;
    char* old_cwd, * errstring;
    int f, ret, aborted = 0;

    /* save old cwd to avoid depending on chdir("..") to be sane :-) */
    if ((old_cwd = getcwd_allocated()) == NULL)
        return(0);

    if (stat_(dirname, &sbuf) == -1 || chdir(dirname) == -1)
    {
        free(old_cwd);
        return(1);	/* it's not great, sure, but keep going */
    }

    if (olddirs)
    {
        /* see if we've done this one before (a symlink loop could cause this).
         * XXX an array isn't exactly the most efficient thing to search...
         */
        for (f = 0; f < num_olddirs; f++)
            if (sbuf.st_dev == olddirs[f].device && sbuf.st_ino == olddirs[f].inode)
            {
                chdir(old_cwd);
                free(old_cwd);
                return(1);
            }

        /* save this as a visited dir */
        ent = num_olddirs;
        num_olddirs++;
        olddir_resize_if_needed(num_olddirs);
        if (olddirs)	/* check again, as might be OOM */
        {
            olddirs[ent].device = sbuf.st_dev;
            olddirs[ent].inode = sbuf.st_ino;
        }
    }

    /* open the dir */
    if ((dir = opendir(".")) == NULL)
    {
        chdir(old_cwd);
        free(old_cwd);
        return(1);
    }

    /* scan through it and recurse into any dirs */
    while ((dent = readdir(dir)) != NULL)
    {
        /* skip hidden files (even if we do allow searching these at
         * some point, would want to skip `.'/`..'/`.xvpics').
         */
        if (dent->d_name[0] == '.')
            continue;

        /* skip if we can't stat it or it's not a dir */
        if ((stat_(dent->d_name, &sbuf)) == -1 || !S_ISDIR(sbuf.st_mode))
            continue;

        /* ok then, recurse. */
        if (!recursive_update_internal(dent->d_name, firstp))
        {
            aborted = 1;
            break;	/* stop looping after an abort */
        }
    }

    closedir(dir);

    /* now update thumbnails for this dir */
    ret = 0; errstring = NULL;
    if (!aborted)
    {
        readgifdir(0);
        ret = update_xvpics_common(0, &errstring, *firstp, 1);
        *firstp = 0;
    }

    /* return to previous dir */
    chdir(old_cwd);
    free(old_cwd);

    if (!ret && !errstring)	/* they aborted */
        return(0);

    return(1);
}


/* recursive thumbnail update for current dir and all subdirs.
 * ignores errors, in the interest of updating all dirs.
 * This trashes old gifdir[], so you'll need readgifdir() etc. after
 * calling.
 */
void recursive_update(void)
{
    int ret, first = 1;

    vga_runinbackground(1);

    olddir_init();
    ret = recursive_update_internal(".", &first);	/* do the recursive bit */
    olddir_uninit();

    wait_for_foreground();

    if (ret)
        usleep(400000);	/* for effect :-) */
}



#ifdef BACKEND_SVGALIB

#ifdef OSTYPE_LINUX 

/* if we're not running directly on a console, try to find a free console
 * and move us to it. Notes old VT so we can switch back to it when finished,
 * if we ran on a different one to start off with.
 *
 * svgalib 1.2.11 and up do something similar (which was based on this,
 * I think), but it doesn't *quite* do what we need (e.g. it changes stdout,
 * which is sensible generally but zgv wants to keep it the same). So, er,
 * that's why this code's still here. :-)
 *
 * NB: *This is run as root*. Think twice before messing with it.
 */
int fixvt()
{
    static char vt_filename[128];
    struct stat sbuf;
    struct vt_stat vts;
    int major, minor;
    int fd;
    int num;

    /* see if terminal is a console */
    fd = dup(2);
    fstat(fd, &sbuf);
    major = sbuf.st_rdev >> 8;
    zgv_vt = minor = sbuf.st_rdev & 0xff;
    close(fd);
    if (major == 4 && minor < 64)
        return(1);	/* if on a console, already ok */

      /* otherwise we need to look for a free VT, redirect std{in,err},
       * and switch to it. If there's no free VTs, give up now.
       */

    separate_vt = 1;

    /* still root perms, so this shouldn't be a problem... */
    if ((fd = open("/dev/console", O_WRONLY)) < 0) return(0);
    ioctl(fd, VT_GETSTATE, &vts);
    original_vt = vts.v_active;
    ioctl(fd, VT_OPENQRY, &num);
    if (num == -1) return(0);	/* no VTs free */

    /* now, before we go for it, we test the *current* VT to see if they
     * own it. If so, the user's probably `genuine'.
     * (NB: the kernel now does this, but there's no harm repeating it.)
     */
    snprintf(vt_filename, sizeof(vt_filename), "/dev/tty%d", original_vt);
    stat(vt_filename, &sbuf);
    if (getuid() && getuid() != sbuf.st_uid)
    {
        fprintf(stderr,
            "zgv: you must be the owner of the current console to run zgv.\n");
        exit(1);
    }

    /* switch to the new VT */
    ioctl(fd, VT_ACTIVATE, num);
    close(fd);

    /* This is incredibly annoying, but the 2.0.x kernel just *will not*
     * work without it. :-(((
     * So, this gives really weird results for `zgv -h' etc., as the parent
     * returns immediately. Redirect stdout if this is a problem.
     */
    if (fork()) exit(0);

    zgv_vt = num;
    snprintf(vt_filename, sizeof(vt_filename), "/dev/tty%d", num);

    setsid();

    if (freopen(vt_filename, "r", stdin) == NULL)	return(0);
    if (freopen(vt_filename, "w", stderr) == NULL)	return(0);

    ioctl(0, VT_WAITACTIVE, num);

    /* not needed, but... just in case... */
    chown(vt_filename, getuid(), getgid());

    /* ok, done it. */
    return(1);
}

#endif	/* OSTYPE_LINUX */

#ifdef OSTYPE_FREEBSD

/* version for FreeBSD */
int fixvt()
{
    static char vt_filename[128];
    struct stat sbuf;
    struct vid_info info;
    int fd;
    int num = -1;

    /* see if terminal is a console */
    info.size = sizeof(info);
    if (ioctl(0, CONS_GETINFO, &info) != -1)
        return(1);	/* if on a console, already ok */

    separate_vt = 1;

    /* still root perms, so this shouldn't be a problem... */
    if ((fd = open("/dev/console", O_WRONLY)) < 0)
    {
        fprintf(stderr, "zgv: can't open /dev/console.\n");
        return(0);
    }
    ioctl(fd, VT_GETACTIVE, &original_vt);
    ioctl(fd, VT_OPENQRY, &num);
    if (num == -1) return(0);	/* no VTs free */

    /* now, before we go for it, we test the *current* VT to see if they
     * own it. If so, the user's probably `genuine'.
     * (NB: the kernel now does this, but there's no harm repeating it.)
     */
    snprintf(vt_filename, sizeof(vt_filename), "/dev/ttyv%x", original_vt - 1);
    if (getuid() && !stat(vt_filename, &sbuf) && getuid() != sbuf.st_uid)
    {
        fprintf(stderr,
            "zgv: you must be the owner of the current console to run zgv.\n");
        exit(1);
    }

    /* switch to the new VT */
    if (-1 == ioctl(fd, VT_ACTIVATE, num))
        fprintf(stderr, "zgv: now running on virtual console %d.\n", num);

    close(fd);
    zgv_vt = num;

    snprintf(vt_filename, sizeof(vt_filename), "/dev/ttyv%x", num - 1);
    chown(vt_filename, getuid(), getgid());
    ioctl(0, VT_WAITACTIVE, num);

    /* not needed, but... just in case... */
    chown(vt_filename, getuid(), getgid());

    /* ok, done it. */
    return(1);
}

#endif	/* OSTYPE_FREEBSD */

#endif	/* BACKEND_SVGALIB */


void switchback()
{
#ifdef BACKEND_SVGALIB
    struct vt_mode vtm;

    /* also change back to stdin blocking;
     * some versions of bash seem to be a little sensitive to it being
     * left non-blocking. (And it's not very nice leaving it like that anyway.)
     */
    fcntl(zgv_ttyfd, F_SETFL, 0);

    if (separate_vt)
    {
        fprintf(stderr, "%c[H%c[J", 27, 27);	/* seems to get junk-filled... */
        ioctl(zgv_ttyfd, VT_GETMODE, &vtm);
        vtm.mode = VT_AUTO;
        ioctl(zgv_ttyfd, VT_SETMODE, &vtm);
        ioctl(zgv_ttyfd, VT_ACTIVATE, original_vt);
    }
#endif

    /* oh, and close mouse if opened. :-) */
    if (has_mouse) mouse_close();
}


void greyfix332(unsigned char** image, int w, int h, int xw7, int w8)
{
    unsigned char* new, * ptr;
    int x, y, lx, c;
    int c0, c1, c2, times2;
    int terr0, terr1, terr2, actual0, actual1, actual2;
    int start, addon, r, g, b;
    int* evenerr, * odderr;
    int* thiserr;
    int* nexterr;

    if ((new = malloc(w8 * h)) == NULL)
    {
        free(*image);
        *image = NULL;
        return;
    }

    memset(new, idx_medium, w8 * h);

    ptr = *image;

    if (cfg.fs16col)
    {
        /* colour */
        /* uh-huh, case and paste mode on */

        if ((evenerr = calloc(3 * (w + 10), sizeof(int))) == NULL ||
            (odderr = calloc(3 * (w + 10), sizeof(int))) == NULL)
            exit(1);

        for (y = 0; y < h; y++)
        {
            ptr = *image + w * y;

            if ((y & 1) == 0)
            {
                start = 0; addon = 1;
                thiserr = evenerr + 3; nexterr = odderr + w * 3;
            }
            else
            {
                start = w - 1; addon = -1;
                thiserr = odderr + 3; nexterr = evenerr + w * 3;
            }

            nexterr[0] = nexterr[1] = nexterr[2] = 0;
            x = start;
            for (lx = 0; lx < w; lx++)
            {
                c = ptr[x];
                b = (c & 3) * 64; g = ((c >> 2) & 7) * 32; r = (c >> 5) * 32;
                r = (r - 128) * 3 / 2 + 128;
                g = (g - 128) * 3 / 2 + 128;
                b = (b - 128) * 3 / 2 + 128;

                terr0 = r + ((thiserr[0] + 8) >> 4);
                terr1 = g + ((thiserr[1] + 8) >> 4);
                terr2 = b + ((thiserr[2] + 8) >> 4);

                actual0 = (terr0 >= 128) ? 255 : 0;
                actual1 = (terr1 >= 128) ? 255 : 0;
                actual2 = (terr2 >= 128) ? 255 : 0;

                if (actual0 < 0) actual0 = 0; if (actual0 > 255) actual0 = 255;
                if (actual1 < 0) actual1 = 0; if (actual1 > 255) actual1 = 255;
                if (actual2 < 0) actual2 = 0; if (actual2 > 255) actual2 = 255;

                c0 = terr0 - actual0;
                c1 = terr1 - actual1;
                c2 = terr2 - actual2;

                times2 = (c0 << 1);
                nexterr[-3] = c0; c0 += times2;
                nexterr[3] += c0; c0 += times2;
                nexterr[0] += c0; c0 += times2;
                thiserr[3] += c0;

                times2 = (c1 << 1);
                nexterr[-2] = c1; c1 += times2;
                nexterr[4] += c1; c1 += times2;
                nexterr[1] += c1; c1 += times2;
                thiserr[4] += c1;

                times2 = (c2 << 1);
                nexterr[-1] = c2; c2 += times2;
                nexterr[5] += c2; c2 += times2;
                nexterr[2] += c2; c2 += times2;
                thiserr[5] += c2;

                new[y * w8 + x + xw7] = (actual0 & 1) * 4 + (actual1 & 1) * 2 + (actual2 & 1) + 1;

                thiserr += 3;
                nexterr -= 3;
                x += addon;
            }
        }

        free(evenerr);
        free(odderr);
    }
    else
        /* greyscale */
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++, ptr++)
            {
                c = *ptr;
                c = ((c & 3) * 2 + ((c >> 2) & 7) + (c >> 5)) * GREY_MAXVAL / 21; /* greyscale 332 colour */
                if (c < 0) c = 0; if (c > GREY_MAXVAL) c = GREY_MAXVAL;
                new[y * w8 + x + xw7] = c + 1;
            }

    free(*image);
    *image = new;
}


void wait_for_foreground()
{
    vga_runinbackground(0);
    //ioctl(zgv_ttyfd, VT_WAITACTIVE, zgv_vt);
}


void ctrlc(int foo)
{
    switchback();		/* and go back to blocking input */
    exit(1);
}


/* creates an array of offsets in gifdir - these are what we
 * randomise for `shuffleslideshow' mode. (But they're used even
 * if we don't shuffle, to simplify the code a little.)
 */
int make_slideshow_idx_array()
{
    int f, t, * ptr;

    for (f = 1, t = 0; f <= gifdirsiz; f++)
        if (gifdir[f].marked) t++;
    slideshow_idx_size = t;

    if ((slideshow_idx = malloc(t * sizeof(int))) == NULL)
        return(0);

    ptr = slideshow_idx;
    for (f = 1; f <= gifdirsiz; f++)
        if (gifdir[f].marked) *ptr++ = f;

    return(1);
}


void shuffle_slideshow_idx()
{
    int f, g, h, left;
    int* tmparr;

    if ((tmparr = malloc(slideshow_idx_size * sizeof(int))) == NULL)
        return;	/* merely won't be random, nothing nasty */

      /* copy indicies to tmparr */
    for (f = 0; f < slideshow_idx_size; f++)
        tmparr[f] = slideshow_idx[f];

    /* now move a random one from that to slideshow_idx N times */
    left = slideshow_idx_size;	/* how many left in tmparr[] */
    for (f = 0; f < slideshow_idx_size; f++)
    {
        g = rand() % left;
        slideshow_idx[f] = tmparr[g];

        /* move the rest of tmparr[] up and decr num left */
        for (h = g; h < left - 1; h++) tmparr[h] = tmparr[h + 1];
        left--;
    }

    free(tmparr);
}



/* allocate some initial space for the gifdir[] array.
 * this should ideally be run before going into graphics mode.
 */
void gifdir_init()
{
    if (gifdir != NULL) return;	/* sanity check */

    if ((gifdir = malloc(gifdir_byte_size)) == NULL)
        fprintf(stderr, "zgv: out of memory\n"), exit(1);
}


/* make gifdir bigger if needed.
 * call this *before writing each new entry*.
 * if this returns zero, the caller should quit with not-enough-memory
 * error, exiting graphics mode if needed.
 *
 * note that gifdir is never made smaller. This would complicate things
 * considerably for a fairly small gain, I think.
 */
int gifdir_resize_if_needed(int newent)
{
    /* this is absurdly conservative, just to be on the safe side :-) */
    if ((newent + 1) * sizeof(struct gifdir_tag) >= gifdir_byte_size)
    {
        gifdir_byte_size += gifdir_byte_incr;
        if ((gifdir = realloc(gifdir, gifdir_byte_size)) == NULL)
            return(0);		/* out of memory */
    }

    return(1);		/* worked */
}
