#include <minwindef.h>
#include <Excpt.h>

extern "C" __declspec(dllexport) void InitializeDll()
{
	Exc::SetFrameHandler(true);
	Exc::SetThrowFunction(true);
}

