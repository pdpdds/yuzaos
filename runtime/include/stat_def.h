//
// sys/stat.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The _stat() and _fstat() families of functions.
//
#pragma once
#include <windef.h>
#include <types.h>
//#include <corecrt.h>
//#include <sys/types.h>

#define _CRT_PACKING 8
__pragma(pack(push, _CRT_PACKING))

#ifdef  __cplusplus
extern "C" {
#endif


	typedef int                           errno_t;
	typedef unsigned short                wint_t;
	typedef unsigned short                wctype_t;
	typedef long                          __time32_t;
	typedef __int64                       __time64_t;
	typedef long _off_t; // file offset value

#pragma warning(push)
#pragma warning(disable: 4820) /* padding added after data member */


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Types
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	struct _stat32
	{
		_dev_t         st_dev;
		_ino_t         st_ino;
		unsigned short st_mode;
		short          st_nlink;
		short          st_uid;
		short          st_gid;
		_dev_t         st_rdev;
		_off_t         st_size;
		__time32_t     st_atime;
		__time32_t     st_mtime;
		__time32_t     st_ctime;
	};

	struct _stat32i64
	{
		_dev_t         st_dev;
		_ino_t         st_ino;
		unsigned short st_mode;
		short          st_nlink;
		short          st_uid;
		short          st_gid;
		_dev_t         st_rdev;
		__int64        st_size;
		__time32_t     st_atime;
		__time32_t     st_mtime;
		__time32_t     st_ctime;
	};

	struct _stat64i32
	{
		_dev_t         st_dev;
		_ino_t         st_ino;
		unsigned short st_mode;
		short          st_nlink;
		short          st_uid;
		short          st_gid;
		_dev_t         st_rdev;
		_off_t         st_size;
		__time64_t     st_atime;
		__time64_t     st_mtime;
		__time64_t     st_ctime;
	};

	struct _stat64
	{
		_dev_t         st_dev;
		_ino_t         st_ino;
		unsigned short st_mode;
		short          st_nlink;
		short          st_uid;
		short          st_gid;
		_dev_t         st_rdev;
		__int64        st_size;
		__time64_t     st_atime;
		__time64_t     st_mtime;
		__time64_t     st_ctime;
	};

#define __stat64 _stat64 // For legacy compatibility

#define _USE_32BIT_TIME_T
#define _CRT_INTERNAL_NONSTDC_NAMES 1

	struct stat
	{
		_dev_t         st_dev;
		_ino_t         st_ino;
		unsigned short st_mode; //0 디렉토리 //1 정규 파일
		short          st_nlink;
		short          st_uid;
		short          st_gid;
		_dev_t         st_rdev;
		_off_t         st_size;
		time_t         st_atime;
		time_t         st_mtime;
		time_t         st_ctime;
	};




	//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//
	// Flags
	//
	//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#define _S_IFMT   0xF000 // File type mask
#define _S_IFDIR  0x4000 // Directory
#define _S_IFCHR  0x2000 // Character special
#define _S_IFIFO  0x1000 // Pipe
#define _S_IFREG  0x8000 // Regular
#define _S_IREAD  0x0100 // Read permission, owner
#define _S_IWRITE 0x0080 // Write permission, owner
#define _S_IEXEC  0x0040 // Execute/search permission, owner

#if _CRT_INTERNAL_NONSTDC_NAMES
#define S_IFMT   _S_IFMT
#define S_IFDIR  _S_IFDIR
#define S_IFCHR  _S_IFCHR
#define S_IFREG  _S_IFREG
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC
#endif



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Functions
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#ifdef _USE_32BIT_TIME_T
#define _fstat      _fstat32
#define _fstati64   _fstat32i64
#define _stat       _stat32
#define _stati64    _stat32i64
#define _wstat      _wstat32
#define _wstati64   _wstat32i64
#else
#define _fstat      _fstat64i32
#define _fstati64   _fstat64
#define _stat       _stat64i32
#define _stati64    _stat64
#define _wstat      _wstat64i32
#define _wstati64   _wstat64
#endif

#ifdef  __cplusplus
}
#endif

#pragma warning(pop)
__pragma(pack(pop))