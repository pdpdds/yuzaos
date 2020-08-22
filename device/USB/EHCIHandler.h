#pragma once
#include <InterruptHandler.h>
//#include <Hal.h>
#include "_pci.h"

typedef void (*DeviceHandler)(struct pciDev*);

class EHCIHandler : public InterruptHandler
{
public:
	EHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName);
	virtual ~EHCIHandler();

	virtual InterruptStatus HandleInterrupt(void* arg = nullptr);

private:
	struct pciDev* m_pDevice;
	DeviceHandler m_fDeviceHandler;
};