#include <orangeos.h>
#include <SystemCall_Impl.h>
#include <stringdef.h>

#include <platformapi.h>
#include <memory.h>
#include <stdbool.h>

extern PlatformAPI g_platformAPI;
extern "C" bool g_emulation = false;

extern "C" __declspec(dllexport) void SetPlatformAPI(PlatformAPI api, bool emulation)
{
	g_emulation = emulation;

	if(g_emulation)
		g_platformAPI = api;

}

/* Extern
* - Access to assembler for the platform */
__EXTERN "C" SCTYPE _syscall(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4);

typedef SCTYPE(*__SysCall0)();
typedef SCTYPE(*__SysCall1)(SCTYPE);
typedef SCTYPE(*__SysCall2)(SCTYPE, SCTYPE);
typedef SCTYPE(*__SysCall3)(SCTYPE, SCTYPE, SCTYPE);
typedef SCTYPE(*__SysCall4)(SCTYPE, SCTYPE, SCTYPE, SCTYPE);
typedef SCTYPE(*__SysCall5)(SCTYPE, SCTYPE, SCTYPE, SCTYPE, SCTYPE);

SCTYPE syscall0(SCTYPE Function) 
{	
	if (g_emulation)
	{
		__SysCall0 Func = (__SysCall0)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func();
	}
		
	return _syscall(Function, 0, 0, 0, 0, 0);
}

SCTYPE syscall1(SCTYPE Function, SCTYPE Arg0) 
{
	if (g_emulation)
	{
		__SysCall1 Func = (__SysCall1)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func(Arg0);
	}
	return _syscall(Function, Arg0, 0, 0, 0, 0);

}

SCTYPE syscall2(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1) 
{
	if (g_emulation)
	{
		__SysCall2 Func = (__SysCall2)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func(Arg0, Arg1);
	}

	return _syscall(Function, Arg0, Arg1, 0, 0, 0);
}

SCTYPE syscall3(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2) 
{	
	if (g_emulation)
	{
		__SysCall3 Func = (__SysCall3)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func(Arg0, Arg1, Arg2);
	}
	return _syscall(Function, Arg0, Arg1, Arg2, 0, 0);

}

SCTYPE syscall4(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3) 
{	
	if (g_emulation)
	{
		__SysCall4 Func = (__SysCall4)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func(Arg0, Arg1, Arg2, Arg3);
	}
	return _syscall(Function, Arg0, Arg1, Arg2, Arg3, 0);
}

SCTYPE syscall5(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4) 
{
	if (g_emulation)
	{
		__SysCall5 Func = (__SysCall5)g_platformAPI._processInterface.sky_GetOrangeOSAPIByIndex(Function);
		return Func(Arg0, Arg1, Arg2, Arg3, Arg4);
	}
	return _syscall(Function, Arg0, Arg1, Arg2, Arg3, Arg4);
}
