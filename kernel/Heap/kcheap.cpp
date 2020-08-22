/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f? die Verwendung dieses Sourcecodes siehe unten
*/

#include "kcheap.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <kassert.h>
#include <SystemAPI.h>
/* The heap provides the malloc/free-functionality, i.e. dynamic allocation of memory.
   It manages a certain amount of continuous virtual memory, starting at "heapStart".
   Whenever more memory is requested than there is available, the heap expands.
   For expansion, the heap asks the paging module to map physical memory to the following virtual addresses
   and increases its "heapSize" variable (but at least by "HEAP_MIN_GROWTH") afterwards.

   To manage the free and reserved (allocated) areas of the heap an array of "region" elements is kept.
   Each region specifies its size and reservation status.
   Free regions always get merged. Regions don't store their addresses.
   The third region address is calculated by adding the first and second region size to "heapStart":
   region_3_addr = heapStart + regions[0].size + regions[1].size

   Before the heap is set up, memory is allocated on a "placement address".
   This is an identity mapped area of continuous memory,
   the allocation just moves a pointer forward by the requested size and returns its previous value.

   The heap's management data is placed at this placement address, too.
   Since this area cannot grow, the heap has a maximum amount of region objects ("regionMaxCount").*/

// TODO: Ensure the heap will not overflow (above KERNEL_heapEnd, cf. memory.h)
#include <Mutex.h>
#include <systemcall_impl.h>

#pragma pack(push, 1)
typedef struct
{
    uint32_t size;
    uint32_t number;
    bool     reserved;
    char     comment[21];
} region_t;
#pragma pack(pop)

extern uint32_t alignUp(uint32_t val, uint32_t alignment);
extern uint32_t alignDown(uint32_t val, uint32_t alignment);


static region_t*      regions         = 0;
static uint32_t       regionCount     = 0;
static uint32_t       regionMaxCount  = 0;
static uint32_t       firstFreeRegion = 0;
static void* firstFreeAddr = 0;
static uint8_t* heapStart       = 0;
static uint32_t       heapSize        = 0;
static const uint32_t HEAP_MIN_GROWTH = 0x10000;

static RecursiveLock mutex("kc_heap");

#ifdef _MEMLEAK_FIND_
static uint32_t counter = 0;
#endif


//static void* placementMalloc(size_t size, uint32_t alignment);
// Placement allocation
#define PLACEMENT_BEGIN   0x600000   // 6 MiB
#define PLACEMENT_END     0xC00000   // 12 MiB
#include <heap/kmalloc.h>
void heap_install(void)
{
	unsigned int pa = 0;
	regionCount = 0;
    // This gets us the current placement address
	regions = (region_t*)kmalloc_ap(PLACEMENT_END - PLACEMENT_BEGIN, &pa);
	memset(regions, 0, PLACEMENT_END - PLACEMENT_BEGIN);
	regionMaxCount = (PLACEMENT_END - PLACEMENT_BEGIN) / sizeof(region_t);
	
	firstFreeAddr = (void*)kmalloc_ap(20000000, &pa);
	heapStart = (uint8*)firstFreeAddr;
	regions[0].size = 20000000;
	regionCount = 1;
}

void* heap_getCurrentEnd(void)
{
    return (heapStart + heapSize);
}

/*static bool heap_grow(size_t size, uint8_t* heapEnd, bool continuous)
{
    // We will have to append another region-object to our array if we can't merge with the last region - check whether there would be enough space to insert the region-object
    if ((regionCount > 0) && regions[regionCount-1].reserved && (regionCount >= regionMaxCount))
    {
        return (false);
    }

	mutex.Lock();
    // Enhance the memory
    if (!paging_alloc(kernelPageDirectory, heapEnd, size, MEM_WRITE))
    {
		mutex.Unlock();
        return (false);
    }

    // Maybe we can merge with the last region object?
    if ((regionCount > 0) && !regions[regionCount-1].reserved)
    {
        regions[regionCount-1].size += size;
    }
    // Otherwise insert a new region object
    else
    {
        regions[regionCount].reserved = false;
        regions[regionCount].size = size;
        regions[regionCount].number = 0;

        ++regionCount;
    }

    heapSize += size;
	mutex.Unlock();
    return (true);
}*/

/*static void* placementMalloc(size_t size, uint32_t alignment)
{
    static void* nextPlacement = (void*)PLACEMENT_BEGIN;

    // Avoid odd addresses
    size = alignUp(size, 4);

    // Ensure alignment
	unsigned int currPlacement = alignUp((uintptr_t)nextPlacement, alignment);

    // Check if there is enough space in placement area
    if ((uintptr_t)currPlacement + size > PLACEMENT_END)
        return (0);

    // Do simple placement allocation
	mutex.Lock();
    nextPlacement = currPlacement + size;
	mutex.Unlock();

    return (currPlacement);
}*/

void* kc_malloc(size_t size, uint32_t alignment, const char* comment)
{
    // consecutive number for detecting the sequence of mallocs at the heap
    static uint32_t consecutiveNumber = 0;

    // Analyze alignment and other requirements
    size_t within = 0xFFFFFFFF;
    if (alignment&HEAP_WITHIN_PAGE)
    {
        ASSERT(size <= PAGE_SIZE);
        within = PAGE_SIZE;
    }
    else if (alignment&HEAP_WITHIN_64K)
    {
        ASSERT(size <= 0x10000);
        within = 0x10000;
    }
    bool continuous = alignment&HEAP_CONTINUOUS;

    alignment &= HEAP_ALIGNMENT_MASK;

    // If the heap is not set up, do placement malloc
    /*if (regions == 0)
    {
        return (placementMalloc(size, alignment));
    }*/

    // Avoid odd addresses
    size = alignUp(size, 4);

	mutex.Lock();
    // Walk the regions and find one being suitable
    bool foundFree = false;
    uint8_t* regionAddress = (uint8_t *)firstFreeAddr;
    for (uint32_t i = firstFreeRegion; i < regionCount; i++)
    {
        // Manage caching of first free region
        if (!regions[i].reserved)
            foundFree = true;
        else if (!foundFree)
        {
            firstFreeRegion = i;
            firstFreeAddr = regionAddress;
        }

        // Calculate aligned address and the additional size needed due to alignment
        uint8_t* alignedAddress = (uint8_t*)alignUp((uintptr_t)regionAddress, alignment);
        uintptr_t additionalSize = (uintptr_t)alignedAddress - (uintptr_t)regionAddress;

        // Check whether this region is free, big enough and fits page requirements
        if (!regions[i].reserved && (regions[i].size >= size + additionalSize) &&
            (within - (uintptr_t)regionAddress%within >= additionalSize))
        {
            // Check if the region consists of continuous physical memory if required
            if (continuous)
            {
                bool iscontinuous = true;
                for (unsigned int virt1 = alignDown((uintptr_t)alignedAddress, PAGE_SIZE); 
					(uintptr_t)(virt1+ PAGE_SIZE) <= (uintptr_t)(alignedAddress + size); 
					virt1+= PAGE_SIZE) 
				{
                    uintptr_t phys1 = Syscall_GetPAFromVM(virt1);
					phys1 += (uint32_t)(virt1) % PAGE_SIZE;
                    uintptr_t phys2 = Syscall_GetPAFromVM(virt1 + PAGE_SIZE);
					phys2 += (uint32_t)(virt1 + PAGE_SIZE) % PAGE_SIZE;
                    if (phys1 + PAGE_SIZE != phys2)
                    {
                        iscontinuous = false;
                        break;
                    }
                }
                if (!iscontinuous)
                    continue;
            }

            // We will split up this region ...
            // +--------------------------------------------------------+
            // |                      Current Region                    |
            // +--------------------------------------------------------+
            //
            // ... into three, the first and third remain free,
            // while the second gets reserved, and its address is returned
            //
            // +------------------+--------------------------+----------+
            // | Before Alignment | Aligned Destination Area | Leftover |
            // +------------------+--------------------------+----------+

            // Split the pre-alignment area
            if (alignedAddress != regionAddress)
            {
                // Check whether we are able to expand
                if (regionCount >= regionMaxCount)
                {
					mutex.Unlock();
                    return (0);
                }

                // Move all following regions ahead to get room for a new one
                memmove(regions + i+1, regions + i, (regionCount-i) * sizeof(region_t));

                ++regionCount;

                // Setup the regions
                regions[i].size     = alignedAddress - regionAddress;
                regions[i].reserved = false;

                regions[i+1].size  -= regions[i].size;

                // "Aligned Destination Area" becomes the "current" region
                regionAddress += regions[i].size;
                i++;
            }

            // Split the leftover
            if (regions[i].size > size + additionalSize)
            {
                // Check whether we are able to expand
                if (regionCount+1 > regionMaxCount)
                {
					mutex.Unlock();
                    return (0);
                }

                // Move all following regions ahead to get room for a new one
                memmove(regions + i+2, regions + i+1, (regionCount-i-1)*sizeof(region_t));

                ++regionCount;

                // Setup the regions
                regions[i+1].size     = regions[i].size - size;
                regions[i+1].reserved = false;
                regions[i+1].number   = 0;

                regions[i].size       = size;
            }

            // Set the region to "reserved" and return its address
            regions[i].reserved = true;
            strncpy(regions[i].comment, comment, 20);
            regions[i].comment[20] = 0;
            regions[i].number = ++consecutiveNumber;

          #ifdef _MEMLEAK_FIND_
            counter++;
            writeInfo(2, "Malloc - free: %u", counter);
          #endif
          #ifdef _MALLOC_FREE_LOG_
            textColor(YELLOW);
            kprintf("\nmalloc: %Xh %s", regionAddress, comment);
            textColor(TEXT);
          #endif

			mutex.Unlock();
            return (regionAddress);

        } //region is free and big enough

        regionAddress += regions[i].size;
    }

	SKY_ASSERT(0, "sdfsfddsd");
    // There is nothing free, try to expand the heap
    //uint32_t sizeToGrow = MAX(HEAP_MIN_GROWTH, alignUp(size*3/2, PAGE_SIZE));
    //bool success = heap_grow(sizeToGrow, (uint8_t*)((uintptr_t)heapStart + heapSize), continuous);

	mutex.Unlock();

    /*if (!success)
    {
        kprintf("\nmalloc (\"%s\") failed, heap could not be expanded!", comment);
        return (0);
    }
    else
    {
      #ifdef _MALLOC_FREE_LOG_
        textColor(YELLOW);
        kprintf("\nheap expanded: %Xh heap end: %Xh", sizeToGrow, (uintptr_t)heapStart + heapSize);
        textColor(TEXT);
      #endif
    }*/

    // Now there should be a region that is large enough
    return kc_malloc(size, alignment, comment);
}


void kc_free(void* addr)
{
  #ifdef _MALLOC_FREE_LOG_
    textColor(LIGHT_GRAY);
    kprintf("\nfree:   %Xh", addr);
    textColor(TEXT);
  #endif

    if (addr == 0)
    {
        return;
    }

  #ifdef _MEMLEAK_FIND_
    counter--;
    writeInfo(2, "Malloc - free: %u", counter);
  #endif

	mutex.Lock();

    // Walk the regions and find the correct one
    uint8_t* regionAddress = heapStart;
    for (uint32_t i=0; i<regionCount; i++)
    {
        if (regionAddress == addr && regions[i].reserved)
        {
          #ifdef _MALLOC_FREE_LOG_
            textColor(LIGHT_GRAY);
            kprintf(" %s", regions[i].comment);
            textColor(TEXT);
          #endif
            regions[i].number = 0;
            regions[i].reserved = false; // free the region

            // Check for a merge with the next region
            if ((i+1 < regionCount) && !regions[i+1].reserved)
            {
                // Adjust the size of the now free region
                regions[i].size += regions[i+1].size; // merge

                // Move all following regions back by one
                memmove(regions + i+1, regions + i+2, (regionCount-2-i)*sizeof(region_t));

                --regionCount;
            }

            // Check for a merge with the previous region
            if (i>0 && !regions[i-1].reserved)
            {
                // Adjust the size of the previous region
                regions[i-1].size += regions[i].size; // merge

                // Move all following regions back by one
                memmove(regions + i, regions + i+1, (regionCount-1-i)*sizeof(region_t));

                --regionCount;
            }

            if (i < firstFreeRegion)
            {
                firstFreeRegion = i;
                firstFreeAddr = regionAddress;
            }

			mutex.Unlock();
            return;
        }

        regionAddress += regions[i].size;
    }

	mutex.Unlock();

    kprintf("\nBroken free: %Xh", addr);
    //printStackTrace(0, 0); // Print a stack trace to get the function call that caused the problem
}

void heap_logRegions(void)
{
    kprintf("\nDebug: Heap regions sent to serial output.\n");
    //serial_log(SER_LOG_HEAP,"\n\nregionMaxCount: %u, regionCount: %u, firstFreeRegion: %u\n", regionMaxCount, regionCount, firstFreeRegion);
	//serial_log(SER_LOG_HEAP,"\n---------------- HEAP REGIONS ----------------\n");
	//serial_log(SER_LOG_HEAP,"#\taddress\t\tsize\t\tnumber\tcomment");

    uintptr_t regionAddress = (uintptr_t)heapStart;

    for (uint32_t i=0; i<regionCount; i++)
    {
        /*if (regions[i].reserved)
            serial_log(SER_LOG_HEAP, "\n%u\t%Xh\t%Xh\t%u\t%s", i, regionAddress, regions[i].size, regions[i].number, regions[i].comment);
        else
            serial_log(SER_LOG_HEAP, "\n%u\t%Xh\t%Xh\t-\t-", i, regionAddress, regions[i].size);*/
        regionAddress += regions[i].size;
    }
	//serial_log(SER_LOG_HEAP,"\n---------------- HEAP REGIONS ----------------\n\n");
}

/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
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
