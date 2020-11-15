#include "TerminalSystem.h"
#include <SystemCall_Impl.h>
#include <minwindef.h>

TerminalSystem::TerminalSystem(char* fileSysName)
	: I_FileSystem(fileSysName)
{

}

TerminalSystem::~TerminalSystem()
{

}

bool TerminalSystem::Initialize(FILE_IO_INTERFACE* io_interface)
{
	return true;

}

int TerminalSystem::chdir(const char* dirname)
{
	return 0;
}

int TerminalSystem::ferror(FILE* stream)
{
	return 0;
}

int TerminalSystem::fflush(FILE* stream)
{
	return 1;
}


FILE* TerminalSystem::freopen(const char* filename, const char* mode, FILE* stream)
{
	return 0;
}

char* TerminalSystem::strerror(int errnum)
{
	return 0;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int TerminalSystem::ftruncate(FILE* fp)
{
	return 0;
}

void TerminalSystem::rewind(FILE* stream)
{

}

int TerminalSystem::rmdir(const char* pathname)
{
	return 0;
}

int TerminalSystem::mkdir(const char* pathname)
{
	return 0;
}

int TerminalSystem::unlink(const char* pathname)
{
	return 0;
}

int TerminalSystem::fstat(char const* const fileName, struct stat* fno)
{
	return 0;
}

int TerminalSystem::opendir(DIR* dp)
{
	return 1;
}

int TerminalSystem::closedir(DIR* dir)
{
	return 0;
}

int TerminalSystem::rename(const char* path_old, const char* path_new)
{
	return 0;
}

struct dirent* TerminalSystem::readdir(DIR* dir)
{
	return 0;
}


int TerminalSystem::feof(FILE* stream)
{
	return 0;
}

long int TerminalSystem::ftell(FILE* stream)
{
	return 0;
}

int TerminalSystem::fseek(FILE* stream, long int offset, int whence)
{
	return 0;
}

bool TerminalSystem::Close(PFILE file)
{
	return false;
}

PFILE TerminalSystem::Open(const char* fileName, const char* mode)
{
	return nullptr;
}

int TerminalSystem::Read(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	int readCount = Syscall_GetCommandFromKeyboard(buffer, size * count);

	return readCount;
}

size_t TerminalSystem::Write(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (size * count <= 0)
		return 0;

	char* buf = new char[size * count + 1];
	memset(buf, 0, size * count);
	memcpy(buf, buffer, size * count);
	buf[size * count] = 0;
	printf((const char*)buf);
	free(buf);
	return size * count;
}

int TerminalSystem::fputs(char const* buffer, FILE* _Stream)
{
	printf((const char*)buffer);
	return strlen((const char*)buffer);
}

int TerminalSystem::fputc(int character, FILE* stream)
{
	printf("%c", character);
	return 1;
}

int TerminalSystem::fprintf(FILE* stream, const char* buf, va_list args)
{
	return vprintf((const char*)buf, args);
}

char* TerminalSystem::fgets(char* dst, int size, FILE* fp)
{
	int readCount = Read(fp, (unsigned char*)dst, size, 1);

	return dst;
}

int TerminalSystem::fgetc(FILE* stream)
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