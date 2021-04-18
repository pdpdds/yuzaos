#include "SkyFacade.h"
#include "PIT.h"
#include "cpu_asm.h"
#include "interrupt.h"
#include "SystemAPI.h"
#include "BootParams.h"
#include <memory_layout.h>
#include "PlatformAPI.h"
#include "memory.h"
#include <MultiBoot.h>
#include <stringdef.h>
#include "SkyConsole.h"
#include "ModuleManager.h"
#include <StackTracer.h>
#include "SystemProfiler.h"
#include "x86arch.h"
#include <Constants.h>
#include <IDT.h>
#include <BuildOption.h>
#include <PEImage.h>
#include <LoadDLL.h>

#if SKY_EMULATOR
#include "../win32stub/OrangeOSAPI.h"
#include "../win32stub/SkyOSWin32Stub.h"
#endif
#include <SkyGUISystem.h>
#include <SkyGUIConsole.h>

#include <KeyBoard/KeyboardController.h>
#include <FileManager.h>

#include "SerialPort.h"
#include <DeviceDriverManager.h>

#include <libconfig.h>

extern unsigned int g_startTickCount;
#if SKY_EMULATOR
unsigned int kHeapBase = 0xc0000000;
unsigned int kIOAreaBase;
#endif 

#define VIDEO_RAM_LOGICAL_ADDRESS_OFFSET	0x01000000
extern void AcpiInit();
extern "C" void InitializeConstructors();

extern "C" FILE* g_stdOut = 0;
extern "C" FILE* g_stdIn = 0;
extern "C" FILE* g_stdErr = 0;

#if SKY_EMULATOR
CRITICAL_SECTION g_interrupt_cs;
#endif

typedef I_FileManager* (*PCreateFileManager)();
typedef FileSysAdaptor* (*PFileSysAdaptor)();

extern "C" {
	void trap0(); 
	void trap1(); 
	void trap2(); 
	void trap3(); 
	void trap4();
	void trap5(); 
	void trap6(); 
	void trap7(); 
	void trap8(); 
	void trap9();
	void trap10(); 
	void trap11(); 
	void trap12(); 
	void trap13(); 
	void trap14();
	void trap16(); 
	void trap17(); 
	void trap18();
	void Trap_TimerHandler_32();
	void trap33();
	void trap34(); 
	void trap35(); 
	void trap36(); 
	void trap37(); 
	void trap38();
	void trap39(); 
	void trap40(); 
	void trap41(); 
	void trap42(); 
	void trap43();
	void trap44(); 
	void trap45(); 
	void trap46(); 
	void trap47(); 
	void trap50();
};

BootParams g_bootParams;
PlatformAPI g_platformAPI;
extern idt_descriptor* g_pIDT;

extern void YuzaOSConsole(char* consoleName);
extern void YuzaOSGUI(char* desktopName);
extern bool InitKernelSystem();
extern void StartNativeSystem(void* param);

void SetInterruptVectors();
void InitInterrupt();
bool BuildPlatformAPI();
bool InitEnvironment();
bool AddEnvironment(config_t& cfg, char* element, char* envName);
bool InitStorageSystem(const char* configName);
bool InitDebuggerSystem(const char* configName);
void KernelThreadProc();
void JumpToNewKernelEntry(int entryPoint, unsigned int procStack);
void InitPCI();
bool AddEnvironmentForced(config_t& cfg, char* element, char* envName);
extern void InterrputDefaultHandler();
bool InitSerialPortSystem();

void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col)
{
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * 1024);
			lfb0[index] = col;
		}
}

void KernelThreadProc()
{	
	kprintf("KernelThreadProc Entered. base : %x size : %x\n", g_bootParams._memoryInfo._kernelBase, g_bootParams._memoryInfo._kernelSize);
	
	InitKernelSystem();

	SetInterruptVector(32, (void(__cdecl &)(void))Trap_TimerHandler_32);
	SetInterruptVector(47, InterrputDefaultHandler, 0);
	StartPITCounter(1000, I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);
	
	ModuleManager::GetInstance()->Initialize();
	
	//윈도우 로더가 아니므로 DLL을 링크했다 하더라도 DLL이 로드된 것은 아니므로
		//임포트 DLL을 분석해서 메모리에 올리고 임포트 함수의 정확한 주소를 임포트 테이블에 연결한다.
	FixIAT((void*)g_bootParams._memoryInfo._kernelBase);
	
	//EXE를 재배치시킨다.
#if !SKY_EMULATOR
	//RelocatePE(g_bootParams._memoryInfo._kernelBase, g_bootParams._memoryInfo._kernelSize, 0x80000000);
#endif

	SetCurrentDriveId('Z');
	SystemProfiler::GetInstance()->Initialize();

	//AcpiInit();
#if !SKY_EMULATOR
	InitSerialPortSystem();
	InitPCI();
#endif
	InitEnvironment(); 
	InitStorageSystem("driver.cfg");
	InitDebuggerSystem("yuza.cfg");

	char buf[256];
	if (g_bootParams.bGraphicMode == true)
	{	
		SkyGUISystem::GetInstance();
		kGetEnvironmentVariable("DESKTOPMGR", buf, 256);
		YuzaOSGUI(buf);
	}
	else
	{
#if !SKY_EMULATOR
		KeyboardController::SetupInterrupts();
		SkyConsole::Print("Keyboard Init..\n");
#endif
		kGetEnvironmentVariable("CONSOLEMGR", buf, 256);
		YuzaOSConsole(buf);
	}
}

#if SKY_EMULATOR
idt_descriptor g_IDT[I86_MAX_INTERRUPTS] = { 0, };
#endif

bool InitOSSystem(BootParams* pBootParam)
{
	BuildPlatformAPI();
#if SKY_EMULATOR
	kInitializeCriticalSection(&g_interrupt_cs);
#endif
	InitializeConstructors();

#if SKY_EMULATOR	
	unsigned long magic = 0;
	g_bootParams._memoryInfo._kernelBase = 0x00800000;
	g_bootParams._memoryInfo._IDT = (DWORD)&g_IDT;

	g_bootParams._memoryInfo._kHeapSize = (g_bootParams._memoryInfo._memorySize/ PAGE_SIZE) * PAGE_SIZE;
	kHeapBase = (DWORD)(g_bootParams.MemoryRegion[0].begin);
	g_bootParams._memoryInfo._kHeapBase = kHeapBase;
	//g_bootParams._heapVirtualEndAddr = (uint32_t)g_bootParams._heapVirtualStartAddr + g_bootParams._heapFrameCount * PAGE_SIZE;
#else
	memcpy(&g_bootParams, pBootParam, sizeof(BootParams));

#endif

	SkyConsole::Initialize();
	SkyConsole::Print("*** YUZA OS Console System Init ***\n");
	SkyConsole::Print("Boot Loader Name : %s\n", g_bootParams._szBootLoaderName);

	InitInterrupt();

	SkyConsole::Print("%x %x\n", g_bootParams._memoryInfo._kHeapBase, g_bootParams._memoryInfo._kHeapSize);
	memset((void*)kHeapBase, 0, g_bootParams._memoryInfo._kHeapSize);
	SkyConsole::Print("%x %x\n", kHeapBase, kHeapBase + g_bootParams._memoryInfo._kHeapSize);

	kmalloc_init(kHeapBase, g_bootParams._memoryInfo._kHeapSize);

#if SKY_EMULATOR	

	
#if (SKY_CONSOLE_MODE == 0)
	g_bootParams.bGraphicMode = true;
#else
	g_bootParams.bGraphicMode = false;
#endif



	SKYOS_MODULE_LIST* pModule = InitSkyOSModule();

	g_bootParams._moduleCount = pModule->_moduleCount;
	if (g_bootParams._moduleCount > 0)
	{
		g_bootParams.Modules = new BootModule[pModule->_moduleCount];

		for (unsigned int i = 0; i < pModule->_moduleCount; i++)
		{
			g_bootParams.Modules[i].ModuleStart = pModule->_module[i]._startAddress;
			g_bootParams.Modules[i].ModuleEnd = pModule->_module[i]._endAddress;
			g_bootParams.Modules[i].Name = pModule->_module[i]._name;
		}
	}
#endif
	if (!SKY_EMULATOR)
	{
		if (g_bootParams.bGraphicMode == true)
		{
			g_bootParams.framebuffer_width = SKY_WIDTH;
			g_bootParams.framebuffer_height = SKY_HEIGHT;
			g_bootParams.framebuffer_bpp = SKY_BPP;

			SkyGUIConsole::Initialize();
		}
	}
	
	g_stdOut = new FILE;

	g_stdIn = new FILE;
	g_stdErr = new FILE;
	strcpy(g_stdOut->_name, "STDOUT");
	strcpy(g_stdIn->_name, "STDIN");
	strcpy(g_stdErr->_name, "STDERR");
	
	JumpToNewKernelEntry((int)KernelThreadProc, g_bootParams._memoryInfo._kStackBase + g_bootParams._memoryInfo._kStackSize);
	
	return true;
}

void InitInterrupt()
{
	SetInterruptVectors();
	//InitializeSysCall();
}



void SetInterruptVectors()
{
	g_pIDT = (idt_descriptor*)g_bootParams._memoryInfo._IDT;
	SetInterruptVector(eHandleDivideByZero, (void(__cdecl &)(void))trap0);
	SetInterruptVector(eHandleSingleStepTrap, (void(__cdecl &)(void))trap1);
	SetInterruptVector(eHandleNMITrap, (void(__cdecl &)(void))trap2);
	SetInterruptVector(eHandleBreakPointTrap, (void(__cdecl &)(void))trap3);
	SetInterruptVector(eHandleOverflowTrap, (void(__cdecl &)(void))trap4);
	SetInterruptVector(eHandleBoundsCheckFault, (void(__cdecl &)(void))trap5);
	SetInterruptVector(eHandleInvalidOpcodeFault, (void(__cdecl &)(void))trap6);
	SetInterruptVector(eHandleNoDeviceFault, (void(__cdecl &)(void))trap7);
	SetInterruptVector(eHandleDoubleFaultAbort, (void(__cdecl &)(void))trap8);
	SetInterruptVector(9, (void(__cdecl &)(void))trap9);
	SetInterruptVector(ekHandleInvalidTSSFault, (void(__cdecl &)(void))trap10);
	SetInterruptVector(eHandleSegmentFault, (void(__cdecl &)(void))trap11);
	SetInterruptVector(eHandleStackFault, (void(__cdecl &)(void))trap12);
	SetInterruptVector(eHandleGeneralProtectionFault, (void(__cdecl &)(void))trap13);
	SetInterruptVector(eHandlePageFault, (void(__cdecl &)(void))trap14);
	SetInterruptVector(eHandlefpu_fault, (void(__cdecl &)(void))trap16);
	SetInterruptVector(eHandleAlignedCheckFault, (void(__cdecl &)(void))trap17);
	SetInterruptVector(eHandleMachineCheckAbort, (void(__cdecl &)(void))trap18);
	
	//SetInterruptVector(33, (void(__cdecl &)(void))trap33);
	SetInterruptVector(34, (void(__cdecl &)(void))trap34);
	SetInterruptVector(35, (void(__cdecl &)(void))trap35);
	SetInterruptVector(36, (void(__cdecl &)(void))trap36);
	SetInterruptVector(37, (void(__cdecl &)(void))trap37);
	SetInterruptVector(38, (void(__cdecl &)(void))trap38);
	SetInterruptVector(39, (void(__cdecl &)(void))trap39);
	SetInterruptVector(40, (void(__cdecl &)(void))trap40);
	SetInterruptVector(41, (void(__cdecl &)(void))trap41);
	SetInterruptVector(42, (void(__cdecl &)(void))trap42);
	SetInterruptVector(43, (void(__cdecl &)(void))trap43);
	SetInterruptVector(44, (void(__cdecl &)(void))trap44);
	SetInterruptVector(45, (void(__cdecl &)(void))trap45);
	SetInterruptVector(46, (void(__cdecl &)(void))trap46);
	SetInterruptVector(47, (void(__cdecl &)(void))trap47);
	SetInterruptVector(50, (void(__cdecl &)(void))trap50);
}

bool BuildPlatformAPI()
{
#if SKY_EMULATOR	
	WIN32_STUB* pStub = GetWin32Stub();

	g_platformAPI._processInterface = *(SKY_PROCESS_INTERFACE*)pStub->_processInterface;
	g_platformAPI._printInterface = *(SKY_PRINT_INTERFACE*)pStub->_printInterface;

	
	g_bootParams._memoryInfo._kStackBase = 0x30000;
	g_bootParams._memoryInfo._kStackSize = 0x40000;

	strcpy(g_bootParams._szBootLoaderName, "YUZA OS Emulator");

	uint32_t startAddress = PAGE_ALIGN_UP((uint32_t)pStub->_virtualAddress);
	uint32_t endAddress = PAGE_ALIGN_DOWN((uint32_t)pStub->_virtualAddress + pStub->_virtualAddressSize);

	g_bootParams.SetAllocated(startAddress, endAddress, MEMORY_REGION_AVAILABLE);
	g_bootParams._memoryInfo._memorySize = endAddress - startAddress;
	g_bootParams._memoryInfo._kernelSize = pStub->_kernelSize;
#else
	extern SKY_PROCESS_INTERFACE _processInterface;
	//extern SKY_PRINT_INTERFACE		_printInterface;

	g_platformAPI._processInterface = _processInterface;
	//g_platformAPI._printInterface = _printInterface;
#endif	

	return true;
}
#if SKY_EMULATOR
bool SetFrameBufferInfo(WIN32_VIDEO* pVideoInfo)
{
	g_bootParams.framebuffer_addr = pVideoInfo->_frameBuffer;
	g_bootParams.framebuffer_bpp = pVideoInfo->_bpp;
	g_bootParams.framebuffer_width = pVideoInfo->_width;
	g_bootParams.framebuffer_height = pVideoInfo->_height;

	return true;
}
#endif	


bool AddStorageModules(config_setting_t* setting);
bool MountStorageDriver(const char* modulename, const char* moduletype, char preferedDrive, bool fromMemoryconst, const char* pakFile);


extern I_FileManager* g_pFileManager;

bool InitEnvironment()
{
	kprintf("InitEnvironment\n");
	config_t cfg;

	const char* str;
	config_init(&cfg);
	char* config_file = "yuza.cfg";
	
	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, config_file))
	{
		
		kPanic("driver config file load fail : %s", config_file);
		kDebugPrint("%s:%d - %s\n", config_file, config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		
	}

	if (!config_lookup_string(&cfg, "name", &str))
		kPanic("No 'name' setting in configuration file\n");

#if SKY_EMULATOR
	AddEnvironment(cfg, "image.FLOPPY", "FLOPPY");
	AddEnvironment(cfg, "image.HARDDISK", "HARDDISK");
#endif
	AddEnvironment(cfg, "desktop.DESKTOPMGR", "DESKTOPMGR");
	AddEnvironment(cfg, "console.CONSOLE", "CONSOLEMGR");

	AddEnvironmentForced(cfg, "environment.BOOT", "BOOT_DRIVE");

	config_destroy(&cfg);


	return true;
}

bool InitStorageSystem(const char* configName)
{
	config_t cfg;
	
	const char* str;
	config_init(&cfg);
	
	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, configName))
	{
		kDebugPrint("%s:%d - %s\n", configName, config_error_line(&cfg), config_error_text(&cfg));
		kPanic("driver config file load fail : %s", configName);
	}
	
	if (!config_lookup_string(&cfg, "name", &str))
	{
		kPanic("%s : No 'name' setting in configuration file.\n", configName);
	}
	
	kDebugPrint("file : %s, name: %s\n\n", configName, str);

	config_setting_t* setting = config_lookup(&cfg, "storage");

	if (!setting)
	{
		kPanic("storage info fail.\n");
	}

	int count = config_setting_length(setting);
	
	for (int i = 0; i < count; ++i)
	{
		config_setting_t* deviceInfo = config_setting_get_elem(setting, i);
		AddStorageModules(deviceInfo);
	}
	
	config_destroy(&cfg);

	kSetCurrentDriveId('C');
	
	return true;
}

bool AddStorageModules(config_setting_t* setting)
{
	int count = config_setting_length(setting);
	int i;

	kDebugPrint("%-10s  %-10s  %-10s  %-10s  %-10s %-10s\n", "module", "author", "type", "prefered", "filesystem", "enable");
	for (i = 0; i < count; ++i)
	{
		config_setting_t* module = config_setting_get_elem(setting, i);

		/* Only output the record if all of the expected fields are present. */
		const char* modulename, * author, * moduleType, * preferedDrive, * filesystem;
		const char* pakFile = 0;
		int enable = 0;
		int fromFile = 0;

		if (!(config_setting_lookup_string(module, "module", &modulename)
			&& config_setting_lookup_string(module, "author", &author)
			&& config_setting_lookup_string(module, "type", &moduleType)
			&& config_setting_lookup_string(module, "filesystem", &filesystem)
			&& config_setting_lookup_string(module, "preferedDrive", &preferedDrive)
			&& config_setting_lookup_int(module, "enable", &enable)))
			continue;

		config_setting_lookup_string(module, "pakFile", &pakFile);
		config_setting_lookup_int(module, "fromFile", &fromFile);

		kDebugPrint("%-10s  %-10s  %-10s  %-10s  %-10s  %d\n", modulename, author, moduleType, preferedDrive, filesystem, enable);

		if (enable)
		{
			if (strcmp(moduleType, "both") == 0)
			{
				MountStorageDriver(modulename, filesystem, preferedDrive[0], fromFile, pakFile);
			}
			else
			{
#if SKY_EMULATOR
				if (strcmp(moduleType, "emulation") == 0)
#else
				if (strcmp(moduleType, "real") == 0)
#endif
				{
					MountStorageDriver(modulename, filesystem, preferedDrive[0], fromFile, pakFile);
				}

			}
		}
	}
	
	return true;
}

bool MountStorageDriver(const char* modulename, const char* moduletype, char preferedDrive, bool fromMemory, const char* pakFile)
{ 
	HWND hwnd = 0;
	PFileSysAdaptor GetFileSysAdaptor;

	hwnd = ModuleManager::GetInstance()->LoadPE(modulename, fromMemory);
	
	GetFileSysAdaptor = (PFileSysAdaptor)ModuleManager::GetInstance()->GetModuleFunction(hwnd, "CreateFileAdaptor");
	FileSysAdaptor* pFileAdaptor = GetFileSysAdaptor();
	
	bool result = g_pFileManager->AddFileSystem(pFileAdaptor, (char*)moduletype, preferedDrive, (void*)pakFile);
	
	return result;
}

void InitPCI()
{
	DeviceDriverManager::GetInstance()->InitPCIDevices();
}

void JumpToNewKernelEntry(int entryPoint, unsigned int procStack)
{

#if SKY_EMULATOR
	KernelThreadProc();
#else
	__asm
	{
		MOV     AX, 0x10;
		MOV     DS, AX
		MOV     ES, AX
		MOV     FS, AX
		MOV     GS, AX

		MOV     ESP, procStack
		PUSH	0; //파라메터
		PUSH	0; //EBP
		PUSH    0x200; EFLAGS
		PUSH    0x08; CS
		PUSH    entryPoint; EIP
		IRETD
	}
#endif
}

int SerialPortServerThread(void* parameter)
{
	kDebugPrint("Starting SerialPortServerThread\n");

	while (1)
	{

		kSleep(1000);
	}

	kExitThread(0);

	return 0;
}

bool InitSerialPortSystem()
{
	InitializeSerialPort();
	kCreateThread(SerialPortServerThread, "SerialPort", nullptr, 15, 0);
	return true;
}

bool AddSymbol(config_t& cfg, char* element);
bool InitDebuggerSystem(const char* configName)
{
	kprintf("InitDebuggerSystem\n");
	config_t cfg;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, configName))
	{

		kDebugPrint("%s:%d - %s\n", configName, config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		kPanic("file load fail : %s", configName);
	}

	config_setting_t* setting;

	setting = config_lookup(&cfg, "debug.CONFIG");
	int enable = 0;
	int addmap = 0;
	if (setting != NULL)
	{
		config_setting_t* env = config_setting_get_elem(setting, 0);

		config_setting_lookup_int(env, "addmap", &addmap);
		config_setting_lookup_int(env, "enable", &enable);
	}

//#if !SKY_EMULATOR
	if (enable)
	{
		StackTracer::GetInstance()->Init("stacktracer.dll");

		if (addmap)
			AddSymbol(cfg, "debug.MAPFILE");
	}

//#endif
	config_destroy(&cfg);

	return true;
}

bool AddSymbol(config_t& cfg, char* element)
{
	config_setting_t* setting;
	/* Output a list of all books in the inventory. */
	setting = config_lookup(&cfg, element);
	if (setting != NULL)
	{
		int count = config_setting_length(setting);
		int i;

		for (i = 0; i < count; ++i)
		{
			config_setting_t* env = config_setting_get_elem(setting, i);

			const char* name = 0;
			if (!(config_setting_lookup_string(env, "name", &name)))
				continue;

			/*std::string dllName = name;
			std::string mapName = name;
			dllName += ".dll";
			mapName += ".map";*/

			char dllName[256];
			char mapName[256];
			strcpy(dllName, name);
			strcpy(mapName, name);
			strcat(dllName, ".dll");
			strcat(mapName, ".map");

			LOAD_DLL_INFO* pInfo = ModuleManager::GetInstance()->GetSystemPE(dllName);

			if (pInfo == nullptr)
			{
				StackTracer::GetInstance()->AddSymbol(mapName);
			}
			else
			{
				StackTracer::GetInstance()->AddSymbol(mapName, pInfo->image_base);
			}
		}
	}
	return true;
}

bool AddEnvironment(config_t& cfg, char* element, char* envName)
{
	config_setting_t* setting;
	/* Output a list of all books in the inventory. */
	setting = config_lookup(&cfg, element);
	if (setting != NULL)
	{
		int count = config_setting_length(setting);
		int i;

		for (i = 0; i < count; ++i)
		{
			config_setting_t* env = config_setting_get_elem(setting, i);

			const char* name = 0;
			
			int enable = 0;

			if (!(config_setting_lookup_string(env, "name", &name)
				&& config_setting_lookup_int(env, "enable", &enable)))
				continue;

			kSetEnvironmentVariable(envName, name);
			
			char buf[256];
			kGetEnvironmentVariable(envName, buf, 256);
			kDebugPrint("Add VirtualImage: %s\n", buf);

		}
	}
	return true;
}

bool AddEnvironmentForced(config_t& cfg, char* element, char* envName)
{
	config_setting_t* setting;
	/* Output a list of all books in the inventory. */
	setting = config_lookup(&cfg, element);
	if (setting != NULL)
	{
		int count = config_setting_length(setting);
		int i;

		for (i = 0; i < count; ++i)
		{
			config_setting_t* env = config_setting_get_elem(setting, i);

			const char* name = 0;
			if (!(config_setting_lookup_string(env, envName, &name)))
				continue;

			kSetEnvironmentVariable(envName, name);
		}
	}
	return true;
}