#pragma once
#include "limits.h"
#include "types.h"
#include <stdbool.h>

#define KERNELDLL 


#define MAXPATH	256
#define MAX_PROCESS_NAME	256

typedef int              ptrdiff_t;
typedef unsigned long  ulong;			//32 bit unsigined integer
typedef unsigned int   uint;			//32 bit unsigined integer
typedef unsigned short ushort;			//16 bit integer
typedef unsigned char  uchar;			//8 bit integer

typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;

typedef unsigned long _fsize_t;

typedef long int __time_t;
typedef __int64 time_t;

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
typedef unsigned int SIZE_T;
//typedef int ssize_t;
typedef long ssize_t;
#define _SIZE_T_DEFINED
#endif


typedef	unsigned long long u_quad_t;	/* quads */
typedef	long long	quad_t;
typedef	quad_t *	qaddr_t;

typedef unsigned short                wint_t;

typedef long clock_t;


#define offsetof(st, m) ((size_t)&(((st *)0)->m))  

#ifndef NULL
#define NULL 0
#endif

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED

typedef long _off_t;
typedef long long _off64_t;

#if (defined _CRT_DECLARE_NONSTDC_NAMES && _CRT_DECLARE_NONSTDC_NAMES) || (!defined _CRT_DECLARE_NONSTDC_NAMES && !__STDC__)
typedef _off_t off_t;
typedef _off64_t off64_t;
#endif
#endif

typedef long long int64;

typedef unsigned int   uint32;
typedef int  int32;
typedef unsigned long spinlock;
typedef unsigned int tss_t;
typedef int cpu_status;

typedef int uid_t;
typedef int gid_t;
typedef unsigned short uint16;
typedef unsigned long long uint64;
typedef unsigned char uint8;
typedef unsigned int phys_addr_t;

typedef int64 bigtime_t;
typedef unsigned long sem_id;
typedef int thread_id;
typedef int thrd_t;
typedef int proc_id;
typedef int area_id;
typedef int region_id;      // vm region id
typedef unsigned long addr_t;




