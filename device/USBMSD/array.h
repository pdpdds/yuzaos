#pragma once

// We use a macro as a substitute for a C++ template. The struct has to be the same structure as array_t in array.c
#define  array(type) struct { type* data; size_t size; size_t reserved; }

#define  array_init() {0, 0, 0}                                       // Initializes an array_t static object
void* array_create(void);                                          // Allocates memory for an array_t object
void     array_construct(void* arr);                                  // Constructs an array_t object
void     array_destruct(void* arr);                                   // Destructs an array_t object
void     array_free(void* arr);                                       // Frees memory of an array_t object
void     array_resize_(void* arr, size_t size, size_t typesize);      // Resizes the array
#define  array_resize(array, items) array_resize_(array, items, sizeof((*(array)->data))) // Resizes the array