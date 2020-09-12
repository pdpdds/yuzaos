#include "MemoryResourceFS.h"
#include <string>
#include "ModuleManager.h"
#include "MultiBoot.h"
#include <SystemAPI.h>
#include <stat_def.h>
#include <GRUBManupulation.h>

typedef struct _MEMORY_FILE {

	DWORD    _fileLength; //파일 길이
	DWORD    _eof; //파일의 끝에 도달했는가
	DWORD    _position; // 현재 위치

	BootModule* pModule;

}MEMORY_FILE, * PMEMORY_FILE;

MemoryResourceFS::MemoryResourceFS(char* fileSysName)
	: I_FileSystem(fileSysName)
{

}

MemoryResourceFS::~MemoryResourceFS()
{

}

bool MemoryResourceFS::Initialize(FILE_IO_INTERFACE* io_interface)
{
	return true;
}


int MemoryResourceFS::feof(FILE* stream)
{
	MEMORY_FILE* pFile = (MEMORY_FILE*)stream->_handle;
	if (stream == 0)
	{
		kprintf("feof stream is null\n");
		return 1;
	}

	if (pFile->_eof != 0)
		return pFile->_eof;

	return 0;
}

//수정을 요함. 기능상으로만 동작하게 작업
int MemoryResourceFS::fseek(FILE* stream, long int offset, int whence)
{
	MEMORY_FILE* pFile = (MEMORY_FILE*)stream->_handle;
	if (SEEK_CUR == whence)
	{
		if (pFile->_position + offset > pFile->_fileLength)
			return -1;

		pFile->_eof = 0;
		pFile->_position += offset;
	}
	else if (SEEK_SET == whence)
	{
		if (offset < 0 || offset >(long int)pFile->_fileLength)
			return -1;

		pFile->_position = offset;

		if (pFile->_position < pFile->_fileLength)
			pFile->_eof = 0;
		else
			pFile->_eof = 1;
	}
	else if (SEEK_END == whence)
	{
		if (offset > 0 || (-offset) >= (long int)pFile->_fileLength)
			return -1;

		pFile->_position = pFile->_fileLength + offset;
		pFile->_eof = 1;
	}
	else
	{
		return -1;
	}

	return 0;
}

long int MemoryResourceFS::ftell(FILE* stream)
{
	MEMORY_FILE* pFile = (MEMORY_FILE*)stream->_handle;
	return (long int)pFile->_position;
}


int MemoryResourceFS::Read(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (file == nullptr)
		return false;

	MEMORY_FILE* pFile = (MEMORY_FILE*)file->_handle;

	int remain = pFile->_fileLength - pFile->_position;

	if (remain == 0)
	{
		pFile->_eof = 1;
		return 0;
	}

	int readCount = size * count;

	if (readCount > remain)
	{
		readCount = remain;
		pFile->_eof = 1;
	}
	BootModule* pModule = pFile->pModule;
	memcpy(buffer, ((char*)pModule->ModuleStart) + pFile->_position, readCount);

	//SkyConsole::Print("%c", buffer[0]);

	pFile->_position += readCount;

	return readCount;
}

bool MemoryResourceFS::Close(PFILE file)
{
	if (file == nullptr)
		return false;

	MEMORY_FILE* pFile = (MEMORY_FILE*)file->_handle;

	delete pFile;
	delete file;
	return true;
}

PFILE MemoryResourceFS::Open(const char* fileName, const char* mode)
{
	BootModule* pModule = FindFileFromMemory(fileName);

	if (pModule)
	{
		PFILE file = new FILE;
		file->_deviceID = 'L';
		strcpy(file->_name, fileName);
		file->_currentCluster = 0;
		file->_flags = FS_FILE;

		MEMORY_FILE* pMemoryModule = new MEMORY_FILE;
		pMemoryModule->_fileLength = pModule->ModuleEnd - pModule->ModuleStart;
		pMemoryModule->_eof = 0;
		pMemoryModule->_position = 0;
		pMemoryModule->pModule = pModule;
		file->_handle = (DWORD)pMemoryModule;

		return file;
	}

	return nullptr;
}

size_t MemoryResourceFS::Write(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	return 0;
}

int MemoryResourceFS::ferror(FILE* stream)
{
	return 0;
}

int MemoryResourceFS::fflush(FILE* stream)
{
	return 0;
}


FILE* MemoryResourceFS::freopen(const char* filename, const char* mode, FILE* stream)
{
	return 0;
}

char* MemoryResourceFS::strerror(int errnum)
{
	return 0;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int MemoryResourceFS::ftruncate(FILE* fp)
{
	return 0;
}

void MemoryResourceFS::rewind(FILE* stream)
{

}


int MemoryResourceFS::rmdir(const char* pathname)
{
	return 0;
}

int MemoryResourceFS::mkdir(const char* pathname)
{
	return 0;
}

int MemoryResourceFS::unlink(const char* pathname)
{
	return 0;
}

int MemoryResourceFS::fstat(char const* const fileName, struct stat* fno)
{
	fno->st_mode = 1;
	return 0;
}
/*
int MemoryResourceFS::utime(const char* filename, FILINFO* fno)
{
	return 0;
}

*/

struct dirent* MemoryResourceFS::readdir(DIR* dirp)
{
	return 0;
}


int MemoryResourceFS::opendir(DIR* dir)
{
	return 1;
}

int MemoryResourceFS::closedir(DIR* dir)
{
	return 1;
}

int MemoryResourceFS::fprintf(FILE* stream, const char* buf)
{
	return 0;
}


int MemoryResourceFS::rename(const char* path_old, const char* path_new)
{
	return 0;
}


int MemoryResourceFS::fgetc(FILE* stream)
{
	if (stream == 0)
	{
		printf("fgetc stream is null\n");
		return EOF;
	}

	char buf[2];
	int readCount = Read(stream, (unsigned char*)buf, 1, 1);

	if (readCount == 0)
		return EOF;

	return buf[0];
}

int MemoryResourceFS::fputs(char const* _Buffer, FILE* _Stream)
{
	return 0;
}

int MemoryResourceFS::fputc(int character, FILE* stream)
{
	return 0;
}

int MemoryResourceFS::chdir(const char* dirname)
{
	return 0;
}

char* MemoryResourceFS::fgets(char* dst, int max, FILE* fp)
{
	
	char temp;
	int count = 0;
	for (; count < max; count++)
	{
		temp = fgetc(fp);

		if (temp == EOF)
		{
			dst[count] = 0;
			break;
		}
		dst[count] = temp;
		if (temp == '\n')
		{
			dst[count + 1] = 0;
			break;
		}
	}

	if (count == 0)
		return 0;
	
	return dst;
}

/*int MemoryResourceFS::fscanf(FILE* stream, const char* format, ...)
{
	return 0;
}*/