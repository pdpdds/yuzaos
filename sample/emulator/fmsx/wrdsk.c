/*************************************************************/
/**                                                         **/
/**                wrdsk.c                                  **/
/**                                                         **/
/** Program to write files to disk-images that can be       **/
/**  used with fMSX                                         **/
/**                                                         **/
/**                                                         **/
/** Copyright (c) Arnold Metselaar 1996                     **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
//#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DiskUtil.h"
#include "Boot.h" /* Contains byte BootBlock[] */

/* drive parameters */
usint maxcl; /* highest clusternumber */
off_t  dir_ofs,  /* offset of first byte of directory */
       data_ofs; /* offset of first byte of data  */ 

/* buffers */
byte *FAT;
char clusbuf[cluslen];

/* general */
int diskim;    /* file descriptor of diskimage */
char *progname;/* name of program (for PError) */ 

/* copy a file to the diskimage and 
   return a directory entry for this file */ 
DirEntry WriteFile(char *name)
{ DirEntry res;
  int file;
  char *p;
  int i;
  long size;
  long bsiz;
  usint curcl,prevcl;

  if ((file=_open(name,O_RDONLY,0))==-1)
    {
      memset(&res,0,sizeof(DirEntry));
      PError(name);
    } 
  else
    {  
      if ( !(p=strrchr(name,(int) '/')) ) 
	p=name;
      else 
	p++;
      for( ; *p=='.'; p++);
      for(i=0; (i<8) && p[i] && p[i]!='.' ; i++) 
	res.d_fname[i]=toupper(p[i]);
      while (i<8) res.d_fname[i++]=' ';
      i=0;
      if ( (p=strrchr(name,'.')) )
	for(p++, i=0; (i<3) && p[i] ; i++) 
	  res.d_ext[i]=toupper(p[i]);
      while (i<3) res.d_ext[i++]=' ';
      res.d_attrib=0;

      /* compute time/date stamps */
      {
	struct stat fst;
	struct tm mtim;
	usint t;
	
	fstat(file, &fst);
	mtim = *localtime(&(fst.st_mtime));
	t=(mtim.tm_sec>>1)+(mtim.tm_min<<5)+(mtim.tm_hour<<11);
	setsh(res.d_time,t);
	t=mtim.tm_mday+((mtim.tm_mon+1)<<5)+((mtim.tm_year+1900-1980)<<9);
	setsh(res.d_date,t);
      }

      for(curcl=2; (curcl <= maxcl) && ReadFAT(curcl); curcl++);
      setsh(res.d_first,curcl);
      printf("%s ==> %.8s.%.3s\n", name, res.d_fname, res.d_ext);

      size=0; prevcl=0; 
      while( (bsiz=_read(file, clusbuf, cluslen)) && (curcl<= maxcl) )
	{
	  size+=bsiz;
	  lseek(diskim, data_ofs+(curcl-2)*cluslen, SEEK_SET);
	  _write(diskim, clusbuf, bsiz);
	  if (prevcl) WriteFAT(prevcl,curcl);
	  for(prevcl=curcl++; (curcl <= maxcl) && ReadFAT(curcl); curcl++);
	}
      if (size) WriteFAT(prevcl,EOF_FAT);
      if (bsiz)
	printf("%s: Diskimage full, %s truncated.\n",progname,name);
      setlg(res.d_size,size);
      if (_close(file))
	PError(name);
    }
  return res;
}

/* open or create a diskimage */
int OpenDisk(char * DiskName)
{ 
  int res;
  off_t pos;

  printf("Diskimage: %s ",DiskName);
  if ((res=_open(DiskName,O_BINARY|O_RDWR))==-1)
    {
      if ((res=_open(DiskName,O_BINARY|O_CREAT|O_RDWR,0666))==-1)
	{ 
	  PError(DiskName); exit(2);
	}
      lseek(res,0,SEEK_SET);
      _write(res,BootBlock,seclen);

      FAT=xalloc(seclen*BootBlock[0x16]);
      memset(FAT,0,seclen*BootBlock[0x16]);
      WriteFAT(0,BootBlock[0x15]+0xF00);
      WriteFAT(1,0xFFF);
    }
  else 
    {
      lseek(res,0,SEEK_SET);
      _read(res,BootBlock,seclen);
      if ( BootBlock[0x0B]+(BootBlock[0x0C]<<8)!=seclen 
      || BootBlock[0x0D]!=cluslen/seclen )
	{
	  printf("%s: %s does not seem to be a valid (MSX-DOS) diskimage\n",
		 progname, DiskName); 
	  exit(2);
	}
      FAT=xalloc(seclen*BootBlock[0x16]);
      _read(res,FAT,seclen*BootBlock[0x16]);
    }
  dir_ofs=(1+BootBlock[0x10]*BootBlock[0x16])*seclen;
  data_ofs=dir_ofs+(BootBlock[0x11]+(BootBlock[0x12]<<8))*sizeof(DirEntry);
  maxcl=((BootBlock[0x13]+(BootBlock[0x14]<<8))*seclen-data_ofs)/cluslen +1;
 
  if (lseek(res,0,SEEK_END)==512)
  {
    puts("(new)");
    memset(clusbuf,0,seclen);
    lseek(res,dir_ofs,SEEK_SET);
    for(pos=dir_ofs; pos<data_ofs; pos+=seclen) 
      _write(res,clusbuf,seclen); /* clear directory */
    lseek(res,data_ofs+(maxcl-1)*cluslen-seclen, SEEK_SET);
    _write(res,clusbuf,seclen); /* set file size */
  }
  else 
    puts("(add)");
  return res;
}      

/* close a diskimage */
void CloseDisk(int img, char * DiskName)
{
  int i;

  lseek(img,seclen,SEEK_SET);
  for( i=0; i<BootBlock[0x10]; i++)
    _write(img,FAT,seclen*BootBlock[0x16]);
  if (_close(img))
    PError(DiskName);
}

/* check whether dp points to a directory entry that is in use */
int Used(off_t dp)
{ 
  char c;

  lseek(diskim,dp,SEEK_SET);
  _read(diskim,&c,1);
  return ( (c!=(char) 0xE5) && (c!='\0') );
}

/* Give some help */
void help(void)
{
  puts("Store files in a diskimage for use with fmsx.");
  printf("Usage: %s <diskimage> <file1> [<file2>] ...\n",progname);
  puts("If <diskimage> does not exist wrdsk will automaticly create it.");
  puts("Wildcards can be used on systems that expand these."); 
}

/* process the command line */
int main (int argc, char *argv[])
{
  int i;
  off_t dirpointer;
  DirEntry dirbuf;

  if (sizeof(DirEntry)!=32)
    { puts("please fix struct de and compile again"); exit(2); }
  puts("wrdsk version 1.0 by Arnold Metselaar, (c) 1996");

  progname=argv[0];
  
  if (argc<3) 
    { help(); exit(1); }
  
  diskim=OpenDisk(argv[1]);
  
  dirpointer=dir_ofs;
  for (i=2; i<argc; i++)
    {
      while( Used(dirpointer) && (dirpointer < data_ofs) ) 
	dirpointer+=sizeof(DirEntry);
      if (dirpointer<data_ofs)
	{
	  dirbuf=WriteFile(argv[i]);
	  if (dirbuf.d_fname[0])
	    {
	      lseek(diskim,dirpointer,SEEK_SET);
	      _write(diskim,(char *)&dirbuf,sizeof(DirEntry));
	    }
	}
      else
	printf("%s: Directory full, %s not written.\n",
		progname,argv[i]);
    }
  CloseDisk(diskim,argv[1]);
  return(0);
}
