/*
 *  mmap.h
 *
 *  copyright (c) 2018 Xiongfei Shi
 *
 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
 *  license: MIT
 */

#ifndef __MMAP_H__
#define __MMAP_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    void *mmap(void *addr, size_t len, int prot, int flags, int fildes, unsigned long off);
    int munmap(void *addr, size_t len);
    int msync(void *addr, size_t len, int flags);
    int mprotect(void *addr, size_t len, int prot);
    int mlock(const void *addr, size_t len);
    int munlock(const void *addr, size_t len);

#ifdef __cplusplus
};
#endif


#endif  /* __MMAP_H__ */
