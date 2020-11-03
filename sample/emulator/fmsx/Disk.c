/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                          Disk.c                         **/
/**                                                         **/
/** This file contains standard disk access drivers working **/
/** with disk images from files.                            **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef DISK

#include "MSX.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <fcntl.h>
//#include <direct.h>
//#include "unistd.h"
#ifdef __BORLANDC__
#include <io.h>
#endif

#ifdef UNIX
#include <unistd.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int Drives[2] = { -1,-1 };        /* Disk image files */
static int RdOnly[2];                    /* 1 = read-only    */

										 /** DiskPresent() ********************************************/
										 /** Return 1 if disk drive with a given ID is present.      **/
										 /*************************************************************/
byte DiskPresent(byte ID)
{
	return((ID<MAXDRIVES) && (Drives[ID] >= 0));
}

/** DiskRead() ***********************************************/
/** Read requested sector from the drive into a buffer.     **/
/*************************************************************/
byte DiskRead(byte ID, byte *Buf, int N)
{
	if ((ID<MAXDRIVES) && (Drives[ID] >= 0))
		if (fseek((FILE*)Drives[ID], N * 512L, 0) == N * 512L)
			return(fread(Buf, 512, 1, (FILE*)Drives[ID]) == 512);
	return(0);
}

/** DiskWrite() **********************************************/
/** Write contents of the buffer into a given sector of the **/
/** disk.                                                   **/
/*************************************************************/
byte DiskWrite(byte ID, byte *Buf, int N)
{
	if ((ID<MAXDRIVES) && (Drives[ID] >= 0) && !RdOnly[ID])
		if (fseek((FILE*)Drives[ID], N * 512L, 0) == N * 512L)
			return(fwrite(Buf, 512, 1, (FILE*)Drives[ID]) == 512);
	return(0);
}

/** ChangeDisk() *********************************************/
/** Change disk image in a given drive. Closes current disk **/
/** image if Name=0 was given. Returns 1 on success or 0 on **/
/** failure.                                                **/
/*************************************************************/
byte ChangeDisk(byte ID, char *Name)
{
	/* We only have MAXDRIVES drives */
	if (ID >= MAXDRIVES) return(0);
	/* Close previous disk image */
	if (Drives[ID] >= 0) { fclose((FILE*)Drives[ID]); Drives[ID] = -1; }
	/* If no disk image given, consider drive empty */
	if (!Name) return(1);
	/* Open new disk image */
	Drives[ID] = (int)fopen(Name, "rb");
	RdOnly[ID] = 0;
	/* If failed to open for writing, open read-only */
	if (Drives[ID]<0)
	{
		Drives[ID] = (int)fopen(Name, "rb");
		RdOnly[ID] = 1;
	}
	/* Return operation result */
	return(Drives[ID] >= 0);
}

#endif /* DISK */
