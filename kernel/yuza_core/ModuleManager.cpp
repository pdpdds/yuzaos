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

#define MODULE_NAME_SYSTEMCALL 0
#define MODULE_NAME_FILEMANAGER 1
#define MODULE_NAME_MATH 2
#define MODULE_NAME_LIBCONFIG 3
#define MODULE_NAME_MAX 4

char* g_SystemModuleName[] =
{
	"SystemCall.dll",
	"FileManager.dll",
	"math.dll",
	"libconfig.dll",
};

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

void* ModuleManager::AddSystemModule(const char* fileName)
{
	void* handle = LoadPE(fileName, true);

	SKY_ASSERT(handle != nullptr, fileName);
	std::string szFileName;
	szFileName = fileName;
	m_systemPEList[szFileName] = (LOAD_DLL_INFO*)handle;

	kprintf("%s Loaded\n", fileName);

	return handle;
}

void ModuleManager::CreateMemoryResourceDisk()
{
	void* handle = (void*)GetSystemPE(g_SystemModuleName[MODULE_NAME_FILEMANAGER]);
	PCreateFileManager pCreateFileManager;
	pCreateFileManager = (PCreateFileManager)GetModuleFunction(handle, "CreateFileManager");

	SKY_ASSERT(pCreateFileManager != nullptr, "FileManager.dll GetFileManager");
	g_pFileManager = pCreateFileManager();

	SKY_ASSERT(g_pFileManager != nullptr, "FileManager.dll g_pFileManager");
	g_pFileManager->AddFileSystem(new MemoryResourceAdaptor("MemoryResourceDisk"), new MemoryResourceFS("MEMORY"), 'Z');
}

ModuleManager::~ModuleManager()
{
}

//SystemCall.dll, FileManager.dll, math.dll, libconfig.dll
bool ModuleManager::Initialize()
{
	kprintf("System Module Load\n");
	
	void* handle = AddSystemModule(g_SystemModuleName[MODULE_NAME_SYSTEMCALL]);
	fSetPlatformAPI pSetPlatformAPI = (fSetPlatformAPI)GetModuleFunction(handle, "SetPlatformAPI");
	SKY_ASSERT(pSetPlatformAPI != nullptr, g_SystemModuleName[MODULE_NAME_SYSTEMCALL]);
	pSetPlatformAPI(g_platformAPI, emulation);
	
	//Register System Call
	RegisterSysCall();

	AddSystemModule(g_SystemModuleName[MODULE_NAME_FILEMANAGER]);
	AddSystemModule(g_SystemModuleName[MODULE_NAME_MATH]);
	AddSystemModule(g_SystemModuleName[MODULE_NAME_LIBCONFIG]);
	
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
	{
		kDebugPrint("LoadPE Fail. PEName:  %s\n", fileName);
		return nullptr;
	}

	PrintPEHierachy(dllInfo);

	fInitializeDll InitializeDll = (fInitializeDll)GetModuleFunction(dllInfo, "InitializeDll");
	if (InitializeDll != nullptr)
		InitializeDll();

	return dllInfo;
}

bool ModuleManager::IsSystemPE(const char* moduleName)
{
	auto iter = m_systemPEList.begin();

	for (; iter != m_systemPEList.end(); iter++)
	{
		if (strcmp(iter->first.c_str(), moduleName) == 0)
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
		if (strcmp(iter->first.c_str(), moduleName) == 0)
			return iter->second;
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

		fInitializeDll InitializeDll = (fInitializeDll)GetModuleFunction(hwnd, "InitializeDll");
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
	
	LOAD_DLL_INFO* p = new LOAD_DLL_INFO;
	strcpy(p->moduleName, moduleName);
	p->refCount = 1;

	DWORD res = LoadDLLFromFileName(moduleName, 0, p);
	if (res != ELoadDLLResult_OK)
	{
		delete p;
		kDebugPrint("LoadPEFromFile Fail. Name : %s, Result : %d\n", moduleName, res);
		return 0;
	}

	return p;
}





