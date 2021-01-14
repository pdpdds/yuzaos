#include <windef.h>
#include <minwindef.h>
#include <winapi.h>
#include <stdio.h>

bool hResult = 0;
UINT TIMER_ID_EVENT = 1001;

static void CALLBACK CallbackTimer(HWND, UINT idEvent, DWORD*, DWORD)
{
	static int count = 0;
	printf("CallbackTimer %d %d\n", count, idEvent);
	count++;
	DWORD nIDEvent = 0;
}

static void CALLBACK CallbackTimer2(HWND, UINT idEvent, DWORD*, DWORD)
{
	static int count = 0;
	printf("CallbackTimer2 %d %d\n", count, idEvent);
	count++;
}

int main(int argc, char* argv[])
{
	HANDLE handles[2];
	handles[0] = SetTimer(NULL, TIMER_ID_EVENT, 1000, CallbackTimer);
	handles[1] = SetTimer(NULL, TIMER_ID_EVENT + 100, 2000, CallbackTimer2);

	Sleep(10000); //10 sec
	
	for (DWORD i = 0; i < 2; i++)
	{
		KillTimer(handles[i], nullptr);
	}

	return 0;
}