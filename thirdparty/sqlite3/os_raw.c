/*
** 2014 March 20
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file implements an example of a simple VFS implementation that 
** omits some complex features.
**
** OVERVIEW
**
**   The code in this file implements a minimal SQLite VFS that can be 
**   used on RAW-OS operating systems with FatFs. The following 
**   system calls are used:
**
**    File-system: f_unlink(), f_getcwd(), f_stat()
**    File IO:     f_open(), f_read(), f_write(), f_sync(), f_close()
**    Other:       raw_time_sleep()
**
**   The following VFS features are omitted:
**
**     1. File locking. The user must ensure that there is at most one
**        connection to each database when using this VFS. Multiple
**        connections to a single shared-cache count as a single connection
**        for the purposes of the previous statement.
**
**     2. The loading of dynamic extensions (shared libraries).
**
**
*********************************************************************************
*/

#include "sqliteInt.h"

#if SQLITE_OS_RAW

#include "os_common.h"
#include <ff.h>
#include "raw_type.h"
#include "rtc.h"
#include <FileService.h>
#include <systemcall_impl.h>

/*******************************************************************************
* Type definitions, macros, and constants                                      *
*******************************************************************************/
#define MAXPATHNAME  ( 256 )

#ifndef MAXDWORD
#define MAXDWORD     ( 0xFFFFFFFF )   /* 4294967295 */
#endif

/*
** Allowed values for ctrlFlags
*/
#define RAWFILE_RDONLY          0x02   /* Connection is read only */
#define RAWFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#define RAWFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
** The rawFile structure is a subclass of sqlite3_file* specific to the RAW-OS
** portability layer.
*/
typedef struct rawFile rawFile;
struct rawFile {
  const sqlite3_io_methods *pMethod;  /* Base class. Must be first */
  sqlite3_vfs *pVfs;                  /* The VFS that created this rawFile */
  FILE * h;                            /* Pointer to access the file */
  DWORD sectorSize;                   /* Sector size of the device file is on */
  const char *zPath;                  /* Name of the file */
  unsigned short int ctrlFlags;       /* Behavioral bits.  RAWFILE_* flags */
  int szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
};

/*******************************************************************************
* External Functions                                                           *
* These functions must be defined outside of the library by the application.   *
*******************************************************************************/
extern RAW_U16 raw_time_sleep(RAW_U16 hours, RAW_U16 minutes, RAW_U16 seconds, RAW_U32 milli);

/*******************************************************************************
* Static Function Prototypes                                                   *
*******************************************************************************/
static int rawClose(sqlite3_file *id);
static int rawRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset);
static int rawWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset);
static int rawTruncate(sqlite3_file *id, sqlite3_int64 nByte);
static int rawSync(sqlite3_file *id, int flags);
static int rawFileSize(sqlite3_file *id, sqlite3_int64 *pSize);
static int rawLock(sqlite3_file *id, int eLock);
static int rawUnlock(sqlite3_file *id, int eLock);
static int rawCheckReservedLock(sqlite3_file *id, int *pResOut);
static int rawFileControl(sqlite3_file *id, int op, void *pArg);
static int rawSectorSize(sqlite3_file *id);
static int rawDeviceCharacteristics(sqlite3_file *id);

/*******************************************************************************
* Static Data                                                                  *
*******************************************************************************/
/*
** This vector defines all the methods that can operate on an
** sqlite3_file for RAW-OS.
*/
static const sqlite3_io_methods rawIoMethod = {
  1,                        /* iVersion */
  rawClose,                 /* xClose */
  rawRead,                  /* xRead */
  rawWrite,                 /* xWrite */
  rawTruncate,              /* xTruncate */
  rawSync,                  /* xSync */
  rawFileSize,              /* xFileSize */
  rawLock,                  /* xLock */
  rawUnlock,                /* xUnlock */
  rawCheckReservedLock,     /* xCheckReservedLock */
  rawFileControl,           /* xFileControl */
  rawSectorSize,            /* xSectorSize */
  rawDeviceCharacteristics  /* xDeviceCharacteristics */
};


/*******************************************************************************
* Local Functions                                                              *
*******************************************************************************/
#if defined(SQLITE_DEBUG_OS_TRACE)
static const char * fatfsErrName(FRESULT rc)
{
  static char const * name[] = {
    "OK",
    "DISK_ERR",
    "INT_ERR",
    "NOT_READY",
    "NO_FILE",
    "NO_PATH",
    "INVALID_NAME",
    "DENIED",
    "EXIST",
    "INVALID_OBJECT",
    "WRITE_PROTECTED",
    "INVALID_DRIVE",
    "NOT_ENABLED",
    "NO_FILE_SYSTEM",
    "MKFS_ABORTED",
    "TIMEOUT",
    "LOCKED",
    "NOT_ENOUGH_CORE",
    "TOO_MANY_OPEN_FILES"
  };

  if(rc >= (sizeof(name)/sizeof(name[0])))
  {
    return NULL;
  }

  return name[rc];
}
#endif

/*
** Create a temporary file name in zBuf.  zBuf must be big enough to
** hold at pVfs->mxPathname characters.
*/
static void osGetTempPath(int len, char *path)
{
    if (len > 2)
    {
        strcpy(path, "0:");
    }
}

static int osGetTempname(int nBuf, char *zBuf){
  static char zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  char zTempPath[MAXPATHNAME+1];
  if( sqlite3_temp_directory ){
    sqlite3_snprintf(MAXPATHNAME-30, zTempPath, "%s", sqlite3_temp_directory);
  }else {
    char *zMulti;
    char zPath[MAXPATHNAME];
    osGetTempPath(MAXPATHNAME-30, zPath);
    zMulti = zPath;
    if( zMulti ){
      sqlite3_snprintf(MAXPATHNAME-30, zTempPath, "%s", zMulti);
    }else{
      return SQLITE_NOMEM;
    }
  }
  for(i=sqlite3Strlen30(zTempPath); i>0 && zTempPath[i-1]=='/'; i--){}
  zTempPath[i] = 0;
  sqlite3_snprintf(nBuf-30, zBuf,
                   "%s/"SQLITE_TEMP_FILE_PREFIX, zTempPath);
  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(20, &zBuf[j]);
  for(i=0; i<20; i++, j++){
    zBuf[j] = (char)zChars[ ((unsigned char)zBuf[j])%(sizeof(zChars)-1) ];
  }
  zBuf[j] = 0;
  OSTRACE(("TEMP FILENAME: %s\n", zBuf));
  return SQLITE_OK; 
}

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Close a file.
*/
static int rawClose(sqlite3_file *id){
  
  rawFile *pFile = (rawFile*)id;

  assert( id!=0 );
  assert( pFile->h!=NULL );
  OSTRACE(("CLOSE file=%p\n", pFile->h));

  fclose(pFile->h);
  pFile->h = NULL;

  OpenCounter(-1);
  OSTRACE(("CLOSE file=%p, name=%s, rc=%s\n", pFile->h, pFile->zPath, fatfsErrName(rc)));
  return SQLITE_OK;
  //return (FR_OK == rc) ? SQLITE_OK : SQLITE_IOERR_CLOSE;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int rawRead(
  sqlite3_file *id,          /* File to read from */
  void *pBuf,                /* Write content into this buffer */
  int amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
  FRESULT rc;
  rawFile *pFile = (rawFile*)id;
  UINT got;

  assert( id!=0 );
  assert( amt>0 );
  assert( offset>=0 && offset <= MAXDWORD);
  SimulateIOError(return SQLITE_IOERR_READ);
  OSTRACE(("READ file=%p, buffer=%p, amount=%d, offset=%lld\n", pFile->h, pBuf, amt, offset));
  
  /* seek pos */
  rc = fseek(pFile->h, (DWORD)offset, SEEK_SET);
  if( rc != 0 ){
    OSTRACE(("READ file=%p, rc=SQLITE_FULL\n", pFile->h));
    return SQLITE_FULL;
  }
  
  /* do read file */
  got = fread(pBuf, 1, amt, pFile->h);
  
  if( got<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((char*)pBuf)[got], 0, amt-got);
    OSTRACE(("READ file=%p, rc=SQLITE_IOERR_SHORT_READ\n", pFile->h));
    return SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ file=%p, rc=SQLITE_OK\n", pFile->h));
  return SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int rawWrite(
  sqlite3_file *id,         /* File to write into */
  const void *pBuf,         /* The bytes to be written */
  int amt,                  /* Number of bytes to write */
  sqlite3_int64 offset      /* Offset into the file to begin writing at */
){
  FRESULT rc;
  rawFile *pFile = (rawFile*)id;
  UINT wrote = 0;

  assert( amt>0 );
  assert( pFile );
  assert( offset>=0 && offset <= MAXDWORD);
  SimulateIOError(return SQLITE_IOERR_WRITE);
  SimulateDiskfullError(return SQLITE_FULL);

  OSTRACE(("WRITE file=%p, buffer=%p, amount=%d, offset=%lld\n", pFile->h, pBuf, amt, offset));

  /* seek pos */
  rc = fseek(pFile->h, (DWORD)offset, SEEK_SET);
  if( rc != 0 ){
    OSTRACE(("WRITE file=%p, rc=SQLITE_FULL\n", pFile->h));
    return SQLITE_FULL;
  }
  
  /* do write file */
  wrote = fwrite(pBuf, 1, amt, pFile->h);
  
  if( (amt > (int)wrote) ){
    OSTRACE(("WRITE file=%p, rc=SQLITE_FULL\n", pFile->h));
    return SQLITE_FULL;
  }

  OSTRACE(("WRITE file=%p, rc=SQLITE_OK\n", pFile->h));
  return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int rawTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  rawFile *pFile = (rawFile*)id;
  FRESULT rc;

  assert( pFile );
  assert( nByte>=0 && nByte<=MAXDWORD);
  SimulateIOError(return SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE file=%p, size=%lld\n", pFile->h, nByte));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  if( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = fseek(pFile->h, (DWORD)nByte, SEEK_SET);
  if( rc != 0 ){
    return SQLITE_IOERR_TRUNCATE;
  }

  rc = ftruncate(fileno(pFile->h), ftell(pFile->h));
  if( rc != 0 ){
    return SQLITE_IOERR_TRUNCATE;
  }
  
  OSTRACE(("TRUNCATE file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
  return SQLITE_OK;
}

/*
** Make sure all writes to a particular file are committed to disk.
*/
static int rawSync(sqlite3_file *id, int flags){
  FRESULT rc;
  rawFile *pFile = (rawFile*)id;

  assert( id!=0 );
  UNUSED_PARAMETER(flags);
  OSTRACE(("SYNC %d\n", pFile->h));

  rc = fsync(pFile->h);
  if(rc == FR_OK){
    OSTRACE(("SYNC %d SQLITE_OK\n", pFile->h));
    return SQLITE_OK;
  }else{
    OSTRACE(("SYNC %d rc=%s\n", pFile->h, fatfsErrName(rc)));
    return SQLITE_IOERR;
  }
}

/*
** Determine the current size of a file in bytes
*/
static int rawFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
  rawFile *pFile = (rawFile*)id;

  assert( id!=0 );
  assert( pSize!=0 );
  SimulateIOError(return SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, pSize));

  /* get file size */  
  
  
  fseek(pFile->h, 0, SEEK_END);
  int fileSize = ftell(pFile->h);
  fseek(pFile->h, 0, SEEK_SET); //go back to where we were
  *pSize = fileSize;
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld\n",
           pFile->h, pSize, *pSize));
  
  return SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
static int rawLock(sqlite3_file *id, int eLock){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(eLock);
  return SQLITE_OK;
}
static int rawUnlock(sqlite3_file *id, int eLock){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(eLock);
  return SQLITE_OK;
}
static int rawCheckReservedLock(sqlite3_file *id, int *pResOut){
  UNUSED_PARAMETER(id);
  *pResOut = 0;
  return SQLITE_OK;
}

/*
** If *pArg is inititially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
static void rawModeBit(rawFile *pFile, unsigned char mask, int *pArg){
  if( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }else if( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }else{
    pFile->ctrlFlags |= mask;
  }
}

/*
** Control and query of the open file handle.
*/
static int rawFileControl(sqlite3_file *id, int op, void *pArg){
  rawFile *pFile = (rawFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  switch( op ){
    case SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(int *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SIZE_HINT: {
      if( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        int rc = rawFileSize(id, &oldSz);
        if( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          if( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = rawTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        return rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_PERSIST_WAL: {
      rawModeBit(pFile, RAWFILE_PERSIST_WAL, (int*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      rawModeBit(pFile, RAWFILE_PSOW, (int*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_VFSNAME: {
      *(char**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_TEMPFILENAME: {
      char *zTFile = sqlite3_malloc( pFile->pVfs->mxPathname );
      if( zTFile ){
        osGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(char**)pArg = zTFile;
      }
      return SQLITE_OK;
    }
  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  return SQLITE_NOTFOUND;
}

/*
** Return the sector size in bytes of the underlying block device for
** the specified file. This is almost always 512 bytes, but may be
** larger for some devices.
**
** SQLite code assumes this function cannot fail. It also assumes that
** if two files are created in the same file-system directory (i.e.
** a database and its journal file) that the sector size will be the
** same for both.
*/
static int rawSectorSize(sqlite3_file *id){
  assert( id!=0 );
  return (int)(((rawFile*)id)->sectorSize);
}

/*
** Return a vector of device characteristics.
*/
static int rawDeviceCharacteristics(sqlite3_file *id){
  rawFile *p = (rawFile*)id;
  assert( id!=0 );
  return SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN |
         ((p->ctrlFlags & RAWFILE_PSOW) ? SQLITE_IOCAP_POWERSAFE_OVERWRITE : 0);
}

/***************************************************************************
** Here ends the I/O methods that form the sqlite3_io_methods object.
**
** The next block of code implements the VFS methods.
****************************************************************************/

/*
** Open a file.
*/
static int rawOpen(
  sqlite3_vfs *pVfs,        /* The VFS for which this is the xOpen method */
  const char *zName,        /* Pathname of file to be opened (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  int flags,                /* Open mode flags */
  int *pOutFlags            /* Status return flags */
){
  rawFile *pFile = (rawFile *)id;
  FILE *fp = 0;
  char fopen_flags[4] = { 0 ,};
  int openFlags = 0;             /* Flags to pass to open() */
  int eType = flags&0xFFFFFF00;  /* Type of file to open */
  int rc = SQLITE_OK;            /* Function Return Code */

  int isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  int isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  int isCreate     = (flags & SQLITE_OPEN_CREATE);
  int isReadonly   = (flags & SQLITE_OPEN_READONLY);
  int isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

  /* If creating a master or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time rawSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  int syncDir = (isCreate && (
        eType==SQLITE_OPEN_MASTER_JOURNAL 
     || eType==SQLITE_OPEN_MAIN_JOURNAL 
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  char zTmpname[MAXPATHNAME+2];

  /* Check the following statements are true: 
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and 
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  assert((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  assert(isCreate==0 || isReadWrite);
  assert(isExclusive==0 || isCreate);
  assert(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and master journal are never 
  ** automatically deleted. Nor are they ever temporary files.  */
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MASTER_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  assert( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB 
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL 
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_MASTER_JOURNAL 
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );


  if( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    assert(isDelete && !syncDir);
    rc = osGetTempname(MAXPATHNAME+2, zTmpname);
    if( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR\n", zName));
      return SQLITE_IOERR;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    assert( zName[strlen(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the 
  ** 'conch file' locking functions later on.  */
  if( isReadWrite ) 
  {
    openFlags = FA_READ | FA_WRITE;
	strcpy(fopen_flags, "rw");
  } else {
    openFlags = FA_READ;
	strcpy(fopen_flags, "r");
  }

  if( isExclusive ) 
  {
    openFlags |= FA_CREATE_NEW;
	strcat(fopen_flags, "+");
  } 
  else if( isCreate ) 
  {
    openFlags |= FA_OPEN_ALWAYS;
	strcat(fopen_flags, "+");
  } 
  else 
  {
    openFlags |= FA_OPEN_EXISTING;
  }

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zName, id, flags, pOutFlags));
  fp = fopen((TCHAR *)zName, fopen_flags);
  if(fp == 0)
  {
    OSTRACE(("OPEN name=%s, rc=%s\n", zName, fatfsErrName(rc)));
    return SQLITE_CANTOPEN;
  }

  if( pOutFlags ){
    if( flags & SQLITE_OPEN_READWRITE ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }else{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }
  
  OSTRACE(("OPEN name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d\n",
           zName, openFlags, pOutFlags, pOutFlags ? *pOutFlags : 0));

  memset(pFile, 0, sizeof(rawFile));
  pFile->pMethod = &rawIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = fp;
  //20191008
  pFile->sectorSize = 512;
  pFile->zPath = zName;
  if( isReadonly ){
    pFile->ctrlFlags |= RAWFILE_RDONLY;
  }
  if( sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pFile->ctrlFlags |= RAWFILE_PSOW;
  }
  
  OpenCounter(+1);

  return SQLITE_OK;
}

/*
** Delete the named file.
**
*/
static int rawDelete(
  sqlite3_vfs *pVfs,          /* Not used */
  const char *zFilename,      /* Name of file to delete (UTF-8) */
  int syncDir                 /* Not used */
){
  FRESULT rc;

  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(return SQLITE_IOERR_DELETE);

  // Delete file
  rc = unlink((TCHAR *)zFilename);
  if(0 == rc) {
    OSTRACE(("DELETE name=%s, SQLITE_OK\n", zFilename));
    return SQLITE_OK;
  }else if(FR_NO_FILE == rc || FR_NO_PATH == rc) {
    OSTRACE(("DELETE name=%s, SQLITE_IOERR_DELETE_NOENT\n", zFilename));
    return SQLITE_IOERR_DELETE_NOENT;
  }else{
    OSTRACE(("DELETE name=%s, SQLITE_IOERR_DELETE\n", zFilename));
    return SQLITE_IOERR_DELETE;
  }
}

/*
** Test the existence of or access permissions of file zPath. The
** test performed depends on the value of flags:
**
**     SQLITE_ACCESS_EXISTS: Return 1 if the file exists
**     SQLITE_ACCESS_READWRITE: Return 1 if the file is read and writable.
**     SQLITE_ACCESS_READONLY: Return 1 if the file is readable.
**
** Otherwise return 0.
*/
#include <stat_def.h>
static int rawAccess(
  sqlite3_vfs *pVfs,         /* Not used on */
  const char *zFilename,     /* Name of file to check (UTF-8) */
  int flags,                 /* Type of test to make on this file */
  int *pResOut               /* OUT: Result */
){
  FRESULT rc;
  struct stat fi;
  UNUSED_PARAMETER(pVfs);

  SimulateIOError( return SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n", zFilename, flags, pResOut));

  assert( pResOut );

  rc = stat((TCHAR *)zFilename, &fi);

  switch( flags ){
    case SQLITE_ACCESS_READ:
    case SQLITE_ACCESS_EXISTS:
    case SQLITE_ACCESS_READWRITE:
      *pResOut = ((FR_OK == rc) ? 1 : 0);
      break;
    default:
      assert(!"Invalid flags argument");
  }

  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  return SQLITE_OK;
}

static int osGetcwd(char *zOut, int nOut)
{
  FRESULT rc;
  assert( zOut );

  //rc = getcwd((TCHAR *)zOut, nOut);

  rc = Syscall_GetCurrentDirectory(nOut, zOut);
  if(0 != rc)
  {
    return 0;
  }

  return 1;
}


/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
static int rawFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zPath,            /* Possibly relative input path */
  int nOut,                     /* Size of output buffer in bytes */
  char *zOut                    /* Output nbuffer */
){
  SimulateIOError( return SQLITE_ERROR );

  assert( pVfs->mxPathname==MAXPATHNAME);
  UNUSED_PARAMETER(pVfs);

  zOut[nOut-1] = '\0';
  if( zPath[1]==':' ){
    sqlite3_snprintf(nOut, zOut, "%s", zPath);
  }else{
    int nCwd;
    if( osGetcwd(zOut, nOut-1)!=0 ){
      return SQLITE_CANTOPEN_BKPT;
    }
    nCwd = (int)strlen(zOut);
    sqlite3_snprintf(nOut-nCwd, &zOut[nCwd], "/%s", zPath);
  }
  return SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
static void *rawDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  return 0;
}

static void rawDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
  zErrMsg[nByte-1] = '\0';
}

static void (*rawDlSym(sqlite3_vfs *pVfs, void *pH, const char *z))(void){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(pH);
  UNUSED_PARAMETER(z);
  return 0;
}

static void rawDlClose(sqlite3_vfs *pVfs, void *pHandle){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(pHandle);
  return;
}


/*
** Write up to nBuf bytes of randomness into zBuf.
*/
static int rawRandomness(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
  UNUSED_PARAMETER(pVfs);

  memset(zBuf, 0, nBuf);
  return nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
static int rawSleep(sqlite3_vfs *pVfs, int microsec){
  int milliseconds = (microsec + 999) / 1000;
  UNUSED_PARAMETER(pVfs);

 // raw_time_sleep(0, 0, 0, milliseconds);
  Syscall_Sleep(milliseconds);
  return milliseconds * 1000;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static int rawCurrentTime(sqlite3_vfs *pVfs, double *prNow){
  static const sqlite3_int64 epoch = 24405875*(sqlite3_int64)8640000;
  sqlite3_int64 t;
  CLK_DATE_TIME dt;
  CLK_TS_SEC ts;

  assert(prNow);
  UNUSED_PARAMETER(pVfs);

  //20191020

  return 1;

  /*if(Clk_GetDateTime(&dt) == DEF_NO)
  {
      return 1;
  }

  if(Clk_DateTimeToTS(&ts, &dt) == DEF_NO)
  {
      return 1;
  }

  t = epoch + 1000*(sqlite3_int64)ts;
  *prNow = (double)(t/86400000.0);

  return 0;*/
}

/*
** The idea is that this function works like a combination of
** GetLastError() and FormatMessage() on windows (or errno and
** strerror_r() on unix). After an error is returned by an OS
** function, SQLite calls this function with zBuf pointing to
** a buffer of nBuf bytes. The OS layer should populate the
** buffer with a nul-terminated UTF-8 encoded error message
** describing the last IO error to have occurred within the calling
** thread.
**
** If the error message is too large for the supplied buffer,
** it should be truncated. The return value of xGetLastError
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated). If non-zero is returned,
** then it is not necessary to include the nul-terminator character
** in the output buffer.
**
** Not supplying an error message will have no adverse effect
** on SQLite. It is fine to have an implementation that never
** returns an error message:
**
**   int xGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
**     assert(zBuf[0]=='\0');
**     return 0;
**   }
**
** However if an error message is supplied, it will be incorporated
** by sqlite into the error message available to the user using
** sqlite3_errmsg(), possibly making IO errors easier to debug.
*/
static int rawGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(nBuf);
  UNUSED_PARAMETER(zBuf);
  return 0;
}

/*******************************************************************************
* Global Functions                                                             *
*******************************************************************************/
/*
** Initialize and deinitialize the operating system interface.
*/
int sqlite3_os_init(void){
  
  static sqlite3_vfs rawVfs = {
    1,                 /* iVersion */
    sizeof(rawFile),   /* szOsFile */
    MAXPATHNAME,       /* mxPathname */
    0,                 /* pNext */
    "raw",             /* zName */
    0,                 /* pAppData */
 
    rawOpen,           /* xOpen */
    rawDelete,         /* xDelete */
    rawAccess,         /* xAccess */
    rawFullPathname,   /* xFullPathname */
    rawDlOpen,         /* xDlOpen */
    rawDlError,        /* xDlError */
    rawDlSym,          /* xDlSym */
    rawDlClose,        /* xDlClose */
    rawRandomness,     /* xRandomness */
    rawSleep,          /* xSleep */
    rawCurrentTime,    /* xCurrentTime */
    rawGetLastError,   /* xGetLastError */
  };

  sqlite3_vfs_register(&rawVfs, 1);
  return SQLITE_OK; 
}

int sqlite3_os_end(void){ 
  return SQLITE_OK;
}

#endif /* SQLITE_OS_RAW */
