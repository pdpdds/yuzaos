#include <orangeos.h>
#include <SystemCall_Impl.h>
#include <stringdef.h>


//#ifdef SKY_EMULATOR	
#include <platformapi.h>
#include <memory.h>

#ifdef  SKY_EMULATOR
SKY_FILE_INTERFACE m_fileInterface;
extern PlatformAPI g_platformAPI;
#endif //  SKY_EMULATOR

extern "C" FILE* g_skyStdOut = 0;
extern "C" FILE* g_skyStdIn = 0;
extern "C" FILE* g_skyStdErr = 0;

extern "C" __declspec(dllexport) void SetPlatformAPI(PlatformAPI api)
{
#ifdef  SKY_EMULATOR
	g_platformAPI = api;
#endif //  SKY_EMULATOR

	g_skyStdOut = api._printInterface.sky_stdout;
	g_skyStdIn = api._printInterface.sky_stdin;
	g_skyStdErr = api._printInterface.sky_stderr;

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
#ifdef SKY_EMULATOR	
	__SysCall0 Func = (__SysCall0)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func();
#else
	return _syscall(Function, 0, 0, 0, 0, 0);
#endif
}

SCTYPE syscall1(SCTYPE Function, SCTYPE Arg0) {
#ifdef SKY_EMULATOR	
	__SysCall1 Func = (__SysCall1)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func(Arg0);
#else
	return _syscall(Function, Arg0, 0, 0, 0, 0);
#endif
}

SCTYPE syscall2(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1) {
#ifdef SKY_EMULATOR	
	__SysCall2 Func = (__SysCall2)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func(Arg0, Arg1);
#else
	return _syscall(Function, Arg0, Arg1, 0, 0, 0);
#endif
}

SCTYPE syscall3(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2) {	
#ifdef SKY_EMULATOR
	__SysCall3 Func = (__SysCall3)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func(Arg0, Arg1, Arg2);
#else
	return _syscall(Function, Arg0, Arg1, Arg2, 0, 0);
#endif
}

SCTYPE syscall4(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3) {	
#ifdef SKY_EMULATOR
	__SysCall4 Func = (__SysCall4)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func(Arg0, Arg1, Arg2, Arg3);
#else
	return _syscall(Function, Arg0, Arg1, Arg2, Arg3, 0);
#endif
}

SCTYPE syscall5(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4) {
#ifdef SKY_EMULATOR
	__SysCall5 Func = (__SysCall5)g_platformAPI._processInterface.sky_kGetOrangeOSAPIByIndex(Function);
	return Func(Arg0, Arg1, Arg2, Arg3, Arg4);
#else
	return _syscall(Function, Arg0, Arg1, Arg2, Arg3, Arg4);
#endif
}
