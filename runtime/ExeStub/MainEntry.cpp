#include <minwindef.h>
#include <tls.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>

extern "C" void _cdecl InitializeConstructors();
extern "C" void _cdecl Exit();
void TrickCode();
extern "C" int main(int argc, char** argv);

FILE g_stdin;
FILE g_stdout;
FILE g_stderr;

extern "C" FILE* stdin = 0;
extern "C" FILE* stdout = 0;
extern "C" FILE* stderr = 0;


extern "C" int MainCRTStartupDLL(void* args)
{
	//printf("MainCRTStartup Start!!\n");

	stdin = &g_stdin;
	stdout = &g_stdout;
	stderr = &g_stderr;

	strcpy(stdin->_name, "STDIN");
	strcpy(stdout->_name, "STDOUT");
	strcpy(stderr->_name, "STDERR");
	
	main_args* param = (main_args*)args;

	int res = main(param->argc, param->argv);

	//printf("MainCRTStartup End!!\n");

	return res;
}

extern "C" BOOL __stdcall _DllMainCRTStartup(HANDLE  hDllHandle, DWORD   dwReason, LPVOID  lpreserved)
{
	if (dwReason == DLL_PROCESS_START)
	{
		
		TrickCode();
		main_args* args = (main_args*)lpreserved;
		//SetPlatformAPI(*(PlatformAPI*)(args->platformapi));
		InitializeConstructors();
		
		//thread_storage_t    Tls;
		//tls_create(&Tls);

		/*char* str = (char*)malloc(256);
		sprintf(str, "tls test : %s\n", args->argv[0]);

		tss_set(5, str);
		char* tlsDate = (char*)tss_get(5);
		printf("tls value %s", tlsDate);*/

		int result = MainCRTStartupDLL(args);
		
		//tls_cleanup(thrd_current(), NULL, 0);
		//tls_destroy(tls_current());

		Exit();
		return result;
	}

	return TRUE;
}

void TrickCode()
{
	//TrickCode
	int trojan = 0;
	if (trojan)
	{
		_DllMainCRTStartup(0, 0, 0);

	}
}

//WIN32 Àü¿ë
extern "C" int __declspec(dllexport) MainCRTStartup(void* args)
{
	InitializeConstructors();
	thread_storage_t    Tls;
	tls_create(&Tls);
	int result = MainCRTStartupDLL(args);
	tls_cleanup(thrd_current(), NULL, 0);
	tls_destroy(tls_current());
	Exit();
	
	return result;
}