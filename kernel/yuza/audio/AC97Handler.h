#pragma once
#include <InterruptHandler.h>
#include <_pci.h>

typedef void (*DeviceHandler)(struct pciDev*);

class AC97Handler : public InterruptHandler
{
public:
	AC97Handler(int vector, DeviceHandler handler, struct pciDev* device, char* deviceName);
	virtual ~AC97Handler();

	virtual InterruptStatus HandleInterrupt(void* arg = nullptr);

private:
	struct pciDev* m_pDevice;
	DeviceHandler m_fDeviceHandler;
};