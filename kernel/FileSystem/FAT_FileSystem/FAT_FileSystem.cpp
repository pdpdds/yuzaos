#include "FAT_FileSystem.h"
#include <minwindef.h>
#include <minwinbase.h>
#include "ff.h"
#include <yuza_file_io.h>
#include <diskio.h>
#include <stat_def.h>
#include <systemcall_impl.h>
#include <assert.h>
#include <dirent.h>

extern "C" __declspec(dllexport) I_FileSystem * CreateFileSystem()
{
	return new FAT_FileSystem("FAT");
}

char* rtrimslash(char* str) {
	char* p;
	p = strchr(str, '\0');
	while (--p >= str && (*p == '/' || *p == '\\')) {};
	p[1] = '\0';
	return str;
}

FILE_IO_INTERFACE m_io_interface;

extern "C" DSTATUS fat_disk_initialize()
{
	return m_io_interface.sky_disk_initialize();
}

extern "C" DRESULT fat_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	return (DRESULT)m_io_interface.sky_disk_read(buff, sector, count);
}

extern "C" DRESULT fat_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	return (DRESULT)m_io_interface.sky_disk_write(buff, sector, count);
}

extern "C" DSTATUS fat_disk_status()
{
	return (DRESULT)m_io_interface.sky_disk_status();
}

FAT_FileSystem::FAT_FileSystem(char* fileSysName)
	: I_FileSystem(fileSysName)
{
	m_fs = new FATFS;
}

FAT_FileSystem::~FAT_FileSystem()
{
	delete m_fs;
}

bool FAT_FileSystem::Initialize(FILE_IO_INTERFACE* io_interface)
{
	m_io_interface = *io_interface;
	FRESULT result = f_mount(m_fs, "", 1);

	if (result == FR_NO_FILESYSTEM)
	{

		BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
	/* Format the default drive with default parameters */
		result = f_mkfs("", FM_FAT32, 0, work, sizeof(work));

		if(result == FR_OK)
			result = f_mount(m_fs, "", 0);
	}

	return result == FR_OK;
}

int FAT_FileSystem::Read(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	UINT readCnt = 0;
	FIL* fp = (FIL*)(file->_handle);
	FRESULT result = f_read(fp, buffer, size * count, &readCnt);

	readCnt = readCnt / size;

	return readCnt;
}

bool FAT_FileSystem::Close(PFILE file)
{
	bool result = 0;
	if (file->_referenceCount > 0)
	{
		file->_referenceCount--;
	}

	if (file->_referenceCount == 0)
	{
		result = f_close((FIL*)(file->_handle));
		delete (FILE*)file->_handle;
		delete file;
		file = 0;
	}
		
	return result;
	
}

#define OPENMODE_NONE 0
#define OPENMODE_APPEND 10
#define OPENMODE_APPEND_OVERWRITE 11

PFILE FAT_FileSystem::Open(const char* fileName, const char* mode)
{
	int openmode = OPENMODE_NONE;
	BYTE _mode = FA_READ;
	if (strcmp(mode, "ab") == 0 || strcmp(mode, "a") == 0)
	{
		openmode = OPENMODE_APPEND;
		_mode = FA_OPEN_APPEND | FA_WRITE;
	}
	else if (strcmp(mode, "rb") == 0 || strcmp(mode, "r") == 0)
		_mode = FA_READ;
	else if (strcmp(mode, "wb") == 0 || strcmp(mode, "w") == 0)
		_mode = FA_WRITE | FA_CREATE_ALWAYS;
	else if (strcmp(mode, "rb+") == 0 || strcmp(mode, "r+") == 0 || strcmp(mode, "rw") == 0)
		_mode = FA_READ | FA_WRITE | FA_OPEN_EXISTING;
	else if (strcmp(mode, "wb+") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "rw+") == 0)
		_mode = FA_READ | FA_WRITE | FA_CREATE_ALWAYS;
	else if (strcmp(mode, "ab+") == 0 || strcmp(mode, "a+") == 0)
	{
		openmode = OPENMODE_APPEND_OVERWRITE;
		_mode = FA_OPEN_APPEND | FA_WRITE | FA_READ;
	}
	else
		return 0;

	FIL* fil = new FIL;
	FRESULT res = f_open(fil, fileName, _mode);

	if (res != FR_OK)
	{
		//printf("File Open Fail : %s %d\n", fileName, res);
		delete fil;
		return nullptr;
	}

	if (openmode == OPENMODE_APPEND_OVERWRITE || openmode == OPENMODE_APPEND)
	{
		res = f_lseek(fil, f_size(fil));
		if (res != FR_OK)
		{
			f_close(fil);
			delete fil;
			return 0;
		}
	}

	PFILE file = new FILE;
	file->_flags = FS_FILE;
	file->_deviceID = 'C';
	strcpy(file->_name, fileName);
	file->_handle = (DWORD)fil;
	file->_referenceCount = 1;

	//printf("FAT_FileSystem::Open Sucess %s %x\n", fileName, file);

	return file;
}

size_t FAT_FileSystem::Write(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	UINT writeCnt = 0;
	FIL* fp = (FIL*)(file->_handle);

	if (fp == 0)
	{
		printf("%s", buffer);
		return size * count;
	}

	FRESULT result = f_write(fp, buffer, size * count, &writeCnt);
	f_sync(fp);
	return writeCnt;
}

int FAT_FileSystem::feof(FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;
	return f_eof(fp);
}

int FAT_FileSystem::fseek(FILE* stream, long int offset, int whence)
{
	FIL* fp = (FIL*)stream->_handle;
	int fileSize = f_size(fp);
	int pos = f_tell(fp);

	if (SEEK_CUR == whence)
	{
		if (pos + offset > fileSize)
			return -7;

		pos += offset;

	}
	else if (SEEK_SET == whence)
	{
		/*if (offset < 0 || offset > fileSize)
		{
			printf("file size %d\n", fileSize);
			return -6;
		}*/
	
		pos = offset;

	}
	else if (SEEK_END == whence)
	{
		if (offset > 0 || (-offset) > fileSize)
			return -1;

		pos = fileSize + offset;

	}

	int res = f_lseek(fp, pos);

	assert(res == 0);

	return (res == 0) ? 0 : -1;
}

long int FAT_FileSystem::ftell(FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;
	return f_tell(fp);
}

int FAT_FileSystem::fgetc(FILE* stream)
{
	char buf[2];
	int readCount = Read(stream, (unsigned char*)buf, 1, 1);

	if (readCount == 0)
		return EOF;

	return buf[0];
}

int FAT_FileSystem::fputs(char const* buffer, FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;

	if (fp == 0)
	{
		printf("%s", buffer);

		return 0;
	}

	return f_puts(buffer, fp);
}

int FAT_FileSystem::fputc(int character, FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;
	return f_putc(character, fp);
}


char* FAT_FileSystem::fgets(char* dst, int max, FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;

	if (fp == 0)
	{
		bool newline = Syscall_GetCommandFromKeyboard(dst, max);

		if (newline)
			return 0;

		return dst;
	}


	return f_gets(dst, max, fp);
}

char* FAT_FileSystem::strerror(int errnum)
{
	//not implemented
	return 0;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int FAT_FileSystem::ftruncate(FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;

	return f_truncate(fp);
}

void FAT_FileSystem::rewind(FILE* stream)
{
	FIL* fp = (FIL*)stream->_handle;
	f_rewind(fp);
}

int FAT_FileSystem::rmdir(const char* pathname)
{
	return f_rmdir(pathname);
}

int FAT_FileSystem::mkdir(const char* pathname)
{
	return f_mkdir(pathname);
}

int FAT_FileSystem::unlink(const char* pathname)
{
	return f_unlink(pathname);
}

int FAT_FileSystem::closedir(DIR* dir)
{
	_DIR* handle = (_DIR *)dir->handle;
	int result = f_closedir(handle);
	free(handle);
	return result;
}

int FAT_FileSystem::opendir(DIR* dir)
{
	char abolutePath[MAX_PATH] = "0:";
	strcat(abolutePath, dir->name);
	rtrimslash((char*)abolutePath);

	for (int i = 0; i < strlen(abolutePath); i++)
	{
		if (abolutePath[i] == '/')
		{
			abolutePath[i] = '\\';
		}
	}
	
	_DIR* dp = (_DIR*)malloc(sizeof(_DIR));
	int result = f_opendir(dp, abolutePath);

	if (result != 0)
	{
		free(dp);
		return result;
	}

	dir->handle = (handle_type)dp;

	return 0;
}


struct dirent* FAT_FileSystem::readdir(DIR* dir)
{
	FILINFO info;
	_DIR* directory = (_DIR *)dir->handle;
	FRESULT result = f_readdir(directory, &info);

	if (directory->sect == 0)
		return 0;

	strcpy(dir->result.d_name, info.fname);
	strcpy(dir->info.cFileName, info.fname);

	if ((info.fattrib & AM_DIR) != 0)
		dir->result.dwAttribute = 0;
	else
		dir->result.dwAttribute = 1;

	dir->result.fsize = info.fsize;
	
	return &dir->result;
}

/*int FAT_FileSystem::utime(const char* filename, FILINFO* fno)
{
	return f_utime(filename, fno);
}*/

int GetAbsolutePathTest(char const* const fileName, struct stat* fno)
{
	char abolutePath[MAX_PATH] = "0:";
	char currentDir[MAX_PATH];

	if (fileName[0] == '.' && fileName[1] == '/')
	{
		Syscall_GetCurrentDirectory(MAX_PATH, currentDir);
		strcat(abolutePath, currentDir);
		strcat(abolutePath, fileName + 2);

	}
	else
	{
		strcat(abolutePath, fileName);
	}

	rtrimslash((char*)abolutePath);

	for (int i = 0; i < strlen(abolutePath); i++)
	{
		if (abolutePath[i] == '/')
		{
			abolutePath[i] = '\\';
		}
	}

	FILINFO info;

	int res = f_stat(abolutePath, &info);

	if (res == 0)
	{
		fno->st_size = info.fsize;
		fno->st_mtime = info.ftime;
		if (info.fattrib & AM_DIR)
			fno->st_mode = 0;
		else
			fno->st_mode = 1;
	}

	return res;
}

int GetRelativePathTest(char const* const fileName, struct stat* fno)
{
	char abolutePath[MAX_PATH] = "0:";
	char currentDir[MAX_PATH];

	if (fileName[0] == '/')
	{
		Syscall_GetCurrentDirectory(MAX_PATH, currentDir);
		strcat(abolutePath, currentDir);
		strcat(abolutePath, fileName + 1);

	}
	else
	{
		Syscall_GetCurrentDirectory(MAX_PATH, currentDir);
		strcat(abolutePath, currentDir);
		strcat(abolutePath, fileName);
	}

	rtrimslash((char*)abolutePath);

	for (int i = 0; i < strlen(abolutePath); i++)
	{
		if (abolutePath[i] == '/')
		{
			abolutePath[i] = '\\';
		}
	}

	FILINFO info;

	int res = f_stat(abolutePath, &info);

	if (res == 0)
	{
		fno->st_size = info.fsize;
		fno->st_mtime = info.ftime;
		if (info.fattrib & AM_DIR)
			fno->st_mode = 0;
		else
			fno->st_mode = 1;
	}

	return (res == 0) ? 0 : -1;
}

int FAT_FileSystem::fstat(char const* const fileName, struct stat* fno)
{
	if (GetAbsolutePathTest(fileName, fno) == 0)
		return 0;

	return GetRelativePathTest(fileName, fno);
}

int FAT_FileSystem::chdir(const char* dirname)
{
	return f_chdir(dirname);
}



int FAT_FileSystem::rename(const char* path_old, const char* path_new)
{
	return f_rename(path_old, path_new);
}

int FAT_FileSystem::fprintf(FILE* stream, const char* buf, va_list args)
{
	if (stream == 0)
	{
		printf(buf, args);
		return 0;
	}

	FIL* fp = (FIL*)stream->_handle;

	if (fp == 0)
	{
		printf(buf, args);
		return 0;
	}

	int done = f_vprintf(fp, buf, args);

	return done;
}

int FAT_FileSystem::ferror(FILE* stream)
{
	return 0;
}

int FAT_FileSystem::fflush(FILE* stream)
{
	return 0;
}


FILE* FAT_FileSystem::freopen(const char* filename, const char* mode, FILE* stream)
{
	//not implemented
	return 0;
}



