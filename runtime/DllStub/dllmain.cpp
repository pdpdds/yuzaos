#include <minwindef.h>
#include <stdio.h>
#include <string.h>
#include <platformapi.h>

PlatformAPI g_platformAPI;
extern "C" void _cdecl InitializeConstructors();
extern "C" void _cdecl Exit();

FILE g_stdin;
FILE g_stdout;
FILE g_stderr;

extern "C" FILE * stdin = 0;
extern "C" FILE * stdout = 0;
extern "C" FILE * stderr = 0;

char* strCpy(char* s1, const char* s2)
{
	char* s1_p = s1;
	while (*s1++ = *s2++);
	return s1_p;
}


bool __stdcall DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{

	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" BOOL __stdcall _DllMainCRTStartup(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		InitializeConstructors();

		stdin = &g_stdin;
		stdout = &g_stdout;
		stderr = &g_stderr;

		strCpy(stdin->_name, "STDIN");
		strCpy(stdout->_name, "STDOUT");
		strCpy(stderr->_name, "STDERR");
	}

	bool retcode = DllMain(hDllHandle, dwReason, lpreserved);

	if (dwReason == DLL_PROCESS_DETACH)
	{
		Exit();
	}

	return retcode;
}