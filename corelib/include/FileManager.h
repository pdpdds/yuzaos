#pragma once
#include <minwindef.h>
#include "dirent.h"
#include "FileSysAdaptor.h"

//저장장치는 최대 26개
#define STORAGE_DEVICE_MAX 26

#ifdef DLL_EXPORT
#define DLL_API   __declspec( dllexport ) 
#else
#define DLL_API   __declspec( dllimport ) 
#endif

class I_FileManager
{
public:
	//파일 메소드
	virtual PFILE OpenFile(const char* fname, const char* mode) = 0;
	virtual int ReadFile(PFILE file, unsigned char* Buffer, unsigned int size, int count) = 0;
	virtual int WriteFile(PFILE file, unsigned char* Buffer, unsigned int size, int count) = 0;
	virtual bool CloseFile(PFILE file) = 0;

	virtual int feof(FILE* stream) = 0;
	virtual int fseek(FILE* stream, long int offset, int whence) = 0;
	virtual long int ftell(FILE* stream) = 0;
	virtual int fgetc(FILE* stream) = 0;
	virtual int fputs(char const* _Buffer, FILE* _Stream) = 0;
	virtual int fputc(int character, FILE* stream) = 0;
	virtual int chdir(const char* dirname) = 0;
	virtual char* fgets(char* dst, int max, FILE* fp) = 0;
	virtual int ferror(FILE* stream) = 0;
	virtual int fflush(FILE* stream) = 0;
	virtual FILE* freopen(const char* filename, const char* mode, FILE* stream) = 0;

	virtual int ftruncate(FILE* fp) = 0;
	virtual void rewind(FILE* stream) = 0;
	virtual int rmdir(const char* pathname) = 0;
	virtual int mkdir(const char* pathname) = 0;
	virtual int unlink(const char* pathname) = 0;
	virtual int fprintf(FILE* stream, const char* buf) = 0;
	virtual int rename(const char* path_old, const char* path_new) = 0;

	virtual void rewinddir(DIR* dp) = 0;
	virtual DIR* opendir(const char* name) = 0;
	virtual struct dirent* readdir(DIR* dir) = 0;
	virtual int closedir(DIR* dir) = 0;
	//virtual int utime(int drive, const char* filename, FILINFO* fno) = 0;
	virtual int fstat(char const* const fileName, struct stat* fno) = 0;

	virtual bool AddFileSystem(FileSysAdaptor* pAdaptor, I_FileSystem* pFileSystem, char drive, void* arg = nullptr) = 0;
	virtual bool AddFileSystem(FileSysAdaptor* pAdaptor, char* fileSystemName, char drive, void* arg = nullptr) = 0;
	virtual bool DriveExist(char drive) = 0;
};

class FileManager : public I_FileManager
{
public:
	FileManager();
	~FileManager();

	//파일 메소드
	PFILE OpenFile(const char* fname, const char *mode);
	int ReadFile(PFILE file, unsigned char* Buffer, unsigned int size, int count);
	int WriteFile(PFILE file, unsigned char* Buffer, unsigned int size, int count);
	bool CloseFile(PFILE file);

	int feof(FILE *stream);
	int fseek(FILE *stream, long int offset, int whence);
	long int ftell(FILE *stream);
	int fgetc(FILE * stream);
	int fputs(char const* pBuffer, FILE* stream);
	int fputc(int character, FILE * stream);
	int chdir(const char *dirname);
	char* fgets(char *dst, int max, FILE *fp);
	int ferror(FILE *stream);
	int fflush(FILE *stream);
	FILE *freopen(const char *filename, const char *mode, FILE *stream);
	
	int ftruncate(FILE* fp);
	void rewind(FILE *stream);
	int rmdir(const char *pathname);
	int mkdir(const char *pathname);
	int unlink(const char *pathname);

	int fprintf(FILE * stream, const char* buf);
	int rename(const char* path_old, const char* path_new);
	
	//int utime(const char *filename, FILINFO* fno);
	int fstat(char const* const fileName, struct stat* fno);

	void rewinddir(DIR* dp);
	DIR* opendir(const char* name);
	struct dirent* readdir(DIR* dir);
	int closedir(DIR* dir);

	I_FileSystem* CreateFileSystem(char* fileSystemName);
	bool AddFileSystem(FileSysAdaptor* pAdaptor, char* fileSystemName, char drive, void* arg = nullptr);
	bool AddFileSystem(FileSysAdaptor* pAdaptor, I_FileSystem* pFileSystem, char drive, void* arg = nullptr);
	bool DriveExist(char drive);

protected:
	FileSysAdaptor* GetAdaptorFromDrive(int id)
	{
		if (id < 0 || id >= STORAGE_DEVICE_MAX)
			return 0;

		return m_fileSystems[id];
	}

	bool RegisterFileSystem(FileSysAdaptor* fsys, char drive);
	bool UnregisterFileSystem(FileSysAdaptor* fsys);
	bool UnregisterFileSystemByID(char deviceID);

	I_FileSystem* GetFileSystem(FILE* stream);
		
	bool MakeFullPath(char* fullpath, char* fname, int size);

	bool IsTerminal(FILE* stream);

private:
	FileSysAdaptor* m_fileSystems[STORAGE_DEVICE_MAX];
	int m_stroageCount;
	I_FileSystem* m_pTerminalSystem;
};