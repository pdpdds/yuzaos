#pragma once
#include <memory.h>
#include  <list>

class I_MapFileReader;

typedef struct tag_SymbolInfo
{
	I_MapFileReader* symbolData;
	unsigned int loadedAddress;
}_SymbolInfo;

class StackTracer
{
public:
	~StackTracer();

	static StackTracer* GetInstance()
	{
		if (m_pStackTracer == 0)
			m_pStackTracer = new StackTracer();

		return m_pStackTracer;
	}
	
	void TraceStackWithSymbol(unsigned int maxFrames, unsigned int faultAddress);
	void TraceStackWithProcessId(int processId);

	bool Init(const char* moduleName);
	bool AddSymbol(const char* symbolFile, unsigned int loadedAddress = 0);

protected:	
	bool ResolveAddressInfo(unsigned int eip);
	bool AlreadySymbolLoaded(const char* symbolName);

private:
	StackTracer();
	static StackTracer* m_pStackTracer;
	std::list<I_MapFileReader*> m_symbolList;
	
	bool m_engineInit;
	void* m_moduleHwnd;
};