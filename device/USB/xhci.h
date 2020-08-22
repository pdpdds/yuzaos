#ifndef XHCI_H
#define XHCI_H

#include "_pci.h"
#include "usb_hc.h"


#define MAX_HC_SLOTS          16    // 6.1   - Max number of USB devices for any host controller
#define MAX_HC_PORTS          16    // 5.3.3 - MaxPorts (HCSPARAMS1, Bit 24-31)
#define NUM_ENDPOINTS         31    // EP0 (IN and OUT), EP1 OUT, EP1 IN, ..., EP15 OUT, EP15 IN

#define APICIRQ                3

/* ****** */
/* USBCMD */   // Table 28
/* ****** */
#define CMD_RUN     BIT(0)          // 5.4.1.1 - Run/Stop (R/S)
#define CMD_RESET   BIT(1)
#define CMD_INTE    BIT(2)
#define CMD_HSEE    BIT(3)
// Bit 4-6 reserved
#define CMD_LHCRST  BIT(7)
#define CMD_CSS     BIT(8)
#define CMD_CRS     BIT(9)
#define CMD_EWE     BIT(10)
#define CMD_EU3S    BIT(11)
// Bit 12-31 reserved

/* ****** */
/* USBSTS */   // Table 29
/* ****** */

#define STS_INTMASK 0x041C
#define STS_HCH     BIT(0)
// Bit 1 reserved
#define STS_HSE     BIT(2)
#define STS_EINT    BIT(3)
#define STS_PCD     BIT(4)
// Bit 5-7 reserved
#define STS_SSS     BIT(8)
#define STS_RSS     BIT(9)
#define STS_SRE     BIT(10)
#define STS_CNR     BIT(11)
#define STS_HCE     BIT(12)
// Bit 13-31 reserved

/* ****** */
/* PORTSC */   // Table 35
/* ****** */

#define PORT_CCS    BIT(0)
#define PORT_PED    BIT(1)
// Bit 2 reserved
#define PORT_OCA    BIT(3)
#define PORT_PR     BIT(4)
//Bit 5-8 Port Link State
#define PORT_PP     BIT(9)
//Bit 10-13 Port Speed
//Bit 14-15 Port Indicator Control
#define PORT_LWS    BIT(16)
#define PORT_CSC    BIT(17)
#define PORT_PEC    BIT(18)
#define PORT_WRC    BIT(19)
#define PORT_OCC    BIT(20)
#define PORT_PRC    BIT(21)
#define PORT_PLC    BIT(22)
#define PORT_CEC    BIT(23)
#define PORT_CAS    BIT(24)
#define PORT_WCE    BIT(25)
#define PORT_WDE    BIT(26)
#define PORT_WOE    BIT(27)
// Bit 28-29 reserved
#define PORT_DR     BIT(30)
#define PORT_WPR    BIT(31)


// TRB type
#define TRB_TYPE_RESERVED               0x00
#define TRB_TYPE_NORMAL                 0x01
#define TRB_TYPE_SETUP_STAGE            0x02
#define TRB_TYPE_DATA_STAGE             0x03
#define TRB_TYPE_STATUS_STAGE           0x04
#define TRB_TYPE_ISOCH                  0x05
#define TRB_TYPE_LINK                   0x06
#define TRB_TYPE_EVENT_DATA             0x07
#define TRB_TYPE_NOOP                   0x08
#define TRB_TYPE_ENABLE_SLOT            0x09
#define TRB_TYPE_DISABLE_SLOT           0x0A
#define TRB_TYPE_ADDRESS_DEVICE         0x0B
#define TRB_TYPE_CONFIGURE_EP           0x0C
#define TRB_TYPE_EVALUATE_CTX           0x0D
#define TRB_TYPE_RESET_EP               0x0E
#define TRB_TYPE_STOP_EP                0x0F
#define TRB_TYPE_SET_TR_DEQUEUE         0x10
#define TRB_TYPE_RESET_DEVICE           0x11
#define TRB_TYPE_FORCE_EVENT            0x12
#define TRB_TYPE_NEGOTIATE_BW           0x13
#define TRB_TYPE_SET_LATENCY_TOL        0x14
#define TRB_TYPE_GET_PORT_BW            0x15
#define TRB_TYPE_FORCE_HEADER           0x16
#define TRB_TYPE_NOOP_CMD               0x17
#define TRB_EVENT_TRANSFER              0x20
#define TRB_EVENT_CMD_COMPLETE          0x21
#define TRB_EVENT_PORT_STS_CHANGE       0x22
#define TRB_EVENT_BW_REQUEST            0x23
#define TRB_EVENT_DOORBELL              0x24
#define TRB_EVENT_HOST_CTRL             0x25
#define TRB_EVENT_DEVICE_NOTIFY         0x26
#define TRB_EVENT_MFINDEX_WRAP          0x27

#pragma pack(push, 1)
typedef struct
{
    volatile uint8_t   caplength;            // 5.3.1 - Capability Register Length
    volatile uint8_t   reserved1;
    volatile uint16_t  hciversion;           // 5.3.2 - Interface Version Number
    volatile uint32_t  hcsparams1;           // 5.3.3 - Structural Parameters 1
    volatile uint32_t  hcsparams2;           // 5.3.4 - Structural Parameters 2
    volatile uint32_t  hcsparams3;           // 5.3.5 - Structural Parameters 3
    volatile uint32_t  hccparams1;           // 5.3.6 - Capability Parameters 1
    volatile uint32_t  dboff;                // 5.3.7 - Doorbell Offset
    volatile uint32_t  rtsoff;               // 5.3.8 - Runtime Register Space Offset
    volatile uint32_t  hccparams2;           // 5.3.9 - Capability Parameters 2
} xhci_CapRegs_t;

typedef struct
{
    volatile uint32_t portsc;              // 5.4.8 - port 1 registers serving as base address for other ports 0x400 - 0x13FF
    volatile uint32_t portpmsc;
    volatile uint32_t portli;
    volatile uint32_t reserved8;
} xhci_PortRegs_t;

typedef struct
{
    volatile uint32_t command;             // 5.4.1 - USB Command                                0x00
    volatile uint32_t status;              // 5.4.2 - USB Status                                 0x04
    volatile uint32_t pagesize;            // 5.4.3 - Page size                                  0x08
    volatile uint32_t reserved1[2];        //                                                    0x0C
    volatile uint32_t devnotifctrl;        // 5.4.4 - Device Notification Control                0x14
    volatile uint64_t crcr;                // 5.4.5 - Command Ring Control                       0x18
    volatile uint32_t reserved2[4];        //                                                    0x20
    volatile uint64_t dcbaap;              // 5.4.6 - Device Context Base Address Array Pointer  0x30
    volatile uint32_t config;              // 5.4.7 - Configure                                  0x38
    volatile uint32_t reserved3[241];      //                                                    0x3C  - 0x3FF
    volatile xhci_PortRegs_t PortReg[255]; // port 1-255                                         0x400 - 0x13FF
} xhci_OpRegs_t;

typedef struct
{
    volatile uint32_t dword;
} xhci_DoorbellRegs_t;

typedef struct
{
    uint32_t physBaseAddressLo; // Bits 0-5 must be zero
    uint32_t physBaseAddressHi;
    uint32_t ringSegmentSize;        //   : 16;
    //volatile uint32_t reserved1         : 16;
    uint32_t reserved2;
} xhci_eventRingSegmentTableEntry_t;

typedef struct
{
    volatile uint32_t IP                                 :  1; // Interrupt Pending
    volatile uint32_t IE                                 :  1; // Interrupt Enable
    volatile uint32_t reserved1                          : 30;

    volatile uint16_t interrupterModerationInterval;
    volatile uint16_t interrupterModerationCounter;

    volatile uint16_t eventRingSegmentTableSize;
    volatile uint16_t reserved2;

    volatile uint32_t reserved3;

    //volatile uint32_t reserved4                          :  6;
    volatile uint32_t eventRingSegmentTableBaseAddressLo; //: 26;

    volatile uint32_t eventRingSegmentTableBaseAddressHi;

    //volatile uint32_t DESI                               :  3;
    //volatile uint32_t EHB                                :  1;
    volatile uint32_t eventRingDequeuePointerLo;//          : 28;
    volatile uint32_t eventRingDequeuePointerHi;

} interrupterRegisterSet_t;

typedef struct
{
    volatile uint32_t MFINDEX        : 14;             // 5.5.1 - Microframe Index Register    0x00
    volatile uint32_t reserved1      : 18;

    volatile uint32_t reserved2[7];                    //                                      0x04

    volatile interrupterRegisterSet_t IRS[128];        // 5.5.2 - Interrupter Register Set     0x20 (max. 1024), Linux: 128
} xhci_RuntimeRegs_t;


typedef struct
{
    volatile uint64_t scratchpadBufferArrBase;
    volatile uint64_t devContextPtr[MAX_HC_SLOTS];
} xhci_DeviceContextArray_t;

typedef struct
{
    volatile uint32_t routeString       : 20;
    volatile uint32_t speed             :  4;
    volatile uint32_t reserved1         :  1;
    volatile uint32_t MTT               :  1;
    volatile uint32_t hub               :  1;
    volatile uint32_t contextEntries    :  5;

    volatile uint16_t maxExitLatency;
    volatile uint8_t rootHubPortNumber;
    volatile uint8_t numberOfPorts;

    volatile uint8_t TThubSlotID;
    volatile uint8_t TTportNumber;
    volatile uint32_t TTT               :  2;
    volatile uint32_t reserved2         :  4;
    volatile uint32_t interrupterTarget : 10;

    volatile uint8_t usbDeviceAddress;
    volatile uint32_t reserved3         : 19;
    volatile uint32_t slotState         :  5;

    volatile uint32_t reserved4[4];
} xhci_slotContext_t;

typedef struct
{
    volatile uint32_t epState           :  3;
    volatile uint32_t reserved1         :  5;
    volatile uint32_t mult              :  2;
    volatile uint32_t maxPStreams       :  5;
    volatile uint32_t lsa               :  1;
    volatile uint8_t  interval;
    volatile uint8_t  reserved2;

    volatile uint32_t reserved3         :  1;
    volatile uint32_t errCount          :  2;
    volatile uint32_t epType            :  3;
    volatile uint32_t reserved4         :  1;
    volatile uint32_t hid               :  1;
    volatile uint8_t  maxBurstSize;
    volatile uint16_t maxPacketSize;

    volatile uint32_t dcs               :  1;
    volatile uint32_t reserved5         :  3;
    volatile uint32_t TRdequeuePtrLo    : 28;

    volatile uint32_t TRdequeuePtrHi;

    volatile uint16_t averageTRBLenghth;
    volatile uint16_t maxESITPayload;

    volatile uint32_t reserved6[3];
} xhci_endpointContext_t;

typedef struct
{
    xhci_slotContext_t     slotContext;
    xhci_endpointContext_t endpointContext[31]; // 0: bidirectional, 1-30: EP[(i+1)/2]->OUT/IN
} xhci_DeviceContext_t;

typedef struct
{
    volatile uint32_t D; // Drop Context flags. D0 and D1 are reserved.
    volatile uint32_t A; // Add Context flags

    uint32_t reserved1[5];

    uint8_t configValue;
    uint8_t interfaceNum;
    uint8_t alternateSetting;
    uint8_t reserved2;
} xhci_InputControlContext_t;

typedef struct
{
    xhci_InputControlContext_t ICC;
    xhci_DeviceContext_t       DC;
} xhci_InputContext_t;


// Note for Command (4.11.4) and Transfer (4.11.2) TRBs:
// All four bytes shall be accessed DWORD-wise, otherwise the last DWORD must be written after the first three are initialized (4.11.1.1)
typedef struct
{
    uint32_t InputContextPtrLo;      // Bit 0-3 RsvdZ - aligned on a 16-byte address boundary
    uint32_t InputContextPtrHi;

    uint32_t reserved1;              // RsvdZ

    uint32_t cycle             :  1;
    uint32_t reserved2         :  8; // RsvdZ
    uint32_t BSR_DC            :  1;
    uint32_t TRBtype           :  6;
    uint32_t reserved3         :  8; // RsvdZ
    uint8_t  SlotID;
} xhci_cmd_DeviceTRB_t;

typedef struct
{
    uint32_t reserved1;              // RsvdZ
    uint32_t reserved2;              // RsvdZ
    uint32_t reserved3;              // RsvdZ

    uint32_t cycle             :  1;
    uint32_t reserved4         :  8; // RsvdZ
    uint32_t TSP               :  1;
    uint32_t TRBtype           :  6;
    uint32_t EndpointID        :  5;
    uint32_t reserved5         :  3; // RsvdZ
    uint8_t  SlotID;
} xhci_cmd_ResetEndpointTRB_t;

typedef struct
{
    uint32_t reserved1;              // RsvdZ
    uint32_t reserved2;              // RsvdZ
    uint32_t reserved3;              // RsvdZ

    uint32_t cycle             :  1;
    uint32_t reserved4         :  9; // RsvdZ
    uint32_t TRBtype           :  6;
    uint32_t EndpointID        :  5;
    uint32_t reserved5         :  2; // RsvdZ
    uint32_t SP                :  1;
    uint8_t  SlotID;
} xhci_cmd_StopEndpointTRB_t;

typedef struct
{
    uint32_t NewTRDeqPtrLo;
    uint32_t NewTRDeqPtrHi;

    uint32_t reserved1         :  16; // RsvdZ
    uint32_t StreamID          :  16;

    uint32_t cycle             :   1;
    uint32_t reserved2         :   9; // RsvdZ
    uint32_t TRBtype           :   6;
    uint32_t EndpointID        :   5;
    uint32_t reserved3         :   3; // RsvdZ
    uint8_t  SlotID;

} xhci_TR_DequeuePointerTRB_t;

typedef struct
{
    uint32_t RingSegmentPtrLo;       // only high order valid! - must be aligned on a 16-byte address boundary
    uint32_t RingSegmentPtrHi;

    uint32_t reserved1         : 22; // RsvdZ
    uint32_t IntTarget         : 10; // Interrupter Target

    uint32_t cycle             :  1;
    uint32_t TC                :  1;
    uint32_t reserved2         :  2; // RsvdZ
    uint32_t CH                :  1;
    uint32_t IOC               :  1;
    uint32_t reserved3         :  4; // RsvdZ
    uint32_t TRBtype           :  6;
    uint32_t reserved4         : 16; // RsvdZ
} xhci_LinkTRB_t;

typedef struct
{
    uint32_t eventDataLo;            // physical memory pointer
    uint32_t eventDataHi;

    uint32_t transferLength    : 24; // only Transfer Event
    uint8_t  completionCode;

    uint32_t cycle             :  1;
    uint32_t reserved1         :  1;
    uint32_t ED                :  1; // only Transfer Event
    uint32_t reserved2         :  7;
    uint32_t TRBtype           :  6;
    uint8_t  byte3;                  // Transfer Event: Endpoint (16-20); Cmd Compl., Doorbell: VF (ID of the Virtual Function that generated the event)
    uint8_t  slot;                   // Transfer Event, Cmd Compl., Bandwidth Request, Doorbell, Device Notification
} xhci_eventTRB_t; // 6.4.2 Event TRBs



// Transfer Ring, Transfer TRBs
typedef struct
{
    uint32_t dataBufferPtrLo;
    uint32_t dataBufferPtrHi;

    uint32_t transferLength    : 17;
    uint32_t TDsize            :  5; // indicator of the number of packets remaining in the TD, cf. 4.11.2.4
    uint32_t IntTarget         : 10; // Interrupter Target

    uint32_t cycle             :  1;
    uint32_t ENT               :  1; // Evaluate Next TRB
    uint32_t ISP               :  1; // Interrupt-on Short Packet
    uint32_t NS                :  1; // No Snoop
    uint32_t CH                :  1; // Chain bit
    uint32_t IOC               :  1; // Interrupt On Completion
    uint32_t IDT               :  1; // Immediate Data
    uint32_t reserved1         :  2;
    uint32_t BEI               :  1; // Block Event Interrupt
    uint32_t TRBtype           :  6; // Normal: ID 1, cf. Table 131: TRB Type Definitions
    uint32_t reserved2         :  16;
} xhci_xfer_NormalTRB_t;

typedef struct
{
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;

    uint16_t wIndex;
    uint16_t wLength;

    uint32_t transferLength    : 17;
    uint32_t reserved1         :  5;
    uint32_t IntTarget         : 10;

    uint32_t cycle             :  1;
    uint32_t reserved2         :  4;
    uint32_t IOC               :  1;
    uint32_t IDT               :  1;
    uint32_t reserved3         :  3;
    uint32_t TRBtype           :  6; // Setup Stage: ID 2, cf. Table 131: TRB Type Definitions
    uint32_t TRT               :  2; // Transfer Type
    uint32_t reserved4         :  14;
} xhci_xfer_SetupStageTRB_t;

typedef struct
{
    uint32_t dataBufferPtrLo;
    uint32_t dataBufferPtrHi;

    uint32_t transferLength    : 17;
    uint32_t TDsize            :  5; // indicator of the number of packets remaining in the TD, cf. 4.11.2.4
    uint32_t IntTarget         : 10; // Interrupter Target

    uint32_t cycle             :  1;
    uint32_t ENT               :  1; // Evaluate Next TRB
    uint32_t ISP               :  1; // Interrupt-on Short Packet
    uint32_t NS                :  1; // No Snoop
    uint32_t CH                :  1; // Chain bit
    uint32_t IOC               :  1; // Interrupt On Completion
    uint32_t IDT               :  1; // Immediate Data
    uint32_t reserved1         :  3;
    uint32_t TRBtype           :  6; // Data Stage: ID 3, cf. Table 131: TRB Type Definitions
    uint32_t DIR               :  1; // Direction
    uint32_t reserved2         :  15;
} xhci_xfer_DataStageTRB_t;

typedef struct
{
    uint32_t reserved1;
    uint32_t reserved2;

    uint32_t reserved3         : 22;
    uint32_t IntTarget         : 10; // Interrupter Target

    uint32_t cycle             :  1;
    uint32_t ENT               :  1; // Evaluate Next TRB
    uint32_t reserved4         :  2;
    uint32_t CH                :  1; // Chain bit
    uint32_t IOC               :  1; // Interrupt On Completion
    uint32_t reserved5         :  4;
    uint32_t TRBtype           :  6; // Status Stage: ID 4, cf. Table 131: TRB Type Definitions
    uint32_t DIR               :  1; // Direction
    uint32_t reserved6         :  15;
} xhci_xfer_StatusStageTRB_t;
#pragma pack(pop)

typedef struct
{
   uint32_t  timeTransfer;
   uint32_t  timeEvent;
   uint8_t   epState;
   bool      pendingTransfer;  // 0: event occurred  1: waiting for event
   uint8_t   transferError;    // cf. 6.4.5 TRB Completion Codes

   // transfer ring
   bool                   TransferRingProducerCycleState; // PCS
   xhci_xfer_NormalTRB_t* virtEnqTransferRingPtr;
   xhci_xfer_NormalTRB_t* virtDeqTransferRingPtr;
   xhci_xfer_NormalTRB_t* TransferRingbase;
   uint32_t               TransferCounter;
} xhci_endpoint_t;

typedef struct
{
    xhci_endpoint_t endpoints[NUM_ENDPOINTS];
    uint8_t         slotState;
} xhci_slot_t;

typedef struct
{
    uint8_t slotNr;
    xhci_LinkTRB_t* cmdPtr;
} xhci_port_slot_Link_t;

typedef struct
{
    hc_t                  hc;                // Generic HC data
    pciDev_t*             PCIdevice;         // PCI device
    void*                 bar;               // base address register
    xhci_OpRegs_t*        OpRegs;
    xhci_CapRegs_t*       CapRegs;
    xhci_RuntimeRegs_t*   RuntimeRegs;
    xhci_DoorbellRegs_t*  DoorbellRegs;      // Doorbell Array: max. 256 Doorbells (one for each Device Slot)

    // device contexts
    xhci_DeviceContextArray_t* virt_deviceContextPointerArrayBase;
    xhci_DeviceContext_t* devContextPtr[MAX_HC_SLOTS];
    xhci_InputContext_t*  devInputContextPtr[MAX_HC_SLOTS];

    // command ring
    xhci_LinkTRB_t*       virtEnqCmdRingPtr;
    xhci_LinkTRB_t*       CmdRingbase;
    uint32_t              CmdCounter;
    uint32_t              CmdPending;

    // event ring
    xhci_eventTRB_t*      virtDeqEvtRingPtr; // address of events
    xhci_eventTRB_t*      EvtRingbase;
    uint32_t              EvtCounter;
    uint32_t              evtSegmentSize;
    uint32_t              evtNumberSegments;

    // transferrings
    xhci_xfer_NormalTRB_t* trb[MAX_HC_SLOTS][NUM_ENDPOINTS];

    // cycle states
    bool                  CmdRingProducerCycleState; // PCS
    bool                  EvtRingConsumerCycleState; // CCS

    // MSI, MSI-X
    bool                  msiCapEnabled;
    uint8_t               irq;

    // ports/slots
    xhci_slot_t*          slots[MAX_HC_SLOTS];
    bool                  portsEnabled[MAX_HC_PORTS];
    xhci_port_slot_Link_t portSlotLink[MAX_HC_PORTS];
} xhci_t;




void xhci_install(pciDev_t* PCIdev);
void xhci_updateEndpointInformation(hc_port_t* hc_port);
void xhci_pollDisk(port_t* dev);

void xhci_setupTransfer(usb_transfer_t* transfer);
uint8_t xhci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length);
void xhci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, void* buffer, size_t length, uint16_t remainingIn);
void xhci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, const void* buffer, size_t length, uint16_t remainingOut);
void xhci_scheduleTransfer(usb_transfer_t* transfer);
bool xhci_pollTransfer(usb_transfer_t* transfer);
void xhci_waitForTransfer(usb_transfer_t* transfer);
void xhci_destructTransfer(usb_transfer_t* transfer);

#endif
