#pragma once

#include "ktypes.h"
#include "SystemAPI.h"

typedef void (*DebugHook)(int argc, const char** argv);

class Debugger
{
public:
	static Debugger* GetInstance()
	{
		if (m_pDebugger == 0)
			m_pDebugger = new Debugger();

		return m_pDebugger;
	}

	void Bootstrap();
	void AddCommand(const char name[], const char description[], DebugHook fn);
	void RemoveCommand(DebugHook hook);
	void BinDump(const char data[], int size);

	bool ExecuteCommand(char* command);
	void DebugKernel();

protected:
	int ParseArguments(char buf[], const char* argv[], int maxArgs);

private:
	Debugger();
	static Debugger* m_pDebugger;

};

