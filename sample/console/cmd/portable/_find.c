#include "cmd.h"

#include <dirent.h>
#include <win32-glob.h>

#define ERROR printf
#define TRACE printf

#define	GLOB_APPEND	0x0001	/* Append to output from previous call. */
#define	GLOB_DOOFFS	0x0002	/* Use gl_offs. */
#define	GLOB_ERR	0x0004	/* Return on error. */
#define	GLOB_MARK	0x0008	/* Append / to matching directories. */
//#define	GLOB_NOCHECK	0x0010	/* Return pattern itself if nothing matches. */
#define	GLOB_NOSORT	0x0020	/* Don't sort. */

#define	GLOB_NOESCAPE	0x1000	/* Disable backslash escaping. */

#define	GLOB_NOSPACE	(-1)	/* Malloc call failed. */
#define	GLOB_ABORTED	(-2)	/* Unignored error. */
#define	GLOB_NOMATCH	(-3)	/* No match, and GLOB_NOCHECK was not set. */
#define	GLOB_NOSYS	(-4)	/* Implementation does not support function. */

typedef struct _find_handle
{
    struct _find_handle *self_addr; /* for pointer verification */

    char   dir[_MAX_DIR];
    char   fname[_MAX_PATH]; /* includes extension */
    glob_t glob;
    char   **next;
} find_obj;


#define ST_ATIME_NSEC(statstruct) 0
#define ST_MTIME_NSEC(statstruct) 0
#define ST_CTIME_NSEC(statstruct) 0



/*++
Function: 
  FILEEscapeSquareBrackets
  
Simple helper function to insert backslashes before square brackets   
to prevent glob from using them as wildcards.
note: this functions assumes all backslashes have previously been
      converted into forwardslashes by _splitpath.
--*/
static void FILEEscapeSquareBrackets(char *pattern, char *escaped_pattern)
{
	printf("Entering FILEEscapeSquareBrackets: [%p][%p]\n",
          pattern,escaped_pattern);
    while(*pattern)
    {
    	if('[' == *pattern || ']' == *pattern)
        {
            *escaped_pattern = '\\';
            escaped_pattern++;
        }
        *escaped_pattern = *pattern;
        pattern++;
        escaped_pattern++;
    }
    *escaped_pattern='\0';

    printf("FILEEscapeSquareBrackets done. escaped_pattern=%s\n",
                escaped_pattern?escaped_pattern:"NULL");
}


/*++
  FILEGlobQsortCompare
  Comparison function required by qsort, so that the
  . and .. directories end up on top of the sorted list
  of directories.
--*/
static int FILEGlobQsortCompare(const void *in_str1, const void *in_str2)
{
     char **str1 = (char**)in_str1;
     char **str2 = (char**)in_str2;
     const int FIRST_ARG_LESS    = -1;
     const int FIRST_ARG_EQUAL   =  0;
     const int FIRST_ARG_GREATER =  1;

     /* If both strings are equal, return immediately */
#if HAVE_CASE_SENSITIVE_FILESYSTEM
     if (strcmp(*(str1), *(str2)) == 0)
#else   // HAVE_CASE_SENSITIVE_FILESYSTEM
     if (strcasecmp(*(str1), *(str2)) == 0)
#endif  // HAVE_CASE_SENSITIVE_FILESYSTEM
     {
         return(FIRST_ARG_EQUAL);
     }

     /* Have '.' always on top than any other search result */
     if (strcmp(*(str1), ".") == 0)
     {
         return (FIRST_ARG_LESS);
     }
     if (strcmp(*(str2), ".") == 0)
     {
         return (FIRST_ARG_GREATER);
     }

     /* Have '..' next on top, over any other search result */
     if (strcmp(*(str1), "..") == 0)
     {
         return (FIRST_ARG_LESS);
     }
     if (strcmp(*(str2), "..") == 0)
     {
         return (FIRST_ARG_GREATER);
     }

     /* Finally, let strcmp do the rest for us */
#if HAVE_CASE_SENSITIVE_FILESYSTEM
     return (strcmp(*(str1),*(str2)));
#else   // HAVE_CASE_SENSITIVE_FILESYSTEM
     return (strcasecmp(*(str1),*(str2)));
#endif  // HAVE_CASE_SENSITIVE_FILESYSTEM
}


/*++
Function:
  FILEMakePathA
Mimics _makepath from windows, except it's a bit safer. 
Any or all of dir, fname, and ext can be NULL.
--*/
static int FILEMakePathA( char *buff, 
                          int buff_size,
                          const char *dir, 
                          const char *fname, 
                          const char *ext )
{
    int dir_len = 0;
    int fname_len = 0;
    int ext_len = 0;
    int len;
    char *p;

    TRACE("Attempting to assemble path from [%s][%s][%s], buff_size = %d\n",
          dir?dir:"NULL", fname?fname:"NULL", ext?ext:"NULL", buff_size);

    if (dir) dir_len = strlen(dir);
    if (fname) fname_len = strlen(fname);
    if (ext) ext_len = strlen(ext);

    len = dir_len + fname_len + ext_len + 1;

    TRACE("Required buffer size is %d bytes\n", len);

    if ( len > buff_size )
    {
        printf("Buffer is too small (%d bytes), needs %d bytes\n", 
              buff_size, len);
        return -1;
    }
    else
    {
        buff[0] = 0;

        p = buff;
        if (dir_len > 0) 
        {
            strncpy( buff, dir, dir_len + 1 );
            p += dir_len;
        }
        if (fname_len > 0)  
        {
            strncpy( p, fname, fname_len + 1 );
            p += fname_len;
        }
        if (ext_len > 0)
        {
            strncpy( p, ext, ext_len + 1);
        }

        TRACE("FILEMakePathA assembled [%s]\n", buff);
        return len - 1;
    }
}


/*++
Function:
  FILEGlobFromSplitPath
Simple wrapper function around glob(3), except that the pattern is accepted
in broken-down form like _splitpath produces.
ie. calling splitpath on a pattern then calling this function should
produce the same result as just calling glob() on the pattern.
--*/
static int FILEGlobFromSplitPath( const char *dir,
                                  const char *fname,
                                  const char *ext,
                                  int flags, 
                                  glob_t *pglob )
{
    int  Ret;
    char Pattern[MAX_PATH];
    //char EscapedPattern[2*MAX_PATH];
    char *unix_path;

    TRACE("We shall attempt to glob from components [%s][%s][%s]\n",
          dir?dir:"NULL", fname?fname:"NULL", ext?ext:"NULL");

    FILEMakePathA( Pattern, MAX_PATH, dir, fname, ext );
    TRACE("Assembled Pattern = [%s]\n", Pattern);

    unix_path = DOSPath2UNIXPath(Pattern);

	/* special handling is needed to handle the case where
        filename contains '[' and ']' */
    //FILEEscapeSquareBrackets( Pattern, EscapedPattern);    
#ifdef GLOB_QUOTE
    flags |= GLOB_QUOTE;
#endif  // GLOB_QUOTE
    Ret = glob( unix_path, flags, NULL, pglob );

#ifdef GLOB_NOMATCH
    if (Ret == GLOB_NOMATCH)
    {
        // pglob->gl_pathc will be 0 in this case.  We'll check
        // the return value to see if an error occurred, so we
        // don't want to return an error if we simply didn't match
        // anything.
        Ret = 0;
    }
#endif  // GLOB_NOMATCH
    
    /* Ensure that . and .. are placed in front, and sort the rest */
    qsort(pglob->gl_pathv, pglob->gl_pathc, sizeof(char*),
          FILEGlobQsortCompare);
    TRACE("Result of glob() is %d\n", Ret);

    free(unix_path);

    return Ret;
}



/*++
Function:
  FILEDosGlobA
Generate pathnames matching a DOS globbing pattern. This function has a similar
prototype to glob(3), and fulfils the same purpose.  However, DOS globbing
is slightly different than Unix in the following ways:
- '.*' at the end of a pattern means "any file extension, or none at all", 
whereas Unix has no concept of file extensions, and will match the '.' like
any other character
- on Unix, filenames beginning with '.' must be explicitly matched. This is
not true in DOS
- in DOS, the first two entries (if they match) will be '.' and '..', followed 
by all other matching entries sorted in ASCII order. In Unix, all entries are 
treated equally, so '+file' would appear before '.' and '..'
- DOS globbing will fail if any wildcard characters occur before the last path
separator
This implementation of glob implements the DOS behavior in all these cases, 
but otherwise attempts to behave exactly like POSIX glob.  The only exception
is its return value -- it returns TRUE if it succeeded (finding matches or
finding no matches but without any error occurring) or FALSE if any error
occurs.  It calls SetLastError() if it returns FALSE.
Sorting doesn't seem to be consistent on all Windows platform, and it's
not required for Rotor to have the same sorting alogrithm than Windows 2000.
This implementation will give slightly different result for the sort list 
than Windows 2000.
--*/
static BOOL FILEDosGlobA( const char *pattern, 
                          int flags, 
                          glob_t *pglob )
{
    char Dir[_MAX_DIR];
    char FilenameBuff[_MAX_FNAME + 1];
    char *Filename = FilenameBuff + 1;
    char Ext[_MAX_EXT];
    int A, B, C;
    BOOL result = TRUE;
    int globResult = 0;

    Dir[0] = 0;
    FilenameBuff[0] = '.';
    FilenameBuff[1] = 0;
    Ext[0] = 0;

    _splitpath( pattern, NULL, Dir, Filename, Ext );

    /* check to see if _splitpath failed */
    if ( Filename[0] == 0 )
    {
        if ( Dir[0] == 0 )
        {
            ERROR("_splitpath failed on path [%s]\n", pattern);
        }
        else
        {
            ERROR("Pattern contains a trailing backslash\n");
        }
        SetLastError(ERROR_PATH_NOT_FOUND);
        result = FALSE;
        goto done;
    }

    TRACE("glob pattern [%s] split into [%s][%s][%s]\n",
          pattern, Dir, Filename, Ext);

    if ( strchr(Dir, '*') != NULL || strchr(Dir, '?') != NULL )
    {
        ERROR("Found wildcard character(s) ('*' and/or '?') before "
              "last path separator\n");
        SetLastError(ERROR_PATH_NOT_FOUND);
        result = FALSE;
        goto done;
    }

    /*
    if (Dir[0] != 0)
    {
         FILEDosToUnixPathA( Dir );
    }
    */

    /* The meat of the routine happens below. Basically, there are three
       special things to check for:
       (A) If the extension is _exactly_ '.*', we will need to do two globs,
       one for 'filename.*' and one for 'filename', EXCEPT if (B) the last
       character of filename is '*', in which case we can eliminate the
       extension altogether, since '*.*' and '*' are the same in DOS.
       (C) If the first character of the filename is '*', we need to do
       an additional glob for each one we have already done, except with
       '.' prepended to the filename of the patterns, because in Unix,
       hidden files need to be matched explicitly.
       We can ignore the extension by calling FILEGlobFromSplitPath with
       the extension parameter as "", and we can prepend '.' to the
       filename by using (Filename - 1), since Filename conveniently points
       to the second character of a buffer which happens to have '.' as
       its first character.
    */
       
    A = (Ext && strncmp(Ext, ".*", 3) == 0);
    B = (Filename[strlen(Filename) - 1] == '*');
    C = (*Filename == '*');

    TRACE("Extension IS%s '.*', filename DOES%s end with '*', "
          "and filename DOES%s begin with '*'\n",
          A?"":" NOT", B?"":" NOT", C?"":" NOT");

    if ( !(A && B) ) 
    {
        /* the original pattern */
        globResult = FILEGlobFromSplitPath(Dir, Filename, Ext, 0, pglob);
        if ( globResult != 0 )
        {
            goto done;
        }

        if (C)
        {
            /* the original pattern but '.' prepended to filename */
            globResult = FILEGlobFromSplitPath( Dir, Filename - 1, Ext,
                                                GLOB_APPEND, pglob );
            if ( globResult != 0 )
            {
                goto done;
            }
        }
    }

    if (A)
    {
        /* if (A && B), this is the first glob() call. The first call
           to glob must use flags = 0, while proceeding calls should
           set the GLOB_APPEND flag. */
        globResult = FILEGlobFromSplitPath( Dir, Filename, "",
                                            (A && B)?0:GLOB_APPEND, 
                                             pglob );
        if ( globResult != 0 )
        {
            goto done;
        }

        if (C)
        {
            /* omit the extension and prepend '.' to filename */
            globResult = FILEGlobFromSplitPath( Dir, Filename - 1, "",
                                                GLOB_APPEND, pglob );
            if ( globResult != 0 )
            {
                goto done;
            }
        }
    }

done:
    if (globResult != 0)
    {
        if (globResult == GLOB_NOSPACE)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            SetLastError(ERROR_INTERNAL_ERROR);
        }
        result = FALSE;
    }
    TRACE("Returning %d\n", result);
    return result;
}



/*
// https://github.com/gbarnett/shared-source-cli-2.0/blob/d63349c09c2e93e4bfc4c8b147ff0805f36cec68/pal/unix/file/find.c
HANDLE FindFirstFile(
    _In_   LPCTSTR lpFileName,
    _Out_  LPWIN32_FIND_DATA lpFindFileData
)
{
    DIR *DirFp = NULL;
    if (!lpFileName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (HANDLE)ERROR_INVALID_PARAMETER;
    }
    if( NULL == lpFindFileData ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (HANDLE)ERROR_INVALID_PARAMETER;
    }
    
    DirFp = opendir(lpFileName);
    if (DirFp)
    {
        if (lpFindFileData)
        {
            FIND_FILE_HANDLE *pHandle = (FIND_FILE_HANDLE *)malloc(sizeof(FIND_FILE_HANDLE));
            if (pHandle)
            {
                memset(lpFindFileData, 0, sizeof(WIN32_FIND_DATA));
                pHandle->fp = DirFp;
                
                strcpy(lpFindFileData->cFileName, "."); // fake first file name
                lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

                return (HANDLE)pHandle;
            }
        }
        closedir(DirFp);
    }
    SetLastError(ERROR_FILE_NOT_FOUND);
    return INVALID_HANDLE_VALUE;
}


BOOL FindNextFile(
    _In_   HANDLE hFindFile,
    _Out_  LPWIN32_FIND_DATA lpFindFileData
)
{
	FIND_FILE_HANDLE *pHandle = (FIND_FILE_HANDLE *)hFindFile;
	if (pHandle && lpFindFileData)
    {
    	while( TRUE ) {    		
			struct dirent *result;
			if( -1 == readdir_r(pHandle->fp, &pHandle->entry, &result) ) {
				return FALSE;
			}

			if (strcmp(result->d_name, ".") != 0 ) {
	            lpFindFileData->dwFileAttributes =
	                (result->d_type == DT_DIR ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL);
	            strncpy(lpFindFileData->cFileName,
	                         result->d_name,
	                         sizeof(lpFindFileData->cFileName));
	            return TRUE;
	        }
    	}
    }
    return FALSE;
}

BOOL FindClose(
    _Inout_  HANDLE hFindFile
)
{
    if (hFindFile)
    {
        FIND_FILE_HANDLE *p = (FIND_FILE_HANDLE *)hFindFile;
        closedir(p->fp);
        free(p);
        return TRUE;
    }
    return FALSE;
}
*/




/*++
Function : 
    FILEGetProperNotFoundError
    
Returns the proper error code, based on the
Windows behavoir.
    IN LPSTR lpPath - The path to check.
    LPDWORD lpErrorCode - The error to set.
*/
#include <stat_def.h>
void FILEGetProperNotFoundError( LPSTR lpPath, LPDWORD lpErrorCode )
{
    struct stat stat_data;
    LPSTR lpDupedPath = NULL;
    LPSTR lpLastPathSeperator = NULL;

    TRACE( "FILEGetProperNotFoundError( %s )\n", lpPath?lpPath:"(null)" );

    if ( !lpErrorCode )
    {
        //ASSERT( "lpErrorCode has to be valid\n" );
        abort();
        return;
    }

    if ( NULL == ( lpDupedPath = strdup( lpPath ) ) )
    {
        ERROR( "strdup() failed!\n" );
        *lpErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    /* Determine whether it's a file not found or path not found. */
    lpLastPathSeperator = strrchr( lpDupedPath, '/');
    if ( lpLastPathSeperator != NULL )
    {
        int nStatRet = 0;
        *lpLastPathSeperator = '\0';
        
        /* If the last path component is a directory,
           we return file not found. If it's a file or
           doesn't exist, we return path not found. */
        if ( '\0' == *lpDupedPath || 
             ( ( nStatRet = fstat( lpDupedPath, &stat_data ) ) == 0 && 
             stat_data.st_mode  == 0  ))
        {
            TRACE( "ERROR_FILE_NOT_FOUND\n" );
            *lpErrorCode = ERROR_FILE_NOT_FOUND;
        }
        else
        {
            TRACE( "ERROR_PATH_NOT_FOUND\n" );
            *lpErrorCode = ERROR_PATH_NOT_FOUND;
        }
    }
    else
    {
        TRACE( "ERROR_FILE_NOT_FOUND\n" );
        *lpErrorCode = ERROR_FILE_NOT_FOUND;
    }
    
    free( lpDupedPath );
    lpDupedPath = NULL;
    TRACE( "FILEGetProperNotFoundError returning TRUE\n" );
    return;
}


HANDLE
FindFirstFile(
           IN LPCSTR lpFileName,
           OUT LPWIN32_FIND_DATA lpFindFileData)
{
    HANDLE hRet = INVALID_HANDLE_VALUE;
    DWORD  dwLastError = NO_ERROR;
    find_obj *find_data = NULL;

    /*
    PERF_ENTRY(FindFirstFileA);
    ENTRY("FindFirstFileA(lpFileName=%p (%s), lpFindFileData=%p)\n",
          lpFileName?lpFileName:"NULL",
          lpFileName?lpFileName:"NULL", lpFindFileData);
    */

    if(NULL == lpFileName)
    {
        ERROR("lpFileName is NULL!\n");
        dwLastError = ERROR_PATH_NOT_FOUND;
        goto done;
    }
    if(NULL == lpFindFileData)
    {
        //ASSERT("lpFindFileData is NULL!\n");
        dwLastError = ERROR_INVALID_PARAMETER;
        goto done;
    }                                        

    find_data = malloc( sizeof(find_obj) );
    if ( find_data == NULL )
    {
        ERROR("Unable to allocate memory for find_data\n");
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    find_data->self_addr = find_data;
    
    // Clear the glob_t so we can safely call globfree() on it
    // regardless of whether FILEDosGlobA ends up calling glob().
    memset(&(find_data->glob), 0, sizeof(find_data->glob));
    
    if (!FILEDosGlobA(lpFileName, 0, &(find_data->glob)))
    {
        // FILEDosGlobA will call SetLastError() on failure.
        goto done;
    }
    else
    {
        // Check if there's at least one match.
        if (find_data->glob.gl_pathc == 0)
        {
            /* Testing has indicated that for this API the
             * last errors are as follows 
             *      c:\temp\foo.txt      - no error
             *      c:\temp\foo          - ERROR_FILE_NOT_FOUND
             *      c:\temp\foo\bar      - ERROR_PATH_NOT_FOUND
             *      c:\temp\foo.txt\bar  - ERROR_DIRECTORY
             *
             */
            LPSTR lpTemp = DOSPath2UNIXPath( lpFileName );
            if ( !lpTemp )
            {
                ERROR( "strdup failed!\n" );
                SetLastError( ERROR_INTERNAL_ERROR );
                goto done;
            }
            //FILEDosToUnixPathA( lpTemp );
            FILEGetProperNotFoundError( lpTemp, &dwLastError );
            
            if ( ERROR_PATH_NOT_FOUND == dwLastError )
            {
                /* If stripping the last segment reveals a file name then
                the error is ERROR_DIRECTORY. */
                struct stat stat_data;
                LPSTR lpLastPathSeparator = NULL;

                lpLastPathSeparator = strrchr( lpTemp, '/');
                
                if ( lpLastPathSeparator != NULL )
                {
                    *lpLastPathSeparator = '\0';
                    
                    if ( fstat( lpTemp, &stat_data) == 0 && 
                         stat_data.st_mode != 0 )
                    {
                        dwLastError = ERROR_DIRECTORY;
                    }
                }
            }
            free( lpTemp );
            lpTemp = NULL;
            goto done;
        }

        find_data->next = find_data->glob.gl_pathv;
    }

    if ( FindNextFile( (HANDLE)find_data, lpFindFileData ) )
    {
        hRet = (HANDLE)find_data;
    }

done:

    if ( hRet == INVALID_HANDLE_VALUE ) 
    {
        if(NULL != find_data)
        {
            globfree( &(find_data->glob) );
            free( find_data );
        }
        if (dwLastError)
        {
            SetLastError(dwLastError);
        }
    }

    /*
    LOGEXIT("FindFirstFileA returns HANDLE %p\n", hRet );
    PERF_EXIT(FindFirstFileA);
    */
    return hRet;
}


static const int64_t SECS_BETWEEN_EPOCHS = (int64_t)(11644473600);
static const int64_t SECS_TO_100NS = 10000000; /* 10^7 */

/*++
Function:
  FILEUnixTimeToFileTime
Convert a time_t value to a win32 FILETIME structure, as described in
MSDN documentation. time_t is the number of seconds elapsed since 
00:00 01 January 1970 UTC (Unix epoch), while FILETIME represents a 
64-bit number of 100-nanosecond intervals that have passed since 00:00 
01 January 1601 UTC (win32 epoch).
--*/
FILETIME FILEUnixTimeToFileTime( time_t sec, long nsec )
{
    int64_t Result;
    FILETIME Ret;

    Result = ((int64_t)sec + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS +
        (nsec / 100);

    Ret.dwLowDateTime = (DWORD)Result;
    Ret.dwHighDateTime = (DWORD)(Result >> 32);

    TRACE("Unix time = [%ld.%09ld] converts to Win32 FILETIME = [%#x:%#x]\n", 
          sec, nsec, Ret.dwHighDateTime, Ret.dwLowDateTime);

    return Ret;
}


BOOL
FindNextFile(
          IN HANDLE hFindFile,
          OUT LPWIN32_FIND_DATA lpFindFileData)
{
    find_obj *find_data;

    BOOL  bRet = FALSE;
    DWORD dwLastError = 0;
    DWORD Attr;

    /*
    PERF_ENTRY(FindNextFileA);
    ENTRY("FindNextFileA(hFindFile=%p, lpFindFileData=%p)\n",
          hFindFile, lpFindFileData);
    */

    find_data = (find_obj*)hFindFile;

    if ( hFindFile == INVALID_HANDLE_VALUE ||
         find_data == NULL || 
         find_data->self_addr != find_data )
    {
        TRACE("FindNextFileA received an invalid handle\n");
        dwLastError = ERROR_INVALID_HANDLE;
        goto done;
    }

    if ( find_data->next)
    {
        struct stat stat_data;
        char ext[_MAX_EXT];
        int stat_result;

        while (*(find_data->next))
        {
            char *path = *(find_data->next);
#if !HAVE_CASE_SENSITIVE_FILESYSTEM
            // Convert the path to the right case.
            char realPath[MAX_PATHLEN];
            char *result;
            char *lastSlash;
            
            // realpath converts a path to its canonical form.  We don't want
            // this for '.' and '..', which we have to return.
            lastSlash = strrchr(path, '/');
            if (lastSlash == NULL || (strcmp(lastSlash, "/.") != 0 &&
                                      strcmp(lastSlash, "/..") != 0))
            {
                result = realpath(path, realPath);
                if (result != NULL)
                {
                    // Replace the existing path.
                    path = result;
                }
            }
#endif  // !HAVE_CASE_SENSITIVE_FILESYSTEM

            TRACE("Found [%s]\n", path);

            // Split the path into a dir and filename.
            _splitpath(path, NULL, find_data->dir, find_data->fname, ext);
            if ( find_data->fname == NULL )
            {
                //ASSERT("_splitpath failed on %s\n", path);
                dwLastError = ERROR_INTERNAL_ERROR;
                goto done;
            }
            strcpy( find_data->fname + strlen(find_data->fname), ext );
            
            /* get the attributes, but continue if it fails */
            Attr = GetFileAttributes(path);
            if ( Attr == -1 )
            {
            	/*
                WARN("GetFileAttributes returned -1 on file [%s]\n",
                  *(find_data->next));
                */
            }
            lpFindFileData->dwFileAttributes = Attr;
            
            /* Note that cFileName is NOT the relative path */
            strcpy( lpFindFileData->cFileName, find_data->fname );
            
            /* we don't support 8.3 filenames, so just leave it empty */
            lpFindFileData->cAlternateFileName[0] = 0;
            
            /* get the filetimes */
            stat_result = fstat(path, &stat_data) == 0; //|| lstat(path, &stat_data) == 0;
    
            find_data->next++;
    
            if ( stat_result )
            {
                    lpFindFileData->ftCreationTime = 
                        FILEUnixTimeToFileTime( stat_data.st_ctime,
                                        ST_CTIME_NSEC(&stat_data) );
                    lpFindFileData->ftLastAccessTime = 
                        FILEUnixTimeToFileTime( stat_data.st_atime,
                                        ST_ATIME_NSEC(&stat_data) );
                    lpFindFileData->ftLastWriteTime = 
                        FILEUnixTimeToFileTime( stat_data.st_mtime,
                                        ST_MTIME_NSEC(&stat_data) );
    
                    /* get file size */
                    lpFindFileData->nFileSizeLow = (DWORD)stat_data.st_size;
    #if SIZEOF_OFF_T > 4
                    lpFindFileData->nFileSizeHigh = 
                           (DWORD)(stat_data.st_size >> 32);
    #else
                    lpFindFileData->nFileSizeHigh = 0;
    #endif
            
                    bRet = TRUE;
            break;
                }
        }
        if(!bRet)
        {
            dwLastError = ERROR_NO_MORE_FILES;
        }
    }
    else
    {
    	abort();
        //ASSERT("find_data->next is (mysteriously) NULL\n");   
    }

done:
    if (dwLastError)
    {
        SetLastError(dwLastError);
    }

    /*
    LOGEXIT("FindNextFileA returns BOOL %d\n", bRet);
    PERF_EXIT(FindNextFileA);
    */
    return bRet;
}

BOOL
FindClose(
      IN OUT HANDLE hFindFile)
{
    find_obj *find_data;
    BOOL  hRet = TRUE;
    DWORD dwLastError = 0;

    /*
    PERF_ENTRY(FindClose);
    ENTRY("FindClose(hFindFile=%p)\n", hFindFile);
    */

    find_data = (find_obj*)hFindFile;

    if ( hFindFile == INVALID_HANDLE_VALUE ||
         find_data == NULL ||
         find_data->self_addr != find_data )
    {
        ERROR("Invalid find handle\n");
        hRet = FALSE;
        dwLastError = ERROR_INVALID_PARAMETER;
        goto done;
    }

    find_data->self_addr = NULL;
    globfree( &(find_data->glob) );
    free( find_data );

done:
    if (dwLastError)
    {
        SetLastError(dwLastError);
    }

    /*
    LOGEXIT("FindClose returns BOOL %d\n", hRet);
    PERF_EXIT(FindClose);
    */
    return hRet;
}