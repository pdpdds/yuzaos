#include <GUIConsoleFramework.h>

extern "C" int grep_main(int argc, char* argv[]);

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, grep_main);	
}