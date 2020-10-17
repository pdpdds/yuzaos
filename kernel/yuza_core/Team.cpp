#include "AddressSpace.h"
#include "cpu_asm.h"
#include "Debugger.h"
#include <stringdef.h>
#include "Team.h"
#include "Thread.h"
#include "intrinsic.h"
#include <ModuleManager.h>
#include <Image.h>
#include <TeamManager.h>
#include <StackTracer.h>
#include <common_struct.h>
#include <kmalloc.h>
#include <memory_layout.h>
#include <PageCache.h>
#include <Area.h>
#include <systemcall_impl.h>

Team::Team(const char* name, int teamId)
	: Resource(OBJ_TEAM, name),
	m_pThreadList(0),
	m_teamId(teamId),
	m_taskId(0),
	m_image(0),
	m_heapHandle(0)
	//m_userHeap(0)
{
	strcpy(m_szCWD, "/");
	char buf[256];
	kGetEnvironmentVariable("BOOT_DRIVE", buf, 256);
	m_currentDrive = buf[0];

	m_addressSpace = new AddressSpace();
	
	int fl = DisableInterrupts();

	RestoreInterrupts(fl);	
}

Team::~Team()
{
	kprintf("Team %s Deleted\n", GetName());

	int fl = DisableInterrupts();

	if(m_image)
		delete m_image;

	TeamManager::GetInstance()->GetTeamList().Remove(this);

	if (m_addressSpace)
		delete m_addressSpace;

#if SKY_EMULATOR 
	kFreeLibrary(m_moduleHandle);
#endif // SKY_EMULATOR
	auto iter = m_loadedDllList.begin();

	for (; iter != m_loadedDllList.end(); iter++)
		ModuleManager::GetInstance()->UnloadPE((*iter));

	m_loadedDllList.clear();
	
	RestoreInterrupts(fl);
}

void Team::ThreadCreated(Thread *thread)
{
	AcquireRef();
	int fl = DisableInterrupts();
	thread->fTeamListNext = m_pThreadList;
	m_pThreadList = thread;
	thread->fTeamListPrev = &m_pThreadList;
	if (thread->fTeamListNext)
		thread->fTeamListNext->fTeamListPrev = &thread->fTeamListNext;
		
	RestoreInterrupts(fl);
}

bool Team::CreateHeap()
{
	unsigned int UserHeapBase = kUserHeapBase;
	int size = kUserHeapTop - kUserHeapBase;
	m_heapHandle = kCreateArea("VirtualAlloc", &UserHeapBase, EXACT_ADDRESS, size, USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE);

	if (m_heapHandle == 0)
	{
		return false;
	}

	//m_userHeap = create_heap(userFreeRegion, userFreeRegion + size, userFreeRegion + size, 0, 0);
	
	//if (m_userHeap)
		//return true;

	return false;
}

LPVOID Team::AllocateMemory(DWORD dwSize)
{
	return 0;
	/*if (m_userHeap == 0)
		return 0;

	return memory_alloc(dwSize, 1, m_userHeap);*/
}

bool Team::DeallocateMemory(LPVOID lpAddress)
{
	return 0;
	/*if (m_userHeap == 0)
		return false;

	memory_free(lpAddress, m_userHeap);
	return true;*/
}

void Team::ThreadTerminated(Thread *thread)
{
	int fl = DisableInterrupts();
	*thread->fTeamListPrev = thread->fTeamListNext;
	if (thread->fTeamListNext)
		thread->fTeamListNext->fTeamListPrev = thread->fTeamListPrev;

	RestoreInterrupts(fl);
	kDebugPrint("Team::ThreadTerminated, %s, 0x%x\n", GetName(), thread);

	ReleaseRef();

}

Team::Team(const char* name, AddressSpace *addressSpace, int teamId)
	:	Resource(OBJ_TEAM, name),
	m_addressSpace(addressSpace),
	m_pThreadList(0),
	m_teamId(teamId),
	m_taskId(0),
	m_heapHandle(0)
{
	strcpy(m_szCWD, "/");
	m_currentDrive = 'C';
}

//실기 전용

DWORD Rva2Offset(DWORD rva, PIMAGE_SECTION_HEADER psh, PIMAGE_NT_HEADERS pnt)
{
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSeh;
	if (rva == 0)
	{
		return (rva);
	}
	pSeh = psh;
	for (i = 0; i < pnt->FileHeader.NumberOfSections; i++)
	{
		if (rva >= pSeh->VirtualAddress && rva < pSeh->VirtualAddress +
			pSeh->Misc.VirtualSize)
		{
			break;
		}
		pSeh++;
	}
	return (rva - pSeh->VirtualAddress + pSeh->PointerToRawData);
}

bool Team::MapDLL(void* image)
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

	PIMAGE_SECTION_HEADER       pSech = IMAGE_FIRST_SECTION(ntHeaders);//Pointer to first section header

	auto importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!importDir.VirtualAddress || !importDir.Size)
		return false;

	//IMAGE_IMPORT_DESCRIPTOR* importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(ULONG_PTR(image) + importDir.VirtualAddress);
	//IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(ULONG_PTR(image) + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	
	IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)image + \
		Rva2Offset(ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSech, ntHeaders));
	auto fixIATCount = 0;

	kDebugPrint("%x %x %x\n", importDir.VirtualAddress, importDescriptor, ULONG_PTR(image));
	
	SKY_ASSERT((uint32_t)image == ntHeaders->OptionalHeader.ImageBase, "IMAGE BASE");

	/*if (importDescriptor->Name == 0)
	{
		for (;;);
	}*/

	for (; importDescriptor->Name; importDescriptor++)
	{
		kDebugPrint("OriginalFirstThunk: %x\n", importDescriptor->OriginalFirstThunk);
		kDebugPrint("     TimeDateStamp: %x\n", importDescriptor->TimeDateStamp);
		kDebugPrint("    ForwarderChain: %x\n", importDescriptor->ForwarderChain);
		//if (!IsBadReadPtr((char*)image + importDescriptor->Name, 2))

		char* dllName = (PCHAR)((DWORD_PTR)ntHeaders->OptionalHeader.ImageBase + Rva2Offset(importDescriptor->Name, pSech, ntHeaders));
		kDebugPrint("              Name: %x %s\n", importDescriptor->Name, dllName);
		
		void* hwnd = 0;
		hwnd = (void*)ModuleManager::GetInstance()->GetSystemPE(dllName);

		if (!hwnd)
			hwnd = ModuleManager::GetInstance()->LoadPE(dllName);

		ASSERT(hwnd != 0);

		m_loadedDllList.push_back(hwnd);

		//auto thunkData = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);

		PIMAGE_THUNK_DATA32 pthunk;
		if (importDescriptor->OriginalFirstThunk == 0)
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + Rva2Offset(importDescriptor->FirstThunk, pSech, ntHeaders));
		else
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + Rva2Offset(importDescriptor->OriginalFirstThunk, pSech, ntHeaders));
		PIMAGE_THUNK_DATA32 nextthunk;
		
		for (int i = 0; pthunk->u1.Function != 0; i++, pthunk++)
		{

			nextthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);
			if ((pthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
			{
				PIMAGE_IMPORT_BY_NAME pname = (PIMAGE_IMPORT_BY_NAME)((PCHAR)image + Rva2Offset(pthunk->u1.AddressOfData, pSech, ntHeaders));

				//kDebugPrint("  ccccccccccccccc  %x %s\n", pthunk->u1.Function, (char*)pname->Name);
				//for (;;);

				void* p = ModuleManager::GetInstance()->GetModuleFunction(hwnd, (char*)pname->Name);

				if (p)
				{
					nextthunk[i].u1.Function = reinterpret_cast<DWORD>(p);
					//kDebugPrint("Function: %x %s\n", nextthunk[i].u1.Function, (char*)pname->Name);
					fixIATCount++;
				}
			}
		}
	}
	
	kDebugPrint("%d imports parsed!\n", fixIATCount);
	
	return true;
}

void FreeArgument(main_args* args)
{
	for (int i = 0; i < args->argc; i++)
	{
		kfree(args->argv[i]);
	}
	kfree(args->argv);
	kfree(args);
}
char* remove_ext(char* myStr, char extSep, char pathSep);

int Team::StartMainThread(ThreadParam* pParam)
{
	m_image = new Image;
	kDebugPrint("Team::StartMainThread %s\n", pParam->name);

	if (strlen((char*)pParam->name) == 0)
	{
		kTraceCallStack();
		for (;;);
	}

	if (m_image->Open((const char*)pParam->name) != E_NO_ERROR)
		return E_NO_SUCH_FILE;

	if (m_image->Load(*Thread::GetRunningThread()->GetTeam()) != E_NO_ERROR)
	{
		kDebugPrint("error loading image : %s\n", pParam->name);
		return E_ERROR;
	}

	char* retStr = remove_ext(pParam->name, '.', '/');
	std::string name = retStr;
	name += ".map";
	StackTracer::GetInstance()->AddSymbol(name.c_str(), (unsigned int)m_image->fBaseAddress);
	free(retStr);

	dll_start_t dllEntry = reinterpret_cast<dll_start_t>(m_image->GetEntryAddress());

	bool result = MapDLL((void*)m_image->fBaseAddress);

	SKY_ASSERT(result == true, "MapDLL");
	kDebugPrint("%s MapDLL Result : %d\n", pParam->name, result);

	main_args* args = (main_args *)pParam->param;
	args->platformapi = (void*)& g_platformAPI;

	SKY_ASSERT(dllEntry != 0, "dllEntry != 0");

	kDebugPrint("%s Process Start %x %x\n", pParam->name, dllEntry, m_image->fBaseAddress);

	/*result = Syscall_CreateHeap();
	
	if (result == false)
	{
		kPanic("User Heap Create Fail!!");

		return 0;
	}*/

	/*DWORD addr = 0x08002bc0;
	__asm
	{ 
		CALL addr;
	}*/
	dllEntry(0, DLL_PROCESS_START, args);


	FreeArgument(args);

	return E_NO_ERROR;
}

