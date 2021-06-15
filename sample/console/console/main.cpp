#include <minwindef.h>
#include <yuzaapi.h>
#include <ConsoleManager.h>
#include <GUIConsoleFramework.h>

void ExecNativeConsole()
{
	ConsoleManager manager;

	char commandBuffer[MAXPATH];

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

	ExecNativeConsole();

	printf("Bye!!\n");

	return 0;
}

int main(int argc, char** argv)
{	
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}