#include <windef.h>
#include <minwinconst.h>
#include "winapi.h"
#include <stdio.h>
#include <time.h>
#include <WinError.h>
#include <iconv.h>
#include <systemcall_impl.h>
#include <atomic.h>
#include <dirent.h>
#include <string.h>
#include <winapi2.h>
#include <winnt.h>

HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, /*LPSECURITY_ATTRIBUTES*/void* lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (lpFileName == NULL)
	{
		printf("lpFileName == NULL\n");
		return NULL;
	}

	char mode[32];
	memset(mode, 0, 32);
	if (dwDesiredAccess == GENERIC_READ && dwCreationDisposition == OPEN_EXISTING)
		strcpy(mode, "rb");
	else if (dwDesiredAccess == GENERIC_WRITE && dwCreationDisposition == CREATE_ALWAYS)
		strcpy(mode, "wb");
	else if (dwDesiredAccess == GENERIC_WRITE && dwCreationDisposition == OPEN_ALWAYS)
		strcpy(mode, "wb");
	else if ((dwDesiredAccess == (GENERIC_WRITE | GENERIC_READ)) && (dwCreationDisposition == CREATE_ALWAYS))
		strcpy(mode, "wb+");
	else if ((dwDesiredAccess == (GENERIC_WRITE | GENERIC_READ)) && (dwCreationDisposition == OPEN_EXISTING))
		strcpy(mode, "rb+");
	else
	{
		return NULL;
	}

	FILE* fp = NULL;
	fp = fopen(lpFileName, mode);

	if (fp == NULL)
	{
		return NULL;
	}

	return fp;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	*lpNumberOfBytesWritten = 0;
	if (hFile == NULL)
	{
		return FALSE;
	}

	DWORD  nRead = fwrite(lpBuffer, 1, nNumberOfBytesToWrite, (FILE*)hFile);
	if (nRead <= 0)
	{
		return FALSE;
	}
	*lpNumberOfBytesWritten = nRead;
	return TRUE;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	*lpNumberOfBytesRead = 0;
	if (hFile == NULL)
	{
		return FALSE;
	}

	size_t nRead = fread(lpBuffer, 1, nNumberOfBytesToRead, (FILE*)hFile);

	if (nRead <= 0)
	{
		return FALSE;
	}
	*lpNumberOfBytesRead = nRead;
	return TRUE;
}

DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	if (hFile == NULL)
	{
		return -1;
	}

	if (dwMoveMethod == FILE_CURRENT)
	{
		fseek((FILE*)hFile, lDistanceToMove, SEEK_CUR);
		return ftell((FILE*)hFile);
	}
	else if (dwMoveMethod == FILE_BEGIN)
	{
		fseek((FILE*)hFile, lDistanceToMove, SEEK_SET);
		return ftell((FILE*)hFile);
	}

	return -1;
}

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	return 1;
}

BOOL apiCreateDirectory(const char* path, void* p)
{
	return FALSE;
}

WINBASEAPI DWORD WINAPI GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
	if (hFile == NULL)
	{
		lpFileSizeHigh = 0;
		return 0xFFFFFFFF;
	}

	fseek((FILE*)hFile, 0, SEEK_END);
	DWORD dwHigh = ftell((FILE*)hFile);
	*lpFileSizeHigh = dwHigh;
	fseek((FILE*)hFile, 0, SEEK_SET);
	return *lpFileSizeHigh;
}

BOOL WINAPI CloseHandle(HANDLE hObject)
{
	return Syscall_CloseHandle(hObject);
}

BOOL WINAPI CloseFile(HANDLE hObject)
{
	if (hObject)
	{
		fclose((FILE*)hObject);
		return TRUE;
	}
	return FALSE;
}

HANDLE WINAPI FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	if (!lpFileName)
	{
		Syscall_SetLastError(ERROR_INVALID_PARAMETER);
		return (HANDLE)ERROR_INVALID_PARAMETER;
	}
	if (NULL == lpFindFileData)
	{
		Syscall_SetLastError(ERROR_INVALID_PARAMETER);
		return (HANDLE)ERROR_INVALID_PARAMETER;
	}

	DIR* dir = opendir(lpFileName);
	if (dir != 0)
	{
		if (lpFindFileData)
		{
			FIND_FILE_HANDLE* pHandle = (FIND_FILE_HANDLE*)malloc(sizeof(FIND_FILE_HANDLE));
			if (pHandle)
			{
				memset(lpFindFileData, 0, sizeof(WIN32_FIND_DATA));
				pHandle->fp = (int)dir;

				strcpy(lpFindFileData->cFileName, "."); // fake first file name
				lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

				return (HANDLE)pHandle;
			}
		}
		closedir(dir);
	}


	Syscall_SetLastError(ERROR_FILE_NOT_FOUND);
	return INVALID_HANDLE_VALUE;
}


BOOL WINAPI FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
	FIND_FILE_HANDLE* pHandle = (FIND_FILE_HANDLE*)hFindFile;
	if (pHandle && lpFindFileData)
	{
		while (TRUE)
		{
			struct dirent* entry = readdir((DIR*)pHandle->fp);
			if (entry == NULL)
			{
				return FALSE;
			}

			if (strcmp(entry->d_name, ".") != 0)
			{
				lpFindFileData->dwFileAttributes = (entry->dwAttribute == 0) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

				DIR* dir = (DIR*)pHandle->fp;
				strcpy(lpFindFileData->cFileName, dir->name);
				strcat(lpFindFileData->cFileName, "\\");
				strcat(lpFindFileData->cFileName, entry->d_name);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL WINAPI FindClose(HANDLE hFindFile)
{
	if (hFindFile)
	{
		FIND_FILE_HANDLE* p = (FIND_FILE_HANDLE*)hFindFile;
		closedir((DIR*)p->fp);
		free(p);
		return TRUE;
	}
	return FALSE;
}

BOOL GetSystemTime(SYSTEMTIME* systime)
{
	tm timeInfo;
	Syscall_GetSystemTime(&timeInfo);

	systime->wYear = timeInfo.tm_year;
	systime->wMonth = timeInfo.tm_mon;
	systime->wDayOfWeek = timeInfo.tm_wday;
	systime->wDay = timeInfo.tm_mday;
	systime->wHour = timeInfo.tm_hour;
	systime->wMinute = timeInfo.tm_min;
	systime->wSecond = (int)timeInfo.tm_sec;
	systime->wMilliseconds = 0;

	return true;
}

int WideCharToMultiByte(
	UINT CodePage,
	DWORD dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cbMultiByte,
	LPCSTR lpDefaultChar,
	BOOL* lpUsedDefaultChar
	)
{
	if (lpWideCharStr == NULL) return 0;
	if (lpMultiByteStr == NULL && cbMultiByte != 0)return 0;

	int wclen;
	//if(cchWideChar == -1) 
	if (lpMultiByteStr == NULL && cbMultiByte == 0)
	{
		WCHAR* p = (WCHAR*)lpWideCharStr;
		wclen = 0;

		while (*p != 0x00)
		{
			p++;
			wclen++;
		}

		return wclen;
	}
	else
	{
		//solution 2
		cbMultiByte = cchWideChar;
		wclen = cchWideChar * 2;
		//printf("wclen=%d\n", wclen);
		BYTE* ttt = (BYTE*)lpWideCharStr;
		for (int i = 0; i < wclen; i++)
		{
			//printf("^^^^^^^^lpWideCharStr=%c",ttt[i]);
		}
		//printf("\n");

		iconv_t cd = iconv_open("UTF-8", "UNICODE");
		if ((iconv_t)-1 == cd)
		{
			printf("Donot support this convert!\n");
			return 0;
		}

		size_t stwclen = wclen;
		size_t stcbMultiByte = cbMultiByte;
		char* pCh = (char*)lpWideCharStr;
		//		printf("iconv before %d , %d\n" , wclen , cbMultiByte);		
		//		int retSize = iconv(cd,&pCh, (size_t*)&wclen, &lpMultiByteStr,(size_t*)&cbMultiByte);
		int retSize = iconv(cd, &pCh, (size_t*)&stwclen, &lpMultiByteStr, (size_t*)&stcbMultiByte);
		wclen = stwclen;
		cbMultiByte = stcbMultiByte;
		//		printf("iconv %d , %d , %d , %s\n" , retSize , wclen , cbMultiByte , lpMultiByteStr);
		//		int retSize = 0;
		//		memset(lpMultiByteStr , 'A' , 1);
		//		cbMultiByte = cchWideChar;
		if (-1 == retSize)
		{
			//perror("iconv");
			return 0;
		}
		iconv_close(cd);
		//printf("iconv successed! retSize = %d, lpMultiByteStr = %s\n", retSize,lpMultiByteStr);

		return cbMultiByte;

		//		wclen = cchWideChar;
		//		printf("wclen=%d,sizeof wchar_t=%d\n", wclen,sizeof(wchar_t));
		//                BYTE *ttt = (BYTE*)lpWideCharStr;
		//		for(int i=0;i<wclen;i+=2)
		//		{
		//		    printf("^^^^^^^^^^^^^^^^^^lpWideCharStr=%c",ttt[i]);
		//		}
		//printf("\n");
		//		int n = wcstombs(lpMultiByteStr,(wchar_t*)lpWideCharStr,wclen);
		//		if(-1 == n)
		//		{
		//			perror("WideCharToMultiByte error!");
		//			return 0;
		//		} 

		//		return n;
	}

	// 	CFStringRef cfstr = CFStringCreateWithBytes(NULL, (UInt8*)lpWideCharStr , wclen * sizeof(WCHAR), 
	// 											kCFStringEncodingUTF16LE,false);
	// 				
	// 
	// 	CFRange rangeToProcess = CFRangeMake(0, CFStringGetLength(cfstr));	
	// 	int TotalConvertlen = 0;
	// 	int TotalCpyLen = 0;
	// 	int TotalMBBufLen = cbMultiByte;
	// 		
	// 	while(rangeToProcess.length > 0)
	// 	{
	// 		UInt8 localBuffer[100];
	// 		CFIndex usedBufferLength;
	// 		CFIndex numChars;
	// 		
	// 		if(cbMultiByte != 0)
	// 		{
	// 			numChars = CFStringGetBytes(cfstr, rangeToProcess, 
	// 								CodePage, 
	// 								'?', FALSE, (UInt8 *)localBuffer, 100, &usedBufferLength);	
	// 		}
	// 		else
	// 		{
	// 			numChars = CFStringGetBytes(cfstr, rangeToProcess, 
	// 									CodePage, 
	// 									'?', FALSE, NULL, 0, &usedBufferLength);
	// 		}
	// 		
	// 		if (numChars == 0) break;   // Failed to convert anything...
	// 		rangeToProcess.location += numChars;
	// 		rangeToProcess.length -= numChars;		
	// 		
	// 		TotalConvertlen += numChars;
	// 		
	// 		if( TotalCpyLen + usedBufferLength <= TotalMBBufLen && cbMultiByte !=0)
	// 		{
	// 			memcpy(lpMultiByteStr+TotalCpyLen , localBuffer , usedBufferLength);
	// 			TotalCpyLen += usedBufferLength;	
	// 		}
	// 
	// 	}
	// 	
	// 	return TotalConvertlen;
	return 0;
}

int MultiByteToWideChar(
	UINT CodePage,
	DWORD dwFlags,
	LPCSTR lpMultiByteStr,
	int cbMultiByte,
	LPWSTR lpWideCharStr,
	int cchWideChar
	)
{
	if (lpMultiByteStr == NULL) return 0;
	if (lpWideCharStr == NULL && cchWideChar != 0)return 0;

	int mblen;
	//if(cbMultiByte == -1) 
	if (lpWideCharStr == NULL && cchWideChar == 0)
	{
		mblen = strlen(lpMultiByteStr);
		return mblen;
	}
	else
	{
		//if((cchWideChar-cbMultiByte)==1)
			//cbMultiByte = cchWideChar;
		mblen = (cbMultiByte + 1) * 2;
		int nLength = cbMultiByte;
		//printf("cbMultiByte=%d\n", cbMultiByte);
				//char* szTmp = new char[mblen];
				//memset(szTmp,0, mblen);

		iconv_t cd = iconv_open("UNICODE", "UTF-8");
		if ((iconv_t)-1 == cd)
		{
			printf("Don't support this convert!\n");
			return 0;
		}
		char* pCh = (char*)lpWideCharStr;
		size_t stcbMultiByte = cbMultiByte;
		size_t stmblen = mblen;

		int retSize = iconv(cd, (char**)&lpMultiByteStr, (size_t*)&stcbMultiByte, &pCh, (size_t*)&stmblen);
		cbMultiByte = stcbMultiByte;
		mblen = stmblen;
		//there are always tow more devil characters.
		//memcpy(szTmp,(char*)lpWideCharStr,mblen);
		//memset((char*)lpWideCharStr,0, mblen);
		//memcpy((char*)lpWideCharStr,szTmp+2, mblen-2);
		//delete szTmp;
		//szTmp = NULL;
		if (-1 == retSize)
		{
			//perror("eeeeeeeerrno iconv! MultiByteToWideChar\n");
			return 0;
		}
		iconv_close(cd);



		/*int m = mbstowcs(lpWideCharStr,lpMultiByteStr,mblen);
		if(m == -1)
		{
			 perror("MultiByteToWideChar error!");
			 return 0;
		}*/

		//printf("fffffffff return=%d\n", nLength-cbMultiByte);
		return nLength - cbMultiByte;
	}


	// 	CFStringRef cfstr = CFStringCreateWithBytes(NULL, (UInt8*)lpMultiByteStr , mblen, CodePage,false);
	// 	//CFStringRef cfstr = CFStringCreateWithCString(NULL,lpMultiByteStr,CodePage);
	// 	CFRange rangeToProcess = CFRangeMake(0, CFStringGetLength(cfstr));	
	// 	int TotalConvertlen = 0;
	// 	int TotalCpyLen = 0;
	// 	int TotalWCBufLen = cchWideChar * sizeof(WCHAR);
	// 	
	// 	
	// 	while(rangeToProcess.length > 0)
	// 	{
	// 		UInt8 localBuffer[100];
	// 		CFIndex usedBufferLength;
	// 		CFIndex numChars;
	// 		
	// 		if(cchWideChar != 0)
	// 		{
	// 			numChars = CFStringGetBytes(cfstr, rangeToProcess, 
	// 								kCFStringEncodingUTF16LE, 
	// 								'?', FALSE, (UInt8 *)localBuffer, 100, &usedBufferLength);	
	// 		}
	// 		else
	// 		{
	// 			numChars = CFStringGetBytes(cfstr, rangeToProcess, 
	// 									kCFStringEncodingUTF16LE, 
	// 									'?', FALSE, NULL, 0, &usedBufferLength);
	// 		}
	// 		
	// 		if (numChars == 0) break;   // Failed to convert anything...
	// 		rangeToProcess.location += numChars;
	// 		rangeToProcess.length -= numChars;		
	// 		
	// 		TotalConvertlen += numChars;
	// 		
	// 		if( TotalCpyLen + usedBufferLength <= TotalWCBufLen && cchWideChar !=0)
	// 		{
	// 			memcpy(lpWideCharStr+TotalCpyLen , localBuffer , usedBufferLength);
	// 			TotalCpyLen += usedBufferLength;	
	// 		}
	// 
	// 	}
	// 	
	// 	return TotalConvertlen;
	// 
		//return 0;
}

BOOL SystemTimeToFileTime(
	const SYSTEMTIME* lpSystemTime,
	LPFILETIME lpFileTime
	)
{
	// 	CFGregorianDate gmtDate;
	// 	gmtDate.year = lpSystemTime->wYear;
	// 	gmtDate.month = lpSystemTime->wMonth;
	// 	gmtDate.day = lpSystemTime->wDay;
	// 	gmtDate.hour = lpSystemTime->wHour;
	// 	gmtDate.minute = lpSystemTime->wMinute;
	// 	gmtDate.second = (double)lpSystemTime->wSecond + (double)lpSystemTime->wMilliseconds / (double)1000;
	// 
	// 	//CFTimeZoneRef tzGMT = CFTimeZoneCopySystem();//Time zone get from system
	// 	CFTimeZoneRef tzGMT = CFTimeZoneCreateWithName(kCFAllocatorDefault,CFSTR("Etc/GMT"),true);//Time Zone GMT+0
	// 	
	// 	CFAbsoluteTime absTime = CFGregorianDateGetAbsoluteTime(gmtDate,tzGMT);
	// 	
	// 	CFGregorianDate gmtDatepast;
	// 	gmtDatepast.year = 1601;
	// 	gmtDatepast.month = 1;
	// 	gmtDatepast.day = 1;
	// 	gmtDatepast.hour = 0;
	// 	gmtDatepast.minute = 0;
	// 	gmtDatepast.second = 0;	
	// 	CFAbsoluteTime absTimepast = CFGregorianDateGetAbsoluteTime(gmtDatepast,tzGMT);	
	// 	
	// 	//CFGregorianUnits unit = CFAbsoluteTimeGetDifferenceAsGregorianUnits(
	// 	//								absTime,absTimepast,tzGMT,kCFGregorianAllUnits);
	// 	double diff = absTime - absTimepast;
	// 	SInt64 nanosec = diff * 10000000;//100 nano seconds
	// 	
	//     DWORD dwLowDateTime;
	//     DWORD dwHighDateTime;
	// 	
	// 	dwLowDateTime = nanosec;
	// 	dwHighDateTime = nanosec >> 32;
	// 
	// 	lpFileTime->dwHighDateTime = dwHighDateTime;
	// 	lpFileTime->dwLowDateTime = dwLowDateTime;
	// 


	if (lpSystemTime == NULL)
	{
		return FALSE;
	}
	struct tm gm = { lpSystemTime->wSecond,
					lpSystemTime->wMinute,
					lpSystemTime->wHour,
					lpSystemTime->wDay,
					lpSystemTime->wMonth - 1,
					lpSystemTime->wYear - 1900,
					lpSystemTime->wDayOfWeek,
					0,
					0 };

	//20190514
	/*time_t tmt = mktime(&gm);
	ULONGLONG QuadPart = (ULONGLONG)tmt*(ULONGLONG)10000000+(ULONGLONG)116444736000000000;
	lpFileTime->dwLowDateTime = QuadPart;
	lpFileTime->dwLowDateTime += (DWORD)lpSystemTime->wMilliseconds*10000;//加上millisecondes
	lpFileTime->dwHighDateTime = QuadPart >> 32;
	*/

	return TRUE;
}

BOOL LocalFileTimeToFileTime(
	const FILETIME* lpLocalFileTime,
	LPFILETIME lpFileTime
	)
{
	// 	CFTimeZoneRef tzGMT0 = CFTimeZoneCreateWithName(kCFAllocatorDefault,CFSTR("Etc/GMT"),true);//Time Zone GMT+0
	// 	CFTimeZoneRef tzGMTSys = CFTimeZoneCopySystem();//Time zone get from system
	// 	
	// 	CFAbsoluteTime absTime_GMTSys = CFAbsoluteTimeGetCurrent(); 
	// 		
	// 	CFGregorianDate gmtDate = CFAbsoluteTimeGetGregorianDate(absTime_GMTSys, tzGMT0);
	// 	
	// 	CFAbsoluteTime absTime_GMT0 = CFGregorianDateGetAbsoluteTime(gmtDate,tzGMT0);
	// 	
	// 	double diff_sec = absTime_GMT0 - absTime_GMTSys;
	// 	SInt64 diff_nanosec = diff_sec * 10000000;//100 nano seconds
	// 	
	//     SInt64 localFileTime64 = lpLocalFileTime->dwHighDateTime;
	// 	localFileTime64 = localFileTime64 << 32;
	// 	localFileTime64 += lpLocalFileTime->dwLowDateTime;
	// 	
	// 	SInt64 toFileTime64 = localFileTime64 + diff_nanosec;
	// 	
	//     lpFileTime->dwHighDateTime = toFileTime64 >> 32;	
	// 	lpFileTime->dwLowDateTime = toFileTime64;	
	// 	

		//在UTC的情况下
	lpFileTime->dwHighDateTime = lpLocalFileTime->dwHighDateTime;
	lpFileTime->dwLowDateTime = lpLocalFileTime->dwLowDateTime;
	return TRUE;
}

HANDLE CreateThread(LPSECURITY_ATTRIBUTES   lpThreadAttributes, SIZE_T  dwStackSize, LPTHREAD_START_ROUTINE  lpStartAddress,
	LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	return (HANDLE)Syscall_CreateThread(lpStartAddress, "user_thread", lpParameter, 16, dwCreationFlags);
}

FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	return (FARPROC)Syscall_GetProcAddress(hModule, lpProcName);
}

__declspec(dllexport) HMODULE LoadLibrary(LPCSTR lpLibFileName)
{
	return (HMODULE)Syscall_LoadLibrary(lpLibFileName);
}

HANDLE GetCurrentThread(void)
{
	return (HANDLE)Syscall_GetCurrentThread();
}

DWORD _stdcall GetCurrentThreadId(void)
{
	return (DWORD)Syscall_GetCurrentThreadId();
}

DWORD SuspendThread(HANDLE hThread)
{
	return (DWORD)Syscall_SuspendThread(hThread);
}

DWORD ResumeThread(HANDLE hThread)
{
	return (DWORD)Syscall_ResumeThread(hThread);
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode)
{
	return (DWORD)Syscall_TerminateThread(hThread, &dwExitCode);
}

WINBASEAPI BOOL TerminateProcess(HANDLE hProcess, UINT   uExitCode)
{
	return FALSE;
}

//not implemented
void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
	memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
	lpSystemInfo->dwPageSize = 4096;
	lpSystemInfo->dwNumberOfProcessors = 1;
}

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName)
{
	return Syscall_CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
}

extern "C" void Sleep(DWORD dwMilliseconds)
{
	Syscall_Sleep(dwMilliseconds);
}

void SetLastError(DWORD dwErrCode)
{
	Syscall_SetLastError(dwErrCode);
}

bool FreeLibrary(HMODULE handle)
{
	return Syscall_FreeLibrary(handle);
}

bool SetEvent(HANDLE hEvent)
{
	return Syscall_SetEvent(hEvent);
}

bool ResetEvent(HANDLE hEvent)
{
	return Syscall_ResetEvent(hEvent);
}

bool PulseEvent(HANDLE hEvent)
{
	return false;
	//return Syscall_ResetEvent(hEvent);
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD  dwMilliseconds)
{
	return (DWORD)Syscall_WaitForSingleObject(hHandle, dwMilliseconds);
}

DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds)
{
	return Syscall_WaitForMultipleObjects(nCount, lpHandles, bWaitAll, dwMilliseconds);
}

HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName)
{
	return (HANDLE)Syscall_CreateSemaphore(lpName, lInitialCount);
}

HANDLE OpenSemaphore(DWORD  dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
	//Not implemented
	return 0;
}

DWORD AquireSemaphore(HANDLE handle, int timeout)
{
	return Syscall_AquireSemaphore(handle, timeout);
}

BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount)
{
	return Syscall_ReleaseSemaphore(hSemaphore, lReleaseCount);
}

bool SetThreadPriority(HANDLE hThread, int nPriority)
{
	return Syscall_SetThreadPriority(hThread, nPriority);
}

int GetThreadPriority(HANDLE hThread)
{
	return Syscall_GetThreadPriority(hThread);
}

DWORD_PTR SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask)
{
	return Syscall_SetThreadAffinityMask(hThread, dwThreadAffinityMask);
}

DWORD WINAPI GetLastError(void)
{
	return Syscall_GetLastError();
}

DWORD GetCurrentProcessId()
{
	return Syscall_GetCurrentProcessId();
}

//-----------------------------------------------------------------------------------
BOOL TlsSetValue(DWORD  dwTlsIndex, LPVOID lpTlsValue)
{
	return Syscall_TlsSetValue(dwTlsIndex, lpTlsValue);
}

LPVOID TlsGetValue(DWORD dwTlsIndex)
{
	return Syscall_TlsGetValue(dwTlsIndex);
}

DWORD TlsAlloc()
{
	return Syscall_TlsAlloc();
}

BOOL TlsFree(DWORD dwTlsIndex)
{
	return Syscall_TlsFree(dwTlsIndex);
}


UINT GetSystemDirectory(LPSTR lpBuffer, UINT  uSize)
{
	return 0;
}

BOOL GetProcessAffinityMask(HANDLE     hProcess, PDWORD_PTR lpProcessAffinityMask, PDWORD_PTR lpSystemAffinityMask)
{
	return FALSE;
}

HANDLE GetCurrentProcess()
{
	return 0;
}

BOOL DuplicateHandle(HANDLE   hSourceProcessHandle, HANDLE   hSourceHandle, HANDLE   hTargetProcessHandle,
	LPHANDLE lpTargetHandle, DWORD    dwDesiredAccess, BOOL     bInheritHandle, DWORD    dwOptions)
{
	return FALSE;
}

HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId)
{
	return 0;
}


LONG InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand)
{
	return _InterlockedCompareExchange(Destination, ExChange, Comperand);
}

LONG InterlockedIncrement(LONG volatile* Addend)
{
	return _InterlockedIncrement(Addend);
}

LONG InterlockedDecrement(LONG volatile* Addend)
{
	return _InterlockedDecrement(Addend);
}

LONG InterlockedExchange(LONG volatile* Target, LONG Value)
{
	return _InterlockedExchange(Target, Value);
}

PVOID InterlockedExchangePointer(PVOID volatile* Target, PVOID Value)
{
	return _InterlockedExchangePointer(Target, Value);
}

PVOID InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID Exchange, PVOID Comperand)
{
	return _InterlockedCompareExchangePointer(Destination, Exchange, Comperand);
}

LONG InterlockedExchangeAdd(LONG volatile* Addend, LONG Value)
{
	return _InterlockedExchangeAdd(Addend, Value);
}

LONG InterlockedAdd(LONG volatile* Addend, LONG Value)
{
	return _InterlockedExchangeAdd(Addend, Value);
}

LONG InterlockedAnd(LONG volatile* Destination, LONG Value)
{
	return _InterlockedAnd(Destination, Value);
}

LONG InterlockedOr(LONG volatile* Destination, LONG Value)
{
	return _InterlockedOr(Destination, Value);
}

LONG InterlockedXor(LONG volatile* Destination, LONG Value)
{
	return _InterlockedXor(Destination, Value);
}

uintptr_t _beginthreadex( // NATIVE CODE
	void* security,
	unsigned stack_size,
	unsigned(__stdcall* start_address)(void*),
	void* arglist,
	unsigned initflag,
	unsigned* thrdaddr
	)
{
	uintptr_t handle = Syscall_CreateThread(start_address, "beiginthreadex", arglist, 16, initflag);
	return handle;
}

void _endthreadex(unsigned retval)
{
	Syscall_ExitThread(retval);
}

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	Syscall_InitializeCriticalSection(lpCriticalSection);
}

void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	Syscall_DeleteCriticalSection(lpCriticalSection);
}

BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	return Syscall_TryEnterCriticalSection(lpCriticalSection);
}

void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	Syscall_EnterCriticalSection(lpCriticalSection);
}

void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	Syscall_LeaveCriticalSection(lpCriticalSection);
}

bool VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType)
{
	return Syscall_VirtualFree(lpAddress, dwSize, dwFreeType);
}

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect)
{
	return Syscall_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

bool VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect)
{
	return Syscall_VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
}

HANDLE SetTimer(HWND hWnd, DWORD nIDEvent, UINT nElapse, void (CALLBACK* lpfnTimer)(HWND, UINT, DWORD*, DWORD))
{
	return Syscall_SetTimer(hWnd, nIDEvent, nElapse, lpfnTimer);
}

bool KillTimer(HWND hWnd, DWORD* nIDEvent)
{
	return Syscall_KillTimer(hWnd, nIDEvent);
}

HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
{
	return (HANDLE)Syscall_CreateMutex(lpName);
}

BOOL ReleaseMutex(HANDLE hMutex)
{
	return CloseHandle(hMutex);
}

void WINAPI RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments)
{
	Syscall_RaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);
}

DWORD WINAPI GetTickCount()
{
	return Syscall_GetTickCount();
}

bool CreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId)
{
	return Syscall_CreateWindow(rect, title, flags, windowId);
}

bool DrawWindow(QWORD* windowId, char* buffer, RECT* rect)
{
	return Syscall_DrawWindow(windowId, buffer, rect);
}

bool DeleteWindow(QWORD* windowId)
{
	return Syscall_DeleteWindow(windowId);
}

bool ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent)
{
	return Syscall_ReceiveEventFromWindowQueue(windowId, pstEvent);
}

bool WINAPI ExitThread(int errorcode)
{
	return Syscall_ExitThread(errorcode);
}

#include <stat_def.h>
DWORD WINAPI GetFileAttributes(LPCSTR lpFileName)
{
	struct stat status;
	if (stat(lpFileName, &status) == 0)
	{
		if (status.st_mode == 0)
			return FILE_ATTRIBUTE_DIRECTORY;
		else
			return FILE_ATTRIBUTE_READONLY;
	}

	return 0;
}

HANDLE CreateFileMapping(HANDLE hFile, PSECURITY_ATTRIBUTES psa, DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName)
{
	return Syscall_CreateFileMapping(hFile, fdwProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pszName);
}

PVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap)
{
	return Syscall_MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
}

void ReadWriteBarrier(void)
{
	_ReadWriteBarrier();
}

bool GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	return Syscall_GetLocalTime(lpSystemTime);
}

DWORD GetEnvironmentVariable(LPCTSTR lpName, LPTSTR  lpBuffer, DWORD  nSize)
{
	return Syscall_GetEnvironmentVariable(lpName, lpBuffer, nSize);
}

BOOL SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue)
{
	return Syscall_SetEnvironmentVariable(lpName, lpValue);
}

BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	return 0;
	//return Syscall_QueryPerformanceCounter(lpPerformanceCount);
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
	return 0;
	//return Syscall_QueryPerformanceFrequency(lpFrequency);
}

DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	return 0;
}

HMODULE GetModuleHandle(LPCSTR lpModuleName)
{
	return (HMODULE)Syscall_GetModuleHandle(lpModuleName);
}

DWORD GetTempPath(DWORD nBufferLength, LPSTR lpBuffer)
{
	strcpy(lpBuffer, "/temp");
	return strlen(lpBuffer);
}

//<path>\<pre><uuuu>.TMP
UINT GetTempFileName(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName)
{
	tmpnam(lpTempFileName);
	return strlen(lpTempFileName);
}


PSLIST_ENTRY InterlockedPopEntrySList(PSLIST_HEADER ListHead)
{
	static BOOLEAN GLLInit = FALSE;
	static HANDLE GlobalListLock;
	PSLIST_ENTRY Result = 0;

	if (!GLLInit)
	{
		Syscall_CreateSpinLock(&GlobalListLock);
		GLLInit = TRUE;
	}

	Syscall_LockSpinLock(&GlobalListLock);
	if (ListHead->DUMMYSTRUCTNAME.Next.Next)
	{
		Result = ListHead->DUMMYSTRUCTNAME.Next.Next;
		ListHead->DUMMYSTRUCTNAME.Next.Next = Result->Next;
	}
	Syscall_UnlockSpinLock(&GlobalListLock);
	return Result;
}
PSLIST_ENTRY InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
{
	PSLIST_ENTRY PrevValue;

	do
	{
		PrevValue = ListHead->DUMMYSTRUCTNAME.Next.Next;
		ListEntry->Next = PrevValue;
	} while (InterlockedCompareExchangePointer((volatile PVOID *)&ListHead->DUMMYSTRUCTNAME.Next.Next,
		ListEntry,
		PrevValue) != PrevValue);

	return (PSLIST_ENTRY)PrevValue;
}

void  InitializeSListHead(PSLIST_HEADER ListHead)
{
	ListHead->Alignment = 0;
	
}