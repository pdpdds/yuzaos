#pragma once

typedef struct _SINGLE_LIST_ENTRY {
    struct _SINGLE_LIST_ENTRY* Next;
} SINGLE_LIST_ENTRY, * PSINGLE_LIST_ENTRY;

typedef struct _SINGLE_LIST_ENTRY SLIST_ENTRY, * PSLIST_ENTRY;

typedef union _SLIST_HEADER {
    
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        WORD   Depth;
        WORD   CpuId;
    } DUMMYSTRUCTNAME;
   
} SLIST_HEADER, * PSLIST_HEADER;

#define MAX_NATURAL_ALIGNMENT sizeof(DWORD)
#define MEMORY_ALLOCATION_ALIGNMENT 8