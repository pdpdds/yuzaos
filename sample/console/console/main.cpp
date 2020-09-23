#include <windef.h>
#include <minwindef.h>
#include <yuzaapi.h>
#include <stdio.h>
#include <ConsoleManager.h>
#include "Test.h"
#include <systemcall_impl.h>
#include <string.h>
#include <GUIConsoleFramework.h>

void TestGrep()
{
	ConsoleManager manager;

	char	commandBuffer[MAXPATH] = "grep.exe IDE driver_config.cfg";
	char	commandBuffer2[MAXPATH] = "helloworld.exe";

	int index = 0;
	while (1)
	{			
		if(index == 0)
			manager.RunCommand(commandBuffer);		
		else manager.RunCommand(commandBuffer2);

		index++;
		index = index % 2;

		Syscall_Sleep(100);
	}
}

void NativeConsole()
{
	ConsoleManager manager;

	char	commandBuffer[MAXPATH];

	while (1)
	{
		char* newPath = ConsoleManager::MakePathName();
		
		printf("%s", newPath);

		memset(commandBuffer, 0, MAXPATH);
		GetCommandFromKeyboard(commandBuffer, MAXPATH - 2);

		if (manager.RunCommand(commandBuffer) == true)
			break;
	}
}

int main_impl(int argc, char** argv)
{
	printf("%s\nConsole Mode Start!!\n", (char*)argv[0]);
	//Test();
	//TestGrep();

	int a = 100;
	int j = 0;
	a = a / j;

	//Syscall_SysCallTest("pe");
	
	//Syscall_TraceCallStack();
	NativeConsole();

	printf("Bye!!\n");

	return 0;
}

int main(int argc, char** argv)
{	
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}