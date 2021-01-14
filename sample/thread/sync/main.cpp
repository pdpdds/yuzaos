#include <minwindef.h>
#include <stdio.h>
#include <systemcall_impl.h>

void MutexTest();
void WaitForSingleObjectTest();

int mutexHandle = 0;
int SampleThread(void* param)
{
	printf("%s\n", param);
	int count = 0;
	while (count < 1000)
	{
		Syscall_LockMutex(mutexHandle);
		printf("SampleThread Lock!!\n");
		//Syscall_Sleep(1000);
		Syscall_UnlockMutex(mutexHandle);
		printf("SampleThread UnLock!!\n");
		//Syscall_Sleep(1000);
		count++;
	}
	
	return 0;
}
HANDLE eventHandle;
int WaitThread(void* param)
{
	printf("WaitThread Entered. %x\n", eventHandle);

	Syscall_Sleep(5000);
	Syscall_SetEvent(eventHandle);
	
	return 0;
}

int main(int argc, char** argv)
{

	WaitForSingleObjectTest();
	MutexTest();

	return 0;
}

void WaitForSingleObjectTest()
{
	eventHandle = Syscall_CreateEvent(0, false, false, "Wait");
	if (eventHandle == nullptr)
		Syscall_Panic("Event Create Fail!!\n");

	Syscall_CreateThread(WaitThread, "Sample", "555", 16, 0);
	printf("Syscall_WaitForSingleObject Entered. %x\n", eventHandle);
	
	Syscall_WaitForSingleObject(eventHandle, -1);
	printf("Syscall_WaitForSingleObject Leave!!\n");
}

void MutexTest()
{
	printf("mutex test!!\n");

	mutexHandle = Syscall_CreateMutex("SampleMutex");
	if (mutexHandle < 0)
		printf("Mutex Create Fail!!\n");

	int result = Syscall_LockMutex(mutexHandle);

	if (0 != result)
		printf("Mutex Lock Error!!\n");

	Syscall_CreateThread(SampleThread, "Sample", "Mutex Thread", 16, 0);

	int count = 0;
	while (count < 1000)
	{
		//Syscall_Sleep(5000);
		Syscall_UnlockMutex(mutexHandle);
		printf("MainThread UnLock!!\n");
		//Syscall_Sleep(1000);
		Syscall_LockMutex(mutexHandle);
		printf("MainThread Lock Aquired!!\n");
		Syscall_Sleep(1);
		count++;
	}

	Syscall_UnlockMutex(mutexHandle);
	Syscall_CloseHandle(mutexHandle);
}

