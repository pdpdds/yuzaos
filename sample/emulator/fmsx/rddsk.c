/*************************************************************/
/**                                                         **/
/**                rddsk.c                                  **/
/**                                                         **/
/** Program to read files from disk-images that can be      **/
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
#include <utime.h>
//#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DiskUtil.h"

/* buffers */
byte *FAT;
char clusbuf[cluslen];
byte BootBlock[seclen];

/* general */
int diskim;    /* file descriptor of diskimage */
char *progname;/* name of program (for PError) */ 

/* drive parameters */
usint maxcl; /* highest clusternumber */
off_t  dir_ofs,  /* offset of first byte of directory */
       data_ofs; /* offset of first byte of data  */ 

/* open a diskimage file for reading */
int OpenDisk(char *DiskName)
{
  int res;

  if ((res=_open(DiskName,O_BINARY|O_RDONLY))==-1)
    {
      PError(DiskName);
      exit(2);
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
  maxcl=(BootBlock[0x13]+(BootBlock[0x14]<<8)-data_ofs/seclen)
    /(cluslen/seclen) +1;

  printf("Diskimage: %s\n",DiskName);
  return res;
}      

/* close diskimage file */
void CloseDisk(int fd,char *DiskName)
{
  if (_close(fd))
    PError(DiskName);
}

/* copy  a file from the diskimage */
void ReadFile(DirEntry msxf)
{
  unsigned long size;
  int i,j,fd;
  usint curcl;
  char r, name[13];

  for(i=j=0; i<8 && msxf.d_fname[i]!=' ';) 
    name[j++]=tolower(msxf.d_fname[i++]);
  name[j++]='.';
  for(i=0; i<3 && msxf.d_ext[i]!=' ';) 
    name[j++]=tolower(msxf.d_ext[i++]);
  if (name[j-1]=='.') j--;
  name[j]='\0';

  if ( (fd=_open(name,O_RDONLY)) != -1)
    {
      _close(fd);
      printf("%s: replace `%s'? ", progname, name);
      do  
	r=getchar();
      while ( !strchr("YyNn",r) );
      if (toupper(r)=='N')
	return;
    }

  printf("%.8s.%.3s ==> %s\n",msxf.d_fname,msxf.d_ext,name);
  if ( (fd=_open(name,O_BINARY|O_CREAT|O_WRONLY|O_TRUNC,0666))==-1)
    {
      PError(name);
      return;
    }

  size=rdlg(msxf.d_size);
  curcl=rdsh(msxf.d_first);

  while(size && (curcl>=2) && (curcl<=maxcl) )
    {
      	lseek(diskim, data_ofs+(curcl-2)*cluslen, SEEK_SET);
	_read(diskim, clusbuf, cluslen);
        _write(fd,clusbuf, (size>cluslen)?cluslen:size);
	size-=(size>cluslen)?cluslen:size;
	curcl=ReadFAT(curcl);
    }
  if (size)
    printf("%s: diskimage corrupt, %s truncated.\n", progname, name);

  if (_close(fd)) 
    PError(name);

  /* use msx-dos time-stamp to set modification time */
  {
    struct utimbuf tbuf;
    struct tm mtim;
    usint t;
    time_t tim;

    tim=time(NULL);
    mtim=*localtime(&tim);/* get timezone info */
    mtim.tm_isdst=-1;     /* check for daylight savings time */
    
    t=rdsh(msxf.d_time);
    mtim.tm_sec =(t&0x001F)<<1;
    mtim.tm_min =(t&0x07E0)>>5;
    mtim.tm_hour=(t&0xF100)>>11;
    t=rdsh(msxf.d_date);
    mtim.tm_mday= (t&0x001F);
    mtim.tm_mon =((t&0x01E0)>>5)-1;
    mtim.tm_year=((t&0xFE00)>>9)+1980-1900;
    
    tbuf.actime =time(NULL);
    tbuf.modtime=mktime(&mtim);
    utime(name, &tbuf);
  }
}

/* Give some help */
void help(void)
{
  puts("Extract files from a diskimage used with fmsx.");
  printf("Usage: %s <diskimage> [-d <directory>] [<fspec1>] [<spec2>] ...\n",
	 progname);
  puts("Use -d <directory> to speify destintion directory.");
  puts("If <fspec>'s  are given only the files matching at least one ");
  puts("  of the <fspec>'s are extracted, following MSX-DOS-rules.");
  puts("Otherwise all files are extracted");  
}

/* process the command line */
int main (int argc, char* argv[])
{ 
  DirEntry de;
  int n, j,k,l;
  off_t i;
  char *fl = 0, *p = 0;

  if (sizeof(DirEntry)!=32)
    { puts("please fix struct de and compile again"); exit(2); }
  puts("rddsk version 1.0 by Arnold Metselaar, (c) 1996");

  progname=argv[0];
  if (argc<2) 
    { help(); exit(1); }

  diskim=OpenDisk(argv[1]);
  n=2;
  if ( (argc>2) && !strcmp(argv[2],"-d"))
    {
      n=4;
      if (argc>3)
	{
	  if (chdir(argv[3]))
	    { PError(argv[3]); exit(2); }
	  else
	    printf("Directory: %s\n",argv[3]);
	}
      else 
	printf("%s: No directory supplied", progname);
    }
  
  if (argc>n)
    {
      fl=xalloc((argc-n)*11);
      for (j=n; j<argc; j++)
	{
	  p=fl+11*(j-n);
	  for(k=l=0; k<11; l++)
	    {
	      switch (argv[j][l])
		{
		case '.' : while (k<8) p[k++]=' '; break;
		case '*' : do p[k++]='?'; while ((k!=8) && (k!=11)); break;
		case '\0': while (k<11) p[k++]=' '; break;
		default  : p[k++]=toupper(argv[j][l]);
		};
	    }
	}
    }

  for(i=dir_ofs; i<data_ofs; i+=sizeof(DirEntry))
    {
      lseek(diskim,i,SEEK_SET);
      _read(diskim, &de, sizeof(DirEntry));
      if ((de.d_fname[0]!='\345') && (de.d_fname[0]!='\0'))
	{
	  for (j=k=0; j<argc-n && k<11 ;j++)
	    {
	      p=fl+11*j;
	      for (k=0 ; (k<11) && ( de.d_fname[k]==p[k] || p[k]=='?' ); k++)
		;
	    }
	  if (k==11 || n==argc) 
	    ReadFile(de);
	}
    }
  CloseDisk(diskim, argv[1]);
  return(0);
}






