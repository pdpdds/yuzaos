#include <string>
#include "DeviceDriverManager.h"
#include "Hal.h"
#include "intrinsic.h"
#include "SystemAPI.h"

DeviceDriverManager* DeviceDriverManager::m_pDeviceDriverManager = nullptr;

DeviceDriverManager::DeviceDriverManager()
	: m_deviceCount(0)
{
}

DeviceDriverManager::~DeviceDriverManager()
{
}

/* This function fills the PCICfg structure using the configuration method 1
*/
BYTE DeviceDriverManager::GetPCIConfigurationSpace(BYTE Bus, BYTE Device, BYTE Function, PCIConfigurationSpace * PCICfg)
{
	int Reg;
	int First = 1;
	for (Reg = 0; Reg < sizeof(PCIConfigurationSpace) / sizeof(DWORD); Reg++)
	{
		DWORD Addr, Value;
		Addr = 0x80000000L |
			(((DWORD)Bus) << 16) |
			(((DWORD)(Device << 3) | Function) << 8) |
			(Reg << 2);

		OutPortDWord(PCI_CONFIG_ADDRESS, Addr);

		Value = InPortDWord(PCI_CONFIG_DATA);

		Addr = InPortDWord(PCI_CONFIG_ADDRESS);		

		((DWORD *)PCICfg)[Reg] = Value;
		OutPortDWord(PCI_CONFIG_ADDRESS, 0);
		if (First)
		{
			if (Value == 0xFFFFFFFF)
				return 0;
			First = 0;
		}
	}
	return Reg;
}

UINT16 DeviceDriverManager::InitPCIDevices()
{
	m_deviceCount = 0;
	BYTE Bus, Device, Function;
	PCIConfigurationSpace PCICfg;
	PCIDeviceDetails* newPCIDev;

	m_listPCIDevices = new list<PCIDeviceDetails *>();

	for (Bus = 0; Bus < 0xFF; Bus++)
		for (Device = 0; Device < 32; Device++)
			for (Function = 0; Function < 8; Function++)
			{
				BYTE Ret = GetPCIConfigurationSpace(Bus, Device, Function, &PCICfg);
				if (!Ret)
					continue;
				if (!(PCICfg.VendorID == 0xFFFF || PCICfg.DeviceID == 0xFFFFFFFF))
				{
					newPCIDev = new PCIDeviceDetails;
					newPCIDev->Bus = Bus;
					newPCIDev->Device = Device;
					newPCIDev->Function = Function;
					kDebugPrint("Device Address %x\n", PCICfg.PreDefinedHeader.NonBridge.BaseAddress0);

					memcpy(&newPCIDev->PCIConfDetails, &PCICfg, sizeof(PCIConfigurationSpace));
					m_listPCIDevices->push_back(newPCIDev);
					m_deviceCount++;
				}
			}

	kDebugPrint("%d device(s) found\n", m_deviceCount);

	return m_deviceCount;
}

void DeviceDriverManager::EnumeratePCIDevices(BYTE(*CallBackFn) (PCIConfigurationSpace *))
{
	list<PCIDeviceDetails *>::iterator iter = m_listPCIDevices->begin();
	for (; iter != m_listPCIDevices->end(); iter++)
	{
		PCIConfigurationSpace* pPCIDet = &(*iter)->PCIConfDetails;

		if (nullptr != pPCIDet)
		{
			CallBackFn(pPCIDet);
		}
	}
}

PCIDeviceDetails* DeviceDriverManager::FindPCIDevice(unsigned int venderId, unsigned int deviceId)
{
	auto iter = m_listPCIDevices->begin();
	for (; iter != m_listPCIDevices->end(); iter++)
	{
		PCIConfigurationSpace* pPCIDetails = &(*iter)->PCIConfDetails;

		if (pPCIDetails->DeviceID == deviceId && pPCIDetails->VendorID == venderId)
		{
			return (*iter);
		}		
	}

	return nullptr;
}

BYTE PrintPCIDeviceList(PCIConfigurationSpace * ptrPCIDet)
{
	kprintf("%x %x ", ptrPCIDet->VendorID, ptrPCIDet->DeviceID);
	BYTE SClCode = 0;
	if (ptrPCIDet->ClassCode < __PCI_MaxClass)
	{
		kprintf(" %s", PCIClassDetails[ptrPCIDet->ClassCode].Description);
		if (ptrPCIDet->SubClass != 0)
		{
			PCISubClass * PCISubCl = PCIClassDetails[ptrPCIDet->ClassCode].SubClass;

			while (!(PCISubCl[SClCode].SubClassCode == 0 && PCISubCl[SClCode].Description == 0))
			{
				if (ptrPCIDet->SubClass == SClCode)

				{
					if (PCISubCl != NULL)
						kprintf(" - %s\n", PCISubCl[SClCode].Description);
					else kprintf("\n");
					break;
				}
				SClCode++;
			}
		}
		else kprintf("\n");
	}

	return SClCode;
}

void DeviceDriverManager::RequestPCIList()
{
	if (m_deviceCount > 0)
	{
		kprintf("Device Vendor Class SubClass\n");
		EnumeratePCIDevices(PrintPCIDeviceList);
	}
	else
		kprintf("Device not detected\n");
}

bool DeviceDriverManager::AddDevice(std::string deviceName)
{
	if (strcmp(deviceName.c_str(), "es1370") == 0)
	{
		PCIDeviceDetails* pciInfo = FindPCIDevice(0x1274, 0x5000);

		if (pciInfo != nullptr)
			return true;		
	}

	return false;
}

bool DeviceDriverManager::IOInit()
{
	// nothing to do on x86 hardware
	return true;
}


uint8 DeviceDriverManager::ReadIOByte(int mapped_io_addr)
{
	return InPortByte(mapped_io_addr);
}


void DeviceDriverManager::WriteIOByte(int mapped_io_addr, uint8 value)
{
	OutPortByte(mapped_io_addr, value);
}


uint16 DeviceDriverManager::ReadIOWord(int mapped_io_addr)
{
	return InPortWord(mapped_io_addr);
}


void DeviceDriverManager::WriteIOWord(int mapped_io_addr, uint16 value)
{
	OutPortWord(mapped_io_addr, value);
}


uint32 DeviceDriverManager::ReadIODWord(int mapped_io_addr)
{
	return InPortDWord(mapped_io_addr);
}


void DeviceDriverManager::WrieIODWord(int mapped_io_addr, uint32 value)
{
	OutPortDWord(mapped_io_addr, value);
}