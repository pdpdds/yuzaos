#include <minwindef.h>
#include <stdio.h>
#include <winapi.h>
#include <math.h>
#include <systemcall_impl.h>

DWORD WINAPI TestProc(LPVOID parameter)
{
	int count = 0;
	for (;;)
	{
		printf("Thread Test %d!!\n", count);
		Syscall_Sleep(2000);
		count++;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	DWORD dwId = 0;
	HANDLE hThread = 0;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestProc, 0, 0, &dwId);

	Syscall_Sleep(5000);
	SuspendThread(hThread);

	printf("Thread 0x%x Suspended\n", hThread);

	WaitForSingleObject(hThread, INFINITE);

	return 0;
}
