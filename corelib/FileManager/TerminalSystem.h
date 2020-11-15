#pragma once
#include "FileSysAdaptor.h"

class TerminalSystem : public I_FileSystem
{
public:
	TerminalSystem(char* fileSysName);
	~TerminalSystem();

	virtual bool Initialize(FILE_IO_INTERFACE* io_interface) override;

	virtual int Read(PFILE file, unsigned char* buffer, unsigned int size, int count) override;
	virtual bool Close(PFILE file)  override;
	virtual PFILE Open(const char* FileName, const char* mode)  override;
	virtual size_t Write(PFILE file, unsigned char* buffer, unsigned int size, int count) override;

	virtual int feof(FILE* stream) override;
	virtual int fseek(FILE* stream, long int offset, int whence) override;
	virtual long int ftell(FILE* stream) override;
	virtual int fgetc(FILE* stream) override;
	virtual int fputs(char const* _Buffer, FILE* _Stream) override;
	virtual int fputc(int character, FILE* stream) override;
	virtual int chdir(const char* dirname) override;
	virtual char* fgets(char* dst, int max, FILE* fp) override;
	virtual int ferror(FILE* stream) override;
	virtual int fflush(FILE* stream) override;
	virtual FILE* freopen(const char* filename, const char* mode, FILE* stream) override;
	virtual char* strerror(int errnum) override;

	virtual int ftruncate(FILE* fp) override;
	virtual void rewind(FILE* stream) override;

	virtual int rmdir(const char* pathname) override;
	virtual int mkdir(const char* pathname) override;
	virtual int unlink(const char* pathname) override;
	virtual int fprintf(FILE* stream, const char* buf, va_list args) override;
	virtual int rename(const char* path_old, const char* path_new) override;
	virtual int fstat(char const* const fileName, struct stat* fno) override;
	virtual int opendir(DIR* dir) override;
	virtual struct dirent* readdir(DIR* dir) override;
	virtual int closedir(DIR* dir) override;
};

