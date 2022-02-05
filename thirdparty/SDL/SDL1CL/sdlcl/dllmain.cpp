//#include <windows.h>
#include <minwindef.h>

extern "C" void initlib();
extern "C" void quitlib(void);

BOOL WINAPI DllMain2(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		//initlib();
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		//quitlib();
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

extern "C" void _cdecl Exit();
extern "C" void _cdecl InitializeConstructors();



extern "C" __declspec(dllexport) void InitializeDll()
{
	initlib();
}

extern "C" BOOL __stdcall _DllMainCRTStartup2(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		InitializeConstructors();
	}

	bool retcode = DllMain2(hDllHandle, dwReason, lpreserved);

	if (dwReason == DLL_PROCESS_DETACH)
	{
		Exit();
	}

	return retcode;
}