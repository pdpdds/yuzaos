#include "cmd.h"

#include <assert.h>
#include <signal.h>
#include <getenv.h>

static char *g_cmdline = NULL;


extern char **environ;

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms683187(v=vs.85).aspx
LPTCH WINAPI GetEnvironmentStrings(void) {
  return NULL; // XXX: implement this properly
  //return environ;
}

DWORD WINAPI GetEnvironmentVariable(
  LPCTSTR lpName,
  LPTSTR  lpBuffer,
  DWORD   nSize
) {
  char *data = getenv(lpName);
  if( ! data ) {
    SetLastError(ERROR_ENVVAR_NOT_FOUND);
    return 0;
  }
  strncpy(lpBuffer, data, nSize);
  SetLastError(ERROR_SUCCESS);
  return strlen(lpBuffer);
}

char * 
str_append(char *a, char *b) 
{ 
    size_t existing_len = a != NULL ? strlen(a) : 0;
    a = realloc(a, 1 + existing_len + strlen(b));
    if (a != NULL) { 
        strcat(a, b); 
    } 
    return a; 
} 

LPTSTR WINAPI GetCommandLine(void) {
  return g_cmdline;
}

VOID WINAPI SetCommandLine(int argc, char **argv) {
  int N;
  for( N = 1; N < argc; N++ ) {
    if( N ) {
      g_cmdline = str_append(g_cmdline, " ");
    }
    g_cmdline = str_append(g_cmdline, argv[N]);
  }
}

BOOL WINAPI GetConsoleMode(
  _In_  FILE*  hConsoleHandle,
  _Out_ LPDWORD lpMode
) {
  if( hConsoleHandle == STDIN ) {
    *lpMode = ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    return TRUE;
  }

  if( hConsoleHandle == STDOUT || hConsoleHandle == STDERR ) {
    *lpMode = ENABLE_WRAP_AT_EOL_OUTPUT;
    return TRUE;
  }

  return FALSE;
}

BOOL WINAPI SetConsoleTitle(
  _In_ LPCTSTR lpConsoleTitle
) {
  return FALSE;
}

DWORD WINAPI GetConsoleTitle(
  _Out_ LPTSTR lpConsoleTitle,
  _In_  DWORD  nSize
) {
  strncpy(lpConsoleTitle, "Console Title", nSize);
  return strlen(lpConsoleTitle);
}

void WINAPI DebugBreak(void) {
  //raise(SIGINT);
    for (;;);
}

// http://www.codeproject.com/Articles/10563/Windows-version-requirements-macros
DWORD WINAPI GetVersion(void) {
  return 0xc0000004;
}


char* strtoupper(char* s) {
  assert(s != NULL);

  char* p = s;
  while (*p != '\0') {
    *p = toupper(*p);
    p++;
  }

  return s;
}

char *strtolower(char *buf)
{
  char *p = buf;
  if (buf != 0) {
    while (*p != 0) {
      *p = tolower(*p);
      p++;
    }
  }  
  return buf;
}

DWORD WINAPI GetModuleFileName(
  HMODULE hModule,
  LPTSTR  lpFilename,
  DWORD   nSize
) {
  assert( hModule == NULL );
  assert( lpFilename != NULL );
  strncpy(lpFilename, "cmd.exe", nSize);
  SetLastError(ERROR_SUCCESS);
  return strlen(lpFilename);
}

// TODO: implement me
UINT WINAPI SetErrorMode(
  _In_ UINT uMode
) {
  return 0;
}

BOOL WINAPI SetEnvironmentVariable(
  _In_     LPCTSTR lpName,
  _In_opt_ LPCTSTR lpValue
) {
  if( lpValue == NULL ) {
    unsetenv(lpName);
  }
  setenv(lpName, lpValue, 1);
  return TRUE;
}

// TODO: implement me
BOOL WINAPI CloseHandle(
  _In_ HANDLE hObject
) {
  abort();
  return FALSE;
}

// TODO: implement me
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
) {
  abort();
  return FALSE;
}

// TODO: implement me
BOOL WINAPI GetExitCodeProcess(
  _In_  HANDLE  hProcess,
  _Out_ LPDWORD lpExitCode
) {
  abort();
  return FALSE;
}

// TODO: implement me
DWORD WINAPI WaitForSingleObject(
  _In_ HANDLE hHandle,
  _In_ DWORD  dwMilliseconds
) {
  abort();
  return FALSE;
}

// TODO: implement me
BOOL WINAPI DuplicateHandle(
  _In_  HANDLE   hSourceProcessHandle,
  _In_  HANDLE   hSourceHandle,
  _In_  HANDLE   hTargetProcessHandle,
  _Out_ LPHANDLE lpTargetHandle,
  _In_  DWORD    dwDesiredAccess,
  _In_  BOOL     bInheritHandle,
  _In_  DWORD    dwOptions
) {
  abort();
  return FALSE;
}

// TODO: implement me
BOOL ShellExecuteEx(
  _Inout_ SHELLEXECUTEINFO *pExecInfo
) {
  abort();
  return FALSE;
}

// TODO: implement me
BOOL WINAPI TerminateProcess(
  _In_ HANDLE hProcess,
  _In_ UINT   uExitCode
) {
  abort();
  return FALSE;
}

// TODO: implement me
SIZE_T WINAPI VirtualQuery(
  _In_opt_ LPCVOID                   lpAddress,
  _Out_    LPVOID lpBuffer,
  _In_     SIZE_T                    dwLength
) {
  abort();
  return 0;
}

void WINAPI OutputDebugStringA(
  _In_opt_ LPCTSTR lpOutputString
) {
  fwrite(lpOutputString, strlen(lpOutputString), 1, stderr);
}
