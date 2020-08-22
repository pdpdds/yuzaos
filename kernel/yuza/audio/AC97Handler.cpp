#include "AC97Handler.h"
#include <stdio.h>


AC97Handler::AC97Handler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName)
	: InterruptHandler(vector, deviceName),
	m_fDeviceHandler(handler),
	m_pDevice(device)
{
	printf("\nAC97Handler Created\n");
}

AC97Handler::~AC97Handler()
{
}

InterruptStatus AC97Handler::HandleInterrupt(void* arg)
{
	printf("\nAC97Handler::HandleInterrupt\n");
	m_fDeviceHandler(m_pDevice);
	return InterruptStatus::kHandledInterrupt;
}