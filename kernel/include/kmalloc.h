#pragma once
/*#if  defined(DLL_HEAP_EXPORT)
#define HEAP_API __declspec(dllexport) 
#else
#define HEAP_API __declspec(dllimport)
#endif*/
#define HEAP_API

#ifdef  __cplusplus
extern "C" {
#endif
HEAP_API void  kmalloc_init(size_t base, size_t heap_size);
HEAP_API void* kmalloc(size_t size);
HEAP_API void  kfree(void*);

HEAP_API void* kmalloc_aligned(size_t, size_t alignment);
HEAP_API void  kfree_aligned(void*);

HEAP_API void* krealloc(void*, size_t);
HEAP_API void* kcalloc(size_t count, size_t size);

#ifdef  __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
    inline void* malloc(size_t size)
    {
        return kmalloc(size);
    }
    inline void free(void* p)
    {
        kfree(p);
    }
#ifdef  __cplusplus
}
#endif