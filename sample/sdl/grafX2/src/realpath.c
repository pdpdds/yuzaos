/* vim:expandtab:ts=2 sw=2:
*/
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#if !defined(WIN32)
#include <limits.h>
#endif

#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < 1060)
#include "gfx2mem.h"
#endif


    #if !defined(PATH_MAX)
        // This is a random default value ...
        #define PATH_MAX 4096
    #endif
  
    static char *sep(char *path)
    {
        char *tmp, c;
        
        tmp = strrchr(path, '/');
        if(tmp) {
            c = tmp[1];
            tmp[1] = 0;
            if (chdir(path)) {
                return NULL;
            }
            tmp[1] = c;
            
            return tmp + 1;
        }
        return path;
    }

	// Find the real path of _path by chdir to it and then getcwd.
	// If resolved_path is null, it is allocated.
    char *Realpath(const char *_path, char *resolved_path)
    {
        #if defined(__AROS__)
        int fd = open("", O_RDONLY); // GrafX2 is compiled without Unix support
        #else
        int fd = open(".", O_RDONLY);
        #endif
        int l;
        char current_dir_path[PATH_MAX];
        char path[PATH_MAX], lnk[PATH_MAX], *tmp = (char *)"";
        
        if (fd < 0) {
            return NULL;
        }
        getcwd(current_dir_path,PATH_MAX);
        strncpy(path, _path, PATH_MAX);
        
        if (chdir(path)) {
            if (errno == ENOTDIR) {
               
                    // No symbolic links and no readlink()
                    l = -1;
                
                if (!(tmp = sep(path))) {
                    resolved_path = NULL;
                    goto abort;
                }
                if (l < 0) {
                    if (errno != EINVAL) {
                        resolved_path = NULL;
                        goto abort;
                    }
                } else {
                    lnk[l] = 0;
                    if (!(tmp = sep(lnk))) {
                        resolved_path = NULL;
                        goto abort;
                    }
                }
            } else {
                resolved_path = NULL;
                goto abort;
            }
        }
        
        if(resolved_path==NULL) // if we called realpath with null as a 2nd arg
            resolved_path = (char*) malloc( PATH_MAX );
                
        if (!getcwd(resolved_path, PATH_MAX)) {
            resolved_path = NULL;
            goto abort;
        }
        
        if(strcmp(resolved_path, "/") && *tmp) {
            strcat(resolved_path, "/");
        }
        
        strcat(resolved_path, tmp);
      abort:
        chdir(current_dir_path);
        close(fd);
        return resolved_path;
    }

