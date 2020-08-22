/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "_pci.h"
#include <memory.h>
//#include "util.h"
//#include "kheap.h"
#include <systemcall_impl.h>

typedef struct
{
    void* data;
    size_t size;
    size_t reserved;
} array_t;


void* array_create(void)
{
    array_t* array = (array_t * )malloc_aligned(sizeof(array_t), 4096);
    array_construct(array);
    return array;
}

void array_construct(void* arr)
{
    array_t* array = (array_t *)arr;
    array->data = 0;
    array->reserved = 0;
    array->size = 0;
}

void array_destruct(void* arr)
{
    array_t* array = (array_t*)arr;
    array->reserved = 0;
    array->size = 0;
    free(array->data);
    array->data = 0;
}

void array_free(void* arr)
{
    array_t* array = (array_t*)arr;
    free(array->data);
    free(array);
}

uint32_t alignUp(uint32_t val, uint32_t alignment)
{
	if (!alignment)
		return val;
	--alignment;
	return (val + alignment) & ~alignment;
}

void array_resize_(void* arr, size_t size, size_t typesize)
{
    array_t* array = (array_t*)arr;
    if (size <= array->reserved)
        array->size = size; // Don't shrink memory
    else
    {
        size_t reserve = alignUp(size, 16);
        void* newarray = malloc_aligned(reserve*typesize, 4096);
        memcpy(newarray, array->data, array->size*typesize);
        free(array->data);
        array->data = newarray;
        array->size = size;
        array->reserved = reserve;
    }
}


/*
* Copyright (c) 2015 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
