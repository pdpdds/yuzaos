#pragma once
#include <ktypes.h>
#include "x86.h"

class PhysicalMap;
class Thread;

#pragma pack( push, 1 )
typedef struct tag_ThreadParam
{
	void* param;
	char name[MAXPATH];
	THREAD_START_ENTRY entryPoint;	
}ThreadParam;
#pragma pack( pop )

/// ThreadContext contains the architecture dependent state of a thread
class ThreadContext 
{
public:
	ThreadContext();	// Used for first task.
	ThreadContext(const PhysicalMap*);
	~ThreadContext();
	
	void SwitchTo();
	void SetupThread(THREAD_START_ENTRY startAddress, void *param, unsigned int userStack, unsigned int kernelStack);
	void PrintStackTrace() const;
	static void SwitchFp();

private:
	static void UserThreadStart(unsigned startAddress, unsigned userStack, unsigned param) NORETURN;

	unsigned fStackPointer;
	unsigned fPageDirectory;
	unsigned fKernelStackBottom;
	bool fKernelThread;
	FpState fFpState;
	static ThreadContext *fCurrentTask;

	// Note: this assumes single processor operation.  It should
	// move to Processor.
	static ThreadContext *fFpuOwner;
	static FpState fDefaultFpState;
};