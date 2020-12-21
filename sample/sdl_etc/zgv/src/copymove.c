/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * copymove.c - guts of the code related to copying/moving files in
 *	the file selector.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 //#include <sys/stat.h>
 //#include <sys/types.h>
#include <unistd.h>
//#include <utime.h>
#include <errno.h>
#include "zgv_io.h"
#include "3deffects.h"
#include "font.h"
#include "readnbkey.h"
#include "zgv.h"
#include "copymove.h"

#define TRANSFER_BUF_SIZE	8192



static int cm_isdir(char* filename)
{
	struct stat sbuf;

	return(fstat(filename, &sbuf) == 0 && S_ISDIR(sbuf.st_mode));
}


/* copy file, returns 0 if failed
 * src must be in the current directory (though this isn't checked)
 * and dstdir must be a directory (fails if it isn't)
 */
int copyfile(char* src, char* dstdir)
{
	static unsigned char copybuf[TRANSFER_BUF_SIZE];
	FILE* in, * out;
	char* dst;
	int siz;

	if (!cm_isdir(dstdir)) return(0);

	if ((in = fopen(src, "rb")) == NULL)
		return(0);

	if ((dst = malloc(strlen(dstdir) + strlen(src) + 2)) == NULL)	/* +2 for / and NUL */
		return(0);

	strcpy(dst, dstdir);
	strcat(dst, "/");
	strcat(dst, src);

	/* check it doesn't already exist */
	if ((out = fopen(dst, "rb")) != NULL)
	{
		fclose(out);
		fclose(in);
		free(dst);
		return(0);
	}

	if ((out = fopen(dst, "wb")) == NULL)
	{
		fclose(in);
		free(dst);
		return(0);
	}

	free(dst);

	/* so now both files are open.
	 * copy from file to file, up to TRANSFER_BUF_SIZE bytes at a time.
	 */

	while ((siz = fread(copybuf, 1, TRANSFER_BUF_SIZE, in)) > 0)
	{
		if (fwrite(copybuf, 1, siz, out) != siz)
		{
			fclose(out);
			fclose(in);
			return(0);
		}
	}

	fclose(out);
	fclose(in);
	return(1);
}


/* move file, returns 0 if failed
 * src must be in the current directory (though this isn't checked)
 * and dstdir must be a directory (fails if it isn't)
 */
int movefile(char* src, char* dstdir)
{
	struct stat sbuf;
	//struct utimbuf utbuf;
	char* dst;

	if (!cm_isdir(dstdir)) return(0);

	if ((dst = malloc(strlen(dstdir) + strlen(src) + 2)) == NULL)	/* +2 for / and NUL */
		return(0);

	strcpy(dst, dstdir);
	strcat(dst, "/");
	strcat(dst, src);

	/* fail if dest file already exists */
	if (fstat(dst, &sbuf) != 0)
	{
		free(dst);
		return(0);
	}

	/* first try a rename() */
	if (rename(src, dst) == 0)
	{
		/* it worked */
		free(dst);
		return(1);
	}

	if (errno != EXDEV)
	{
		/* if the reason it failed wasn't because it couldn't `rename'
		 * between filesystems, we should fail too.
		 */
		free(dst);
		return(0);
	}

	/* resort to copying then deleting */

	if (!copyfile(src, dstdir))
	{
		free(dst);
		return(0);
	}

	/* try to copy file times/owner/group/perms.
	 * I don't consider this important enough to give an error if
	 * it screws up (esp. 'cos setting owner might in some cases screw up
	 * quite reasonably if we're not root :-)), but it's worth doing I
	 * think - not least for consistency with the case where rename() works
	 * (where everything is preserved).
	 */
	/*if (stat(src, &sbuf) == 0)
	{
		// set times 
		utbuf.actime = sbuf.st_atime;
		utbuf.modtime = sbuf.st_mtime;
		utime(dst, &utbuf);	/* don't much care if it fails 

		// set owner/group and perms - these don't change the times set above 
		chown(dst, sbuf.st_uid, sbuf.st_gid);
		chmod(dst, sbuf.st_mode);
		//again, not too bothered if they fail
	}*/

	/* the copy worked, delete the original */
	if (unlink(src) == 0)
	{
		/* the delete worked, all done */
		free(dst);
		return(1);
	}

	/* if we couldn't delete the original, delete the copy instead,
	 * because the overall move failed and it would be messy to leave
	 * it there. However, we're not really that bothered if *this*
	 * delete fails. :-)
	 */
	unlink(dst);

	free(dst);
	return(0);
}


