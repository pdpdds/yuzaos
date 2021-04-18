#pragma once

#include <stdint.h>
#include "list.h"

#define PAGE_SIZE 4096

#define BYTE1(a) ( (a)      & 0xFF)
#define BYTE2(a) (((a)>> 8) & 0xFF)
#define BYTE3(a) (((a)>>16) & 0xFF)
#define BYTE4(a) (((a)>>24) & 0xFF)

#define WORD1(a) ( (a)      & 0xFFFF)
#define WORD2(a) (((a)>>16) & 0xFFFF)

typedef enum
{
	IRQ_TIMER = 0,
	IRQ_KEYBOARD = 1,
	IRQ_FLOPPY = 6,
	IRQ_MOUSE = 12,
	IRQ_ATA_PRIMARY = 14,
	IRQ_ATA_SECONDARY = 15,
	//IRQ_SYSCALL = 95 // PrettyOS SYSCALL_NUMBER 127 minus 32 // cf. interrupts.asm
} IRQ_NUM_t;


#define BIT(n) (1U<<(n))
#define getField(addr, byte, shift, len) ((((uint8_t*)(addr))[byte]>>(shift)) & (BIT(len)-1))

#define PCI_CONFIGURATION_ADDRESS 0x0CF8   // Address I/O Port
#define PCI_CONFIGURATION_DATA    0x0CFC   // Data    I/O Port

#define PCI_VENDOR_ID   0x00 // length: 0x02 reg: 0x00 offset: 0x00
#define PCI_DEVICE_ID   0x02 // length: 0x02 reg: 0x00 offset: 0x02
#define PCI_COMMAND     0x04
#define PCI_STATUS      0x06
#define PCI_REVISION    0x08
#define PCI_CLASS       0x0B
#define PCI_SUBCLASS    0x0A
#define PCI_INTERFACE   0x09
#define PCI_HEADERTYPE  0x0E
#define PCI_BAR0        0x10
#define PCI_BAR1        0x14
#define PCI_BAR2        0x18
#define PCI_BAR3        0x1C
#define PCI_BAR4        0x20
#define PCI_BAR5        0x24
#define PCI_CAPLIST     0x34
#define PCI_IRQLINE     0x3C
#define PCI_IRQPIN      0x3D
#define PCI_IRQ_ABCD    0x60
#define PCI_IRQ_EFGH    0x68

#define PCI_SECONDARY   0x19


//Command Register
#define PCI_CMD_IO                           BIT(0)
#define PCI_CMD_MMIO                         BIT(1)
#define PCI_CMD_BUSMASTER                    BIT(2)
#define PCI_CMD_SPECIALCYCLES                BIT(3)
#define PCI_CMD_MEMORYWRITEINVALIDATEENABLE  BIT(4)
#define PCI_CMD_VGAPALETTESNOOP              BIT(5)
#define PCI_CMD_PARITYERRORRESPONSE          BIT(6)
#define PCI_CMD_SERRENABLE                   BIT(8)
#define PCI_CMD_FASTBACKTOBACKENABLE         BIT(9)
#define PCI_CMD_INTERRUPTDISABLE             BIT(10)

//Status Register
#define PCI_STS_INTERRUPTSTATUS              BIT(3)
#define PCI_STS_CAPABILITIESLIST             BIT(4)
#define PCI_STS_66MHZCAPABLE                 BIT(5)
#define PCI_STS_FASTBACKTOBACKCAPABLE        BIT(7)
#define PCI_STS_MASTERDATAPARITYERROR        BIT(8)
#define PCI_STS_SIGNALEDTARGETABORT          BIT(11)
#define PCI_STS_RECEIVEDTARGETABORT          BIT(12)
#define PCI_STS_RECEIVEDMASTERABORT          BIT(13)
#define PCI_STS_SIGNALEDSYSTEMERROR          BIT(14)
#define PCI_STS_DETECTEDPARITYERROR          BIT(15)

#define PCIBUSES      256
#define PCIDEVICES    32
#define PCIFUNCS      8

// MSI
#define MSI_ENABLED                          BIT(0)
#define MSI_64BITADDR                        BIT(7)

enum
{
    PCI_MMIO, PCI_IO, PCI_INVALIDBAR
};

typedef struct
{
    uint32_t baseAddress;
    size_t   memorySize;
    uint8_t  memoryType;
} pciBar_t;

typedef struct pciDev
{
    uint16_t  vendorID;
    uint16_t  deviceID;
    uint8_t   classID;
    uint8_t   subclassID;
    uint8_t   interfaceID;
    uint8_t   revID;
    uint8_t   bus;
    uint8_t   device;
    uint8_t   func;
    uint8_t   irq;
    uint8_t   interruptPin;
    pciBar_t  bar[6];
    void*     data; // Pointer to internal data of associated driver.
    list_t    extendedCapabilities;
} pciDev_t;
	

extern list_t pci_devices;

void pci_init();
void     pci_scan(void);
void     pci_installDevices(void);
uint32_t pci_configRead       (pciDev_t* pciDev, uint8_t reg_off, uint8_t length);
void     pci_configWrite_byte (pciDev_t* pciDev, uint8_t reg, uint8_t  val);
void     pci_configWrite_word (pciDev_t* pciDev, uint8_t reg, uint16_t val);
void     pci_configWrite_dword(pciDev_t* pciDev, uint8_t reg, uint32_t val);
void     pci_analyzeHostSystemError(pciDev_t* pciDev);
bool     pci_deviceSentInterrupt(pciDev_t* dev);
bool     pci_getExtendedCapability(pciDev_t* dev, uint8_t id, uint8_t* value);
bool     pci_trySetMSIVector(pciDev_t* dev, IRQ_NUM_t irq);
void     pci_switchToMSI(pciDev_t* dev);
void*    pci_aquireMemoryForMMIO(pciBar_t* bar); // Calls paging_allocMMIO() with correct alignment and size, allocating the entire BAR