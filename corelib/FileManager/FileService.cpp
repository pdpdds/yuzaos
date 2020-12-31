#include "FileService.h"
#include <sprintf.h>
#include <stringdef.h>
#include <systemcall_impl.h>
#include <FileManager.h>
#include  <stat_def.h>
#include <cwalk.h>

extern I_FileManager* g_pFileManager;

////////////////////////////////////////////////////////////////////////
//파일, 디렉토리
extern "C" unsigned int GetCurrentDirectory(unsigned int nBufferLength, char* lpBuffer)
{
	return Syscall_GetCurrentDirectory(nBufferLength, lpBuffer);
}

extern "C" bool SetCurrentDriveId(char drive)
{
	if (!g_pFileManager->DriveExist(drive))
	{
		return false;
	}
	return Syscall_SetCurrentDriveId(drive);
}

extern "C" bool SetCurrentDirectory(const char* lpPathName)
{
	if (strlen(lpPathName) >= MAXPATH)
		return false;

	struct stat info;
	int result = g_pFileManager->fstat(lpPathName, &info);

	if (result == 0 && (info.st_mode == 0))
	{
		return Syscall_SetCurrentDirectory(lpPathName);
	}

	return false;
}

char* getcwd(char* buffer, size_t size)
{
	
	int len = 0;
	if (buffer == 0)
	{
		char* buf = (char*)malloc(256);

		len = Syscall_GetCurrentDirectory(256, buf);
		if (len != 0)
			return buf;

		free(buf);
		return 0;
	}
	else
	{
		len = Syscall_GetCurrentDirectory(size, buffer);
		if (len != 0)
			return buffer;
	}

	return 0;
}

//File Service Call Routine
FILE* fopen(const char* filename, const char* mode)
{
	
	FILE* fp = g_pFileManager->OpenFile(filename, mode);

	//printf("FileService fopen %s %x\n", filename, fp);

	return fp;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
	return g_pFileManager->ReadFile(stream, (unsigned char*)ptr, size, count);
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
	return g_pFileManager->WriteFile(stream, (unsigned char*)ptr, size, count);
}

int fclose(FILE* stream)
{
	return g_pFileManager->CloseFile(stream);
}

int feof(FILE* stream)
{
	return g_pFileManager->feof(stream);
}

int fseek(FILE* stream, long int offset, int whence)
{
	return g_pFileManager->fseek(stream, offset, whence);
}

long int ftell(FILE* stream)
{
	return g_pFileManager->ftell(stream);
}

int fgetc(FILE* stream)
{
	return g_pFileManager->fgetc(stream);
}

int fputs(char const* _Buffer, FILE* stream)
{
	return g_pFileManager->fputs(_Buffer, stream);
}

int fputc(int character, FILE* stream)
{
	return g_pFileManager->fputc(character, stream);
}

int chdir(const char* dirname)
{
	char szResult[FILENAME_MAX];
	cwk_path_normalize(dirname, szResult, sizeof(szResult));

	int result = g_pFileManager->chdir(szResult);

	if(result == 0)
		Syscall_SetCurrentDirectory(szResult);

	return result;
}


char* fgets(char* dst, int max, FILE* fp)
{
	return g_pFileManager->fgets(dst, max, fp);
}

int ferror(FILE* stream)
{
	return g_pFileManager->ferror(stream);
}

int fflush(FILE* stream)
{
	return g_pFileManager->fflush(stream);
}

FILE* freopen(const char* filename, const char* mode, FILE* stream)
{
	return g_pFileManager->freopen(filename, mode, stream);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int ftruncate(int fildes, int length)
{
	FILE* fp = g_pFileManager->GetFile(fildes);
	
	if (fp == 0)
		return -1;

	int curPos = g_pFileManager->ftell(fp);
	g_pFileManager->fseek(fp, 0, SEEK_END);

	int size = g_pFileManager->ftell(fp);
	g_pFileManager->fseek(fp, curPos, SEEK_SET);

	if (length == size)
		return 0;

	int res = 0;
	if (length < size)
	{
		g_pFileManager->fseek(fp, length, SEEK_SET);
		res = g_pFileManager->ftruncate(fp);
	}
	else
	{
		for (int i = 0; i < length - size; i++)
		{
			res = g_pFileManager->WriteFile(fp, (unsigned char*)"\0", 1, 1);

			if (res != 1)
				assert(0);
		}
	}
	

	return (res == 0) ? 0 : -1;
}

void rewind(FILE* stream)
{
	return g_pFileManager->rewind(stream);
}

int rmdir(const char* pathname)
{
	return g_pFileManager->rmdir(pathname);
}

int mkdir(const char* pathname, int mode)
{
	return g_pFileManager->mkdir(pathname);
}

int unlink(const char* pathname)
{
	return g_pFileManager->unlink(pathname);
}

int fstat(char const* const fileName, struct stat* fno)
{
	return g_pFileManager->fstat(fileName, fno);
}

extern "C" int remove(const char* pathname)
{
	return g_pFileManager->rmdir(pathname);
}

static char errorMsg[] = "error";
extern "C" char* strerror(int errnum)
{
	//not implemented
	return errorMsg;
}


int fprintf(FILE* stream, const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	int done = g_pFileManager->fprintf(stream, format, arglist);
	va_end(arglist);
	return done;
}

FILE* fdopen(int fd, const char* mode)
{
	FILE* fp = (FILE*)fd;
	
	return fp;
}

int vfprintf(FILE* stream, const char* format, va_list ap)
{
	int done = g_pFileManager->fprintf(stream, format, ap);
	return done;
}

int rename(const char* path_old, const char* path_new)
{
	return  g_pFileManager->rename(path_old, path_new);
}

struct dirent* readdir(DIR* dir)
{
	return g_pFileManager->readdir(dir);
}

void rewinddir(DIR* dir)
{
	g_pFileManager->rewinddir(dir);
}

DIR* opendir(const char* name)
{
	return g_pFileManager->opendir(name);
}

int closedir(DIR* dir)
{
	return  g_pFileManager->closedir(dir);
}

int fsync(FILE* fp)
{
	return 0;
}

extern "C" int vfscanf(FILE * stream, const char* format, va_list args);
int fscanf(FILE* stream, const char* format, ...)
{
	int done = 0;
	if (stream)
	{
		
		va_list arglist;
		va_start(arglist, format);
		done = vfscanf(stream, format, arglist);
		va_end(arg);
	}

	return done;
}

int ungetc(int c, FILE* stream)
{
	fseek(stream, -1, SEEK_CUR);
	//fputc(c, stream);
	//fseek(stream, -1, SEEK_CUR);

	return c;
}

/*int utime(const char *filename, FILINFO* fno)
{
	return  g_pFileManager->utime(filename, fno);
}*/
