#ifndef PORTABLE_FIND_H_
#define PORTABLE_FIND_H_

#define HAVE_CASE_SENSITIVE_FILESYSTEM 1

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365740(v=vs.85).aspx
typedef struct _WIN32_FIND_DATA {
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  TCHAR    cFileName[MAX_PATH];
  TCHAR    cAlternateFileName[14];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

HANDLE FindFirstFile(
    _In_   LPCTSTR lpFileName,
    _Out_  LPWIN32_FIND_DATA lpFindFileData
);

BOOL FindNextFile(
    _In_   HANDLE hFindFile,
    _Out_  LPWIN32_FIND_DATA lpFindFileData
);

BOOL FindClose(
    _Inout_  HANDLE hFindFile
);

#endif
