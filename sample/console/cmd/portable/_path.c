#include "cmd.h"

#include <assert.h>

#define TRACE(a)

static void
str_replace( char *str, char find_char, char replace_char ) {
  while( *str ) {
    if( *str == find_char ) {
      *str = replace_char;
    }
    str++;
  }
}



/*++
FILECanonicalizePath
    Removes all instances of '/./', '/../' and '//' from an absolute path. 
    
Parameters:
    LPSTR lpUnixPath : absolute path to modify, in Unix format
(no return value)                                             
 
Notes :
-behavior is undefined if path is not absolute
-the order of steps *is* important: /one/./../two would give /one/two 
 instead of /two if step 3 was done before step 2
-reason for this function is that GetFullPathName can't use realpath(), since 
 realpath() requires the given path to be valid and GetFullPathName does not.
--*/
void FILECanonicalizePath(LPSTR lpUnixPath)
{
    LPSTR slashslashptr;
    LPSTR dotdotptr;
    LPSTR slashdotptr;
    LPSTR slashptr;

    /* step 1 : replace '//' sequences by a single '/' */

    slashslashptr = lpUnixPath;
    while(1)
    {
        slashslashptr = strstr(slashslashptr,"//");
        if(NULL == slashslashptr)
        {
            break;
        }
        /* remove extra '/' */
        //TRACE("stripping '//' from %s\n", lpUnixPath);
        memmove(slashslashptr,slashslashptr+1,strlen(slashslashptr+1)+1);
    }

    /* step 2 : replace '/./' sequences by a single '/' */

    slashdotptr = lpUnixPath;
    while(1)
    {
        slashdotptr = strstr(slashdotptr,"/./");
        if(NULL == slashdotptr)
        {
            break;
        }
        /* strip the extra '/.' */
        //TRACE("removing '/./' sequence from %s\n", lpUnixPath);
        memmove(slashdotptr,slashdotptr+2,strlen(slashdotptr+2)+1);
    }

    /* step 3 : replace '/<name>/../' sequences by a single '/' */

    while(1)
    {
        dotdotptr = strstr(lpUnixPath,"/../");
        if(NULL == dotdotptr)
        {
            break;
        }
        if(dotdotptr == lpUnixPath)
        {
            /* special case : '/../' at the beginning of the path are replaced
               by a single '/' */
            //TRACE("stripping leading '/../' from %s\n", lpUnixPath);
            memmove(lpUnixPath, lpUnixPath+3,strlen(lpUnixPath+3)+1);
            continue;
        }
        
        /* null-terminate the string before the '/../', so that strrchr will 
           start looking right before it */
        *dotdotptr = '\0';
        slashptr = strrchr(lpUnixPath,'/');
        if(NULL == slashptr)
        {
            /* this happens if this function was called with a relative path. 
               don't do that.  */
            ASSERT("can't find leading '/' before '/../ sequence\n");
            break;
        }
        //TRACE("removing '/<dir>/../' sequence from %s\n", lpUnixPath);
        memmove(slashptr,dotdotptr+3,strlen(dotdotptr+3)+1);
    }

    /* step 4 : remove a trailing '/..' */

    dotdotptr = strstr(lpUnixPath,"/..");
    if(dotdotptr == lpUnixPath)
    {
        /* if the full path is simply '/..', replace it by '/' */
        lpUnixPath[1] = '\0';
    }
    else if(NULL != dotdotptr && '\0' == dotdotptr[3])
    {
        *dotdotptr = '\0';
        slashptr = strrchr(lpUnixPath,'/');
        if(NULL != slashptr)
        {
            *slashptr = '\0';
        }
    }

    /* step 5 : remove a traling '/.' */

    slashdotptr = strstr(lpUnixPath,"/.");
    if (slashdotptr != NULL && slashdotptr[2] == '\0')
    {
        if(slashdotptr == lpUnixPath)
        {
            // if the full path is simply '/.', replace it by '/' */
            lpUnixPath[1] = '\0';
        }
        else
        {
            *slashdotptr = '\0';
        }
    }
}


/*++
Function:
  FileDosToUnixPathA
Abstract:
  Change a DOS path to a Unix path.
  Replaces '\' by '/', removes any trailing dots on directory/filenames,
  and changes '*.*' to be equal to '*'
Parameter:
  IN/OUT lpPath: path to be modified
--*/
void
FILEDosToUnixPathA(
       LPSTR lpPath)
{
    LPSTR p;
    LPSTR pPointAtDot=NULL;
    char charBeforeFirstDot='\0';


    printf("Original DOS path = [%s]\n", lpPath);

    if (!lpPath)
    {
        return;
    }

    for (p = lpPath; *p; p++)
    {
        /* Make the \\ to / switch first */
        if (*p == '\\')
        {
            /* Replace \ with / */
            *p = '/';
        }

        if (pPointAtDot)
        {
            /* If pPointAtDot is not NULL, it is pointing at the first encountered
               dot.  If we encountered a \, that means it could be a trailing dot */
            if (*p == '/')
            {
                /* If char before the first dot is a '\' or '.' (special case if the
                   dot is the first char in the path) , then we leave it alone,
                   because it is either . or .., otherwise it is a trailing dot
                   pattern and will be truncated */
                if (charBeforeFirstDot != '.' && charBeforeFirstDot != '/')
                {
                    memmove(pPointAtDot,p,(strlen(p)*sizeof(char))+1);
                    p = pPointAtDot;
                }
                pPointAtDot = NULL; /* Need to reset this */
            }
            else if (*p == '*')
            {
                /* Check our size before doing anything with our pointers */
                if ((p - lpPath) >= 3)
                {
                    /* At this point, we know that there is 1 or more dots and
                       then a star.  AND we know the size of our string at this
                       point is at least 3 (so we can go backwards from our pointer
                       safely AND there could possilby be two characters back)
                       So lets check if there is a '*' and a '.' before, if there
                       is, replace just a '*'.  Otherwise, reset pPointAtDot to NULL
                       and do nothing */
                    if (p[-2] == '*' &&
                        p[-1] == '.' &&
                        p[0] == '*')
                    {
                        memmove(&(p[-2]),p,(strlen(p)*sizeof(char))+1);
                    }

                    pPointAtDot  = NULL;
                }
            }
            else if (*p != '.')
            {
                /* If we are here, that means that this is NOT a trailing dot,
                   some other character is here, so forget our pointer */
                pPointAtDot = NULL;
            }
        }
        else
        {
            if (*p == '.')
            {
                /* If pPointAtDot is NULL, and we encounter a dot, save the pointer */
                pPointAtDot = p;
                if (pPointAtDot != lpPath)
                {
                    charBeforeFirstDot = p[-1];
                }
                else
                {
                    charBeforeFirstDot = lpPath[0];
                }
            }
        }
    }

    /* If pPointAtDot still points at anything, then we still have trailing dots.
       Truncate at pPointAtDot, unless the dots are path specifiers (. or ..) */
    if (pPointAtDot)
    {
        /* make sure the trailing dots don't follow a '/', and that they aren't 
           the only thing in the name */
        if(pPointAtDot != lpPath && *(pPointAtDot-1) != '/')
        {
            *pPointAtDot = '\0';
        }
    }

    printf("Resulting Unix path = [%s]\n", lpPath);
}



/** Convert C:\Dos\Path to /Dos/Path */
char *
DOSPath2UNIXPath( const char* dos_path ) {
  assert( dos_path != NULL );

  char *unix_path = malloc(strlen(dos_path));

  // If absolute path, e.g. C:\Derp\ - ensure it's C:
  // But then omit drive prefix
  if( dos_path[1] == ':' ) {
    if( toupper(dos_path[0]) != 'C' ) {
      SetLastError(ERROR_INVALID_DRIVE);
      abort();
      return NULL;
    }
    strcpy(unix_path, &dos_path[2]);
  }
  else {
    strcpy(unix_path, dos_path);
  }

  str_replace(unix_path, '\\', '/');
  return unix_path;
}


/** Convert /Unix/Path to C:\Unix\Path */
char *
UNIXPath2DOSPath( const char *unix_path ) {
  assert( unix_path != NULL );
  assert( unix_path[0] == '/' );
  char *dos_path = malloc(strlen(unix_path) + 2);
  dos_path[0] = 'C';
  dos_path[1] = ':';
  strcpy(&dos_path[2], unix_path);
  str_replace(dos_path, '/', '\\');
  return dos_path;
}


void
_splitpath(
            const char *dospath,
            char *drive,
            char *dir,
            char *fname,
            char *ext)
{
    CHAR *path;
    LPCSTR slash_ptr = NULL;
    LPCSTR period_ptr = NULL;
    int size = 0;

    /*
    PERF_ENTRY(_wsplitpath);
    ENTRY("_wsplitpath (path=%p (%S), drive=%p, dir=%p, fname=%p, ext=%p)\n",
          path?path:W16_NULLSTRING,
          path?path:W16_NULLSTRING, drive, dir, fname, ext);
    */
    /* Do performance intensive error checking only in debug builds.
    
    NOTE: This function must fail predictably across all platforms.
    Under Windows this function throw an access violation if NULL
    was passed in as the value for path.
    */
#if _DEBUG
    if ( !dospath )
    {
        ERROR( "path cannot be NULL!\n" );
    }
#endif
    
    if( strlen( dospath ) >= _MAX_PATH )
    {    
        //ERROR("Path length is > _MAX_PATH (%d)!\n", _MAX_PATH);
	   //ON_ERROR;
    }

    path = DOSPath2UNIXPath(dospath);

    /* no drive letters in the PAL */
    if( drive != NULL )
    {
        drive[0] = 0;
    }

    /* find last path separator char */
    slash_ptr = strrchr(path, '/');

    if( slash_ptr == NULL )
    {
        //TRACE("No path separator in path\n");
        slash_ptr = path - 1;
    }
    /* find extension separator, if any */
    period_ptr = strrchr(path, '.');

    /* make sure we only consider periods after the last path separator */
    if( period_ptr < slash_ptr )
    {
        period_ptr = NULL;
    }

    /* if the only period in the file is a leading period (denoting a hidden
       file), don't treat what follows as an extension */
    if( period_ptr == slash_ptr+1 )
    {
        period_ptr = NULL;
    }

    if( period_ptr == NULL )
    {
        //TRACE("No extension in path\n");
        period_ptr = path + strlen(path);
    }

    size = slash_ptr - path + 1;
    if( dir != NULL )
    {
        int i;

        if( (size + 1 ) > _MAX_DIR )
        {
           // ERROR("Directory component needs %d characters, _MAX_DIR is %d\n",
             //     size+1, _MAX_DIR);
            //ON_ERROR;
        }
        
        memcpy(dir, path, size*sizeof(WCHAR));
        dir[size] = 0;

        /* only allow / separators in returned path */
        i = 0;
        while( dir[ i ] )
        {
            if( dir[ i ] == '\\' )
            {
                dir[i]='/';
            }
            i++;
        }
    }

    size = period_ptr-slash_ptr-1;
    if( fname != NULL )
    {
        if( (size+1) > _MAX_FNAME )
        {
           // ERROR("Filename component needs %d characters, _MAX_FNAME is %d\n",
              //   size+1, _MAX_FNAME);
           // ON_ERROR;
        }
        memcpy(fname, slash_ptr+1, size*sizeof(WCHAR));
        fname[size] = 0;
    }

    size = 1 + strlen( period_ptr );
    if( ext != NULL )
    {
        if( size > _MAX_EXT )
        {
            //ERROR("Extension component needs %d characters, _MAX_EXT is %d\n",
              //   size, _MAX_EXT);
           // ON_ERROR;
        }
        memcpy(ext, period_ptr, size*sizeof(WCHAR));
        ext[size-1] = 0;
    }

    free(path);
    
    //TRACE("Path components are '%S' '%S' '%S'\n", dir, fname, ext);

//done:
    
    //LOGEXIT("_wsplitpath returns.\n");
    //PERF_EXIT(_wsplitpath);
}