#ifndef OHCI_H
#define OHCI_H

#include "os.h"
#include "pci.h"
#include "usb_hc.h"

#define OHCIPORTMAX      8  // max number of OHCI device ports

#define NUM_ED          10  // number of EDs in memory pool
#define NUM_ED_CONTROL   0  // EDs for control transfer
#define NUM_ED_BULK      5  // EDs for bulk transfer

#define OHCI_HCCA_ALIGN          (0x0100 | HEAP_CONTINUOUS)
#define OHCI_DESCRIPTORS_ALIGN   (0x0010 | HEAP_CONTINUOUS)

#define MPS_FULLSPEED 64

// HcControl Register
#define OHCI_CTRL_CBSR (BIT(0)|BIT(1))   // relation between control und bulk (cbsr+1 vs. 1)
#define OHCI_CTRL_PLE   BIT(2)           // activate periodical transfers
#define OHCI_CTRL_IE    BIT(3)           // activate isochronous transfers
#define OHCI_CTRL_CLE   BIT(4)           // activate control transfers
#define OHCI_CTRL_BLE   BIT(5)           // activate bulk transfers
#define OHCI_CTRL_HCFS (BIT(6)|BIT(7))   // HostControllerFunctionalState for USB
#define OHCI_CTRL_IR    BIT(8)           // redirect IRQ to SMB
#define OHCI_CTRL_RWC   BIT(9)           // remote wakeup
#define OHCI_CTRL_RWE   BIT(10)          // activate remote wakeup
#define OHCI_CTRL_FSLARGESTDATAPACKET (BIT(16)|BIT(17)|BIT(18)|BIT(19)|BIT(20)|BIT(21)|BIT(22)|BIT(23)|BIT(24)|BIT(25)|BIT(26)|BIT(27)|BIT(28)|BIT(29)|BIT(30))

// HcCommandStatus Register
#define OHCI_STATUS_RESET BIT(0)         // reset
#define OHCI_STATUS_CLF   BIT(1)         // control list filled
#define OHCI_STATUS_BLF   BIT(2)         // bulk list filled
#define OHCI_STATUS_OCR   BIT(3)         // ownership change request
#define OHCI_STATUS_SOC  (BIT(16)|BIT(17)) // scheduling overrun count

// HcInterruptStatus
// HcInterruptEnable, HcInterruptDisable Register
#define OHCI_INT_SO     BIT(0)           // scheduling overrun
#define OHCI_INT_WDH    BIT(1)           // writeback DoneHead
#define OHCI_INT_SF     BIT(2)           // start of frame
#define OHCI_INT_RD     BIT(3)           // resume detected
#define OHCI_INT_UE     BIT(4)           // unrecoverable error
#define OHCI_INT_FNO    BIT(5)           // frame number overflow
#define OHCI_INT_RHSC   BIT(6)           // root hub status change
#define OHCI_INT_OC     BIT(30)          // ownership change
#define OHCI_INT_MIE    BIT(31)          // master interrupt enable


// Root Hub Partition

// HcRhDescriptorA Register
#define OHCI_RHA_NDP    0x000000FF       // number downstream ports (max. 15)
#define OHCI_RHA_PSM    BIT(8)           // PowerSwitchingMode
#define OHCI_RHA_NPS    BIT(9)           // NoPowerSwitching
#define OHCI_RHA_DT     BIT(10)          // DeviceType (always 0)
#define OHCI_RHA_OCPM   BIT(11)          // OverCurrentProtectionMode
#define OHCI_RHA_NOCP   BIT(12)          // NoOverCurrentProtection
#define OHCI_RHA_POTPGT 0xFF000000       // PowerOnToPowerGoodTime

// HcRhStatus Register
#define OHCI_RHS_LPS    BIT(0)           // LocalPowerStatus
#define OHCI_RHS_OCI    BIT(1)           // OverCurrentIndicator
#define OHCI_RHS_DRWE   BIT(15)          // DeviceRemoteWakeupEnable
#define OHCI_RHS_LPSC   BIT(16)          // LocalPowerStatusChange
#define OHCI_RHS_OCIC   BIT(17)          // OverCurrentIndicatorChange
#define OHCI_RHS_CRWE   BIT(31)          // ClearRemoteWakeupEnable

// HcRhPortStatus[1:NDP] Register
#define OHCI_PORT_CCS   BIT(0)           // CurrentConnectStatus (0 = no device connected, 1 = device connected)
#define OHCI_PORT_PES   BIT(1)           // PortEnableStatus (0 = port is disabled, 1 = port is enabled)
#define OHCI_PORT_PSS   BIT(2)           // PortSuspendStatus
#define OHCI_PORT_POCI  BIT(3)           // PortOverCurrentIndicator
#define OHCI_PORT_PRS   BIT(4)           // PortResetStatus
#define OHCI_PORT_PPS   BIT(8)           // PortPowerStatus
#define OHCI_PORT_LSDA  BIT(9)           // LowSpeedDeviceAttached (0 = full speed device, 1 = low speed device)
#define OHCI_PORT_CSC   BIT(16)          // ConnectStatusChange
#define OHCI_PORT_PESC  BIT(17)          // PortEnableStatusChange
#define OHCI_PORT_PSSC  BIT(18)          // PortSuspendStatusChange
#define OHCI_PORT_OCIC  BIT(19)          // PortOverCurrentIndicatorChange
#define OHCI_PORT_PRSC  BIT(20)          // PortResetStatusChange

// USB - operational states of the HC
#define OHCI_USB_RESET       0
#define OHCI_USB_RESUME      BIT(6)
#define OHCI_USB_OPERATIONAL BIT(7)
#define OHCI_USB_SUSPEND     (BIT(6)|BIT(7))

// ED
#define OHCI_ED_TD     0
#define OHCI_ED_OUT    1
#define OHCI_ED_IN     2

// TD
#define OHCI_TD_SETUP  0
#define OHCI_TD_OUT    1
#define OHCI_TD_IN     2
#define OHCI_TD_NOINT  7
#define OHCI_TD_NOCC  15


/*
There are two communication channels between the HC and the HC Driver.
The first channel uses a set of operational registers located on the HC. The HC is the target for all communication on this channel.
The operational registers contain control, status, and list pointer registers.
Within the operational register set is a pointer to a location in shared memory named the HC Communications Area (HCCA). ...
*/

typedef struct
{
    volatile uint32_t HcRevision;                 // BCD version of HCI spec implemented by this HC
    volatile uint32_t HcControl;                  // operating modes for the HC
    volatile uint32_t HcCommandStatus;            // current status of the HC
    volatile uint32_t HcInterruptStatus;          // hardware interrupts
    volatile uint32_t HcInterruptEnable;          // enabled intterupts
    volatile uint32_t HcInterruptDisable;         // disabled intterupts
    volatile uint32_t HcHCCA;                     // physical address of the HC Communication Area
    volatile uint32_t HcPeriodCurrentED;          // physical address of the current Isochronous or Interrupt Endpoint Descriptor
    volatile uint32_t HcControlHeadED;            // physical address of the first Endpoint Descriptor of the Control list
    volatile uint32_t HcControlCurrentED;         // physical address of the current Endpoint Descriptor of the Control list
    volatile uint32_t HcBulkHeadED;               // physical address of the first Endpoint Descriptor of the Bulk list
    volatile uint32_t HcBulkCurrentED;            // physical address of the current endpoint of the Bulk list
    volatile uint32_t HcDoneHead;                 // physical address of the last completed Transfer Descriptor that was added to the Done queue
    volatile uint32_t HcFmInterval;               // 14-bit: bit time interval in a Frame, 15-bit: Full Speed maximum packet size
    volatile uint32_t HcFmRemaining;              // 14-bit down counter showing the bit time remaining in the current Frame
    volatile uint32_t HcFmNumber;                 // 16-bit counter
    volatile uint32_t HcPeriodicStart;            // 14-bit: earliest time HC should start processing the periodic list
    volatile uint32_t HcLSThreshold;              // 11-bit: HC determines whether to commit to the transfer of a maximum of 8-byte LS packet before EOF
    volatile uint32_t HcRhDescriptorA;            // characteristics of the Root Hub
    volatile uint32_t HcRhDescriptorB;            // characteristics of the Root Hub
    volatile uint32_t HcRhStatus;                 // lower word: Root Hub Status field, upper word: Root Hub Status Change field
    volatile uint32_t HcRhPortStatus[OHCIPORTMAX];// port events on a per-port basis, max. NDP = 15, we assume 8 (taken from VBox)
} __attribute__((packed)) ohci_OpRegs_t;

/*
... The HCCA is the second communication channel. The HC is the master for all communication on this channel.
The HCCA contains the head pointers to the interrupt Endpoint Descriptor lists, the head pointer to the done queue,
and status information associated with start-of-frame processing.
HCCA is a 256-byte structure of system memory used to send and receive control/status information to and from the HC.
This structure must be located on a 256-byte boundary. Phys. address of this structure has to be stored into the OpReg HcHCCA.
*/
typedef struct
{
    volatile uint32_t interrruptTable[32];        // pointers to interrupt EDs
    volatile uint16_t frameNumber;                // current frame number
    volatile uint16_t pad1;                       // when the HC updates frameNumber, it sets this word to 0
    volatile uint32_t doneHead;                   // holds at frame end the current value of HcDoneHead, and interrupt is sent
             uint8_t  reserved[116];
} __attribute__((packed)) ohci_HCCA_t;

// Endpoint Descriptor
typedef struct
{
    volatile uint32_t devAddr :  7; // device address
    volatile uint32_t endpNum :  4; // number of endpoint
    volatile uint32_t dir     :  2; // transfer direction
    volatile uint32_t speed   :  1; // 0 = fullspeed, 1 = lowspeed
    volatile uint32_t sKip    :  1; // HC skips to the next ED w/o attempting access to the TD queue
    volatile uint32_t format  :  1; // bit with isochronous transfers
    volatile uint32_t mps     : 11; // maximum packet size
    volatile uint32_t ours    :  5; // available

    volatile uint32_t tdQueueTail; // last TD in queue
    volatile uint32_t tdQueueHead; // head TD in queue

    volatile uint32_t nextED;      // next ED on the list
} __attribute__((packed)) ohciED_t;

// Transfer Descriptor
typedef struct
{
    volatile uint32_t  ours               : 18;  // available
    volatile uint32_t  bufRounding        :  1;  // If the bit is 1, then the last data packet may be smaller than the defined buffer without causing an error
    volatile uint32_t  direction          :  2;  // transfer direction
    volatile uint32_t  delayInt           :  3;  // wait delayInt frames before sending interrupt. If DelayInterrupt is 111b, then there is no interrupt at completion of this TD.

    volatile uint32_t  toggle             :  1;  // toggle // This 2-bit field is used to generate/compare the data PID value (DATA0 or DATA1).
                                                           // The MSb of this field is ??when the data toggle value is acquired from the toggleCarry field in the ED
    volatile uint32_t  toggleFromTD       :  1;            // and ??when the data toggle value is taken from the LSb of this field

    volatile uint32_t  errCnt             :  2;  // Number of errors occured - if 11b status is stored in field ohciTD_t::cond
    volatile uint32_t  cond               :  4;  // status of the last attempted transaction

    volatile uintptr_t curBuffPtr;               // data ptr
    volatile uintptr_t nextTD;                   // next TD
    volatile uintptr_t buffEnd;                  // last byte in buffer
} __attribute__((packed)) ohciTD_t;


typedef struct
{
    hc_t           hc;              // Generic HC data
    pciDev_t*      PCIdevice;       // PCI device
    ohci_OpRegs_t* OpRegs;          // operational registers (MMIO space)
    ohci_HCCA_t*   hcca;            // HC Communications Area (virtual address)
    ohciED_t*      interrupt_baseEDs; // Points to 6 static EDs for interrupt transfers
    list_t         interruptEDs[6]; // list of EDs nested to each baseED
    ohciED_t*      pooledED;        // EDs for bulk and control transfers
    ohciTD_t**     pooledTD;        // TDs (redundant TDs required per ED by 5.2.8.1)
    uint16_t       indexED_control; // index at ED
    uint16_t       indexED_bulk;    // index at ED
    bool           enabledPortFlag; // root ports enabled
    uint8_t        num;             // number of the OHCI
} ohci_t;


void ohci_install(pciDev_t* PCIdev);

void ohci_setupTransfer(usb_transfer_t* transfer);
void ohci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length);
void ohci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, void* buffer, size_t length);
void ohci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, const void* buffer, size_t length);
void ohci_scheduleTransfer(usb_transfer_t* transfer);
bool ohci_pollTransfer(usb_transfer_t* transfer);
void ohci_waitForTransfer(usb_transfer_t* transfer);
void ohci_destructTransfer(usb_transfer_t* transfer);


#endif
