#pragma once
#include <stdio.h>
#include <string.h>

//파일 시스템
class I_FileSystem;
struct DIR;
typedef struct tag_FILE_IO_INTERFACE FILE_IO_INTERFACE;

class FileSysAdaptor
{
public:
	FileSysAdaptor(char* deviceName)
	{
		m_pFileSystem = 0;
		
		strcpy(m_deviceName, deviceName);
		m_deviceID = 0xffffffff;
	}
	
	~FileSysAdaptor() {}

	virtual bool Initialize(I_FileSystem* pFileSystem, void* arg) = 0;
	I_FileSystem* GetFileSystem() { return m_pFileSystem; }
	
	char m_deviceName[MAXPATH];
	unsigned int m_deviceID;

protected:
	I_FileSystem* m_pFileSystem;
};

class I_FileSystem
{
public:
	I_FileSystem(char* fileSysName)
	{
		strcpy(m_fileSysName, fileSysName);
	}
	~I_FileSystem() {}

	virtual bool Initialize(FILE_IO_INTERFACE* io_interface) = 0;
	char* GetFileSystemName() 
	{
		return m_fileSysName;
	}

	virtual FILE* Open(const char* FileName, const char* mode) = 0;
	virtual int Read(PFILE file, unsigned char* buffer, unsigned int size, int count) = 0;
	virtual size_t Write(PFILE file, unsigned char* buffer, unsigned int size, int count) = 0;
	virtual bool Close(PFILE file) = 0;

	virtual int feof(FILE* stream) = 0;
	virtual long int ftell(FILE* stream) = 0;
	virtual int fseek(FILE* stream, long int offset, int whence) = 0;

	virtual int fgetc(FILE* stream) = 0;
	virtual int fputs(char const* _Buffer, FILE* _Stream) = 0;
	virtual int fputc(int character, FILE* stream) = 0;
	virtual int chdir(const char* dirname) = 0;
	virtual char* fgets(char* dst, int max, FILE* fp) = 0;
	virtual int ferror(FILE* stream) = 0;
	virtual int fflush(FILE* stream) = 0;
	virtual FILE* freopen(const char* filename, const char* mode, FILE* stream) = 0;
	virtual char* strerror(int errnum) = 0;

	virtual int ftruncate(FILE* fp) = 0;
	virtual void rewind(FILE* stream) = 0;
	
	virtual int rmdir(const char* pathname) = 0;
	virtual int mkdir(const char* pathname) = 0;
	virtual int unlink(const char* pathname) = 0;
	
	virtual int fprintf(FILE* stream, const char* buf, va_list args) = 0;
	virtual int rename(const char* path_old, const char* path_new) = 0;
	
	virtual int fstat(char const* const fileName, struct stat* fno) = 0;
	//virtual int utime(const char* filename, FILINFO* fno) = 0;
	
	virtual int opendir(DIR* dir) = 0;
	virtual int closedir(DIR* dir) = 0;
	virtual struct dirent* readdir(DIR* dir) = 0;

protected:

private:
	char m_fileSysName[MAXPATH];
	
};