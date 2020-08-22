#ifndef PORTABLE_PATH_H_
#define PORTABLE_PATH_H_

// https://www.gnu.org/software/libc/manual/html_node/Limits-for-Files.html
// http://web.mit.edu/cygwin/cygwin_v1.3.2/usr/include/mingw/stdlib.h
#define NAME_MAX 256
#define _MAX_PATH PATH_MAX
#define _MAX_DIR NAME_MAX
#define _MAX_EXT (NAME_MAX-1)
#define _MAX_FNAME NAME_MAX
#define _MAX_DRIVE (3)

void FILECanonicalizePath(LPSTR lpUnixPath);

void FILEDosToUnixPathA(LPSTR lpPath);

char *DOSPath2UNIXPath( const char* dos_path );

char *UNIXPath2DOSPath( const char *unix_path );

void _splitpath(const char *dospath, char *drive, char *dir, char *fname, char *ext);

#endif
