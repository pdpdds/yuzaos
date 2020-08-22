/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2016 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

/* Thread management routines for SDL */

#include "SDL_thread.h"
#include "../SDL_systhread.h"

#ifndef STACK_SIZE_PARAM_IS_A_RESERVATION
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000
#endif

typedef struct ThreadStartParms
{
	void *args;	
} tThreadStartParms, *pThreadStartParms;

static DWORD
RunThread(void *data)
{
	pThreadStartParms pThreadParms = (pThreadStartParms)data;	
	void *args = pThreadParms->args;
	SDL_free(pThreadParms);
	SDL_RunThread(args);	
	return (0);
}

static DWORD WINAPI
RunThreadViaCreateThread(LPVOID data)
{
	return RunThread(data);
}

#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
int
SDL_SYS_CreateThread(SDL_Thread * thread, void *args,
                     pfnSDL_CurrentBeginThread pfnBeginThread,
                     pfnSDL_CurrentEndThread pfnEndThread)
#else
int
SDL_SYS_CreateThread(SDL_Thread * thread, void *args)
#endif /* SDL_PASSED_BEGINTHREAD_ENDTHREAD */
{

	pThreadStartParms pThreadParms =
		(pThreadStartParms)SDL_malloc(sizeof(tThreadStartParms));
	const DWORD flags = thread->stacksize ? STACK_SIZE_PARAM_IS_A_RESERVATION : 0;
	if (!pThreadParms) {
		return SDL_OutOfMemory();
	}
	/* Save the function which we will have to call to clear the RTL of calling app! */
	
	/* Also save the real parameters we have to pass to thread function */
	pThreadParms->args = args;

    //return SDL_SetError("Threads are not supported on this platform");
	DWORD dwThreadId = 0;
	thread->handle = (SYS_ThreadHandle)CreateThread(NULL, 0, RunThreadViaCreateThread, pThreadParms, 0, &dwThreadId);
	thread->threadid = thread->handle;
	
	if (thread->handle == 0) {
		return SDL_SetError("Not enough resources to create thread");
	}
	return 0;
}

void
SDL_SYS_SetupThread(const char *name)
{
    return;
}

SDL_threadID
SDL_ThreadID(void)
{
	return GetCurrentThreadId();
}

int
SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority)
{
    return (0);
}

void SDL_SYS_WaitThread(SDL_Thread * thread)
{
	WaitForSingleObject((HANDLE)thread->handle, -1);
	CloseHandle((HANDLE)thread->handle);
}


void SDL_SYS_DetachThread(SDL_Thread * thread)
{
	CloseHandle((HANDLE)thread->handle);
}

/* vi: set ts=4 sw=4 expandtab: */
