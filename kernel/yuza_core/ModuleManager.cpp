#include "ModuleManager.h"
#include "LoadDLL.h"
#include "PlatformAPI.h"
#include <stringdef.h>
#include "MultiBoot.h"
#include "SystemAPI.h"
#include "LoadDLL.h"
#include "PEImage.h"
#include "PEFile.h"
#include <GRUBManupulation.h>
#include <FileManager.h>
#include <MemoryResourceAdaptor.h>
#include "MemoryResourceFS.h"
#include <StackTracer.h>
#include <BuildOption.h>

ModuleManager* ModuleManager::m_pModuleManager = nullptr;
I_FileManager* g_pFileManager = 0;
extern void RegisterSysCall();

#if SKY_EMULATOR
#include "../win32stub/SkyOSWin32Stub.h"
bool emulation = true;
#else
bool emulation = false;
#endif

ModuleManager::ModuleManager()
{
}

ModuleManager::~ModuleManager()
{
}

bool ModuleManager::Initialize()
{
	//System PE Load

	kprintf("Module Load\n");
	
	//SystemCall.dll
	void* handle = LoadPE("SystemCall.dll", true);
	
	SKY_ASSERT(handle != nullptr, "SystemCall.dll");
	pSetPlatformAPI SetPlatformAPI = (pSetPlatformAPI)GetModuleFunction(handle, "SetPlatformAPI");
	SKY_ASSERT(SetPlatformAPI != nullptr, "SystemCall.dll SetPlatformAPI");
	SetPlatformAPI(g_platformAPI, emulation);
	m_systemPEList.push_back((LOAD_DLL_INFO*)handle);

	kprintf("SystemCall.dll Load\n");
	
	//Register System Call
	RegisterSysCall();

	//FileManager.dll
	handle = LoadPE("FileManager.dll", true);
	SKY_ASSERT(handle != nullptr, "FileManager.dll");
	PCreateFileManager GetFileManager;
	GetFileManager = (PCreateFileManager)GetModuleFunction(handle, "CreateFileManager");
	SKY_ASSERT(GetFileManager != nullptr, "FileManager.dll GetFileManager");
	g_pFileManager = GetFileManager();
	SKY_ASSERT(g_pFileManager != nullptr, "FileManager.dll g_pFileManager");
	g_pFileManager->AddFileSystem(new MemoryResourceAdaptor("MemoryResourceDisk"), new MemoryResourceFS("MEMORY"), 'Z');
	m_systemPEList.push_back((LOAD_DLL_INFO*)handle);

	kprintf("FileManager.dll Load\n");
	
	handle = LoadPE("math.dll", true);
	SKY_ASSERT(handle != nullptr, "math.dll");
	m_systemPEList.push_back((LOAD_DLL_INFO*)handle);

	kprintf("math.dll Load\n");
	
	handle = LoadPE("libconfig.dll", true);
	SKY_ASSERT(handle != nullptr, "libconfig.dll");
	m_systemPEList.push_back((LOAD_DLL_INFO*)handle);

	kprintf("libconfig.dll Load\n");

	/*handle = LoadPE("libwin32.dll", true);
	SKY_ASSERT(handle != nullptr, "libwin32.dll");
	m_systemPEList.push_back((LOAD_DLL_INFO*)handle);
	*/

	return true;
}

void* ModuleManager::LoadPE(const char* fileName, bool fromMemory)
{
	kDebugPrint("LoadPE %s\n", fileName);

#if SKY_EMULATOR_DLL
	void* hwnd = (void*)g_platformAPI._processInterface.sky_LoadLibrary(fileName);
	if (hwnd == nullptr)
	{
		kDebugPrint("LoadPE Fail. PEName:  %s\n", fileName);
		return nullptr;
	}

	FixIAT(hwnd);
	InitPE(hwnd);

	return hwnd;
#endif

	LOAD_DLL_INFO* dllInfo = 0;
	
	if (fromMemory == true)
		dllInfo = (LOAD_DLL_INFO*)LoadPEFromMemory(fileName);
	else
		dllInfo = (LOAD_DLL_INFO*)LoadPEFromFile(fileName);
	
	if (dllInfo == nullptr)
		kPanic("LoadPE Fail. PEName:  %s\n", fileName);

	PrintPEHierachy(dllInfo);

	pInitializeDll InitializeDll = (pInitializeDll)GetModuleFunction(dllInfo, "InitializeDll");
	if (InitializeDll != nullptr)
		InitializeDll();

	return dllInfo;
}

bool ModuleManager::IsSystemPE(const char* moduleName)
{
	auto iter = m_systemPEList.begin();

	for (; iter != m_systemPEList.end(); iter++)
	{
		if (strcmp((*iter)->moduleName, moduleName) == 0)
			return true;
	}

	return false;
}

bool ModuleManager::UnloadPE(void* handle)
{
#if SKY_EMULATOR_DLL
	return (void*)g_platformAPI._processInterface.sky_FreeLibrary(handle);

#endif

	LOAD_DLL_INFO* info = (LOAD_DLL_INFO*)handle;
	if (IsSystemPE(info->moduleName))
		return true;
	
	kDebugPrint("UnloadPE %s\n", info->moduleName);
	
	bool res = UnloadDLL(info);
	assert(res == true);

	delete handle;

	return res;
}

void* ModuleManager::GetModuleFunction(void* handle, const char* func_name)
{
	if (strcmp(func_name, "sky_fopen") == 0 || 
		strcmp(func_name, "sky_fread") == 0 || 
		strcmp(func_name, "sky_fwrite") == 0 || 
		strcmp(func_name, "sky_fseek") == 0 || 
		strcmp(func_name, "sky_fclose") == 0)
		return (void*)g_platformAPI._processInterface.sky_GetProcAddress(handle, func_name);
#if SKY_EMULATOR_DLL
	return (void*)g_platformAPI._processInterface.sky_GetProcAddress(handle, func_name);
#else
	return (void*)myGetProcAddress_LoadDLLInfo((MODULE_HANDLE)handle, func_name);
#endif
}


void ModuleManager::PrintPEHierachy(HMODULE hwnd)
{
	LOAD_DLL_INFO* pInfo = (LOAD_DLL_INFO*)hwnd;

	if (pInfo == nullptr || pInfo->num_import_modules == 0)
		return;

	kDebugPrint("DLL Hierarchy %s\n", pInfo->moduleName);

	for (int i = 0; i < (int)pInfo->num_import_modules; ++i)
		kDebugPrint("%s\n", pInfo->loaded_import_modules_array[i].moduleName);
}

LOAD_DLL_INFO* ModuleManager::GetSystemPE(const char* moduleName)
{
	auto iter = m_systemPEList.begin();

	for (; iter != m_systemPEList.end(); iter++)
	{
		if (strcmp((*iter)->moduleName, moduleName) == 0)
			return (*iter);
	}

	return nullptr;
}



bool ModuleManager::InitPE(void* image)
{
	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	/* only supporting for i386 archs */
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	/* only support 32 bit executable images */
	if (!(ntHeaders->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
		return false;

	/*
	Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
	kernel stack and page directory. If you want to support loading below 1MB,
	make sure to move these into kernel land
	*/

	//로드되는 프로세스의 베이스 주소는 0x00400000다. 
	//비쥬얼 스튜디오에서 속성=> 링커 => 고급의 기준주소 항목에서 확인 가능하다
	if ((ntHeaders->OptionalHeader.ImageBase < 0x400000) || (ntHeaders->OptionalHeader.ImageBase > 0x80000000))
		return false;

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	auto importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!importDir.VirtualAddress || !importDir.Size)
		return false;

	auto importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(ULONG_PTR(image) + importDir.VirtualAddress);
	auto fixIATCount = 0;

	for (; importDescriptor->FirstThunk; importDescriptor++)
	{
		kDebugPrint("OriginalFirstThunk: %x\n", importDescriptor->OriginalFirstThunk);
		kDebugPrint("     TimeDateStamp: %x\n", importDescriptor->TimeDateStamp);
		kDebugPrint("    ForwarderChain: %x\n", importDescriptor->ForwarderChain);
		kDebugPrint("              Name: %x %s\n", importDescriptor->Name, (char*)image + importDescriptor->Name);

		char* dllName = (char*)image + importDescriptor->Name;

		HMODULE hwnd = g_platformAPI._processInterface.sky_GetModuleHandle(dllName);

		pInitializeDll InitializeDll = (pInitializeDll)GetModuleFunction(hwnd, "InitializeDll");
		if (InitializeDll != nullptr)
			InitializeDll();

		InitPE((void*)hwnd);
	}

	return true;
}

MODULE_HANDLE ModuleManager::LoadPEFromMemory(const char* moduleName)
{
	BootModule* pModule = FindFileFromMemory(moduleName);

	if (!pModule)
		kPanic("LoadPEFromMemory Fail : %s Load Fail!!", moduleName);
	else if(false == ValidatePEImage((void*)pModule->ModuleStart))
		kPanic("LoadPEFromMemory. invalid PE : %s\n", pModule->Name);

	kDebugPrint("LoadPEFromMemory %s %x\n", moduleName, pModule);
	
	LOAD_DLL_INFO *p = new LOAD_DLL_INFO;
	strcpy(p->moduleName, moduleName);
	p->refCount = 1;

	DWORD res = LoadDLLFromMemory((void*)pModule->ModuleStart, ((size_t)(pModule->ModuleEnd) - (size_t)pModule->ModuleStart), 0, p);
	if (res != ELoadDLLResult_OK)
		kPanic("LoadPEFromMemory Fail. Name : %s, Result : %d\n", pModule->Name, res);
	
	kDebugPrint("%s Module Loaded\n", moduleName);

	return p;
}

MODULE_HANDLE ModuleManager::LoadPEFromFile(const char* moduleName)
{
	LOAD_DLL_INFO *p = new LOAD_DLL_INFO;
	strcpy(p->moduleName, moduleName);
	p->refCount = 1;

	DWORD res = LoadDLLFromFileName(moduleName, 0, p);
	if (res != ELoadDLLResult_OK)
		kPanic("LoadPEFromFile Fail. Name : %s, Result : %d\n", moduleName, res);
	
	return p;
}





