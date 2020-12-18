/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * rcfile.c - config file handling.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>	/* for getopt() */
 #include "getopt.h"
#include "zgv_io.h"
#include "rc_config.h"
#include "zgv.h"	/* for ZGV_VER */
#include "rcfile.h"
#include <getenv.h>

#ifndef GLASTMODE
/* not sure if this is possible, but just in case...
 * (this is fairly conservative, of course)
 */
#define GLASTMODE	G1280x1024x16M
#endif

static int line;		/* current line in config file */
int in_config = 0;		/* true if reading config file */
char* config_file_name = NULL;	/* full path to config file used */


/* required prototypes */
void get_bool(char* arg, void* dataptr);
void get_bool_and_inv(char* arg, void* dataptr);
void get_int(char* arg, void* data);
void get_double(char* arg, void* data);
int get_modenumber(char* tptr, int* mp, int bpp_force);
void get_rgbval(char* arg, void* data);
void get_jis(char* arg, void* data);
void do_allmodesbad(char* arg, void* data);
void do_allmodesgood(char* arg, void* data);
void do_badmode(char* arg, void* data);
void do_goodmode(char* arg, void* data);
void do_startmode(char* arg, void* data);
void do_warn(char* arg, void* data);
int modematch(int x, int y, int bpp);
void print_version(char* arg, void* dataptr);
void usage_help(char* arg, void* dataptr);



struct zgv_config cfg;

struct cfglookup_tag
{
	char* name;
	int allow_config;		/* if zero, only here for cmdline opt lookup */
	void (*funcptr)(char*, void*);
	void* dataptr;
};


/* include opt/var tables generated from options.src by mkopts.awk;
 * rcfile_var.h has to be included after defs of above struct and
 * some prototypes, so may as well keep them together.
 */
#include "rcfile_opt.h"
#include "rcfile_var.h"

 /* and this one defines the short-option string. */
#include "rcfile_short.h"


/* macro for `zgv:'-ish error prefix, the format of which should
 * differ when reading a config file.
 */
#define CONFIG_ERR_PREFIX() \
	do \
	  { \
	  if(in_config) \
	    printf("zgv:%s:%d: ",config_file_name,line); \
	  else \
	    printf("zgv: "); \
	  } \
	while(0)



 /* find next char which isn't NUL, space, or tab. */
void find_token(char** ptr)
{
	while (*(*ptr) && strchr(" \t", *(*ptr)) != NULL)
		(*ptr)++;
}


int token_length(char* ptr)
{
	char* start = ptr;

	while (*ptr && strchr(" \t", *ptr) == NULL)
		ptr++;

	return(ptr - start);
}


/* returns 1 if equal, 0 otherwise */
int token_compare(char* tptr, char* txt)
{
	int tlen;

	tlen = token_length(tptr);
	if (tlen != strlen(txt))
		return(0);

	return(strncmp(tptr, txt, tlen) == 0);
}


void fixconfig(void)
{
	/* XXX should probably add many more to this, and perhaps give
	 * error messages for some.
	 */

	 /* don't allow funny values for this; zero is disallowed because
	  * it could make it difficult to abort if all the files load very quickly.
	  */
	if (cfg.tag_timer <= 0) cfg.tag_timer = 1;

	if (cfg.jpegspeed < 1) cfg.jpegspeed = 1;
	if (cfg.jpegspeed > 3) cfg.jpegspeed = 3;

	if (cfg.mousescale < 1) cfg.mousescale = 1;
}


void parse_config(FILE* in)
{
	static char inpline[1024];
	char* ptr;
	int f, c, inpc, found = 0;

	line = 0;

	while (fgets(inpline, sizeof(inpline), in) != NULL)
	{
		line++;

		if (inpline[strlen(inpline) - 1] == '\n') inpline[strlen(inpline) - 1] = 0;
		if ((ptr = strchr(inpline, '#')) != NULL)
			*ptr = 0;

		if (*inpline)
		{
			found = 0;
			ptr = inpline;
			find_token(&ptr);
			inpc = *ptr;

			for (f = 0; (c = cfglookup[f].name[0]); f++)
				if (inpc == c && token_compare(ptr, cfglookup[f].name) &&
					cfglookup[f].allow_config)
				{
					ptr += token_length(ptr);		/* skip current token */
					find_token(&ptr);		/* find next (if any) */
					if (*ptr == 0)
						ptr = NULL;

					/* we supply the token (or lack of one) whether they asked
					 * for it or not - it's up to them whether to use it or ignore it.
					 * first arg is NULL if no arg was present.
					 */
					(*cfglookup[f].funcptr)(ptr, cfglookup[f].dataptr);
					found = 1;
					break;
				}

			if (!found)
			{
				CONFIG_ERR_PREFIX();
				printf( "bad variable name.\n");
				exit(1);
			}
		}
	}
}


void defaultcfg(void)
{
	int f;

	/* it'll use 360x480x8 if you haven't got 640x480x8,
	 * and 320x200x8 if you've locked that out.
	 */
	cfg.videomode = G640x480x256;
	/* for selector, it's 640x480x8 if possible, otherwise 640x480x4. */
	cfg.fs_startmode = G640x480x256;
	cfg.zoom = 0;
	cfg.zoom_reduce_only = 0;
	cfg.vkludge = 0;
	cfg.brightness = 0;
	cfg.jpeg24bit = 1;	/* only if possible, of course */
	cfg.jpegspeed = 2;	/* slow int by default */
	cfg.jpegindexstyle = 2;	/* jpeg thumbnail speed/quality */
	cfg.betterpgm = 0;
	cfg.centreflag = 1;
	cfg.blockcursor = 0;
	cfg.thicktext = 0;
	cfg.hicolmodes = 0;	/* don't force high-colour mode test to true */
	cfg.nodelprompt = 0;	/* default to prompting on delete */
	cfg.nodelprompt_tagged = 0; /* prompt for tagged delete too */
	cfg.perfectindex = 0;	/* make selector cols look nice, not index cols */
	cfg.xvpic_index = 1;	/* visual index */
	cfg.onefile_progress = 1;	/* progress report while loading file given as arg */
	cfg.cleartext = 0;	/* clear text screen on startup/exit? */
	cfg.repeat_timer = 0;	/* don't reread after a timeout */
	cfg.tag_timer = 4;	/* 4 seconds per tagged file */
	cfg.force16fs = 0;        /* don't force 16-colour mode for file selector */
	cfg.revert = 1;		/* auto-reset scaling to off between pics */
	cfg.revert_orient = 1;	/* auto-reset pic orientation between pics */
	cfg.fakecols = 1;		/* try to fake more greys/colours in 8-bit modes */
	cfg.selecting = 0;	/* no selection normally */
	cfg.fs16col = 0;		/* normally use mono in 16-col file selector */
	cfg.viewer16col = 0;	/* normally use mono in 16-col viewer mode */
	cfg.fastdither16col = 1;	/* 1=use ordered dither, 0=error-diffused (in 16col) */
	cfg.echotagged = 0;	/* echo tagged files on exit */
	cfg.forgetoldpos = 0;	/* don't return to old position in revisited dir */
	cfg.linetext = 0;		/* use old line-based text instead of bitmap fonts */
	cfg.smallfstext = 0;	/* use smaller font for filenames in file selector */
	cfg.writefile = 0;
	cfg.slowupdate = 0;
	cfg.shuffleslideshow = 0;
	cfg.scrollbar = 1;
	cfg.mousekludge = 1;
	cfg.mousescale = 16;	/* gives fair results for trackballs/mice I've tried */
	cfg.stupid_gnu_verbosity = 0;
	cfg.bc_order_rev = 0;
	cfg.xzgvkeys = 0;
	cfg.automodefit = 0;
	cfg.deltamodefit = 0;
	cfg.initial_picgamma = 1.0;
	cfg.fsmagic = 0;
	cfg.black_bg = 0;
	cfg.auto_animate = 0;
	cfg.fullscreen = 1;
	cfg.dither_hicol = 0;

	cfg.contrast = (double)1;
	cfg.black.r = cfg.black.g = cfg.black.b = 0;
	cfg.dark.r = cfg.dark.g = cfg.dark.b = 20;
	cfg.medium.r = cfg.medium.g = cfg.medium.b = 30;
	cfg.light.r = cfg.light.g = cfg.light.b = 40;
	cfg.marked.r = cfg.marked.g = cfg.marked.b = 0; cfg.marked.r += 30;
	cfg.loop = 0;
	cfg.svgalib_mouse = 0;
	for (f = 0; f < 256; f++) cfg.mode_allowed[f] = 0;
	cfg.showxvpicdir = 0;
	for (f = 0; f <= GLASTMODE; f++)
		cfg.mode_allowed[f] = vga_hasmode(f) ? 1 : 0;
	cfg.errignore = 0;
	cfg.pcdres = 3;
}


void getconfig(void)
{
	static char cfgfile[1024];
	FILE* in;
	int got_rcfile = 0;
	char* home = getenv("HOME");

	defaultcfg();

	in_config = 1;

	*cfgfile = 0;
	if (home && strlen(home) < sizeof(cfgfile) - strlen("/.zgvrc") - 1)
		sprintf(cfgfile, "%s/.zgvrc", home);

	if ((in = fopen(cfgfile, "r")) != NULL)
		got_rcfile = 1, config_file_name = "~/.zgvrc";	/* shortened name for that */
	else if ((in = fopen(config_file_name = "zgv.conf", "r")) != NULL)
		got_rcfile = 1;

	if (got_rcfile)
	{
		parse_config(in);
		fclose(in);
	}

	in_config = 0;
}


void get_bool(char* arg, void* data)
{
	int* boolptr = (int*)data;

	if (!arg)
	{
		if (!in_config)
		{
			/* if no arg, and not in config file, set the flag. */
			*boolptr = 1;
			return;
		}
		else	/* in config file, so it's an error */
		{
			CONFIG_ERR_PREFIX();
			printf("option-setting arg (on/off, yes/no, ...) missing.\n");
			exit(1);
		}
	}

	/* otherwise, set depending on value given. */
	if (token_compare(arg, "on") || *arg == 'y' || *arg == '1')
		*boolptr = 1;
	else
		if (token_compare(arg, "off") || *arg == 'n' || *arg == '0')
			*boolptr = 0;
		else
		{
			CONFIG_ERR_PREFIX();
			printf(
				"bad option-setting arg\n\t(use on/off, y/n, yes/no, or 1/0).\n");
			exit(1);
		}
}


void get_bool_and_inv(char* arg, void* data)
{
	int* ptr = (int*)data;
	int orig = *ptr;

	*ptr = -1;	/* so we can spot changes */
	get_bool(arg, data);
	if (*ptr == -1)
		*ptr = orig;	/* restore old val */
	else
		*ptr = !*ptr;	/* invert the assignment */
}


void get_int(char* arg, void* data)
{
	int* ptr = (int*)data;

	if (arg == NULL)
	{
		CONFIG_ERR_PREFIX();
		printf("missing arg.\n");
		exit(1);
	}

	*ptr = atoi(arg);
}


void get_double(char* arg, void* data)
{
	double* ptr = (double*)data;

	if (arg == NULL)
	{
		CONFIG_ERR_PREFIX();
		printf("missing arg.\n");
		exit(1);
	}

	*ptr = atof(arg);
}


/* get mode number from something like `640 480 8' -
 * returns 0 if it can't find one that matches, else 1.
 * (the mode number is put into *mp)
 *
 * If bpp_force is non-zero, it sets the mode's bpp, overriding the
 * value specified in the string.
 */
int get_modenumber(char* tptr, int* mp, int bpp_force)
{
	int x, y, bpp, rtn;

	if (sscanf(tptr, "%d %d %d", &x, &y, &bpp) != 3)
	{
		CONFIG_ERR_PREFIX();
		printf( "bad or missing arg.\n");
		exit(1);
	}

	if (bpp_force)
		bpp = bpp_force;

	rtn = modematch(x, y, bpp);
	if (rtn >= 0)
	{
		*mp = rtn;
		return(0);
	}
	else
		return(-1);
}


void get_rgbval(char* arg, void* data)
{
	struct cfgrgb_tag* p = (struct cfgrgb_tag*)data;

	if (sscanf(arg, "%d %d %d", &(p->r), &(p->g), &(p->b)) != 3)
	{
		CONFIG_ERR_PREFIX();
		printf("bad or missing arg.\n");
		exit(1);
	}
}


void get_jis(char* arg, void* data)
{
	int* jisp = (int*)data;

	get_int(arg, data);
	if (*jisp < 1) *jisp = 1;
	if (*jisp > 3) *jisp = 3;
}


void do_allmodesbad(char* arg, void* data)
{
	int f;

	for (f = 0; f < 256; f++) cfg.mode_allowed[f] = 0;
}


void do_allmodesgood(char* arg, void* data)
{
	int f;

	/* we still disallow ones it hasn't got */
	for (f = 0; f <= GLASTMODE; f++)
		cfg.mode_allowed[f] = vga_hasmode(f) ? 1 : 0;
}


void do_badmode(char* arg, void* data)
{
	int f;

	if (get_modenumber(arg, &f, 0) == -1)
	{
		CONFIG_ERR_PREFIX();
		printf( "bad or unsupported mode.\n");
		exit(1);
	}
	cfg.mode_allowed[f] = 0;
}


void do_goodmode(char* arg, void* data)
{
	int f;

	if (get_modenumber(arg, &f, 0) == -1)
	{
		CONFIG_ERR_PREFIX();
		printf( "bad or unsupported mode.\n");
		exit(1);
	}
	cfg.mode_allowed[f] = 1;
}


/* used for both startmode and fs_startmode. */
void do_startmode(char* arg, void* data)
{
	int* mp = (int*)data;
	int f;
	int bpp_force = 0;

	/* fairly kludgey... */
	if (mp == &cfg.fs_startmode)
		bpp_force = 8;			/* make sure file-sel modes are 8-bit */

	if (get_modenumber(arg, &f, bpp_force) == -1)
	{
		CONFIG_ERR_PREFIX();
		printf( "bad or unsupported mode.\n");
		exit(1);
	}
	*mp = f;
}


void do_warn(char* arg, void* data)
{
	char* ptr = (char*)data;

	CONFIG_ERR_PREFIX();
	printf("warning: the `%s' option is obsolete,\n"
		"\t\t\tand is being ignored.\n",
		ptr);
}


/* returns mode number which matches x,y,bpp or -1 if none did.
 * put `-1' in x,y or bpp to wildcard them.
 *
 * maybe this routine should be somewhere else?
 */
int modematch(int x, int y, int bpp)
{
	int numcols, f, bytepp;
	vga_modeinfo* vminfo;

	if ((bpp > 32) || (bpp == 2))
	{
		/* they must have used numcols, not bpp. hmm, let 'em get away with it. */
		numcols = bpp;
		bytepp = (numcols + 255) / 256;
	}
	else
	{
		numcols = (1 << bpp);
		if (bpp == 32) numcols = (1 << 24);
		bytepp = (bpp + 7) / 8;
	}

	/* we check 0 - 255 at the most */
	for (f = 0; f < 256; f++)
	{
		vminfo = vga_getmodeinfo(f);
		if (vminfo != NULL)
			if ((x == -1 || x == vminfo->width) &&
				(y == -1 || y == vminfo->height) &&
				(numcols == -1 ||
				(numcols == vminfo->colors && bytepp == vminfo->bytesperpixel)))
				break;
	}

	if (f < 255)
		return(f);
	else
		return(-1);
}


int parsecommandline(int argc, char* argv[])
{
	const char* name;	/* const needed because of struct option declaration */
	int entry, ret, f, c, namec, found;

	do
	{
		/* the GNU libc docs don't make it clear whether optarg is set to NULL
		 * when a *short* option doesn't have an arg, so I play it safe here.
		 */
		optarg = NULL;

		/* SHORTOPT_STRING is defined in rcfile_short.h, as gen'd from options.src */
		ret = getopt_long(argc, argv, SHORTOPT_STRING, long_opts, &entry);

		if (ret == '?')
		{
			/* no need for error message, it's already been done */
			exit(1);
		}

		if (ret != -1)
		{
			/* if we have a short option, it returns char code; find relevant
			 * long-option entry. It also returns char code for long options
			 * with equivalent short option - it doesn't hurt to look
			 * it up in that case as well though. It's not like this takes huge
			 * amounts of CPU... ;-)
			 */
			if (isalnum(ret))
			{
				found = 0;

				for (f = 0; long_opts[f].name; f++)
				{
					if (long_opts[f].val == ret)
					{
						entry = f;
						found = 1;
						break;
					}
				}

				if (!found)
				{
					printf(
						"short option not found in long_opts[] - can't happen!\n");
					continue;
				}
			}

			/* now we have a valid entry in long_opts[], lookup name in
			 * cfglookup to get funcptr/dataptr and run the function.
			 */
			name = long_opts[entry].name;
			namec = *name;
			found = 0;

			for (f = 0; (c = cfglookup[f].name[0]); f++)
				if (namec == c && strcmp(name, cfglookup[f].name) == 0)
				{
					(*cfglookup[f].funcptr)(optarg, cfglookup[f].dataptr);
					found = 1;
					break;
				}

			if (!found)
				printf(
					"long option not found in cfglookup[] - can't happen!\n");
		}
	} while (ret != -1);

	return(argc - optind);
}


void print_version(char* arg, void* dataptr)
{
	printf("zgv " ZGV_VER "\n");
	exit(0);
}

#include <include/systemcall_impl.h>
void usage_help(char* arg, void* dataptr)
{
	printf("zgv " ZGV_VER
		" - copyright (C) 1993-2005 Russell Marks.\n\n");
	/*printf("usage: zgv [options] [dir | file ...]\n"
		"\n"
		"   -A	--auto-animate\n"
		"			automatically animate multiple-image GIFs.\n"
		"			This greatly limits how such images can be viewed,\n"
		"			but can be useful for slideshows.\n"
		"	--auto-mode-fit\n"
		"			automatically switch modes to suit image size.\n"
		"	--auto-mode-fit-diff diff\n"
		"			Add diff to the width and height of each mode when\n"
		"			doing an auto-mode-fit (default 0).\n"
		"   -p	--avoid-single-progress\n"
		"			don't show progress indicator when loading a\n"
		"			single file from the command-line.\n"
		"	--black-background\n"
		"			try to use a black background in the viewer when\n"
		"			displaying an 8-bit image.\n"
		"   -b	--block-cursor	use a blocky outline cursor, which is rather\n"
		"			unsubtle but more obvious.\n"
		"	--brightness adjust_val\n"
		"			specify how much to add to colour values to\n"
		"			change brightness (default 0).\n"
		"	--centre		(or --center)\n"
		"			(normally enabled, use --centre=off to disable)\n"
		"			if *disabled*, don't centre images onscreen.\n"
		"	--clear-screen-on-exit\n"
		"			clear text console's contents on exit.\n"
		"	--col-black \"r g b\"\n"
		"			set colour used for `black' (text) in selector;\n"
		"			RGB values should be in the range 0-63 for this\n"
		"			and the other colour settings below, and the\n"
		"			three numbers should be quoted as shown.\n"
		"	--col-dark \"r g b\"\n"
		"			set colour used for lowlights in selector.\n"
		"	--col-light \"r g b\"\n"
		"			set colour used for highlights in selector.\n"
		"	--col-medium \"r g b\"\n"
		"			set colour used for background in selector.\n"
		"	--col-tagged \"r g b\"\n"
		"			set colour used for tagged files in selector.\n"
		"	--contrast multiplier\n"
		"			specify how much to multiply colour values by\n"
		"			to change contrast (default 1.0).\n"
		"	--delete-single-prompt\n"
		"			(normally enabled, use --delete-single-prompt=off to\n"
		"			disable) if *disabled*, don't prompt for confirmation\n"
		"			when deleting a single file (with the Delete key).\n"
		"	--delete-tagged-prompt\n"
		"			(normally enabled, use --delete-tagged-prompt=off to\n"
		"			disable) if *disabled*, don't prompt for confirmation\n"
		"			when deleting tagged files (with `D').\n"
		"	--dither-16col-fast\n"
		"			(normally enabled, use --dither-16col-fast=off to\n"
		"			disable) if *disabled*, use (much) slower but\n"
		"			more accurate dithering in the viewer's 16-colour\n"
		"			mode.\n"
		"	--dither-hicol	if enabled, use dithering in `high-colour' modes,\n"
		"			i.e. 15/16-bit modes. This makes colour gradients\n"
		"			smoother, but slows things down quite a bit.\n"
		"	--fake-cols	(normally enabled, use --fake-cols=off to disable)\n"
		"			if *disabled*, don't fake extra greyscales and\n"
		"			colour depth in 8-bit modes.\n"
		"	--force-fs-16col\n"
		"			force the use of 16-colour mode for the selector.\n"
		"   -j	--force-viewer-8bit\n"
		"			force all images to be loaded as 8-bit. Normally\n"
		"			zgv will load 24-bit images (e.g. colour JPEGs)\n"
		"			as 24-bit if you have any modes capable of\n"
		"			displaying it in 15, 16, 24, or 32-bit colour.\n"
		"	--fs-16col-colour-thumbnails	(or --fs-16col-color-thumbnails)\n"
		"			if using 16-colour mode for the selector,\n"
		"			show dithered colour thumbnails (rather than mono).\n"
		"	--fs-ignore-old-pos\n"
		"			don't recall previous cursor position in a\n"
		"			directory when returning to it later.\n"
		"	--fs-magic	use a slow, but precise (magic-number-based)\n"
		"			method to identify picture files to list\n"
		"			in the selector.\n"
		"	--fs-perfect-cols\n"
		"			don't tweak thumbnail colours to make the rest of\n"
		"			the selector look right.\n"
		"	--fs-slow-thumbnail-update\n"
		"			show current cursor position at all times when\n"
		"			updating thumbnails, which in most cases will\n"
		"			make it markedly slower.\n"
		"	--fs-small-text\n"
		"			use smaller text than usual in the selector.\n"
		"	--fs-start-mode modespec\n"
		"			specify initial mode for the selector; the `modespec'\n"
		"			should be the width, height, and depth in\n"
		"			quotes, e.g. \"640 480 8\". Note that the depth\n"
		"			specified is actually ignored (and forced to 8).\n"
		"   -t	--fs-thick-text\n"
		"			use bold text in selector, which may be useful\n"
		"			for those with mildly impaired vision (especially\n"
		"			when combined with the `-b' option).\n"
		"	--fullscreen	(normally enabled, use --fullscreen=off to disable)\n"
		"			if *disabled*, make zgv run in a window where\n"
		"			possible (e.g. SDL version running under X).\n"
		"   -G	--gamma val	set gamma adjustment. The default is 1.0, i.e.\n"
		"			no adjustment. (See info file or man page for\n"
		"			details, and a discussion of gamma issues.)\n"
		"	--gnulitically-correct\n"
		"			GNU has POSIXLY_CORRECT for compatibility with\n"
		"			silly POSIX misfeatures, and zgv has...\n"
		"   -i	--ignore-errors\n"
		"			when loading a single file from the command-line,\n"
		"			ignore (some) errors.\n"
		"	--jpeg-index-style style\n"
		"			specify how to read JPEGs when creating\n"
		"			thumbnails (default 2):\n"
		"			  1 = the quickest (but this sometimes generates\n"
		"			      rather fuzzy/blocky thumbnails);\n"
		"			  2 = fairly cautious, but still quite fast;\n"
		"			  3 = an extremely cautious and slow method.\n"
		"   -J	--jpeg-speed type\n"
		"			set JPEG speed/quality tradeoff (default 2):\n"
		"			  1 = floating-point - slow but accurate;\n"
		"			  2 = slow integer - faster but not as accurate;\n"
		"			  3 = fast integer - fastest but least accurate.\n"
		"   -h	--help		give this usage help.\n"
		"	--line-text	use old line-based text.\n"
		"   -M	--mouse		enable mouse support; this uses svgalib's mouse\n"
		"			configuration, see the libvga.config(5) man page\n"
		"			for details.\n"
		"	--mouse-scale scale\n"
		"			set the ratio of `mouse pixels' to onscreen pixels.\n"
		"			In effect, sets the mouse speed. Defaults to 16.\n"
		"	--pcd-res resnum\n"
		"			set Photo-CD resolution to use, if support was\n"
		"			enabled at compile-time. (See info file/man page\n"
		"			for further details.)\n"
		"   -g	--pgm-truecol	treat PGM files as 24-bit, ensuring that\n"
		"			256 separate greyscales can be shown if a 24-bit\n"
		"			mode is available.\n"
		"   -r	--reload-delay seconds\n"
		"			re-read and redisplay picture every `seconds' seconds.\n"
		"	--reverse-bc-order\n"
		"			apply brightness modification before contrast.\n"
		"	--revert-orient	(normally enabled, use --revert-orient=off to disable)\n"
		"			if *disabled*, orientation (flip/mirror/rotate) state\n"
		"			is retained between pictures.\n"
		"	--revert-scale	(normally enabled, use --revert-scale=off to disable)\n"
		"			if *disabled*, scaling is retained between pictures.\n"
		"	--scrollbar	show a scrollbar below the selector.\n"
		"   -s	--show-dimensions\n"
		"			print dimensions of section of picture being viewed\n"
		"			to stdout on exit.\n"
		"   -T	--show-tagged	show names of tagged files on exit (they're listed\n"
		"			to stdout).\n"
		"	--show-xvpics-dir\n"
		"			show any `.xvpics' directory, so you can view\n"
		"			thumbnails there even if the pictures they\n"
		"			represent have been deleted.\n"
		"   -S	--slideshow-delay delay\n"
		"			set the delay (in seconds) for which one picture\n"
		"			in a slideshow is shown before reading the next.\n"
		"			The default is 4.\n"
		"   -l	--slideshow-loop\n"
		"			loop in slideshows `forever' (until you exit).\n"
		"   -R	--slideshow-randomise	(or --slideshow-randomize)\n"
		"			randomise order of pictures in slideshows.\n"
		"	--viewer-16col-colour	(or --viewer-16col-color)\n"
		"			use a dithered colour display (rather than mono)\n"
		"			in the viewer's 16-colour mode.\n"
		"   -m	--viewer-start-mode modespec\n"
		"			specify initial mode for the viewer; the `modespec'\n"
		"			should be the width, height, and depth in\n"
		"			quotes, e.g. \"640 480 8\".\n"
		"	--visual	(normally enabled, use --visual=off to disable)\n"
		"			if *disabled*, no thumbnails are shown.\n"
		"   -k	--vkludge	smooth image when zoom mode is reducing a\n"
		"			picture to fit the screen, and when in virtual\n"
		"			modes.\n"
		"	--version	report version number.\n"
		"   -w	--write-ppm	write file as PPM (on stdout) rather than viewing it.\n"
		"			(This only works if you run zgv on a single file,\n"
		"			specified on the command-line.)\n"
		"	--xzgv-keys	remap keys to (mostly) match those used in xzgv.\n"
		"   -z	--zoom		fit picture to the full screen size, whatever the\n"
		"			picture's actual size.\n"
		"	--zoom-reduce-only\n"
		"			when zooming, only *reduce* pictures to fit; i.e.\n"
		"			make big pictures viewable all-at-once while leaving\n"
		"			small pictures intact.\n"
		"\n"
		"	dir		start zgv on a certain directory.\n"
		"	file ...	view (only) the file(s) specified. If more than\n"
		"			one file is given, the files are viewed as a\n"
		"			slideshow.\n"
		"\n"
		"All options are processed after any ~/.zgvrc or zgv.conf \n"
		"file. Most long options (minus the `--') can used in either file with e.g.\n"
		"`zoom on'. (But be sure to omit any quoting mentioned above in config\n"
		"files.)\n"
		"\n"
		"On/off settings (such as zoom) are enabled by e.g. `-z' or `--zoom';\n"
		"however, the long-option form `--option=off' can be used to disable\n"
		"them (needed when they are enabled by default - revert-scale, for\n"
		"example - or to override them being enabled in a config file).\n"
		"\n"
		"(This syntax actually lets you both disable *and* enable options,\n"
		"using (for the arg after `=') on/off, y/n, yes/no, or 1/0.)");

	if (isatty(1))
		printf("\n"
			"If this ridiculously long help text has just flown past :-), try\n"
			"`zgv -h |less' or similar.\n");*/

	//exit(0);
	Syscall_exit(0);
}
