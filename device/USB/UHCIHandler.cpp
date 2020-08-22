#include "UHCIHandler.h"



UHCIHandler::UHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName)
	: InterruptHandler(vector, deviceName),
	m_fDeviceHandler(handler),
	m_pDevice(device)
{
}

UHCIHandler::~UHCIHandler()
{
}

InterruptStatus UHCIHandler::HandleInterrupt(void* arg)
{
	m_fDeviceHandler(m_pDevice);
	return InterruptStatus::kHandledInterrupt;
}