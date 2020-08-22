#include "XHCIHandler.h"
#include <stdio.h>


XHCIHandler::XHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName)
	: InterruptHandler(vector, deviceName),
	m_fDeviceHandler(handler),
	m_pDevice(device)
{
	printf("\nXHCIHandler Created\n");
}

XHCIHandler::~XHCIHandler()
{
}

InterruptStatus XHCIHandler::HandleInterrupt(void* arg)
{
	//printf("\nXHCIHandler::HandleInterrupt\n");
	m_fDeviceHandler(m_pDevice);
	return InterruptStatus::kHandledInterrupt;
}