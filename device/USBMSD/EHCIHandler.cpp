#include "EHCIHandler.h"
#include <stdio.h>

EHCIHandler::EHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName)
	: InterruptHandler(vector, deviceName),
	m_fDeviceHandler(handler),
	m_pDevice(device)
{
	printf("EHCIHandler Created\n");
}

EHCIHandler::~EHCIHandler()
{
}

InterruptStatus EHCIHandler::HandleInterrupt(void* arg)
{
	m_fDeviceHandler(m_pDevice);
	return InterruptStatus::kHandledInterrupt;
}