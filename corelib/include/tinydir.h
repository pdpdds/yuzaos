/*
Copyright (c) 2013-2019, tinydir authors:
- Cong Xu
- Lautis Sun
- Baudouin Feildel
- Andargor <andargor@yahoo.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TINYDIR_H
#define TINYDIR_H

#ifdef __cplusplus
extern "C" {
#endif

#if ((defined _UNICODE) && !(defined UNICODE))
#define UNICODE
#endif

#if ((defined UNICODE) && !(defined _UNICODE))
#define _UNICODE
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

# include <dirent.h>
//# include <libgen.h>
# include <stat_def.h>
# include <stddef.h>
#include <unistd.h>
#include <memory.h>
/* types */

/* Windows UNICODE wide character support */

# define _tinydir_char_t char
# define TINYDIR_STRING(s) s
# define _tinydir_strlen strlen
# define _tinydir_strcpy strcpy
# define _tinydir_strcat strcat
# define _tinydir_strcmp strcmp
# define _tinydir_strrchr strrchr
# define _tinydir_strncmp strncmp

# include <minwindef.h>
# define _TINYDIR_PATH_MAX MAX_PATH

#ifndef _TINYDIR_PATH_MAX
#define _TINYDIR_PATH_MAX 4096
#endif

# define _TINYDIR_PATH_EXTRA 0

#define _TINYDIR_FILENAME_MAX 256

#define _TINYDIR_DRIVE_MAX 3

# define _TINYDIR_FUNC static __inline


/* readdir_r usage; define TINYDIR_USE_READDIR_R to use it (if supported) */
#ifdef TINYDIR_USE_READDIR_R

/* readdir_r is a POSIX-only function, and may not be available under various
 * environments/settings, e.g. MinGW. Use readdir fallback */
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE ||\
	_POSIX_SOURCE
# define _TINYDIR_HAS_READDIR_R
#endif
#if _POSIX_C_SOURCE >= 200112L
# define _TINYDIR_HAS_FPATHCONF
# include <unistd.h>
#endif
#if _BSD_SOURCE || _SVID_SOURCE || \
	(_POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)
# define _TINYDIR_HAS_DIRFD
# include <sys/types.h>
#endif
#if defined _TINYDIR_HAS_FPATHCONF && defined _TINYDIR_HAS_DIRFD &&\
	defined _PC_NAME_MAX
# define _TINYDIR_USE_FPATHCONF
#endif
#if defined __MINGW32__ || !defined _TINYDIR_HAS_READDIR_R ||\
	!(defined _TINYDIR_USE_FPATHCONF || defined NAME_MAX)
# define _TINYDIR_USE_READDIR
#endif

/* Use readdir by default */
#else
# define _TINYDIR_USE_READDIR
#endif

#define _TINYDIR_DIR DIR
#define _tinydir_dirent dirent
#define _tinydir_opendir opendir
#define _tinydir_readdir readdir
#define _tinydir_closedir closedir


/* Allow user to use a custom allocator by defining _TINYDIR_MALLOC and _TINYDIR_FREE. */
#if    defined(_TINYDIR_MALLOC) &&  defined(_TINYDIR_FREE)
#elif !defined(_TINYDIR_MALLOC) && !defined(_TINYDIR_FREE)
#else
#error "Either define both alloc and free or none of them!"
#endif

#if !defined(_TINYDIR_MALLOC)
	#define _TINYDIR_MALLOC(_size) malloc(_size)
	#define _TINYDIR_FREE(_ptr)    free(_ptr)
#endif /* !defined(_TINYDIR_MALLOC) */

typedef struct tinydir_file
{
	_tinydir_char_t path[_TINYDIR_PATH_MAX];
	_tinydir_char_t name[_TINYDIR_FILENAME_MAX];
	_tinydir_char_t *extension;
	int is_dir;
	int is_reg;

	struct stat _s;

} tinydir_file;

typedef struct tinydir_dir
{
	_tinydir_char_t path[_TINYDIR_PATH_MAX];
	int has_next;
	size_t n_files;

	tinydir_file *_files;

	_TINYDIR_DIR *_d;
	struct _tinydir_dirent *_e;
#ifndef _TINYDIR_USE_READDIR
	struct _tinydir_dirent *_ep;
#endif
} tinydir_dir;


/* declarations */

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const _tinydir_char_t *path);
_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const _tinydir_char_t *path);
_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir);

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir);
_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file);
_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i);
_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i);

_TINYDIR_FUNC
int tinydir_file_open(tinydir_file *file, const _tinydir_char_t *path);
_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file);
_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b);

#ifndef _TINYDIR_USE_READDIR
_TINYDIR_FUNC
size_t _tinydir_dirent_buf_size(_TINYDIR_DIR *dirp);
#endif


/* definitions*/

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const _tinydir_char_t *path)
{

#ifndef _TINYDIR_USE_READDIR
	int error;
	int size;	/* using int size */
#endif

	_tinydir_char_t *pathp;

	if (dir == NULL || path == NULL || _tinydir_strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (_tinydir_strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* initialise dir */
	dir->_files = NULL;
	dir->_d = NULL;
#ifndef _TINYDIR_USE_READDIR
	dir->_ep = NULL;
#endif
	tinydir_close(dir);

	_tinydir_strcpy(dir->path, path);
	/* Remove trailing slashes */
	pathp = &dir->path[_tinydir_strlen(dir->path) - 1];
	while (pathp != dir->path && (*pathp == TINYDIR_STRING('\\') || *pathp == TINYDIR_STRING('/')))
	{
		*pathp = TINYDIR_STRING('\0');
		pathp++;
	}
	dir->_d = _tinydir_opendir(path);
	if (dir->_d == NULL)
	{

		goto bail;
	}

	/* read first file */
	dir->has_next = 1;
#ifdef _TINYDIR_USE_READDIR
	dir->_e = _tinydir_readdir(dir->_d);
#else
	/* allocate dirent buffer for readdir_r */
	size = _tinydir_dirent_buf_size(dir->_d); /* conversion to int */
	if (size == -1) return -1;
	dir->_ep = (struct _tinydir_dirent*)_TINYDIR_MALLOC(size);
	if (dir->_ep == NULL) return -1;

	error = readdir_r(dir->_d, dir->_ep, &dir->_e);
	if (error != 0) return -1;
#endif
	if (dir->_e == NULL)
	{
		dir->has_next = 0;
	}

	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const _tinydir_char_t *path)
{
	/* Count the number of files first, to pre-allocate the files array */
	size_t n_files = 0;
	if (tinydir_open(dir, path) == -1)
	{
		return -1;
	}
	while (dir->has_next)
	{
		n_files++;
		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}
	}
	tinydir_close(dir);

	if (n_files == 0 || tinydir_open(dir, path) == -1)
	{
		return -1;
	}

	dir->n_files = 0;
	dir->_files = (tinydir_file *)_TINYDIR_MALLOC(sizeof *dir->_files * n_files);
	if (dir->_files == NULL)
	{
		goto bail;
	}
	while (dir->has_next)
	{
		tinydir_file *p_file;
		dir->n_files++;

		p_file = &dir->_files[dir->n_files - 1];
		if (tinydir_readfile(dir, p_file) == -1)
		{
			goto bail;
		}

		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}

		/* Just in case the number of files has changed between the first and
		second reads, terminate without writing into unallocated memory */
		if (dir->n_files == n_files)
		{
			break;
		}
	}

	qsort(dir->_files, dir->n_files, sizeof(tinydir_file), _tinydir_file_cmp);

	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		return;
	}

	memset(dir->path, 0, sizeof(dir->path));
	dir->has_next = 0;
	dir->n_files = 0;
	_TINYDIR_FREE(dir->_files);
	dir->_files = NULL;

	if (dir->_d)
	{
		_tinydir_closedir(dir->_d);
	}
	dir->_d = NULL;
	dir->_e = NULL;
#ifndef _TINYDIR_USE_READDIR
	_TINYDIR_FREE(dir->_ep);
	dir->_ep = NULL;
#endif
}

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (!dir->has_next)
	{
		errno = ENOENT;
		return -1;
	}


#ifdef _TINYDIR_USE_READDIR
	dir->_e = _tinydir_readdir(dir->_d);
#else
	if (dir->_ep == NULL)
	{
		return -1;
	}
	if (readdir_r(dir->_d, dir->_ep, &dir->_e) != 0)
	{
		return -1;
	}
#endif
	if (dir->_e == NULL)
	{
		dir->has_next = 0;

	}

	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file)
{
	const _tinydir_char_t *filename;
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (dir->_e == NULL)

	{
		errno = ENOENT;
		return -1;
	}
	filename =

		dir->_e->d_name;

	if (_tinydir_strlen(dir->path) +
		_tinydir_strlen(filename) + 1 + _TINYDIR_PATH_EXTRA >=
		_TINYDIR_PATH_MAX)
	{
		/* the path for the file will be too long */
		errno = ENAMETOOLONG;
		return -1;
	}
	if (_tinydir_strlen(filename) >= _TINYDIR_FILENAME_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	_tinydir_strcpy(file->path, dir->path);
	if (_tinydir_strcmp(dir->path, TINYDIR_STRING("/")) != 0)
		_tinydir_strcat(file->path, TINYDIR_STRING("/"));
	_tinydir_strcpy(file->name, filename);
	_tinydir_strcat(file->path, filename);

	if (fstat(

		file->path, &file->_s) != 0)
	{
		return -1;
	}

	_tinydir_get_ext(file);

	file->is_dir = S_ISDIR(file->_s.st_mode);
	file->is_reg = S_ISREG(file->_s.st_mode);


	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i)
{
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files)
	{
		errno = ENOENT;
		return -1;
	}

	memcpy(file, &dir->_files[i], sizeof(tinydir_file));
	_tinydir_get_ext(file);

	return 0;
}

_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i)
{
	_tinydir_char_t path[_TINYDIR_PATH_MAX];
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files || !dir->_files[i].is_dir)
	{
		errno = ENOENT;
		return -1;
	}

	_tinydir_strcpy(path, dir->_files[i].path);
	tinydir_close(dir);
	if (tinydir_open_sorted(dir, path) == -1)
	{
		return -1;
	}

	return 0;
}

char* dirname(char* path)
{
	static const char dot[] = ".";
	char* last_slash;
	/* Find last '/'.  */
	last_slash = path != NULL ? (char*)strrchr(path, '/') : NULL;
	if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
	{
		/* Determine whether all remaining characters are slashes.  */
		char* runp;
		for (runp = last_slash; runp != path; --runp)
			if (runp[-1] != '/')
				break;
		/* The '/' is the last character, we have to look further.  */
		if (runp != path)
			last_slash = (char*)memrchr(path, '/', runp - path);
	}
	if (last_slash != NULL)
	{
		/* Determine whether all remaining characters are slashes.  */
		char* runp;
		for (runp = last_slash; runp != path; --runp)
			if (runp[-1] != '/')
				break;
		/* Terminate the path.  */
		if (runp == path)
		{
			/* The last slash is the first character in the string.  We have to
			   return "/".  As a special case we have to return "//" if there
			   are exactly two slashes at the beginning of the string.  See
			   XBD 4.10 Path Name Resolution for more information.  */
			if (last_slash == path + 1)
				++last_slash;
			else
				last_slash = path + 1;
		}
		else
			last_slash = runp;
		last_slash[0] = '\0';
	}
	else
		/* This assignment is ill-designed but the XPG specs require to
		   return a string containing "." in any case no directory part is
		   found and so a static and constant string is required.  */
		path = (char*)dot;
	return path;
}


char* basename(const char* filename)
{
	char* p = (char*)strrchr(filename, '/');
	return p ? p + 1 : (char*)filename;
}

/* Open a single file given its path */
_TINYDIR_FUNC
int tinydir_file_open(tinydir_file *file, const _tinydir_char_t *path)
{
	tinydir_dir dir;
	int result = 0;
	int found = 0;
	_tinydir_char_t dir_name_buf[_TINYDIR_PATH_MAX];
	_tinydir_char_t file_name_buf[_TINYDIR_FILENAME_MAX];
	_tinydir_char_t *dir_name;
	_tinydir_char_t *base_name;

	if (file == NULL || path == NULL || _tinydir_strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (_tinydir_strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* Get the parent path */
	_tinydir_strcpy(dir_name_buf, path);
	dir_name = dirname(dir_name_buf);
	_tinydir_strcpy(file_name_buf, path);
	base_name = basename(file_name_buf);

	if ((_tinydir_strcmp(base_name, TINYDIR_STRING("/"))) == 0)
	{
		memset(file, 0, sizeof * file);
		file->is_dir = 1;
		file->is_reg = 0;
		_tinydir_strcpy(file->path, dir_name);
		file->extension = file->path + _tinydir_strlen(file->path);
		return 0;
	}

	/* Open the parent directory */
	if (tinydir_open(&dir, dir_name) == -1)
	{
		return -1;
	}

	/* Read through the parent directory and look for the file */
	while (dir.has_next)
	{
		if (tinydir_readfile(&dir, file) == -1)
		{
			result = -1;
			goto bail;
		}
		if (_tinydir_strcmp(file->name, base_name) == 0)
		{
			/* File found */
			found = 1;
			break;
		}
		tinydir_next(&dir);
	}
	if (!found)
	{
		result = -1;
		errno = ENOENT;
	}

bail:
	tinydir_close(&dir);
	return result;
}

_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file)
{
	_tinydir_char_t *period = (char*)_tinydir_strrchr(file->name, TINYDIR_STRING('.'));
	if (period == NULL)
	{
		file->extension = &(file->name[_tinydir_strlen(file->name)]);
	}
	else
	{
		file->extension = period + 1;
	}
}

_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b)
{
	const tinydir_file *fa = (const tinydir_file *)a;
	const tinydir_file *fb = (const tinydir_file *)b;
	if (fa->is_dir != fb->is_dir)
	{
		return -(fa->is_dir - fb->is_dir);
	}
	return _tinydir_strncmp(fa->name, fb->name, _TINYDIR_FILENAME_MAX);
}

#ifndef _TINYDIR_USE_READDIR
/*
The following authored by Ben Hutchings <ben@decadent.org.uk>
from https://womble.decadent.org.uk/readdir_r-advisory.html
*/
/* Calculate the required buffer size (in bytes) for directory      *
* entries read from the given directory handle.  Return -1 if this  *
* this cannot be done.                                              *
*                                                                   *
* This code does not trust values of NAME_MAX that are less than    *
* 255, since some systems (including at least HP-UX) incorrectly    *
* define it to be a smaller value.                                  */
_TINYDIR_FUNC
size_t _tinydir_dirent_buf_size(_TINYDIR_DIR *dirp)
{
	long name_max;
	size_t name_end;
	/* parameter may be unused */
	(void)dirp;

#if defined _TINYDIR_USE_FPATHCONF
	name_max = fpathconf(dirfd(dirp), _PC_NAME_MAX);
	if (name_max == -1)
#if defined(NAME_MAX)
		name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
#else
		return (size_t)(-1);
#endif
#elif defined(NAME_MAX)
 	name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
#else
#error "buffer size for readdir_r cannot be determined"
#endif
	name_end = (size_t)offsetof(struct _tinydir_dirent, d_name) + name_max + 1;
	return (name_end > sizeof(struct _tinydir_dirent) ?
		name_end : sizeof(struct _tinydir_dirent));
}
#endif

#ifdef __cplusplus
}
#endif


#endif
