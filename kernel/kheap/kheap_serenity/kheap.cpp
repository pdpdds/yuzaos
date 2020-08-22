#include "kheap.h"
#include "memory.h"
#include <InterruptDisabler.h>
#include <kassert.h>
#include <ktypes.h>
#include <systemcall_impl.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push ,1)
typedef struct
{
    uint32_t size;
    uint32_t number;
    bool     reserved;
} region_t;
#pragma pack(pop)

#define MAX_REGION_COUNT 0x20000
static region_t       regions[MAX_REGION_COUNT] = { 0, };
static uint32_t       regionCount     = 0;
static uint32_t       regionMaxCount  = 0;
static uint32_t       firstFreeRegion = 0;
static void*          firstFreeAddr   = 0;
static uint8_t*       heapStart       = 0;
static uint32_t       heapSize        = 0;
static const uint32_t HEAP_MIN_GROWTH = 0x10000;

static inline uint32_t alignUp(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    --alignment;
    return (val + alignment) & ~alignment;
}

static inline uint32_t alignDown(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    return val & ~(alignment - 1);
}

#ifdef _MEMLEAK_FIND_
static uint32_t counter = 0;
#endif

//static void* placementMalloc(size_t size, uint32_t alignment);

void  kmalloc_init(size_t base, size_t heap_size)
{
    // This gets us the current placement address
    //regions = placementMalloc(0, 0);

    // We take the rest of the placement area
    regionCount = 0;
    regionMaxCount = MAX_REGION_COUNT;

    firstFreeAddr = (void*)base;
    heapStart = (uint8_t*)base;
}

void* heap_getCurrentEnd(void)
{
    return (heapStart + heapSize);
}

static bool heap_grow(size_t size, uint8_t* heapEnd, bool continuous)
{
   
    InterruptDisabler disabler;

    // We will have to append another region-object to our array if we can't merge with the last region - check whether there would be enough space to insert the region-object
    if ((regionCount > 0) && regions[regionCount-1].reserved && (regionCount >= regionMaxCount))
    {
        return (false);
    }

    //20200407
    // Enhance the memory
    //if (!paging_alloc(kernelPageDirectory, heapEnd, size, MEM_WRITE))
      //  return false;

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
  
    return (true);
}

/*static void* placementMalloc(size_t size, uint32_t alignment)
{
    InterruptDisabler disabler;
    static void* nextPlacement = (void*)PLACEMENT_BEGIN;

    // Avoid odd addresses
    size = alignUp(size, 4);

    // Ensure alignment
    void* currPlacement = (void*)alignUp((uintptr_t)nextPlacement, alignment);

    // Check if there is enough space in placement area
    if ((uintptr_t)currPlacement + size > PLACEMENT_END)
        return (0);

    nextPlacement = currPlacement + size;

    return (currPlacement);
}*/

void* kmalloc(size_t size)
{
    return pretty_malloc(size, PAGE_SIZE);
}

void  kfree(void* ptr)
{
    pretty_free(ptr);
}

void* kmalloc_aligned(size_t size, size_t alignment)
{
    return pretty_malloc(size, PAGE_SIZE);
}

void  kfree_aligned(void* ptr)
{
    pretty_free(ptr);
}

size_t kmalloc_size(void* addr)
{
    InterruptDisabler disabler;
    
    if (addr == 0)
        return 0;
   
    uint8_t* regionAddress = heapStart;
    for (uint32_t i = 0; i < regionCount; i++)
    {
        if (regionAddress == addr && regions[i].reserved)
        {
            return regions[i].size;
        }

        regionAddress += regions[i].size;
    }

    return 0;
}

//메모리 크기를 재조정한다.
void* krealloc(void* ptr, size_t size) //메모리 크기를 재조정한다.
{
    InterruptDisabler disabler;

    void* _new = 0;
    if (!ptr) 
    { //기존 포인터가 널이면 size 크기의 새 버퍼를 생성하고 리턴한다.
        _new = (void*)kmalloc(size);
        if (!_new) { goto error; }
    }
    else 
    {
        if (kmalloc_size(ptr) < size) //기존에 할당된 크기가 새롭게 요청된 크기보다 작으면
        {//새로운 버퍼를 메모리에 할당한뒤 기존버퍼의 내용을 새 버퍼에 복사하고 리턴
            _new = (void*)kmalloc(size);
            if (!_new)
            {
                goto error;
            }
            memcpy(_new, ptr, kmalloc_size(ptr));
            kfree(ptr);
        }
        else //새롭게 요청된 할당 크기가 기존의 크기보다 작다면 
        {//기존 포인터를 리턴
            _new = ptr;
        }
    }
    return _new;
error:
    printf("\nkrealloc fail: %xh", ptr);
    return 0;
}

void* kcalloc(size_t count, size_t size)
{
    void* ptr = (void*)kmalloc(count * size);
    memset(ptr, 0, count * size);
    return ptr;
}

void* pretty_malloc(size_t size, uint32_t alignment)
{
    InterruptDisabler disabler;
    // consecutive number for detecting the sequence of mallocs at the heap
    static uint32_t consecutiveNumber = 0;

    // Analyze alignment and other requirements
    size_t within = 0xFFFFFFFF;
    if (alignment & HEAP_WITHIN_PAGE)
    {
        SKY_ASSERT(size <= PAGE_SIZE, "size <= PAGE_SIZE");
        within = PAGE_SIZE;
    }
    else if (alignment&HEAP_WITHIN_64K)
    {
        SKY_ASSERT(size <= 0x10000, "size <= 0x10000");
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
                /*bool iscontinuous = true;
                for (void* virt1 = (void*)alignDown((uintptr_t)alignedAddress, PAGE_SIZE); (uintptr_t)(virt1+PAGE_SIZE) <= (uintptr_t)(alignedAddress + size); virt1+= PAGE_SIZE) {
                    uintptr_t phys1 = paging_getPhysAddr(virt1);
                    uintptr_t phys2 = paging_getPhysAddr(virt1 + PAGE_SIZE);
                    if (phys1 + PAGE_SIZE != phys2)
                    {
                        iscontinuous = false;
                        break;
                    }
                }
                if (!iscontinuous)
                    continue;*/
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
            regions[i].number = ++consecutiveNumber;

          #ifdef _MEMLEAK_FIND_
            counter++;
            writeInfo(2, "Malloc - free: %u", counter);
          #endif
          #ifdef _MALLOC_FREE_LOG_
            textColor(YELLOW);
            printf("\nmalloc: %Xh", regionAddress);
            textColor(TEXT);
          #endif

            return (regionAddress);

        } //region is free and big enough

        regionAddress += regions[i].size;
    }

    // There is nothing free, try to expand the heap
    uint32_t sizeToGrow = max(HEAP_MIN_GROWTH, alignUp(size*3/2, PAGE_SIZE));
    bool success = heap_grow(sizeToGrow, (uint8_t*)((uintptr_t)heapStart + heapSize), continuous);

    if (!success)
    {
        printf("\nmalloc failed, heap could not be expanded!");
        return (0);
    }
    else
    {
      #ifdef _MALLOC_FREE_LOG_
        textColor(YELLOW);
        printf("\nheap expanded: %Xh heap end: %Xh", sizeToGrow, (uintptr_t)heapStart + heapSize);
        textColor(TEXT);
      #endif
    }

    // Now there should be a region that is large enough
    return pretty_malloc(size, alignment);
}

void pretty_free(void* addr)
{
    InterruptDisabler disabler;

  #ifdef _MALLOC_FREE_LOG_
    textColor(LIGHT_GRAY);
    printf("\nfree:   %Xh", addr);
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

    // Walk the regions and find the correct one
    uint8_t* regionAddress = heapStart;
    for (uint32_t i=0; i<regionCount; i++)
    {
        if (regionAddress == addr && regions[i].reserved)
        {
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

            return;
        }

        regionAddress += regions[i].size;
    }

    printf("\nBroken free: %Xh", addr);
}

void heap_logRegions(void)
{
    /*printf("\nDebug: Heap regions sent to serial output.\n");
    printf("\n\nregionMaxCount: %u, regionCount: %u, firstFreeRegion: %u\n", regionMaxCount, regionCount, firstFreeRegion);
    printf("\n---------------- HEAP REGIONS ----------------\n");
    printf("#\taddress\t\tsize\t\tnumber");

    uintptr_t regionAddress = (uintptr_t)heapStart;

    for (uint32_t i=0; i<regionCount; i++)
    {
        if (regions[i].reserved)
            printf("\n%u\t%Xh\t%Xh\t%u", i, regionAddress, regions[i].size, regions[i].number);
        else
            printf("\n%u\t%Xh\t%Xh\t-\t-", i, regionAddress, regions[i].size);
        regionAddress += regions[i].size;
    }
    printf("\n---------------- HEAP REGIONS ----------------\n\n");*/
}
