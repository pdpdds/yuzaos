#include "MINT64_FileSystem.h"
#include "FileSystem.h"
#include <string>
#include <SystemCall_Impl.h>
#include <yuza_file_io.h>

extern "C" __declspec(dllexport) I_FileSystem * CreateFileSystem()
{
	return new MINT64_FileSystem("MINT64");
}


FILE_IO_INTERFACE g_io_interface;

extern FILESYSTEMMANAGER   gs_stFileSystemManager;
extern BYTE gs_vbTempBuffer[FILESYSTEM_SECTORSPERCLUSTER * 512];
extern DWORD g_mutex;
HDDINFORMATION g_hdd_Information;

MINT64_FileSystem::MINT64_FileSystem(char* fileSysName)
	: I_FileSystem(fileSysName)
{
	
}

MINT64_FileSystem::~MINT64_FileSystem()
{
	
}

extern fReadHDDInformation gs_pfReadHDDInformation;
extern fReadHDDSector gs_pfReadHDDSector;
extern fWriteHDDSector gs_pfWriteHDDSector;

bool ReadHDDInformation(bool bPrimary, bool bMaster, HDDINFORMATION* pstHDDInformation)
{
	g_io_interface.sky_disk_info(pstHDDInformation);
	return true;
}

int ReadHDDSector(bool bPrimary, bool bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer)
{
	return g_io_interface.sky_disk_read((BYTE*)pcBuffer, dwLBA, iSectorCount) == 0;
}

int WriteHDDSector(bool bPrimary, bool bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer)
{
	return g_io_interface.sky_disk_write((BYTE*)pcBuffer, dwLBA, iSectorCount) == 0;
}

bool MINT64_FileSystem::Initialize(FILE_IO_INTERFACE* io_interface)
{
	g_io_interface = *io_interface;
	
	if (0 != g_io_interface.sky_disk_initialize())
		return false;

	// 초기화가 성공하면 함수 포인터를 램 디스크용 함수로 설정
	gs_pfReadHDDInformation = ReadHDDInformation;
	gs_pfReadHDDSector = ReadHDDSector;
	gs_pfWriteHDDSector = WriteHDDSector;

	// 램 디스크는 데이터가 남아있지 않으므로 매번 파일 시스템을 생성함
	if (Format() == false)
	{
		return false;
	}

	// 파일 시스템 연결
	if (Mount() == false)
	{
		return false;
	}

	// 핸들을 위한 공간을 할당
	gs_stFileSystemManager.pstHandlePool = (MFILE*) new char[FILESYSTEM_HANDLE_MAXCOUNT * sizeof(MFILE)];
	memset(gs_stFileSystemManager.pstHandlePool, 0, FILESYSTEM_HANDLE_MAXCOUNT * sizeof(MFILE));
	// 메모리 할당이 실패하면 하드 디스크가 인식되지 않은 것으로 설정
	if (gs_stFileSystemManager.pstHandlePool == nullptr)
	{
		gs_stFileSystemManager.bMounted = false;
		return false;
	}

	//동기화를 위한 뮤텍스 생성
	g_mutex = Syscall_CreateMutex("Mint64Mutex");

	GetHDDInformation(&g_hdd_Information);

	PrintRamDiskInfo();

	return true;
	
}

int MINT64_FileSystem::fgetc(FILE* stream)
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

int MINT64_FileSystem::fputs(char const* _Buffer, FILE* _Stream)
{
	return 0;
}

int MINT64_FileSystem::fputc(int character, FILE* stream)
{
	return 0;
}

int MINT64_FileSystem::chdir(const char* dirname)
{
	return 0;
}


char* MINT64_FileSystem::fgets(char* dst, int max, FILE* fp)
{
	int c = 0;
	char* p = nullptr;



	/* get max bytes or upto a newline */

	for (p = dst, max--; max > 0; max--) {
		if ((c = this->fgetc(fp)) == EOF)
		{
			break;
		}

		if (c == 0x0d) //carriage return
		{
			continue;
		}

		*p++ = c;
		if (c == 0x0a) //new line
		{
			break;
		}
	}
	*p = 0;
	//SkyConsole::Print("token %s\n", dst);
	if (p == dst || c == EOF)
		return NULL;
	return (dst);
}
/*extern "C" int vfscanf(FILE * stream, const char* format, va_list args);
int MINT64_FileSystem::fscanf(FILE* stream, const char* format, ...)
{
	va_list arg;
	int done;
	va_start(arg, format);
	done = vfscanf(stream, format, arg);
	va_end(arg);
	return done;
}*/

int MINT64_FileSystem::ferror(FILE* stream)
{
	return 0;
}

int MINT64_FileSystem::fflush(FILE* stream)
{
	return 0;
}


FILE* MINT64_FileSystem::freopen(const char* filename, const char* mode, FILE* stream)
{
	return 0;
}

char* MINT64_FileSystem::strerror(int errnum)
{
	return 0;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int MINT64_FileSystem::ftruncate(FILE* fp)
{
	return 0;
}

void MINT64_FileSystem::rewind(FILE* stream)
{

}

int MINT64_FileSystem::rmdir(const char* pathname)
{
	return 0;
}

int MINT64_FileSystem::mkdir(const char* pathname)
{
	return 0;
}

int MINT64_FileSystem::unlink(const char* pathname)
{
	return 0;
}

int MINT64_FileSystem::fstat(char const* const fileName, struct stat* fno)
{
	return 0;
}

int MINT64_FileSystem::opendir(DIR* dp)
{
	return 1;
}

int MINT64_FileSystem::closedir(DIR* dir)
{
	return 0;
}

int MINT64_FileSystem::fprintf(FILE* stream, const char* buf, va_list args)
{
	return 0;
}


int MINT64_FileSystem::rename(const char* path_old, const char* path_new)
{
	return 0;
}

struct dirent* MINT64_FileSystem::readdir(DIR* dir)
{
	return 0;
}


int MINT64_FileSystem::feof(FILE* stream)
{
	MFILE* pFile = (MFILE*)stream->_handle;
	FILEHANDLE handle = pFile->stFileHandle;

	if (handle.dwCurrentOffset == handle.dwFileSize)
		return 1;

	return 0;
}

long int MINT64_FileSystem::ftell(FILE* stream)
{
	MFILE* pFile = (MFILE*)stream->_handle;
	FILEHANDLE handle = pFile->stFileHandle;

	return handle.dwCurrentOffset;
}

int MINT64_FileSystem::fseek(FILE* stream, long int offset, int whence)
{
	MFILE* pFile = (MFILE*)stream->_handle;
	return kSeekFile(pFile, offset, whence);
}

int MINT64_FileSystem::Read(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (file == nullptr)
		return false;

	int readCount = kReadFile(buffer, size, count, (MFILE*)file->_handle);

	return readCount;
}

bool MINT64_FileSystem::Close(PFILE file)
{
	if (file == nullptr)
		return false;

	return (-1 != kCloseFile((MFILE*)file->_handle));
}

PFILE MINT64_FileSystem::Open(const char* fileName, const char* mode)
{
	MFILE* pMintFile = kOpenFile(fileName, mode);

	if (pMintFile)
	{
		PFILE file = new FILE;
		file->_deviceID = 'K';
		file->_flags = FS_FILE;
		strcpy(file->_name, fileName);
		file->_handle = (DWORD)pMintFile;
		return file;
	}

	return nullptr;
}

size_t MINT64_FileSystem::Write(PFILE file, unsigned char* buffer, unsigned int size, int count)
{
	if (file == nullptr)
		return 0;

	return kWriteFile(buffer, size, count, (MFILE*)file->_handle);
}

bool MINT64_FileSystem::GetFileList()
{
	DIRECTORYENTRY entry;
	bool result = true;
	int index = 0;
	while (result != false)
	{
		kGetDirectoryEntryData(index, &entry);

		if (result == true && entry.dwStartClusterIndex != 0)
		{
			printf(" %s\n", entry.vcFileName);
			index++;
		}
		else
		{
			result = false;
		}
	}

	return true;
}

void MINT64_FileSystem::PrintRamDiskInfo()
{
	printf("RamDisk Info\n");
	printf("Total Sectors : %d\n", g_hdd_Information.dwTotalSectors);
	printf("Serial Number : %s\n", g_hdd_Information.vwSerialNumber);
	printf("Model Number : %s\n", g_hdd_Information.vwModelNumber);
}













	


	
