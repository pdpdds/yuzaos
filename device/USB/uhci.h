#ifndef UHCI_H
#define UHCI_H

//#include "os.h"
#include "_pci.h"
//#include "tasking/synchronisation.h"
//#include "storage/devicemanager.h"
#include "usb_hc.h"

#define UHCIPORTMAX  8  // max number of UHCI device ports

#define UHCI_USBCMD         0x00
#define UHCI_USBSTS         0x02
#define UHCI_USBINTR        0x04
#define UHCI_FRNUM          0x06
#define UHCI_FRBASEADD      0x08
#define UHCI_SOFMOD         0x0C
#define UHCI_PORTSC1        0x10
#define UHCI_PORTSC2        0x12


/* ****** */
/* USBCMD */
/* ****** */

#define UHCI_CMD_MAXP     BIT(7)
#define UHCI_CMD_CF       BIT(6)
#define UHCI_CMD_SWDBG    BIT(5)
#define UHCI_CMD_FGR      BIT(4)
#define UHCI_CMD_EGSM     BIT(3)
#define UHCI_CMD_GRESET   BIT(2)
#define UHCI_CMD_HCRESET  BIT(1)
#define UHCI_CMD_RS       BIT(0)


/* ******* */
/* USBSTS  */
/* ******* */

#define UHCI_STS_HCHALTED            BIT(5)
#define UHCI_STS_HC_PROCESS_ERROR    BIT(4)
#define UHCI_STS_HOST_SYSTEM_ERROR   BIT(3)
#define UHCI_STS_RESUME_DETECT       BIT(2)
#define UHCI_STS_USB_ERROR           BIT(1)
#define UHCI_STS_USBINT              BIT(0)
#define UHCI_STS_MASK                0x3F

/* ******* */
/* USBINTR */
/* ******* */

#define UHCI_INT_SHORT_PACKET_ENABLE BIT(3)
#define UHCI_INT_IOC_ENABLE          BIT(2)
#define UHCI_INT_RESUME_ENABLE       BIT(1)
#define UHCI_INT_TIMEOUT_ENABLE      BIT(0)
#define UHCI_INT_MASK                0xF

/* ******* */
/* PORTSC  */
/* ******* */

#define UHCI_SUSPEND                 BIT(12)
#define UHCI_PORT_RESET              BIT(9)
#define UHCI_PORT_LOWSPEED_DEVICE    BIT(8)
#define UHCI_PORT_VALID              BIT(7) // reserved an readonly; always read as 1
#define UHCI_PORT_RESUME_DETECT      BIT(6)
#define UHCI_PORT_ENABLE_CHANGE      BIT(3)
#define UHCI_PORT_ENABLE             BIT(2)
#define UHCI_PORT_CS_CHANGE          BIT(1)
#define UHCI_PORT_CS                 BIT(0)

/* *************** */
/* LEGACY SUPPORT  */
/* *************** */

// Register in PCI (uint16_t)
#define UHCI_PCI_LEGACY_SUPPORT          0xC0

// Interrupt carried out as a PCI interrupt
#define UHCI_PCI_LEGACY_SUPPORT_PIRQ     0x2000

// RO Bits
#define UHCI_PCI_LEGACY_SUPPORT_NO_CHG   0x5040

// Status bits that are cleared by setting to 1
#define UHCI_PCI_LEGACY_SUPPORT_STATUS   0x8F00


/* ***** */
/* QH TD */
/* ***** */

#define BIT_T    BIT(0)
#define BIT_QH   BIT(1)
#define BIT_Vf   BIT(2)

/*
Packet Identification (PID). This field contains the Packet ID to be used for this transaction. Only
the IN (69h), OUT (E1h), and SETUP (2Dh) tokens are allowed. Any other value in this field causes
a consistency check failure resulting in an immediate halt of the Host Controller. Bits [3:0] are
complements of bits [7:4].
*/
#define UHCI_TD_SETUP  0x2D  // 00101101
#define UHCI_TD_IN     0x69  // 11100001
#define UHCI_TD_OUT    0xE1  // 11100001

#pragma pack(push, 1)
// Transfer Descriptors (TD) are always aligned on 16-byte boundaries.
// All transfer descriptors have the same basic, 32-byte structure.
// The last 4 DWORDs are for software use.
typedef struct
{
    // pointer to another TD or QH
    // inclusive control bits  (DWORD 0)
    uint32_t next;

    // TD CONTROL AND STATUS  (DWORD 1)
    uint32_t actualLength      : 11; //  0-10
    uint32_t reserved1         :  5; // 11-15
    uint32_t reserved2         :  1; //    16
    uint32_t bitstuffError     :  1; //    17
    uint32_t crc_timeoutError  :  1; //    18
    uint32_t nakReceived       :  1; //    19
    uint32_t babbleDetected    :  1; //    20
    uint32_t dataBufferError   :  1; //    21
    uint32_t stall             :  1; //    22
    uint32_t active            :  1; //    23
    uint32_t intOnComplete     :  1; //    24
    uint32_t isochrSelect      :  1; //    25
    uint32_t lowSpeedDevice    :  1; //    26
    uint32_t errorCounter      :  2; // 27-28
    uint32_t shortPacketDetect :  1; //    29
    uint32_t reserved3         :  2; // 30-31

    // TD TOKEN  (DWORD 2)
    uint8_t PacketID;                //  0-7
    uint32_t deviceAddress     :  7; //  8-14
    uint32_t endpoint          :  4; // 15-18
    uint32_t dataToggle        :  1; //    19
    uint32_t reserved4         :  1; //    20
    uint32_t maxLength         : 11; // 21-31

    // TD BUFFER POINTER (DWORD 3)
    uintptr_t        buffer;

    // RESERVED FOR SOFTWARE (DWORDS 4-7)
    uint32_t         dWord4;
    void*            virtBuffer;
    uint32_t         dWord6;
    uint32_t         dWord7;
} uhciTD_t;

// Queue Heads support the requirements of Control, Bulk, and Interrupt transfers
// and must be aligned on a 16-byte boundary
typedef struct
{
    // QUEUE HEAD LINK POINTER
    // inclusive control bits (DWORD 0)
    uintptr_t   next;

    // QUEUE ELEMENT LINK POINTER
    // inclusive control bits (DWORD 1)
    uintptr_t   transfer;
} uhciQH_t;

typedef struct
{
    uintptr_t frPtr[1024];
} frPtr_t;


typedef struct
{
    hc_t           hc;                // Generic HC data
    pciDev_t*      PCIdevice;         // PCI device
    frPtr_t*       framelistAddrVirt; // virtual adress of frame list
    uhciQH_t*      qhPointerVirt;     // virtual adress of QH
    size_t         memSize;           // memory size of IO space
    uint16_t       bar;               // start of I/O space (base address register)
    bool           enabledPortFlag;   // root ports enabled
    uint8_t        num;               // Number of the UHCI
    bool           periodicScheduled; // Indicates whether there are periodic elements in the queue
} uhci_t;
#pragma pack(pop)

void uhci_install(pciDev_t* PCIdev);
void uhci_pollDisk(port_t* dev);

void uhci_setupTransfer(usb_transfer_t* transfer);
void uhci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length);
void uhci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, void* buffer, size_t length);
void uhci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, const void* buffer, size_t length);
void uhci_scheduleTransfer(usb_transfer_t* transfer);
bool uhci_pollTransfer(usb_transfer_t* transfer);
void uhci_waitForTransfer(usb_transfer_t* transfer);
void uhci_destructTransfer(usb_transfer_t* transfer);


#endif
