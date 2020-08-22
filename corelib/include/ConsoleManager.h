#pragma once
class ConsoleManager
{
public:
	ConsoleManager();
	virtual ~ConsoleManager();

	bool RunCommand(const char* buf);
	static char* MakePathName();
};