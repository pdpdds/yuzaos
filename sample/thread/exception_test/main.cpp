#include <minwindef.h>
#include "Excpt.h"

__declspec(noreturn) extern "C" void _stdcall _CxxThrowException(void* pExceptionObject, _ThrowInfo * pThrowInfo)
{

}

extern "C"  __declspec(naked)  EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler3(
	
	EXCEPTION_REGISTRATION_RECORD * pRN,
	PCONTEXT pContext,
	PVOID pDC)
{
	
	__asm {
		
		ret     0
	}
}

int main(int argc, char* argv[])
{

	Exc::SetFrameHandler(true);
	Exc::SetThrowFunction(true);

	try {
		
		throw 100;

	}
	catch (int err)
	{
		int j = 1;
	}

	return 0;
}