#include <stdio.h>
#include <systemcall_impl.h>
#include <minwindef.h>
#include <time.h>

DWORD WINAPI Test(LPVOID parameter)
{
	printf("Message Sent From Main Thread : %s\n", parameter);
	return 0;
}

int main(int argc, char** argv)
{
	printf("Hello World!!\n"); 

	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("Current local time and date: %s", asctime(timeinfo));

	HANDLE handle = (HANDLE)Syscall_CreateThread(Test, "Child Thread", "Hello World1", 16, 0);
	 handle = (HANDLE)Syscall_CreateThread(Test, "Child Thread2", "Hello World2", 16, 0);
	 handle = (HANDLE)Syscall_CreateThread(Test, "Child Thread3", "Hello World3", 16, 0);
	 handle = (HANDLE)Syscall_CreateThread(Test, "Child Thread4", "Hello World4", 16, 0);
	 handle = (HANDLE)Syscall_CreateThread(Test, "Child Thread5", "Hello World5", 16, 0);
	//Syscall_CloseHandle(handle);

	//Syscall_Sleep(1000);

	return 0;
}
