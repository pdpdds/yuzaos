 #ifndef _MEM_FS_INCLUDED_H_
 #define _MEM_FS_INCLUDED_H_
// #if !(_MSC_VER >= 800)
// typedef  unsigned int UINT;
// typedef  unsigned long DWORD;
// typedef long LONG ;
// typedef  unsigned char BYTE;
// //typedef  bool BOOL;
// typedef int BOOL;
// //typedef  DWORD HANDLE;
// typedef  const char* LPCSTR;
// #define TRUE true
// #define FALSE false
// 
// typedef struct _OVERLAPPED { // o 
//     DWORD  Internal; 
//     DWORD  InternalHigh; 
//     DWORD  Offset; 
//     DWORD  OffsetHigh; 
//     HANDLE hEvent; 
// } OVERLAPPED; 
// typedef struct _SECURITY_ATTRIBUTES {
//     DWORD nLength;
//     void* lpSecurityDescriptor;
//     BOOL bInheritHandle;
// } SECURITY_ATTRIBUTES;
// #define FILE_BEGIN           0
// #define FILE_CURRENT         1
// #define FILE_END             2
// 
// #define FILE_FLAG_WRITE_THROUGH         0x80000000
// #define FILE_FLAG_OVERLAPPED            0x40000000
// #define FILE_FLAG_NO_BUFFERING          0x20000000
// #define FILE_FLAG_RANDOM_ACCESS         0x10000000
// #define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
// #define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
// #define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
// #define FILE_FLAG_POSIX_SEMANTICS       0x01000000
// #define FILE_FLAG_OPEN_REPARSE_POINT    0x00200000
// #define FILE_FLAG_OPEN_NO_RECALL        0x00100000
// 
// #define CREATE_NEW          1
// #define CREATE_ALWAYS       2
// #define OPEN_EXISTING       3
// #define OPEN_ALWAYS         4
// #define TRUNCATE_EXISTING   5
// #define FILE_ATTRIBUTE_NORMAL 0x00080
// #else
//# include <windows.h>
//#include <winbase.h>
//typedef HANDLE *PHANDLE;
#include <windef.h>
#include "winapi.h"
#include "Support.h"
//#endif //end of !(_MSC_VER >= 800)

#ifdef __cplusplus
extern "C" {  //for the sake of calling c++ function from C
#endif

#if defined(__STDC__) || defined(__cplusplus)

//extern HANDLE RAMCreateFile(
//  LPCSTR lpFileName,          // pointer to name of the file
//  DWORD dwDesiredAccess       // access (read-write) mode
//);
BOOL RAMPathFileExist(LPCSTR lpFileName);
extern HANDLE RAMCreateFile(
  LPCSTR lpFileName,          // pointer to name of the file
  DWORD dwDesiredAccess,       // access (read-write) mode
  DWORD dwShareMode,           // share mode
  SECURITY_ATTRIBUTES* lpSecurityAttributes,
                               // pointer to security attributes
  DWORD dwCreationDisposition,  // how to create
  DWORD dwFlagsAndAttributes,  // file attributes
  HANDLE hTemplateFile         // handle to file with attributes to 
                               // copy
);

extern BOOL RAMDeleteFile(
  LPCSTR lpFileName   // pointer to name of file to delete
);
extern DWORD RAMGetFileAttributes(
  LPCSTR lpFileName   // pointer to the name of a file or directory
);
extern BOOL RAMCloseHandle(
  HANDLE hObject   // handle to object to close
);
extern BOOL RAMReadFile(
  HANDLE hFile,                // handle of file to read
  void* lpBuffer,             // pointer to buffer that receives data
  DWORD nNumberOfBytesToRead,  // number of bytes to read
  DWORD* lpNumberOfBytesRead, // pointer to number of bytes read
  OVERLAPPED* lpOverlapped    // pointer to structure for data
);
extern BOOL RAMWriteFile(
  HANDLE hFile,                    // handle to file to write to
  const void* lpBuffer,                // pointer to data to write to file
  DWORD nNumberOfBytesToWrite,     // number of bytes to write
  DWORD* lpNumberOfBytesWritten,  // pointer to number of bytes written
  OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
);
extern DWORD RAMSetFilePointer(
							   HANDLE hFile,          // handle of file
							   ULONGLONG lDistanceToMove,  // number of bytes to move file pointer
							   LONG* lpDistanceToMoveHigh,
							   // pointer to high-order DWORD of 
							   // distance to move
							   DWORD dwMoveMethod     // how to move
							   );
extern BOOL RAMFlushFileBuffers(
  HANDLE hFile   // open handle to file whose buffers are to be 
                 // flushed
);
extern BOOL RAMSetEndOfFile(
  HANDLE hFile   // handle of file whose EOF is to be set
);

extern DWORD RAMGetFileSize(
  HANDLE hFile,  // handle of file to get size of
  DWORD* lpFileSizeHigh 
                 // pointer to high-order word for file size
);
extern BOOL RAMLockFile(
  HANDLE hFile,           // handle of file to lock
  DWORD dwFileOffsetLow,  // low-order word of lock region offset
  DWORD dwFileOffsetHigh,  // high-order word of lock region offset
  DWORD nNumberOfBytesToLockLow,
                          // low-order word of length to lock
  DWORD nNumberOfBytesToLockHigh 
                          // high-order word of length to lock
);
extern BOOL RAMLockFileEx(
  HANDLE hFile,      // handle of file to lock
  DWORD dwFlags,     // functional behavior modification flags
  DWORD dwReserved,  // reserved, must be set to zero
  DWORD nNumberOfBytesToLockLow,
                     // low-order 32 bits of length to lock
  DWORD nNumberOfBytesToLockHigh,
                     // high-order 32 bits of length to lock
  OVERLAPPED* lpOverlapped 
                     // addr. of structure with lock region start 
                     // offset
);
extern BOOL RAMUnlockFile(
  HANDLE hFile,           // handle of file to unlock
  DWORD dwFileOffsetLow,  // low-order word of lock region offset
  DWORD dwFileOffsetHigh,  // high-order word of lock region offset
  DWORD nNumberOfBytesToUnlockLow,
                          // low-order word of length to unlock
  DWORD nNumberOfBytesToUnlockHigh 
                          // high-order word of length to unlock
);
extern BOOL RAMUnlockFileEx(
  HANDLE hFile,      // handle of file to unlock
  DWORD dwReserved,  // reserved, must be set to zero
  DWORD nNumberOfBytesToUnlockLow,
                     // low order 32-bits of length to unlock
  DWORD nNumberOfBytesToUnlockHigh,
                     // high order 32-bits of length to unlock
  OVERLAPPED* lpOverlapped 
                     // addr. of struct. with unlock region start 
                     // offset
);
extern BOOL RAMInitialMemFS();
extern BOOL RAMCloseMemFS();
extern BOOL RAMAddFile2MemFS(
  LPCSTR lpFileName,
  const BYTE*  pbFileBuffer,
  DWORD   dwBufferLength
);
extern BOOL RAMGetFileFromMemFS(
  LPCSTR lpFileName,
  BYTE*  pbFileBuffer,
  DWORD   dwBufferLength
);
extern void DumpMemFS();
#endif //end of defined(__STDC__) || defined(__cplusplus)

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif //end of _MEM_FS_INCLUDED_H_