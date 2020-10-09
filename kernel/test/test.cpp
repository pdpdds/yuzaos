#include "revision.h"
#include <yuza_support.h>

void OrangeOSConsole(char* consoleName)
{
	SkyConsole::Clear();
	PrintCurrentTime();
	//HANDLE handle = kCreateProcess(consoleName, nullptr, 16);
	
	Debugger::GetInstance()->DebugKernel();

	WatchDogProc(0);
}

void OrangeOSGUI(char* desktopName)
{

}

