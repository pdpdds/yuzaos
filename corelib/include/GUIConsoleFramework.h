#pragma once
#include <minwindef.h>

typedef struct tag_CONSOLE_START_STRUCT
{
	int argc;
	char** argv;
	MAIN_IMPL entry;
}CONSOLE_START_STRUCT;

class GUIConsoleFramework
{
public:
	bool Run();
	bool Run(int argc, char** argv, MAIN_IMPL entry);
	bool MainLoop(CONSOLE_START_STRUCT* args);
	bool MainLoop2(CONSOLE_START_STRUCT* args);
};

