#pragma once
#include <InterruptHandler.h>
#include "_pci.h"

typedef void (*DeviceHandler)(struct pciDev*);

class UHCIHandler : public InterruptHandler
{
public:
	UHCIHandler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName);
	virtual ~UHCIHandler();

	virtual InterruptStatus HandleInterrupt(void* arg = nullptr);

private:
	struct pciDev* m_pDevice;
	DeviceHandler m_fDeviceHandler;
};

