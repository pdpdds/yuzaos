#pragma once
#include "windef.h"
#include <kmalloc.h>
#include <stdint.h>

#define BIT(n) (1U<<(n))
#define HEAP_ALIGNMENT_MASK 0x00FFFFFF
#define HEAP_WITHIN_PAGE BIT(24)
#define HEAP_WITHIN_64K BIT(25)
#define HEAP_CONTINUOUS BIT(31)

void* heap_getCurrentEnd(void);
void  heap_install(void);
void  heap_logRegions(void);
void* pretty_malloc(size_t size, uint32_t alignment);
void  pretty_free(void* addr);
