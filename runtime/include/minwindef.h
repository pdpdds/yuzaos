/****************************************************************************
*                                                                           *
* minwindef.h -- Basic Windows Type Definitions for minwin partition        *
*                                                                           *
* Copyright (c) Microsoft Corporation. All rights reserved.                 *
*                                                                           *
****************************************************************************/


#ifndef _MINWINDEF_
#define _MINWINDEF_
#pragma once

#if !defined(WIN95_CMD)

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif /* NO_STRICT */

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * BASETYPES is defined in ntdef.h if these types are already defined
     */

#ifndef BASETYPES
#define BASETYPES
    typedef unsigned long ULONG;
    typedef ULONG* PULONG;
    typedef unsigned short USHORT;
    typedef USHORT* PUSHORT;
    typedef unsigned char UCHAR;
    typedef UCHAR* PUCHAR;
    typedef char* PSZ;
#endif  /* !BASETYPES */

    typedef void* PVOID;
    typedef PVOID HANDLE, * PHANDLE, * LPHANDLE;
    typedef HANDLE HWND;
    typedef wchar_t		WCHAR;	/* 16-bit unsigned integer */
    typedef HANDLE HINSTANCE;
    typedef HINSTANCE HMODULE;
    typedef char* LPSTR, * PSTR;
    typedef long LONG;
    typedef long LONG_PTR, * PLONG_PTR;
    typedef unsigned long ULONG_PTR, * PULONG_PTR;
    typedef ULONG_PTR DWORD_PTR, * PDWORD_PTR;
    typedef ULONG_PTR DWORD_PTR, * PDWORD_PTR;
    typedef __int64 LONG64, * PLONG64;


#ifndef _ULONGLONG_
    typedef __int64 LONGLONG;
    typedef unsigned __int64 ULONGLONG;
    typedef LONGLONG* PLONGLONG;
    typedef ULONGLONG* PULONGLONG;
#endif // _ULONGLONG_

#define _DWORDLONG_
    typedef ULONGLONG  DWORDLONG;
    typedef DWORDLONG* PDWORDLONG;

#define DLL_PROCESS_ATTACH   1    
#define DLL_THREAD_ATTACH    2    
#define DLL_THREAD_DETACH    3    
#define DLL_PROCESS_START    4    
#define DLL_PROCESS_DETACH   0    

#define MAX_PATH          260

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#undef far
#undef near
#undef pascal

#define far
#define near

#if (!defined(_MAC)) && ((_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED))
#define pascal __stdcall
#else
#define pascal
#endif

#if defined(DOSWIN32) || defined(_MAC)
#define cdecl _cdecl
#ifndef CDECL
#define CDECL _cdecl
#endif
#else
#define cdecl
#ifndef CDECL
#define CDECL
#endif
#endif

#ifdef _MAC
#define CALLBACK    PASCAL
#define WINAPI      CDECL
#define WINAPIV     CDECL
#define APIENTRY    WINAPI
#define APIPRIVATE  CDECL
#ifdef _68K_
#define PASCAL      __pascal
#else
#define PASCAL
#endif
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define CALLBACK    __stdcall
//#define WINAPI      __stdcall
#define WINAPI      
#define WINAPIV     __cdecl
#define APIENTRY    WINAPI
#define APIPRIVATE  __stdcall
#define PASCAL      __stdcall
#else
#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY    WINAPI
#define APIPRIVATE
#define PASCAL      pascal
#endif

#ifndef _M_CEE_PURE
#ifndef WINAPI_INLINE
#define WINAPI_INLINE  WINAPI
#endif
#endif

#undef FAR
#undef  NEAR
#define FAR                 far
#define NEAR                near
#ifndef CONST
#define CONST               const
#endif

    typedef unsigned long       DWORD;
#ifdef __cplusplus
typedef int                 BOOL;
#endif
    typedef unsigned char       BYTE;
    typedef unsigned short      WORD;
    typedef float               FLOAT;
    typedef FLOAT* PFLOAT;
#ifdef __cplusplus
    typedef BOOL near* PBOOL;
    typedef BOOL far* LPBOOL;
#endif
    typedef BYTE near* PBYTE;
    typedef BYTE far* LPBYTE;
    typedef int near* PINT;
    typedef int far* LPINT;
    typedef WORD near* PWORD;
    typedef WORD far* LPWORD;
    typedef long far* LPLONG;
    typedef DWORD near* PDWORD;
    typedef DWORD far* LPDWORD;
    typedef void far* LPVOID;
    typedef CONST void far* LPCVOID;
    typedef const char* LPCSTR, * PCSTR;
    typedef char* LPSTR, * PSTR;
    typedef LPCSTR				LPCTSTR;
    typedef LPSTR				LPTSTR;
    typedef long long			QWORD;

    typedef int                 INT;
    typedef unsigned int        UINT;
    typedef unsigned int* PUINT;

    typedef signed char         INT8, * PINT8;
    typedef signed short        INT16, * PINT16;
    typedef signed int          INT32, * PINT32;
    typedef signed __int64      INT64, * PINT64;
    typedef unsigned char       UINT8, * PUINT8;
    typedef unsigned short      UINT16, * PUINT16;
    typedef unsigned int        UINT32, * PUINT32;
    typedef unsigned __int64    UINT64, * PUINT64;

    typedef char CHAR;
    typedef short SHORT;
    typedef CHAR* PCHAR, * LPCH, * PCH;
    typedef CONST CHAR* LPCCH, * PCCH;

    typedef BYTE  BOOLEAN;
    typedef BOOLEAN* PBOOLEAN;

    typedef LONG* PLONG;
    typedef WCHAR* LPWSTR, * PWSTR;
    typedef const wchar_t* LPCWSTR, * PCWSTR;
    //typedef void VOID;

    #define IN
    #define OUT

    typedef char TCHAR;

   #define NTAPI __stdcall
    typedef void(NTAPI* PAPCFUNC)(ULONG_PTR Parameter);

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2


#define UNALIGNED
#define INFINITE            0xFFFFFFFF  // Infinite timeout

#define FALSE   0
#define TRUE    1

#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation

#ifdef __cplusplus
    typedef bool (__stdcall *dll_start_t) (HANDLE hDllHandle, DWORD dwReason, LPVOID  lpreserved);
#endif
    typedef int(*THREAD_START_ENTRY)(void*);
    typedef int(*MAIN_IMPL)(int argc, char** argv);

#if defined(_WIN64)
    typedef __int64 INT_PTR, * PINT_PTR;
    typedef unsigned __int64 UINT_PTR, * PUINT_PTR;

    typedef __int64 LONG_PTR, * PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, * PULONG_PTR;

#define __int3264   __int64

#else
    typedef int INT_PTR, * PINT_PTR;
    typedef unsigned int UINT_PTR, * PUINT_PTR;

    typedef long LONG_PTR, * PLONG_PTR;
    typedef unsigned long ULONG_PTR, * PULONG_PTR;

#define __int3264   __int32

#endif

    /* Types use for passing & returning polymorphic values */
    typedef UINT_PTR            WPARAM;
    typedef LONG_PTR            LPARAM;
    typedef LONG_PTR            LRESULT;

#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

    typedef HANDLE NEAR* SPHANDLE;
    typedef HANDLE FAR* LPHANDLE;
    typedef HANDLE              HGLOBAL;
    typedef HANDLE              HLOCAL;
    typedef HANDLE              GLOBALHANDLE;
    typedef HANDLE              LOCALHANDLE;

#ifndef _MANAGED
#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4255) // () treated as (void)
#endif
#ifndef _MAC
#ifdef _WIN64
    typedef INT_PTR(FAR WINAPI* FARPROC)();
    typedef INT_PTR(NEAR WINAPI* NEARPROC)();
    typedef INT_PTR(WINAPI* PROC)();
#else
    typedef int (FAR WINAPI* FARPROC)();
    typedef int (NEAR WINAPI* NEARPROC)();
    typedef int (WINAPI* PROC)();
#endif  // _WIN64
#else
    typedef int (CALLBACK* FARPROC)();
    typedef int (CALLBACK* NEARPROC)();
    typedef int (CALLBACK* PROC)();
#endif
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
#else
    typedef INT_PTR(WINAPI* FARPROC)(void);
    typedef INT_PTR(WINAPI* NEARPROC)(void);
    typedef INT_PTR(WINAPI* PROC)(void);
#endif

    typedef WORD                ATOM;   //BUGBUG - might want to remove this from minwin

    /*DECLARE_HANDLE(HKEY);
    typedef HKEY* PHKEY;
    DECLARE_HANDLE(HMETAFILE);
    DECLARE_HANDLE(HINSTANCE);
    typedef HINSTANCE HMODULE;      // HMODULEs can be used in place of HINSTANCEs 
    DECLARE_HANDLE(HRGN);
    DECLARE_HANDLE(HRSRC);
    DECLARE_HANDLE(HSPRITE);
    DECLARE_HANDLE(HLSURF);
    DECLARE_HANDLE(HSTR);
    DECLARE_HANDLE(HTASK);
    DECLARE_HANDLE(HWINSTA);
    DECLARE_HANDLE(HKL);*/

#ifndef _MAC
    typedef int HFILE;
#else
    typedef short HFILE;
#endif

    //
    //  File System time stamps are represented with the following structure:
    //

    typedef struct _FILETIME {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME, * PFILETIME, * LPFILETIME;
#define _FILETIME_

#ifdef __cplusplus
}
#endif

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM | WINAPI_PARTITION_GAMES) */
#pragma endregion

#endif