#include <stdio.h>
#include <EASTL/tuple.h>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

eastl::tuple<int, int, int, int> MakeSampleTuple(int a, int b)
{
	return eastl::make_tuple(a + b, a - b, a * b, a / b);
}/*
#include <minwindef.h>
struct EHExceptionRecord
{
	//EXCEPTION_RECORD ExceptionRecord;

};

struct EHRegistrationNode
{
	EHRegistrationNode* prev;
	DWORD ehhandler_code;
	DWORD id;
	DWORD saved_ebp;
};

int __CxxFrameHandler3(
	EHExceptionRecord* pExcept,
	EHRegistrationNode* pRN,
	void* pContext,
	void* pDC)
{
	return 0;
}*/
#include "exceptional.h"
#include <assert.h>

int main(void)
{
	int add, sub, div;
	eastl::tie(add, sub, eastl::ignore, div) = MakeSampleTuple(15, 18);

	printf("%d, %d, %d\n", add, sub, div);

	printf("-> simple try/catch with raise\n");

	volatile int testvar = 0;
	++testvar;
	try {
		printf("inside try\n");
		++testvar;
		throw(42);
		assert(false);
	}
	catch (err) {
		printf("catch: should be run, thrown error code: %d\n", err);
		assert(err == 42);
		++testvar;
	}
	++testvar;
	assert(testvar == 4);

	return 0;
}