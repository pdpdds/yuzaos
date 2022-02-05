/* MollenOS
 *
 * Copyright 2011 - 2017, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * MollenOS C11-Support Threading Implementation
 * - Definitions, prototypes and information needed.
 */

#ifndef __STDC_TLS__
#define __STDC_TLS__

#include <orangeos.h>
#include <crtdefs.h>
#include <buffer.h>
#include <errno.h>
#include <wchar.h>

#if defined(_X86_32) || defined(__i386__) || defined(SKYOS32)
#define TLS_VALUE   uint32_t
#define TLS_READ    __asm { __asm mov ebx, [_Offset] __asm mov eax, gs:[ebx] __asm mov [Value], eax }
#define TLS_WRITE   __asm { __asm mov ebx, [_Offset] __asm mov eax, [Value] __asm mov gs:[ebx], eax }
#elif defined(amd64) || defined(__amd64__)
#define TLS_VALUE   uint64_t
#define TLS_READ    __asm { __asm mov rbx, [_Offset] __asm mov rax, gs:[rbx] __asm mov [Value], rax }
#define TLS_WRITE   __asm { __asm mov rbx, [_Offset] __asm mov rax, [Value] __asm mov gs:[rbx], rax }
#else
#error "Implement rw for tls for this architecture"
#endif

#define TSS_DTOR_ITERATIONS     4
#define TSS_KEY_INVALID         UINT_MAX

typedef void(*tss_dtor_t)(void*);

/* __get_reserved
* Read and write the magic tls thread-specific
* pointer, we need to take into account the compiler here */
inline size_t __get_reserved(size_t Index) 
{
	TLS_VALUE Value = 0;
	size_t _Offset = (Index * sizeof(TLS_VALUE));
	TLS_READ;
	return (size_t)Value;
}

/* __set_reserved
* Read and write the magic tls thread-specific
* pointer, we need to take into account the compiler here */
inline void __set_reserved(size_t Index, TLS_VALUE Value) 
{
	size_t _Offset = (Index * sizeof(TLS_VALUE));
	TLS_WRITE;
}

// Number of tls entries
#define TLS_NUMBER_ENTRIES          64


/* Thread Local Storage
 * This is the structure that exists seperately for each running
 * thread, and can be retrieved with TLSGetCurrent() which returns
 * the local copy of this structure */


#pragma pack(push,1)
typedef struct tag_thread_storage
{
    thrd_t          thr_id;
    void*           handle;
    errno_t         err_no;
    void*           locale;
    //mbstate_t       mbst;
    unsigned int    seed;
    char*           strtok_next;
    struct tm       tm_buffer;
    char            asc_buffer[26];
    DmaBuffer_t*    transfer_buffer;
    uintptr_t       tls_array[TLS_NUMBER_ENTRIES];

    // Exception & RTTI Support for msc++
#if defined(_MSC_VER) && !defined(__clang__)
    void                    *TerminateHandler;
    void                    *UnexpectedHandler;
    void                    *SeTranslator;
    void                    *ExceptionInfo;
    void                    *ExceptionRecord;
    void                    *ExceptionList;
    void                    *StackLow;
    void                    *StackHigh;
    int                      IsDebugging;
#endif
}thread_storage;

typedef thread_storage thread_storage_t;

#pragma pack(pop)


enum {
	thrd_success = 0,
	thrd_nomem = 1,
	thrd_timedout = 2,
	thrd_busy = 3,
	thrd_error = -1
};

_CODE_BEGIN
/* tls_current 
 * Retrieves the local storage space for the current thread */
CRTDECL(thread_storage_t*, tls_current(void));

/* tls_create
 * Initializes a new thread-storage space for the caller thread.
 * Part of CRT initialization routines. */
CRTDECL(int,        tls_create(thread_storage_t *Tls));

/* tls_destroy
 * Destroys a thread-storage space should be called by thread crt */
CRTDECL(int,        tls_destroy(thread_storage_t *Tls));

CRTDECL(thrd_t, thrd_current(void));

/* tls_cleanup
 * Destroys the TLS for the specific thread
 * by freeing resources and calling c11 destructors. */
CRTDECL(void, tls_cleanup(thrd_t thr, void* DsoHandle, int ExitCode));
CRTDECL(void, tls_cleanup_quick(thrd_t thr, void* DsoHandle, int ExitCode));

/* tss_create
 * Creates new thread-specific storage key and stores it in the object pointed to by tss_key.
 * Although the same key value may be used by different threads,
 * the values bound to the key by tss_set are maintained on a per-thread
 * basis and persist for the life of the calling thread. */
CRTDECL(int,
	tss_create(
		tss_t*     tss_key,
		tss_dtor_t destructor));

/* tss_get
 * Returns the value held in thread-specific storage for the current thread
 * identified by tss_key. Different threads may get different values identified by the same key. */
CRTDECL(void*,
	tss_get(
		tss_t tss_key));

/* tss_set
 * Sets the value of the thread-specific storage identified by tss_id for the
 * current thread to val. Different threads may set different values to the same key. */
CRTDECL(int,
	tss_set(
		tss_t tss_id,
		void* val));

/* tss_delete
 * Destroys the thread-specific storage identified by tss_id. */
CRTDECL(void,
	tss_delete(
		tss_t tss_id));
_CODE_END

#endif //!__STDC_TLS__
