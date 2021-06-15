#pragma once
#include <BuildOption.h>
#include "SkyFacade.h"
#include <MultiBoot.h>
#include "SkyConsole.h"
#include "BootParams.h"
#include <PIT.h>
#include <IDT.h>
#include "SystemAPI.h"
#include "ModuleManager.h"
#include <StackTracer.h>
#include <Constants.h>
#include <PEImage.h>
#include <LoadDLL.h>
#include <SkyGUISystem.h>
#include <SkyGUIConsole.h>
#include <KeyBoard/KeyboardController.h>
#include <FileManager.h>
#include "SerialPort.h"
#include <DeviceDriverManager.h>
#include <libconfig.h>

void SetInterruptVectors();
bool MakePlatformAPI();
bool InitEnvironment(const char* config_file);
bool AddEnvironment(config_t& cfg, char* element, char* envName);
bool InitStorageSystem(const char* configName);
bool InitDebuggerSystem(const char* configName);
void JumpToNewKernelEntry(int entryPoint);
void InitPCI();
bool AddEnvironmentForced(config_t& cfg, char* element, char* envName);
extern void InterrputDefaultHandler();
bool InitSerialPortSystem();
bool AddStorageModules(config_setting_t* setting);
bool AddSymbol(config_t& cfg, char* element);
bool MountStorageDriver(const char* modulename, const char* moduletype, char preferedDrive, bool fromMemoryconst, const char* pakFile);

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

extern I_FileManager* g_pFileManager;
extern unsigned int g_startTickCount;
extern idt_descriptor* g_pIDT;

#define VIDEO_RAM_LOGICAL_ADDRESS_OFFSET	0x01000000
extern void AcpiInit();
extern "C" void InitializeConstructors();
extern void YuzaOSConsole(char* consoleName);
extern void YuzaOSGUI(char* desktopName);
extern bool InitKernelSystem();

#if SKY_EMULATOR
#include "../win32stub/OrangeOSAPI.h"
#include "../win32stub/SkyOSWin32Stub.h"

unsigned int kHeapBase = 0xc0000000;
unsigned int kIOAreaBase;
CRITICAL_SECTION g_interrupt_cs;
#else
#include "memory_layout.h"
#endif