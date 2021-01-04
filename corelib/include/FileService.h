#pragma once
#include <va_list.h>
#include <size_t.h>
#include <errno.h>
//#include <stat_def.h>

#ifdef DLL_EXPORT
#define DLL_API   __declspec( dllexport ) 
#else
#define DLL_API   __declspec( dllimport ) 
#endif

#ifndef EOF
#define EOF (-1)
#endif

#define SEEK_SET        0               /* seek to an absolute position */
#define SEEK_CUR        1               /* seek relative to current position */
#define SEEK_END        2               /* seek relative to end of file */

//파일 플래그
#define FS_FILE       0
#define FS_DIRECTORY  1
#define FS_INVALID    2

#define FILENAME_MAX 256
#define MAXPATH      256
typedef struct _FILE 
{

	char     _name[MAXPATH]; //파일 이름
	unsigned int	 _flags; //플래그
	unsigned int    _handle; //핸들
	unsigned int    _currentCluster; //현재 클러스터 위치
	unsigned int    _deviceID; //디바이스 아이디
	unsigned int    _fileType; //파일타입.
	unsigned int    _referenceCount; //파일 참조 카운트.

}FILE, *PFILE;


#ifdef __cplusplus
extern "C" {
#endif
	typedef struct DIR DIR;

	DLL_API FILE* fopen(const char* filename, const char* mode);
	DLL_API size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	DLL_API size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
	DLL_API int fclose(FILE *stream);

	DLL_API long int ftell(FILE* stream);
	DLL_API int fseek(FILE* stream, long int offset, int whence);
	DLL_API int feof(FILE *stream);

	DLL_API FILE *freopen(const char *filename, const char *mode, FILE *stream);
	
	DLL_API int fgetc(FILE * stream);
	DLL_API int ungetc(int c, FILE *stream);
	DLL_API char* fgets(char *dst, int max, FILE *fp);
	DLL_API int fputs(char const* _Buffer, FILE* _Stream);
	DLL_API int fputc(int character, FILE * stream);
	
	DLL_API int fprintf(FILE * stream, const char * format, ...);
	DLL_API int vfprintf(FILE* stream, const char* format, va_list ap);

	DLL_API int fscanf(FILE* fp, const char* fmt, ...);

	DLL_API int ftruncate(int fildes, int length);
	
    DLL_API void rewind(FILE *stream); 
	DLL_API int rmdir(const char *pathname);
	DLL_API int mkdir(const char *pathname, int mode);
	DLL_API int unlink(const char *pathname);
	DLL_API int rename(const char* path_old, const char* path_new);
	DLL_API int chdir(const char* dirname);

	DLL_API int fflush(FILE* stream);
	DLL_API int fsync(FILE* fp);

	DLL_API DIR* opendir(const char*);
	DLL_API int  closedir(DIR*);
	DLL_API struct dirent* readdir(DIR*);
	DLL_API void  rewinddir(DIR*);
	DLL_API int stat(char const* const fileName, struct stat* fno);
	DLL_API int fstat(int fd, struct stat* buf);

	DLL_API int ferror(FILE* stream);
	DLL_API char* strerr(int errnum);
	DLL_API char* strerror(int errnum);
	DLL_API errno_t strerror_s(char* buffer, size_t sizeInBytes, int errnum);

	DLL_API int remove(const char* pathname);

	DLL_API int open(const char* filename, int flags, ...);
	DLL_API int creat(const char* filename, int mode);
	DLL_API FILE* fdopen(int fd, const char* mode);
	DLL_API ssize_t write(int fd, const void* buf, size_t count);
	DLL_API int dup(int fd);
	DLL_API int close(int fd);
	DLL_API size_t read(int fd, void* buf, size_t count);
	DLL_API int lseek(int fd, int offset, int whence);
	DLL_API int fileno(FILE* fp);
	DLL_API char* getcwd(char* buf, size_t size);
	DLL_API  unsigned int GetCurrentDirectory(unsigned int nBufferLength, char* lpBuffer);
	DLL_API  bool SetCurrentDriveId(char drive);
	DLL_API  bool SetCurrentDirectory(const char* lpPathName);
	
	//int utime(const char* filename, FILINFO* fno);
	//int scandir(const char* dirname, FILINFO*** namelist, int (*sdfilter)(FILINFO*), int (*dcomp)(const void*, const void*));
	 
#ifdef __cplusplus
}
#endif
