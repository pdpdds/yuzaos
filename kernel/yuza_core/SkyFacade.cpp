#include "SkyFacade.h"

BootParams g_bootParams;
PlatformAPI g_platformAPI;

extern "C" FILE* g_stdOut = 0;
extern "C" FILE* g_stdIn = 0;
extern "C" FILE* g_stdErr = 0;

void InitStdIO()
{
	g_stdOut = new FILE;
	g_stdIn = new FILE;
	g_stdErr = new FILE;
	strcpy(g_stdOut->_name, "STDOUT");
	strcpy(g_stdIn->_name, "STDIN");
	strcpy(g_stdErr->_name, "STDERR");
}

void InitHeap()
{
	SkyConsole::Print("%x %x\n", g_bootParams._memoryInfo._kHeapBase, g_bootParams._memoryInfo._kHeapSize);
	memset((void*)kHeapBase, 0, g_bootParams._memoryInfo._kHeapSize);
	SkyConsole::Print("%x %x\n", kHeapBase, kHeapBase + g_bootParams._memoryInfo._kHeapSize);

	kmalloc_init(kHeapBase, g_bootParams._memoryInfo._kHeapSize);
}

extern "C" void KernelMainEntry()
{	
	kprintf("KernelMainEntry Entered. base : %x size : %x\n", g_bootParams._memoryInfo._kernelBase, g_bootParams._memoryInfo._kernelSize);
	
	DWORD kernelBase = g_bootParams._memoryInfo._kernelBase;
	DWORD kernelSize = g_bootParams._memoryInfo._kernelSize;
	
	InitKernelSystem();
	
	SetInterruptVector(32, (void(__cdecl &)(void))Trap_TimerHandler_32);
	SetInterruptVector(47, InterrputDefaultHandler, 0);
	StartPITCounter(1000, I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);
	
	ModuleManager::GetInstance()->Initialize();
	//윈도우 로더가 아니므로 DLL을 링크했다 하더라도 DLL이 로드된 것은 아니므로
	//임포트 DLL을 분석해서 메모리에 올리고 임포트 함수의 정확한 주소를 임포트 테이블에 연결한다.
	
	FixIAT((void*)g_bootParams._memoryInfo._kernelBase);

	ModuleManager::GetInstance()->CreateMemoryResourceDisk();
	
	//EXE를 재배치시킨다.
	//RelocatePE(kernelBase, kernelSize, 0x80000000);
	//SystemProfiler::GetInstance()->Initialize();

	SetCurrentDriveId('Z');
	
	//AcpiInit();
#if !SKY_EMULATOR
	InitSerialPortSystem();
	InitPCI();
#endif
	InitEnvironment("yuza.cfg");
	InitDebuggerSystem("yuza.cfg");
	InitStorageSystem("driver.cfg");

	kSetCurrentDriveId('C');

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
#define MEMORY_FILE_COUNT 11
char* g_memoryFileName[] =
{
	"SystemCall.dll",
	"FileManager.dll",
	"math.dll",
	"libconfig.dll",
	"stacktracer.dll",
	"FAT_FileSystem.dll",
	"IDE.dll",
	"floppy.dll",
	"USBMSD.dll",
	"yuza.cfg",
	"driver.cfg",
};

void LoadModules()
{
	YUZAOS_MODULE_LIST* pModule = InitYuzaOSModule((char**)g_memoryFileName, MEMORY_FILE_COUNT);

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
}
#endif

void MakeBootParam(BootParams* pBootParam)
{
#if SKY_EMULATOR	
	unsigned long magic = 0;
	g_bootParams._memoryInfo._kernelBase = 0x00800000;
	g_bootParams._memoryInfo._IDT = (DWORD)&g_IDT;

	g_bootParams._memoryInfo._kHeapSize = (g_bootParams._memoryInfo._memorySize / PAGE_SIZE) * PAGE_SIZE;
	kHeapBase = (DWORD)(g_bootParams.MemoryRegion[0].begin);
	g_bootParams._memoryInfo._kHeapBase = kHeapBase;
	//g_bootParams._heapVirtualEndAddr = (uint32_t)g_bootParams._heapVirtualStartAddr + g_bootParams._heapFrameCount * PAGE_SIZE;

#if (SKY_CONSOLE_MODE == 0)
	g_bootParams.bGraphicMode = true;
#else
	g_bootParams.bGraphicMode = false;
#endif
#else
	memcpy(&g_bootParams, pBootParam, sizeof(BootParams));
#endif
}

void InitGrahphics()
{
	if (g_bootParams.bGraphicMode == true)
	{
		g_bootParams.framebuffer_width = SKY_WIDTH;
		g_bootParams.framebuffer_height = SKY_HEIGHT;
		g_bootParams.framebuffer_bpp = SKY_BPP;

		SkyGUIConsole::Initialize();
	}
}

bool InitOSSystem(BootParams* pBootParam)
{
	MakePlatformAPI();
	
	InitializeConstructors();
	
	MakeBootParam(pBootParam);

	SkyConsole::Initialize();
	SkyConsole::Print("*** YUZA OS Console System Init ***\n");
	SkyConsole::Print("Boot Loader Name : %s\n", g_bootParams._szBootLoaderName);
	
	SetInterruptVectors(); // 인터럽트 초기화

	InitHeap(); // 힙 초기화

#if SKY_EMULATOR
	LoadModules();
#else 
	InitGrahphics();
#endif
	
	InitStdIO();

	// 실제 커널 엔트리로 점프한다
	JumpToNewKernelEntry((int)KernelMainEntry);
	
	return true;
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

bool MakePlatformAPI()
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

	kInitializeCriticalSection(&g_interrupt_cs);
#else
	extern SKY_PROCESS_INTERFACE _processInterface;
	g_platformAPI._processInterface = _processInterface;
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

bool InitEnvironment(const char* config_file)
{
	kprintf("InitEnvironment\n");
	config_t cfg;

	const char* str;
	config_init(&cfg);

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

void JumpToNewKernelEntry(int entryPoint)
{

#if SKY_EMULATOR
	KernelMainEntry();
#else
	unsigned int stackPointer = g_bootParams._memoryInfo._kStackBase + g_bootParams._memoryInfo._kStackSize;

	__asm
	{
		MOV     AX, 0x10;
		MOV     DS, AX
		MOV     ES, AX
		MOV     FS, AX
		MOV     GS, AX

		MOV     ESP, stackPointer
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

bool InitDebuggerSystem(const char* configName)
{
	kprintf("Initialize Debugger System\n");
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

	if (enable)
	{
		StackTracer::GetInstance()->Init("stacktracer.dll");

		if (addmap)
			AddSymbol(cfg, "debug.MAPFILE");
	}

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