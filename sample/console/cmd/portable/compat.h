#ifndef COMPAT_H_
#define COMPAT_H_

// Oracle says this code is fine to copy from MSDN because it's just APIs...
#define WIN95_CMD
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//#include <strings.h>

#include <wchar.h>
#include <stddef.h>


#define PORTABLE_CMD

#define MAX_PATH PATH_MAX

#define CONST const

#define O_BINARY 0
#define _O_APPEND O_APPEND
#define _O_BINARY O_BINARY

#define INFINITE  0xFFFFFFFF
#define CONTROL_C_EXIT   ((DWORD)0xC000013A)
#define CTRL_C_EVENT 0
#define CTRL_BREAK_EVENT 1

#if INTPTR_MAX == INT64_MAX
#define _ARCH64BIT
#elif INTPTR_MAX == INT32_MAX
#define _ARCH32BIT
#else
#error Unknown pointer size or missing size macros!
#endif

//#define MAX_PATH PATH_MAX

#define CONST const

#define O_CLOEXEC		0x0040	/* close fd on exec */
#define	O_NONBLOCK		0x0080	/* non blocking io */
#define	O_EXCL			0x0100	/* exclusive creat */
#define O_CREAT			0x0200	/* create and open file */
#define O_TRUNC			0x0400	/* open with truncation */
#define O_APPEND		0x0800	/* to end of file */
#define O_NOCTTY    	0x1000  /* currently unsupported */
#define	O_NOTRAVERSE	0x2000	/* do not traverse leaf link */
#define O_ACCMODE   	0x0003  /* currently unsupported */
#define O_TEXT			0x4000	/* CR-LF translation	*/
//#define O_BINARY		0x8000	/* no translation	*/

#define O_BINARY 0
#define _O_APPEND O_APPEND
#define _O_BINARY O_BINARY

/* flags for open() */
#define O_RDONLY		0	/* read only */
#define O_WRONLY		1	/* write only */
#define O_RDWR			2	/* read and write */
#define O_RWMASK		3	/* Mask to get open mode */

// See: https://msdn.microsoft.com/en-gb/library/windows/desktop/aa383751(v=vs.85).aspx
typedef unsigned char BYTE;
typedef unsigned char *PBYTE;
typedef char TCHAR, *LPTCH;
typedef char CHAR;
typedef char CCHAR;
typedef wchar_t WCHAR;
typedef WCHAR *LPWSTR;
typedef unsigned char UCHAR;
typedef CHAR *PCHAR;
typedef CONST CHAR *LPCSTR;
typedef CONST CHAR *LPCTSTR;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef LONG *PLONG;
typedef uint32_t ULONG, UINT;
typedef ULONG *PULONG;
#if defined(_ARCH64BIT)
 typedef uint64_t ULONG_PTR;
#else
 typedef uint32_t ULONG_PTR;
#endif
typedef ULONG_PTR *PULONG_PTR;
typedef TCHAR* PTCHAR;
typedef void VOID;
typedef VOID* PVOID;
typedef CONST VOID* LPCVOID;
typedef unsigned short USHORT;
typedef bool BOOLEAN;
//typedef bool BOOL;
typedef BOOL* PBOOL;
typedef BOOL* LPBOOL;
typedef uint32_t DWORD;
typedef DWORD *LPDWORD;
typedef int64_t LONGLONG;
typedef PVOID HANDLE, *PHANDLE, *LPHANDLE;
typedef HANDLE HINSTANCE, HMODULE;
typedef HANDLE HWND;
typedef HANDLE* PHANDLE;
typedef HANDLE HKEY;
typedef unsigned char BYTE, *LPBYTE;
typedef void *LPVOID;
typedef size_t SIZE_T;

typedef HANDLE HDESK, HWINSTA;
#define SW_SHOWNORMAL 1

typedef uint64_t ULONGLONG;
typedef ULONGLONG *PULONGLONG;

#define FALSE false
#define TRUE true

typedef char *LPSTR;
#ifdef UNICODE
 typedef LPWSTR LPTSTR;
#else
 typedef LPSTR LPTSTR;
#endif

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  };
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284(v=vs.85).aspx
typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;


typedef struct _SHELLEXECUTEINFO {
  DWORD     cbSize;
  ULONG     fMask;
  HWND      hwnd;
  LPCTSTR   lpVerb;
  LPCTSTR   lpFile;
  LPCTSTR   lpParameters;
  LPCTSTR   lpDirectory;
  int       nShow;
  HINSTANCE hInstApp;
  LPVOID    lpIDList;
  LPCTSTR   lpClass;
  HKEY      hkeyClass;
  DWORD     dwHotKey;
  union {
    HANDLE hIcon;
    HANDLE hMonitor;
  } DUMMYUNIONNAME;
  HANDLE    hProcess;
} SHELLEXECUTEINFO, *LPSHELLEXECUTEINFO;


// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686331(v=vs.85).aspx
typedef struct _STARTUPINFO {
  DWORD  cb;
  LPTSTR lpReserved;
  LPTSTR lpDesktop;
  LPTSTR lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  WORD   wShowWindow;
  WORD   cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFO, *LPSTARTUPINFO;

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD  dwProcessId;
  DWORD  dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _SECURITY_ATTRIBUTES {
  DWORD  nLength;
  LPVOID lpSecurityDescriptor;
  BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;


// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724950(v=vs.85).aspx
typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724251(v=vs.85).aspx
#define DUPLICATE_CLOSE_SOURCE 0x00000001

#define WINAPI
#define _CRTAPI1

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363854(v=vs.85).aspx
typedef DWORD (WINAPI *LPPROGRESS_ROUTINE)(
         LARGE_INTEGER TotalFileSize,
         LARGE_INTEGER TotalBytesTransferred,
         LARGE_INTEGER StreamSize,
         LARGE_INTEGER StreamBytesTransferred,
         DWORD         dwStreamNumber,
         DWORD         dwCallbackReason,
         HANDLE        hSourceFile,
         HANDLE        hDestinationFile,
    	 LPVOID        lpData
);

LPTSTR WINAPI GetCommandLine(void);
VOID WINAPI SetCommandLine(int argc, char **argv);
DWORD WINAPI GetVersion(void);

char *strtolower(char *buf);
char* strtoupper(char* s);

#define _tcslwr strtolower
#define lstrcmpi strcasecmp
#define _tcsstr strstr
#define _tcsrchr strrchr
#define _tcsicmp strcasecmp
#define _tcschr strchr
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcsncpy strncpy
#define _tcscat strcat
#define _tcsncmp strncmp
#define _tcscmp strcmp
#define _tcstol strtol
#define _tcsnicmp strncasecmp
#define _tcsupr strtoupper
#define _istspace isspace
#define _istxdigit isxdigit
#define _istalpha isalpha
#define _istdigit isdigit
#define _totlower tolower
#define _totupper toupper
#define _sntprintf snprintf
#define _stprintf sprintf
#define _vsntprintf vsnprintf 
#define _vsnprintf vsnprintf 
#define TEXT(x) x
#define __TEXT(x) x

#define _getch getchar

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

#define UNREFERENCED_PARAMETER(p)          ((p)=(p))
#define DBG_UNREFERENCED_PARAMETER(p)      ((p)=(p))
#define DBG_UNREFERENCED_LOCAL_VARIABLE(p) ((p)=(p))

#define _In_
#define _In_opt_
#define _Inout_opt_
#define _Inout_
#define _Reserved_
#define _Out_
#define _Out_opt_
#define IN
#define OUT
#define OPTIONAL

// http://ftp.icpdas.com/pub/beta_version/VHM/wince600/at91sam9g45m10ek_armv4i/cesysgen/ddk/inc/ntintsafe.h
#ifndef UInt32x32To64
#define UInt32x32To64(a, b) ((uint64_t)((unsigned long)(a)) * (uint64_t)((unsigned long)(b)))
#endif // !UInt32x32To64

void _ultoa(unsigned long value, char* string, unsigned char radix);
void _ltoa(long value, char* string, unsigned char radix);

BOOL WINAPI GetConsoleMode(
  _In_  FILE*  hConsoleHandle,
  _Out_ LPDWORD lpMode
);

DWORD WINAPI GetConsoleTitle(
  _Out_ LPTSTR lpConsoleTitle,
  _In_  DWORD  nSize
);

BOOL WINAPI SetConsoleTitle(
  _In_ LPCTSTR lpConsoleTitle
);

void WINAPI DebugBreak(void);


#define INVALID_HANDLE_VALUE NULL
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)


#define FILE_ATTRIBUTE_READONLY 1
#define FILE_ATTRIBUTE_HIDDEN 2
#define FILE_ATTRIBUTE_SYSTEM 4
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_DEVICE 0x40
#define FILE_ATTRIBUTE_ARCHIVE 32
#define FILE_ATTRIBUTE_NORMAL 128
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 8192
#define FILE_ATTRIBUTE_OFFLINE 4096
#define FILE_ATTRIBUTE_TEMPORARY 256

#define PROGRESS_CONTINUE 0
#define PROGRESS_CANCEL 1
#define PROGRESS_STOP 2
#define PROGRESS_QUIET 3

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa364960(v=vs.85).aspx
#define FILE_TYPE_CHAR 0x0002
#define FILE_TYPE_DISK 0x0001
#define FILE_TYPE_PIPE 0x0003
#define FILE_TYPE_REMOTE 0x8000
#define FILE_TYPE_UNKNOWN 0x0000

// https://msdn.microsoft.com/en-gb/library/windows/desktop/aa364939(v=vs.85).aspx
#define DRIVE_UNKNOWN 0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED 3
#define DRIVE_REMOTE 4
#define DRIVE_CDROM 5
#define DRIVE_RAMDISK 6

// http://www.jbox.dk/sanos/source/include/win32.h.html
#define GENERIC_READ                     0x80000000
#define GENERIC_WRITE                    0x40000000
#define GENERIC_EXECUTE                  0x20000000
#define GENERIC_ALL                      0x10000000

// http://www.jbox.dk/sanos/source/include/win32.h.html
#define CREATE_NEW                       1
#define CREATE_ALWAYS                    2
#define OPEN_EXISTING                    3
#define OPEN_ALWAYS                      4
#define TRUNCATE_EXISTING                5

// http://www.jbox.dk/sanos/source/include/win32.h.html
#define STD_INPUT_HANDLE                 ((DWORD)-10)
#define STD_OUTPUT_HANDLE                ((DWORD)-11)
#define STD_ERROR_HANDLE                 ((DWORD)-12)

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686033(v=vs.85).aspx
#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_LINE_INPUT 0x0002
#define ENABLE_ECHO_INPUT 0x0004
#define ENABLE_MOUSE_INPUT 0x0010
#define ENABLE_WRAP_AT_EOL_OUTPUT 0x0002
#define ENABLE_PROCESSED_OUTPUT 0x0001

// https://github.com/gasgas4/NT_4.0_SourceCode/blob/84a03f73738328ea66034dca7cda79a857623720/nt4/private/windows/inc/shellapi.w
#define SEE_MASK_HASTITLE       0x00080000       
#define SEE_MASK_FLAG_DDEWAIT   0x00000100                        
#define SEE_MASK_NO_CONSOLE     0x00008000
#define SEE_MASK_NOCLOSEPROCESS 0x00000040

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms684863(v=vs.85).aspx
#define CREATE_NEW_CONSOLE 0x00000010
#define CREATE_NEW_PROCESS_GROUP 0x00000200
#define CREATE_NO_WINDOW 0x08000000

// https://webcache.googleusercontent.com/search?q=cache:xaaywBK_sCkJ:https://sourceforge.net/u/cstrauss/w32api/ci/f3077014e6186628344d580460bfafe4d3b52c9d/tree/include/winbase.h%3Fformat%3Draw+&cd=1&hl=en&ct=clnk&gl=uk
#define NORMAL_PRIORITY_CLASS 32
#define IDLE_PRIORITY_CLASS 64
#define HIGH_PRIORITY_CLASS 128
#define REALTIME_PRIORITY_CLASS 256

// https://webcache.googleusercontent.com/search?q=cache:xaaywBK_sCkJ:https://sourceforge.net/u/cstrauss/w32api/ci/f3077014e6186628344d580460bfafe4d3b52c9d/tree/include/winbase.h%3Fformat%3Draw+&cd=1&hl=en&ct=clnk&gl=uk
#define STARTF_USESHOWWINDOW 1
#define STARTF_USESHOWWINDOW 1

// http://www.lugaru.com/man/Other.Process.Primitives.html
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351(v=vs.85).aspx
#define FORMAT_MESSAGE_FROM_HMODULE 0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx
#define SEM_FAILCRITICALERRORS 0x0001

#if !defined(HINSTANCE_ERROR)
#define HINSTANCE_ERROR 32
#endif

#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define SCS_POSIX_BINARY 4

typedef DWORD NTSTATUS;
#define STATUS_SUCCESS                   ((DWORD)0x00000000L)
#define STATUS_UNSUCCESSFUL              ((DWORD)0xC0000001L)
#define STATUS_BUFFER_OVERFLOW           ((DWORD)0x80000005L)
#define STATUS_INVALID_PARAMETER         ((DWORD)0xC000000DL)

// FormatMessageWithArgs
// https://github.com/gasgas4/NT_4.0_SourceCode/blob/84a03f73738328ea66034dca7cda79a857623720/nt4/private/windows/shell/cabinet/message.c

// FormatMessage
// https://github.com/kerneltravel/longene_travel/blob/daa01b979536498ec2552db8e60abeef31894d72/wine-1.0-longene/dlls/kernel32/format_msg.c
// https://github.com/gasgas4/NT_4.0_SourceCode/blob/84a03f73738328ea66034dca7cda79a857623720/nt4/private/windows/base/client/message.c

#define UOI_NAME 2

#if defined(__GNUC__) || defined(__clang__)
#define FIELD_OFFSET(type, field) offsetof(type, field)
#else
#define FIELD_OFFSET(type, field) ((LONG)(INT_PTR)&(((type *)0)->field))
#endif

DWORD WINAPI GetEnvironmentVariable(
  LPCTSTR lpName,
  LPTSTR  lpBuffer,
  DWORD   nSize
);

DWORD WINAPI GetModuleFileName(
  _In_opt_ HMODULE hModule,
  _Out_    LPTSTR  lpFilename,
  _In_     DWORD   nSize
);


BOOL WINAPI SetEnvironmentVariable(
  _In_     LPCTSTR lpName,
  _In_opt_ LPCTSTR lpValue
);

BOOL WINAPI CloseHandle(
  _In_ HANDLE hObject
);

BOOL WINAPI CreateProcess(
  _In_opt_    LPCTSTR               lpApplicationName,
  _Inout_opt_ LPTSTR                lpCommandLine,
  _In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
  _In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
  _In_        BOOL                  bInheritHandles,
  _In_        DWORD                 dwCreationFlags,
  _In_opt_    LPVOID                lpEnvironment,
  _In_opt_    LPCTSTR               lpCurrentDirectory,
  _In_        LPSTARTUPINFO         lpStartupInfo,
  _Out_       LPPROCESS_INFORMATION lpProcessInformation
);

BOOL WINAPI GetExitCodeProcess(
  _In_  HANDLE  hProcess,
  _Out_ LPDWORD lpExitCode
);

DWORD WINAPI WaitForSingleObject(
  _In_ HANDLE hHandle,
  _In_ DWORD  dwMilliseconds
);

BOOL WINAPI DuplicateHandle(
  _In_  HANDLE   hSourceProcessHandle,
  _In_  HANDLE   hSourceHandle,
  _In_  HANDLE   hTargetProcessHandle,
  _Out_ LPHANDLE lpTargetHandle,
  _In_  DWORD    dwDesiredAccess,
  _In_  BOOL     bInheritHandle,
  _In_  DWORD    dwOptions
);

BOOL ShellExecuteEx(
  _Inout_ SHELLEXECUTEINFO *pExecInfo
);

BOOL WINAPI TerminateProcess(
  _In_ HANDLE hProcess,
  _In_ UINT   uExitCode
);

SIZE_T WINAPI VirtualQuery(
  _In_opt_ LPCVOID                   lpAddress,
  _Out_    LPVOID lpBuffer,
  _In_     SIZE_T                    dwLength
);

void WINAPI OutputDebugStringA(
  _In_opt_ LPCTSTR lpOutputString
);


char *str_append(char *a, char *b);


#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(Argument) (Argument != 0)
#endif // ARGUMENT_PRESENT

#include "_locale.h"
#include "_time.h"
#include "_memory.h"
#include "_file.h"
#include "_msg.h"
#include "_error.h"
#include "_path.h"
#include "_find.h"

#endif
