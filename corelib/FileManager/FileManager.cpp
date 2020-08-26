#include "FileManager.h"
#include <memory.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <systemcall_impl.h>
#include <errno.h>
#include "TerminalSystem.h"

typedef I_FileSystem* (*PFileSystem)();
I_FileManager* g_pFileManager;
int errno = 0;

extern "C" __declspec(dllexport) I_FileManager* CreateFileManager()
{
	g_pFileManager = new FileManager();
	return g_pFileManager;
}

FileManager::FileManager()
{
	memset(m_fileSystems, 0, sizeof(FileSysAdaptor*) * STORAGE_DEVICE_MAX);
	m_stroageCount = 0;

	m_pTerminalSystem = new TerminalSystem("terminal");
}

FileManager::~FileManager()
{
}

bool FileManager::MakeFullPath(char* fullpath, char* fname, int size)
{
	if (fname[0] == '\\') //this mean full path
		return false;

	if (fname[1] == ':') //this mean full path
		return false;

	GetCurrentDirectory(MAX_PATH, fullpath);

	if (strlen(fullpath) == 1)
	{
		return false;
	}
	int len = strlen(fullpath);

	if(fullpath[len - 1] != '\\')
		strcat(fullpath, "\\");
	
	strcat(fullpath, fname);

	return true;
}

//파일 메소드
PFILE FileManager::OpenFile(const char* fname, const char *mode)
{
	I_FileSystem* pFileSys = GetFileSystem(0);
	
	if (pFileSys == 0 || strlen(fname) == 0)
	{
		printf("FileManager::OpenFile %d %d\n", pFileSys, strlen(fname));
		return 0;
	}
	PFILE fp = 0;
	char fullPath[MAX_PATH];

	bool result = MakeFullPath(fullPath, (char*)fname, MAX_PATH);

	if (result == true)
	{
		fp = pFileSys->Open(fullPath, mode);

		if (fp)
			return fp;
	}

	return pFileSys->Open(fname, mode);
}

int FileManager::ReadFile(PFILE file, unsigned char* Buffer, unsigned int size, int count)
{
	I_FileSystem* pFileSys = GetFileSystem(file);

	if (file && pFileSys)
		return pFileSys->Read(file, Buffer, size, count);

	printf("fread stream is null\n");

	return 0;
}
int FileManager::WriteFile(PFILE file, unsigned char* Buffer, unsigned int size, int count)
{
	I_FileSystem* pFileSys = GetFileSystem(file);

	if (file && pFileSys)
		return pFileSys->Write(file, Buffer, size, count);

	printf("fwrite stream is null\n");

	return 0;
}

bool FileManager::CloseFile(PFILE file)
{
	I_FileSystem* pFileSys = GetFileSystem(file);

	if (file && pFileSys)
		return pFileSys->Close(file);

	printf("fclose stream is null\n");

	return 0;
}

int FileManager::feof(FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->feof(stream);

	printf("feof stream is null\n");

//파일의 끝에 도달하지 못했을 때 0을 반환한다.
//에러가 발생했으면 파일 끝에 도달했다고 생각하고
//0이 아닌 값을 리턴한다.
	return 1;
}

int FileManager::fseek(FILE *stream, long int offset, int whence)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->fseek(stream, offset, whence);

	printf("fseek stream is null\n");

	return 1;
}

long int FileManager::ftell(FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (pFileSys)
		return pFileSys->ftell(stream);

	printf("ftell stream is null\n");

	return 0;
}

bool FileManager::IsTerminal(FILE* stream)
{
	if ((strcmp("STDIN", stream->_name) == 0) || (strcmp("STDOUT", stream->_name) == 0) || (strcmp("STDERR", stream->_name) == 0))
		return true;

	return false;
}

I_FileSystem* FileManager::GetFileSystem(FILE* stream)
{
	if (stream && IsTerminal(stream))
	{
		return m_pTerminalSystem;
	}

	int drive = Syscall_GetCurrentDriveId();
	
	FileSysAdaptor* pAdaptor = GetAdaptorFromDrive(drive);

	if (pAdaptor)
		return pAdaptor->GetFileSystem();

	printf("pAdaptor is null %d\n", drive);

	return NULL;
}

int FileManager::fgetc(FILE * stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);
	
	if (stream && pFileSys)
		return pFileSys->fgetc(stream);

	printf("fgetc stream is null\n");
	return 0;
}

int FileManager::fputs(char const* pBuffer, FILE* stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	/*if (stream && strcmp(stream->_name, "STDOUT") == 0)
	{
		printf("%s", pBuffer);
		return sizeof(pBuffer);
	}*/

	if (stream && pFileSys)
		return pFileSys->fputs(pBuffer, stream);

	printf("fputs stream is null\n");

	return 0;
}

int FileManager::fputc(int character, FILE* stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->fputc(character, stream);

	printf("fputc stream is null\n");

	return 0;
}

int FileManager::chdir(const char *dirname)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->chdir(dirname);

	return 0;
}


char* FileManager::fgets(char *dst, int max, FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->fgets(dst, max, stream);

	printf("fgets stream is null\n");

	return 0;
}

int FileManager::ferror(FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->ferror(stream);

	return 0;
}

int FileManager::fflush(FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (stream && pFileSys)
		return pFileSys->fflush(stream);

	return 0;
}


FILE *FileManager::freopen(const char *filename, const char *mode, FILE *stream)
{
	if (filename == 0 || stream == 0)
		return 0;

	fclose(stream);

	return fopen(filename, mode);
}

int FileManager::ftruncate(FILE* fp)
{
	I_FileSystem* pFileSys = GetFileSystem(fp);

	if (fp && pFileSys)
		return pFileSys->ftruncate(fp);

	return 0;
}

void FileManager::rewind(FILE *stream)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (pFileSys)
		pFileSys->rewind(stream);
}

void FileManager::rewinddir(DIR *dir)
{
	I_FileSystem* pFileSys = GetFileSystem(0);
	if (dir && dir->handle && pFileSys)
	{
		closedir(dir);
		int result = pFileSys->opendir(dir);
		
		if (result == 0)
		{
			memset(dir->result.d_name, 0, MAX_PATH);
		}
	}
	else
	{
		errno = EBADF;
	}
}

int FileManager::closedir(DIR* dir)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->closedir(dir);

	return -1;
}

int FileManager::rmdir(const char *pathname)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->rmdir(pathname);

	return 0;
}

int FileManager::mkdir(const char *pathname)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->mkdir(pathname);

	return 0;
}

int FileManager::unlink(const char *pathname)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->unlink(pathname);

	return 0;
}

int FileManager::fstat(char const* const fileName, struct stat* fno)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->fstat(fileName, fno);

	return -1;
}

/*int FileManager::utime(const char *filename, FILINFO* fno)
{
	int drive = 0;
	bool result = Syscall_SetCurrentDriveId(&drive);
	if(result == 0)
		return 0;
	FileSysAdaptor* pAdaptor = GetDrive(drive);

	if (pAdaptor)
		return pAdaptor->GetFileSystem()->utime(filename, fno);

	return 0;
}*/

DIR* FileManager::opendir(const char* name)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	DIR* dir = 0;

	if (name && name[0] && pFileSys)
	{
		size_t base_length = strlen(name);
		const char* all = /* search pattern must end with suitable wildcard */
			strchr("/\\", name[base_length - 1]) ? "*" : "/*";

		if ((dir = (DIR*)malloc(sizeof(DIR))) != 0 &&
			(dir->name = (char*)malloc(base_length + strlen(all) + 1)) != 0)
		{
			//20200131
			//strcat(strcpy(dir->name, name), all);
			strcpy(dir->name, name);
			int result = pFileSys->opendir(dir);

			if (result == 0)
			{
				memset(dir->result.d_name, 0, MAX_PATH);
			}
			else /* rollback */
			{
				free(dir->name);
				free(dir);
				dir = 0;
			}
		}
		else /* rollback */
		{
			free(dir);
			dir = 0;
			errno = ENOMEM;
		}
	}
	else
	{
		errno = EINVAL;
	}

	return dir;
}

struct dirent* FileManager::readdir(DIR* dir)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	struct dirent* entry = 0;

	if (dir && dir->handle && pFileSys)
	{
		if (!dir->result.d_name || pFileSys->readdir(dir) != 0)
		{
			entry = (struct dirent*)&dir->result;
			strcpy(entry->d_name, dir->info.cFileName);
		}
	}
	else
	{
		errno = EBADF;
	}

	return entry;
}

int FileManager::fprintf(FILE * stream, const char *buf)
{
	I_FileSystem* pFileSys = GetFileSystem(stream);

	if (pFileSys)
		return pFileSys->fprintf(stream, buf);

	return 0;
}

int FileManager::rename(const char* path_old, const char* path_new)
{
	I_FileSystem* pFileSys = GetFileSystem(0);

	if (pFileSys)
		return pFileSys->rename(path_old, path_new);

	return 0;
}

I_FileSystem* FileManager::CreateFileSystem(char* fileSystemName)
{
	HWND hwnd = 0;
	if (strcmp("FAT32", fileSystemName) == 0 || strcmp("FAT12", fileSystemName) == 0 || strcmp("FAT16", fileSystemName) == 0)
	{
		hwnd = (HWND)Syscall_LoadLibrary("FAT_FileSystem.dll");
	}
	else if (strcmp("MINT64", fileSystemName) == 0)
	{
		hwnd = (HWND)Syscall_LoadLibrary("MINT64_FileSystem.dll");
	}
	else if (strcmp("NATIVE", fileSystemName) == 0)
	{
		hwnd = (HWND)Syscall_LoadLibrary("Native_FileSystem.dll");
	}


	if (hwnd == 0)
		Syscall_Panic("FileManager::CreateFileSystem");
		
	PFileSystem GetFileSystem;
	GetFileSystem = (PFileSystem)Syscall_GetProcAddress(hwnd, "CreateFileSystem");
	I_FileSystem* pFileSystem = GetFileSystem();
	
	return pFileSystem;
}

bool FileManager::AddFileSystem(FileSysAdaptor* pAdaptor, char* fileSystemName, char drive, void* arg)
{
	I_FileSystem* pFileSystem = CreateFileSystem(fileSystemName);

	assert(pFileSystem != 0);

	bool result = pAdaptor->Initialize(pFileSystem, arg);
	if (false == result)
		return false;

	RegisterFileSystem(pAdaptor, drive);

	return true;
}

bool FileManager::AddFileSystem(FileSysAdaptor* pAdaptor, I_FileSystem* pFileSystem, char drive, void* arg)
{
	
	bool result = pAdaptor->Initialize(pFileSystem, arg);
	if (false == result)
		return false;

	RegisterFileSystem(pAdaptor, drive);

	return true;
}

bool FileManager::DriveExist(char drive)
{
	if (drive - 'A' < 0 || drive - 'A' >= 26)
		return false;

	if (m_fileSystems[drive - 'A'] == 0)
		return false;

	return true;
}

bool FileManager::RegisterFileSystem(FileSysAdaptor* fsys, char drive)
{
	if (!fsys)
		return false;

	char base_device_index = toupper(drive);
	base_device_index -= 'A';

	if (m_stroageCount < STORAGE_DEVICE_MAX && base_device_index >= 0 && base_device_index < STORAGE_DEVICE_MAX)
	{
		for(char i = base_device_index; i < STORAGE_DEVICE_MAX; i++)
		{
			if (m_fileSystems[i])
				continue;

			m_fileSystems[i] = fsys;
			fsys->m_deviceID = i;
			m_stroageCount++;
			return true;
		}
	}

	return false;
}

bool FileManager::UnregisterFileSystem(FileSysAdaptor* fsys)
{
	for (int i = 0; i < STORAGE_DEVICE_MAX; i++)
	{
		if (m_fileSystems[i] == fsys)
		{
			m_fileSystems[i] = nullptr;
			m_stroageCount--;
			return true;
		}
	}

	return false;
}

bool FileManager::UnregisterFileSystemByID(char drive)
{
	char deviceIndex = toupper(drive);
	deviceIndex -= 'A';
	if (deviceIndex < STORAGE_DEVICE_MAX)
	{
		if (m_fileSystems[deviceIndex] != nullptr)
		{
			m_fileSystems[deviceIndex] = nullptr;
			m_stroageCount--;
			return true;
		}
	}

	return false;
}