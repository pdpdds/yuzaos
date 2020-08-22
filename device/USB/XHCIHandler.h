#pragma once
#include <InterruptHandler.h>
#include "_pci.h"

typedef void (*DeviceHandler)(struct pciDev*);

class XHCIHandler : public InterruptHandler
{
public:
	XHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName);
	virtual ~XHCIHandler();

	virtual InterruptStatus HandleInterrupt(void* arg = nullptr);

private:
	struct pciDev* m_pDevice;
	DeviceHandler m_fDeviceHandler;
};