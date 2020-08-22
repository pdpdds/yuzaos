#pragma once
#include "include/BootParams.h"
#include <kmalloc.h>
#include <list>
#include <FileManager.h>

struct LOAD_DLL_INFO;
typedef LOAD_DLL_INFO* MODULE_HANDLE;

typedef I_FileManager* (*PCreateFileManager)();
typedef FileSysAdaptor* (*PFileSysAdaptor)();

class ModuleManager
{
public:	
	~ModuleManager();

	static ModuleManager* GetInstance()
	{
		if (m_pModuleManager == 0)
			m_pModuleManager = new ModuleManager();

		return m_pModuleManager;
	}

	bool Initialize();

	void* LoadPE(const char* dllName, bool fromMemory = false);
	bool UnloadPE(void* handle);
	void* GetModuleFunction(void* handle, const char* func_name);
	LOAD_DLL_INFO* GetSystemPE(const char* moduleName);
	void PrintPEHierachy(HMODULE hwnd);
	bool IsSystemPE(const char* moduleName);

protected:
	MODULE_HANDLE LoadPEFromMemory(const char* moduleName);
	MODULE_HANDLE LoadPEFromFile(const char* dll_path);
	
	bool InitPE(void* image);
	
private:
	ModuleManager();
	static ModuleManager* m_pModuleManager;
	std::list<LOAD_DLL_INFO*> m_systemPEList;
};