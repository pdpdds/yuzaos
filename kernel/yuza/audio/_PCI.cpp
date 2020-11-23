/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f? die Verwendung dieses Sourcecodes siehe unten
*/
#include <stdint.h>
#include <stdio.h>
#include <_pci.h>
#include <sprintf.h>
#include <intrinsic.h>
#include <Constants.h>
#include <string.h>
//#include "usb_hc.h"
#include <systemcall_impl.h>
//#include "network/network.h"
#include <audio.h>
#include <ktypes.h>

list_t pci_devices = list_init();
static DWORD pci_mutex;

void pci_init()
{
	pci_mutex = Syscall_CreateMutex("pci_mutex");
}

void pci_analyzeHostSystemError(pciDev_t* pciDev)
{
    // check pci status register of the device
	uint16_t pciStatus = pci_configRead(pciDev, PCI_STATUS, 2);
    printf("\nPCI status word: %xh\n", pciStatus);
    // bits 0...2 reserved
    if (pciStatus & PCI_STS_INTERRUPTSTATUS)       printf("Interrupt Status\n");
    if (pciStatus & PCI_STS_CAPABILITIESLIST)      printf("Capabilities List\n");
    if (pciStatus & PCI_STS_66MHZCAPABLE)          printf("66 MHz Capable\n");
    // bit 6 reserved
    if (pciStatus & PCI_STS_FASTBACKTOBACKCAPABLE) printf("Fast Back-to-Back Transactions Capable\n");
    if (pciStatus & PCI_STS_MASTERDATAPARITYERROR) printf("Master Data Parity Error\n");
    // DEVSEL Timing: bits 10:9
    if (pciStatus & PCI_STS_SIGNALEDTARGETABORT)   printf("Signalled Target-Abort\n");
    if (pciStatus & PCI_STS_RECEIVEDTARGETABORT)   printf("Received Target-Abort\n");
    if (pciStatus & PCI_STS_RECEIVEDMASTERABORT)   printf("Received Master-Abort\n");
    if (pciStatus & PCI_STS_SIGNALEDSYSTEMERROR)   printf("Signalled System Error\n");
    if (pciStatus & PCI_STS_DETECTEDPARITYERROR)   printf("Detected Parity Error\n");
 
}

static uint32_t config_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg_off, uint8_t length)
{
    uint8_t reg    = reg_off & 0xFC;     // bit mask: 11111100b
    uint8_t offset = reg_off % 0x04;     // remainder of modulo operation provides offset

	Syscall_LockMutex(pci_mutex);
	OutPortDWord(PCI_CONFIGURATION_ADDRESS,
             0x80000000
             | (bus    << 16)
             | (device << 11)
             | (func   <<  8)
             |  reg);

    // use offset to find searched content
    uint32_t readVal = InPortDWord(PCI_CONFIGURATION_DATA) >> (8 * offset);
	Syscall_UnlockMutex(pci_mutex);

    switch (length)
    {
        case 1:
            readVal &= 0x000000FF;
            break;
        case 2:
            readVal &= 0x0000FFFF;
            break;
        case 4:
            readVal &= 0xFFFFFFFF;
            break;
    }
    return readVal;
}

static void config_write_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint8_t val)
{
	Syscall_LockMutex(pci_mutex);
	OutPortDWord(PCI_CONFIGURATION_ADDRESS,
             0x80000000
             | (bus     << 16)
             | (device  << 11)
             | (func    <<  8)
             | (reg & 0xFC));

    OutPortByte(PCI_CONFIGURATION_DATA + (reg & 0x03), val);
	Syscall_UnlockMutex(pci_mutex);
}

static void config_write_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint16_t val)
{
	Syscall_LockMutex(pci_mutex);
	OutPortDWord(PCI_CONFIGURATION_ADDRESS,
             0x80000000
             | (bus     << 16)
             | (device  << 11)
             | (func    <<  8)
             | (reg & 0xFC));

    OutPortWord(PCI_CONFIGURATION_DATA + (reg & 0x02), val);
	Syscall_UnlockMutex(pci_mutex);
}

static void config_write_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint32_t val)
{
	Syscall_LockMutex(pci_mutex);
	OutPortDWord(PCI_CONFIGURATION_ADDRESS,
             0x80000000
             | (bus     << 16)
             | (device  << 11)
             | (func    <<  8)
             | (reg & 0xFC));

	OutPortDWord(PCI_CONFIGURATION_DATA, val);
	Syscall_UnlockMutex(pci_mutex);
}

uint32_t pci_configRead(pciDev_t* pciDev, uint8_t reg_off, uint8_t length)
{
    return config_read(pciDev->bus, pciDev->device, pciDev->func, reg_off, length);
}

void pci_configWrite_byte(pciDev_t* pciDev, uint8_t reg, uint8_t  val)
{
    config_write_byte(pciDev->bus, pciDev->device, pciDev->func, reg, val);
}

void pci_configWrite_word(pciDev_t* pciDev, uint8_t reg, uint16_t val)
{
    config_write_word(pciDev->bus, pciDev->device, pciDev->func, reg, val);
}

void pci_configWrite_dword(pciDev_t* pciDev, uint8_t reg, uint32_t val)
{
    config_write_dword(pciDev->bus, pciDev->device, pciDev->func, reg, val);
}


bool pci_deviceSentInterrupt(pciDev_t* dev)
{
    uint16_t statusRegister = pci_configRead(dev, PCI_STATUS, 2);
    return (statusRegister & PCI_STS_INTERRUPTSTATUS);
}

bool pci_getExtendedCapability(pciDev_t* dev, uint8_t id, uint8_t* value)
{
    for (dlelement_t* e = dev->extendedCapabilities.head; e; e = e->next)
    {
        uint8_t this_id = BYTE1((uintptr_t)e->data);
        if (this_id == id) {
            *value = BYTE2((uintptr_t)e->data);
            return true;
        }
    }
    return false;
}

bool pci_trySetMSIVector(pciDev_t* dev, IRQ_NUM_t irq)
{
    uint8_t xCP;
    if (pci_getExtendedCapability(dev, 5, &xCP))
    {
        printf("\nCapability for MSI found!"); // DEBUG

        uint8_t MESSAGEADDRLO;
        uint8_t MESSAGEADDRHI;
        uint8_t MESSAGEDATA;
        uint8_t MESSAGECONTROL;
        MESSAGECONTROL = xCP + 2; // Message Control (bit0: MSI on/off, bit7: 0: 32-bit-, 1: 64-bit-addresses)
        MESSAGEADDRLO = xCP + 4;  // Message Address

        uint16_t msiMessageControl = pci_configRead(dev, MESSAGECONTROL, 2); // xHCI spec, Figure 49 (5.2.6.1 MSI configuration)

        if (msiMessageControl & MSI_64BITADDR)
        {
            printf("\n64-bit-address");

            MESSAGEADDRHI = xCP + 8; // Message Upper Address
            MESSAGEDATA = xCP + 12;  // Message Data
        }
        else
        {
            printf("\n32-bit-address");

            MESSAGEADDRHI = 0;     // Does not exist
            MESSAGEDATA = xCP + 8; // Message Data
        }

        /* Intel 3A, 10.11.1 Message Address Register Format:
           Bits 31-20 ?These bits contain a fixed value for interrupt messages (0FEEH).
           This value locates interrupts at the 1-MByte area with a base address of 4G ?18M.
           All accesses to this region are directed as interrupt messages.
           Care must to be taken to ensure that no other device claims the region as I/O space. */
        //20191112
		//uint32_t address = 0xFEE00000 | lapic_getID() << 12;
		uint32_t address = 0xFEE00000 | 0 << 12;

        uint16_t data = irq + 32; // vector, edge
        data |= pci_configRead(dev, MESSAGEDATA, 2) & 0x3800; // "Reserved fields are not assumed to be any value. Software must preserve their contents on writes."

        pci_configWrite_dword(dev, MESSAGEADDRLO, address);
        if (MESSAGEADDRHI)
            pci_configWrite_dword(dev, MESSAGEADDRHI, 0);

        pci_configWrite_word(dev, MESSAGEDATA, data);
        return true;
    }
    printf("\nCapability for MSI not found!");
    return false;
}

void pci_switchToMSI(pciDev_t* dev)
{
    // Disable legacy interrupt
    uint16_t cmd = pci_configRead(dev, PCI_COMMAND, 2);
    pci_configWrite_word(dev, PCI_COMMAND, cmd | PCI_CMD_INTERRUPTDISABLE);

    // Get PCI capability for MSI
    uint8_t MSI_xCP = 0;
    pci_getExtendedCapability(dev, 5, &MSI_xCP);
    uint8_t MESSAGECONTROL = MSI_xCP + 2;

    // Enable the MSI interrupt mechanism by setting the MSI Enable flag in the MSI Capability Structure Message Control register
    uint16_t control = pci_configRead(dev, MESSAGECONTROL, 2);
    pci_configWrite_word(dev, MESSAGECONTROL, control | MSI_ENABLED);
}
extern uint32_t alignUp(uint32_t val, uint32_t alignment);
#define USER_READ 1
#define USER_WRITE 2
#define USER_EXEC 4
#define SYSTEM_READ 8
#define SYSTEM_WRITE 16
#define SYSTEM_EXEC 32

void* pci_aquireMemoryForMMIO(pciBar_t* bar)
{
    if (bar->memoryType != PCI_MMIO)
        return 0;
    size_t offset = bar->baseAddress % PAGE_SIZE;   
	int mmioSize = alignUp(bar->memorySize + offset, PAGE_SIZE) / PAGE_SIZE;
	printf("base : %x, offset: %x, size %x\n", bar->baseAddress, offset, mmioSize);


	unsigned int va = 0;
	//Syscall_CreateArea("mmio", &va, alignUp(bar->memorySize + offset, PAGE_SIZE) / PAGE_SIZE, AREA_WIRED, USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE);
	
	va = Syscall_MapPhysicalMemory("MemoryForMMIO", bar->baseAddress - offset, alignUp(bar->memorySize + offset, PAGE_SIZE), USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE, 0 );
	va += offset;
	printf("va %x\n", va);
	return (void*)va;
	
	//return paging_allocMMIO(bar->baseAddress - offset, alignUp(bar->memorySize + offset, PAGE_SIZE) / PAGE_SIZE) + offset;
}

void pci_scanBus(uint8_t bus, int64_t* counter)
{
    for (uint8_t device = 0; device < PCIDEVICES; ++device)
    {
        uint8_t headerType = config_read(bus, device, 0, PCI_HEADERTYPE, 1);
        uint8_t funcCount = PCIFUNCS;
        if (!(headerType & 0x80)) // Bit 7 in header type (Bit 23-16) --> multifunctional
        {
            funcCount = 1; // --> not multifunctional, only function 0 used
        }

        for (uint8_t func = 0; func < funcCount; ++func)
        {
            uint16_t vendorID = config_read(bus, device, func, PCI_VENDOR_ID, 2);

            if (vendorID != 0xFFFF)
            {
                dlelement_t* elem = list_alloc_elem(sizeof(pciDev_t), "pciDev_t");
                pciDev_t* PCIdev = (pciDev_t *)elem->data;

                PCIdev->bus = bus;
                PCIdev->device = device;
                PCIdev->func = func;

                PCIdev->data = 0;
                PCIdev->vendorID = vendorID;
                PCIdev->deviceID = pci_configRead(PCIdev, PCI_DEVICE_ID, 2);
                PCIdev->classID = pci_configRead(PCIdev, PCI_CLASS, 1);
                PCIdev->subclassID = pci_configRead(PCIdev, PCI_SUBCLASS, 1);
                PCIdev->interfaceID = pci_configRead(PCIdev, PCI_INTERFACE, 1);
                PCIdev->revID = pci_configRead(PCIdev, PCI_REVISION, 1);
                PCIdev->irq = pci_configRead(PCIdev, PCI_IRQLINE, 1);
                PCIdev->interruptPin = pci_configRead(PCIdev, PCI_IRQPIN, 1);
                if ((PCIdev->classID == 0x06) && (PCIdev->subclassID == 0x04))
                {
                    uint8_t secondaryBus = pci_configRead(PCIdev, PCI_SECONDARY, 1);
                  #ifdef _DIAGNOSIS_
                    printf("\nPCI-to-PCI Bridge Device, secondary bus: %u", secondaryBus);
                  #endif
                    pci_scanBus(secondaryBus, counter); // Scan recursively
                }
              #ifdef _IOAPIC_ENABLE_
                else if ((PCIdev->classID == 0x06) && (PCIdev->subclassID == 0x01 || PCIdev->subclassID == 0x02) && (PCIdev->interfaceID == 0x00))
                {
                    uint32_t irqABCD = pci_configRead(PCIdev, PCI_IRQ_ABCD, 4);
                    uint32_t irqEFGH = pci_configRead(PCIdev, PCI_IRQ_EFGH, 4);

                  #ifdef _DIAGNOSIS_
                    textColor(IMPORTANT);
                    printf("\nPCI-to-(E)ISA-Bridge Device: #A %u  #B %u  #C %u  #D %u",
                        BYTE1(irqABCD), BYTE2(irqABCD), BYTE3(irqABCD), BYTE4(irqABCD));
                    printf("\nPCI-to-(E)ISA-Bridge Device: #E %u  #F %u  #G %u  #H %u",
                        BYTE1(irqEFGH), BYTE2(irqEFGH), BYTE3(irqEFGH), BYTE4(irqEFGH));
                    textColor(TEXT);
                  #endif

                    ipc_setTaskWorkingNode("PrettyOS/PCI/ISABridge", true);
                    int64_t irqA = BYTE1(irqABCD); ipc_setInt(">/irqA", &irqA, IPC_UINT);
                    int64_t irqB = BYTE2(irqABCD); ipc_setInt(">/irqB", &irqB, IPC_UINT);
                    int64_t irqC = BYTE3(irqABCD); ipc_setInt(">/irqC", &irqC, IPC_UINT);
                    int64_t irqD = BYTE4(irqABCD); ipc_setInt(">/irqD", &irqD, IPC_UINT);
                    int64_t irqE = BYTE1(irqEFGH); ipc_setInt(">/irqE", &irqE, IPC_UINT);
                    int64_t irqF = BYTE2(irqEFGH); ipc_setInt(">/irqF", &irqF, IPC_UINT);
                    int64_t irqG = BYTE3(irqEFGH); ipc_setInt(">/irqG", &irqG, IPC_UINT);
                    int64_t irqH = BYTE4(irqEFGH); ipc_setInt(">/irqH", &irqH, IPC_UINT);
                }
              #endif

                // Read BARs
                for (uint8_t i = 0; i < 6; i++)
                {
                    if (i < 2 || !(headerType & 0x01)) // Devices with header type 0x00 have 6 bars
                    {
                        PCIdev->bar[i].baseAddress = pci_configRead(PCIdev, PCI_BAR0 + i * 4, 4);
                        if (PCIdev->bar[i].baseAddress) // Valid bar
                        {
                            uint32_t mask;

                            // Check memory type
                            PCIdev->bar[i].memoryType = PCIdev->bar[i].baseAddress & 0x01;
                            if (PCIdev->bar[i].memoryType == 0) // MMIO bar
                                mask = 0xFFFFFFF0;
                            else                                // IO bar
                                mask = 0xFFFC;

                            // Check Memory Size
							int flags = DisableInterrupts();
                            pci_configWrite_dword(PCIdev, PCI_BAR0 + 4 * i, mask | PCIdev->bar[i].baseAddress);
                            PCIdev->bar[i].memorySize = ((~pci_configRead(PCIdev, PCI_BAR0 + 4 * i, 4) & mask) | (~mask & 0x0F)) + 1;
                            pci_configWrite_dword(PCIdev, PCI_BAR0 + 4 * i, PCIdev->bar[i].baseAddress);
							RestoreInterrupts(flags);

                            PCIdev->bar[i].baseAddress &= mask;
                        }
                        else
                            PCIdev->bar[i].memoryType = PCI_INVALIDBAR;
                    }
                    else
                        PCIdev->bar[i].memoryType = PCI_INVALIDBAR;
                }

                list_construct(&PCIdev->extendedCapabilities);
                uint16_t pciStatusRegister = pci_configRead(PCIdev, PCI_STATUS, 2);
                if (pciStatusRegister & PCI_STS_CAPABILITIESLIST) // capabilities list exists -> read it
                {
                    uint8_t xCP = pci_configRead(PCIdev, PCI_CAPLIST, 1);

                    while (xCP) // 00h indicates end of the cap. list.
                    {
                        uint8_t xCP_id = pci_configRead(PCIdev, xCP, 1);
                        list_append(&PCIdev->extendedCapabilities, (void*)(uintptr_t)(xCP_id | (xCP << 8)));

                        xCP = pci_configRead(PCIdev, xCP + 1, 1);
                    }
                }

                list_append_elem(&pci_devices, elem);
//20191112
                // Fill IPC
                /*char path[30];
                snprintf(path, 30, "SkyOS/PCI/%u", (uint32_t)*counter);
                ipc_setTaskWorkingNode(path, true);

                int64_t Bus = bus;
                ipc_setInt(">/Bus", &Bus, IPC_QWORD);

                int64_t Device = device;
                ipc_setInt(">/Device", &Device, IPC_QWORD);

                int64_t Function = func;
                ipc_setInt(">/Function", &Function, IPC_QWORD);

                int64_t Irq = PCIdev->irq;
                ipc_setInt(">/IRQ", &Irq, IPC_UINT);

                int64_t Irqp = PCIdev->interruptPin;
                ipc_setInt(">/InterruptPin", &Irqp, IPC_UINT);

                int64_t VendorID = PCIdev->vendorID;
                ipc_setInt(">/VendorID", &VendorID, IPC_QWORD);

                int64_t DeviceID = PCIdev->deviceID;
                ipc_setInt(">/DeviceID", &DeviceID, IPC_QWORD);

                int64_t ClassID = PCIdev->classID;
                ipc_setInt(">/ClassID", &ClassID, IPC_QWORD);

                int64_t SubclassID = PCIdev->subclassID;
                ipc_setInt(">/SubclassID", &SubclassID, IPC_QWORD);

                int64_t InterfaceID = PCIdev->interfaceID;
                ipc_setInt(">/InterfaceID", &InterfaceID, IPC_QWORD);*/

              #ifdef _DIAGNOSIS_
                if (PCIdev->irq != 255)
                {
                    printf("%d:%d.%d   %d", PCIdev->bus, PCIdev->device, PCIdev->func, PCIdev->irq);
                    printf("   vender: %xh, dev: %xh   %xh   %xh\n", PCIdev->vendorID, PCIdev->deviceID, PCIdev->classID, PCIdev->subclassID);
                }
              #endif
                (*counter)++;
            } // if pciVendor
        } // for function
    } // for device
}

void pci_scan(void)
{
  #ifdef _DIAGNOSIS_
    //textColor(LIGHT_GRAY);
    printf("=> PCI devices:");
    //textColor(TABLE_HEADING);
    printf("\nB:D:F   IRQ   ClassID   SubClassID   Description");
    printf("\n--------------------------------------------------------------------------------");
    //textColor(TEXT);
  #endif
    int64_t counter = 0;

    pci_scanBus(0, &counter); // Scan always bus 0, all others will be scanned recursively, if PCI-to-PCI bridges are found

  #ifdef _DIAGNOSIS_
   // textColor(TABLE_HEADING);
    printf("--------------------------------------------------------------------------------\n");
  //  textColor(TEXT);
  #endif
}

void pci_installDevices(void)
{
    // Install device drivers
    for (dlelement_t* elem = pci_devices.head; elem; elem = elem->next)
    {
        pciDev_t* PCIdev = (pciDev_t * )elem->data;

        //if (PCIdev->classID == 0x0C && PCIdev->subclassID == 0x03)      // USB Host Controller
            //usb_hc_install(PCIdev);
		//else 
		if (PCIdev->classID == 0x04 && PCIdev->subclassID == 0x01)  // Multimedia Controller Audio
			audio_installDevice(PCIdev);
        /*else if (PCIdev->classID == 0x02 && PCIdev->subclassID == 0x00) // Network Adapters
            network_installDevice(PCIdev);
        else if (PCIdev->classID == 0x03)  // Graphics Adapter
            vga_installPCIDevice(PCIdev);
        */
    }
}