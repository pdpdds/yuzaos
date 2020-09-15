#include <windef.h>
#include <stdio.h>
#include "GUIConsoleFramework.h"

extern int luamain(int argc, char** argv);

int main_impl(int argc, char** argv)
{
	printf("%s\nLua 5.4.0 Console Start!!\n", (char*)argv[0]);

	luamain(argc, argv);

	return 0;
}

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}