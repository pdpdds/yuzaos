#include "array.h"
#include <stdint.h>
#include <memory.h>

typedef struct
{
    void* data;
    size_t size;
    size_t reserved;
} array_t;


void* array_create(void)
{
    array_t* array = (array_t * )malloc(sizeof(array_t));
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