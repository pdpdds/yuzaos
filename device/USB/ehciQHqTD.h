#ifndef EHCI_QH_QTD_H
#define EHCI_QH_QTD_H

#include "windef.h"
#include "ehci.h"

#pragma pack(push, 1)
typedef struct
{
    uint8_t status;

    uint8_t  pid          :  2;
    uint8_t  errorCounter :  2;
    uint8_t  currPage     :  3;
    uint8_t  _interrupt    :  1;
    uint16_t bytes        : 15;
    uint16_t dataToggle   :  1;
} ehci_qtdToken_t;

typedef struct
{
    uint32_t        next;
    uint32_t        nextAlt;
    ehci_qtdToken_t token;
    uint32_t        buffer0;
    uint32_t        buffer1;
    uint32_t        buffer2;
    uint32_t        buffer3;
    uint32_t        buffer4;
    uint32_t        extend0; // 64-bit data structures as documented in appendix B
    uint32_t        extend1;
    uint32_t        extend2;
    uint32_t        extend3;
    uint32_t        extend4;
} ehci_qtd_t;

typedef struct ehci_qhd
{
    uint32_t   horizontalPointer;
    uint32_t   deviceAddress       :  7;
    uint32_t   inactive            :  1;
    uint32_t   endpoint            :  4;
    uint32_t   endpointSpeed       :  2;
    uint32_t   dataToggleControl   :  1;
    uint32_t   H                   :  1;
    uint32_t   maxPacketLength     : 11;
    uint32_t   controlEndpointFlag :  1;
    uint32_t   nakCountReload      :  4;
    uint8_t    interruptScheduleMask;
    uint8_t    splitCompletionMask;
    uint16_t   hubAddr             :  7;
    uint16_t   portNumber          :  7;
    uint16_t   mult                :  2;
    uint32_t   current;
    ehci_qtd_t qtd; // transfer overlay
} ehci_qhd_t;

typedef struct
{
    ehci_qtd_t* qTD;
    void*       qTDBuffer;
} ehci_transaction_t;



/////////////////////////////
// periodic list           //
/////////////////////////////

// Isochronous data streams are managed using Isochronous Transaction Descriptors (iTD)
// Isochronous split-transaction data streams are managed with Split-transaction Isochronous Transfer Descriptors (siTD)
// Interrupt data streams are managed via QH and qTD

typedef struct
{
    uint32_t transactionOffset              : 12; // offset in bytes, from buffer start. This field is concatenated onto the buffer page pointer (PageSelect)
    uint32_t PageSelect                     :  3; // selects buffer ptr, valid values: 0 to 6
    uint32_t IOC                            :  1; // interrupt on complete
    uint32_t transactionLength              : 12; // The maximum value this field may contain is 0xC00 (3072)
    uint32_t status_TransactionError        :  1;
    uint32_t status_Babble                  :  1;
    uint32_t status_DataBufferError         :  1;
    uint32_t status_Active                  :  1;
} ehci_iTDStatusControl_t;

typedef struct
{
    uintptr_t               nextLP;    // Next Link Pointer: for schedule linkage purposes only
    ehci_iTDStatusControl_t tsc[8];    // control and status for one ?frame's worth of transactions for a single hispeed-isochronous endpoint
    uintptr_t               buffer[7]; // cf. Figure 3-4. Isochronous Transaction Descriptor (iTD), 3.3.3 iTD Buffer Page Pointer List (Plus)
                                       // physical memory pointers to data buffers that are 4K aligned
                                       // Endpoint capabilities: This area utilizes the unused low-order 12 bits of the buffer page pointer array
    uintptr_t               extend[7];
    uint32_t                padding;   // 92 ==> 96 byte for 32 byte alignment
} ehci_iTD_t;
#pragma pack(pop)

typedef struct
{
    uint16_t times;
    uint16_t numChain;
    ehci_iTD_t* iTD;
    void* iTDvirtBuffer0;
    ehci_iTD_t* veryLast_iTD;
} ehci_iTDchainTransfer_t;


void ehci_createQH(ehci_qhd_t* head, uint32_t horizPtr, ehci_qtd_t* firstQTD, uint8_t H, uint32_t device, uint8_t endpoint, uint32_t packetSize, uint8_t s_mask);
ehci_qtd_t* ehci_createQTD_SETUP(uintptr_t next, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length, void** buffer);
ehci_qtd_t* ehci_createQTD_IO(uintptr_t next, uint8_t direction, bool toggle, uint16_t tokenBytes, void** buffer);

void ehci_createiTD(ehci_t* e, ehci_iTD_t* iTD, uint8_t devNum, uint8_t epAddr, uint16_t packetSize, void* virtBuffer0);
void ehci_createiTDchain(usb_transfer_t* transfer, ehci_iTDchainTransfer_t* iTDchainTransfer);

void ehci_initializeAsyncScheduler(ehci_t* e);
void ehci_addToAsyncScheduler(ehci_t* e, usb_transfer_t* transfer, uint8_t velocity);

void ehci_initializePeriodicList(ehci_t* e);
void ehci_addToPeriodicScheduler(ehci_t* e, usb_transfer_t* transfer);

uint8_t ehci_showStatusbyteQTD(const ehci_qtd_t* qTD);
uint8_t ehci_getNakCounter(const ehci_qhd_t* qh);

void ehci_show_iTD(ehci_iTD_t* iTD);


#endif
