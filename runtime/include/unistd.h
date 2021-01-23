#pragma once
#include <stat_def.h>
#include <time.h>

#define R_OK 0
#define W_OK 1
#define X_OK 2
#define F_OK 3

#define	S_IRWXU	0000700			/* RWX mask for owner */
#define	S_IRUSR	0000400			/* R for owner */
#define	S_IWUSR	0000200			/* W for owner */
#define	S_IXUSR	0000100			/* X for owner */

#define	S_IRWXO	0000007			/* RWX mask for other */
#define	S_IROTH	0000004			/* R for other */
#define	S_IWOTH	0000002			/* W for other */
#define	S_IXOTH	0000001			/* X for other */

#define	S_IRWXG	0000070			/* RWX mask for group */
#define	S_IRGRP	0000040			/* R for group */
#define	S_IWGRP	0000020			/* W for group */
#define	S_IXGRP	0000010			/* X for group */

//2020820
//#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
//#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISDIR(m)	(m == 0)	/* directory */
#define	S_ISREG(m)	(m == 1)	/* regular file */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000)	/* fifo */
#ifndef _POSIX_SOURCE
#define	S_ISLNK(m)	((m & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	((m & 0170000) == 0140000)	/* socket */
#endif

#if !defined(mode_t)
typedef int mode_t;
#endif

#ifdef  __cplusplus
extern "C" {
#endif

	struct utimbuf
	{
		time_t actime;   // 접근시간
		time_t modtime;	 // 변경시간
	};	

	int access(const char* pathname, int mode);
	int lstat(const char* filename, struct stat* buf);
	int chmod(const char* filename, int pmode);
	unsigned int getpid();
	unsigned int getppid();
	int gethostname(char* name, size_t len);
	int sethostname(const char* name, size_t len);
	int utime(const char* filename, struct utimbuf* buf);
	void perror(const char* str);
	int futimes(int fd, const struct timeval* times);

	void* bsearch(const void* key, const void* base0,
		size_t nmemb, size_t size,
		int (*compar)(const void*, const void*));
	
#ifdef  __cplusplus
}
#endif