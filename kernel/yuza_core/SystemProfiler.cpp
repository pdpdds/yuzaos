#include "SystemProfiler.h"

SystemProfiler* SystemProfiler::m_pSystemProfiler = nullptr;

SystemProfiler::SystemProfiler()
{
}

SystemProfiler::~SystemProfiler()
{
}

bool SystemProfiler::Initialize()
{
	/*GlobalSate state;

#if SKY_EMULATOR
	state._HeapLoadAddress = bootParams.allocatedRange[0].begin + 0x2000000;
#else
	state._HeapLoadAddress = bootParams._memoryLayout._kHeapBase;
#endif
	state._heapSize = HeapManager::GetHeapSize();
	state._kernelLoadAddress = bootParams._memoryLayout._kernelBase;
	
	state._kernelSize = bootParams._kernelSize;

	//state._stackPhysicalPoolAddress = g_stackPhysicalAddressPool;

#if !SKY_EMULATOR
	UINT16 pciDevices = DeviceDriverManager::GetInstance()->InitPCIDevices();
	SkyConsole::Print("%d device(s) found\n", pciDevices);

	ScanPCIDevices();
	state._pciDevices = pciDevices;
#else
	state._pciDevices = 0;
#endif 
	SystemProfiler::GetInstance()->SetGlobalState(state);

	return true;*/
	return false;
}

void SystemProfiler::PrintMemoryState()
{
	/*int memorySize = bootParams._memorySize;
	int freeSize = PhysicalMemoryManager::GetFreeMemorySize();
	int heapSize = HeapManager::GetHeapSize();
	int usedHeapSize = HeapManager::GetUsedHeapSize();

	SkyConsole::Print("Total Memory : %d(MB)\n", memorySize / MEGA_BYTES);
	SkyConsole::Print("Available Memory : %d(MB)\n", freeSize / MEGA_BYTES);

	SkyConsole::Print("Heap Size : %d(MB)\n", heapSize / MEGA_BYTES);
	SkyConsole::Print("Used Heap Size : %d(Bytes)\n", usedHeapSize);*/
}

void SystemProfiler::PrintGlobalState()
{
	/*SkyConsole::Print("Kernel Load Address : 0x%x\n", m_state._kernelLoadAddress);
	SkyConsole::Print("Kernel Size : %d(KB)\n", m_state._kernelSize/ KILO_BYTES);

	SkyConsole::Print("Heap Start Address : 0x%x\n", m_state._HeapLoadAddress);
	SkyConsole::Print("Heap Size : %d(MB)\n", m_state._heapSize / MEGA_BYTES);

	SkyConsole::Print("Stack Physical Pool Address : 0x%x\n", m_state._stackPhysicalPoolAddress);	*/
}