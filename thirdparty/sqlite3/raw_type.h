/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

/* 	2012-4  Created by jorya_txj
  *	xxxxxx   please added here
  */
  
#ifndef RAW_OS_TYPE_H
#define  RAW_OS_TYPE_H

#define  RAW_INLINE             static __inline					/*INLINE*/

typedef  void        			RAW_VOID;						 /*void*/
typedef  signed char        	RAW_S8;                        /* 8-bit signed character */
typedef  unsigned  char       	RAW_BOOLEAN;                  /*  8-bit unsigned boolean or logical */
typedef  unsigned  char        	RAW_U8;                      /*  8-bit unsigned character */

typedef  unsigned  short       	RAW_U16;                      /* 16-bit unsigned short integer */
typedef  signed  short       	RAW_S16;                      /* 16-bit   signed integer */
typedef  unsigned  int         	RAW_U32;                      /* 32-bit unsigned integer */
typedef  signed int         	RAW_S32;                      /* 32-bit   signed integer     */
typedef  signed long long       RAW_S64;                      /* 64-bit   signed integer     */
typedef  unsigned long long     RAW_U64;                      /* 64-bit   signed integer     */

typedef  unsigned int        	PORT_STACK;                   /* 32-bit   unsigned integer width stack */

typedef  signed int	            RAW_PROCESSOR_INT;	                   /* Processor bit width signed integer */
typedef  unsigned int	        RAW_PROCESSOR_UINT;	                   /* Processor bit width unsigned integer */

	
#endif

