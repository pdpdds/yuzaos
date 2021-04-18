#include <stdio.h>
#include <memory.h>
#include "ehciQHqTD.h"
#include "ehci.h"
#include "usb_hc.h"
#include <string.h>
#include <systemcall_impl.h>
#include <InterruptHandler.h>
#include <math.h>

//20200330
extern uint32_t* memsetl(uint32_t* dest, uint32_t val, size_t dwords);

static inline uint32_t alignDown(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    return val & ~(alignment - 1);
}

bool waitForIRQ(int irq, unsigned int milliSecond)
{
    if (milliSecond == 0)
    {
        return true;
    }
    else
    {
        int waitCount = milliSecond;
        while (waitCount > 0)
        {
            Syscall_Sleep(1);
            waitCount--;
        }
    }

    return true;

}

#include <SpinLock.h>

#define printfe printf
#define textColor(a)
#define serial_log(a, ...)
#define sleepMilliSeconds Syscall_Sleep
#define writeInfo printf

#define SER_LOG_EHCI_ITD 0
#define PAGE_SIZE 4096

/*#define WAIT_FOR_CONDITION(condition, runs, wait)\
    for (unsigned int timeout_ = 0; !(condition); timeout_++) {\
        if (timeout_ >= runs) {\
             break;\
        }\
        Syscall_Sleep(wait);\
    }*/

extern unsigned int paging_getPhysAddr(void* va);

/////////////////////
// Queue Head (QH) //
/////////////////////

void ehci_createQH(ehci_qhd_t* head, uint32_t horizPtr, ehci_qtd_t* firstQTD, uint8_t H, uint32_t device, uint8_t endpoint, uint32_t packetSize, uint8_t s_mask)
{
    memset(head, 0, sizeof(ehci_qhd_t));
                                                  // bit 31:5 Horizontal Link Pointer, bit 4:3 reserved,
    head->horizontalPointer     = horizPtr | 0x2; // bit 2:1  type:  00b iTD,   01b QH,   10b siTD,   11b FSTN
                                                  // bit 0    T-Bit: is set to zero
    head->deviceAddress         = device;         // The device address
    head->inactive              = 0;
    head->endpoint              = endpoint;       // endpoint 0 contains Device infos such as name
    head->endpointSpeed         = 2;              // 00b = full speed; 01b = low speed; 10b = high speed
    head->dataToggleControl     = 1;              // get the Data Toggle bit out of the included qTD
    head->H                     = H;              // mark a queue head as being the head of the reclaim list
    head->maxPacketLength       = packetSize;     // 64 byte for a control transfer to a high speed device
    head->controlEndpointFlag   = 0;              // only used if endpoint is a control endpoint and not high speed
    head->nakCountReload        = 0;              // this value is used by EHCI to reload the Nak Counter field. 0=ignores NAK counter.
    head->interruptScheduleMask = s_mask;         // 0 for async schedule, non-zero for periodic schedule
    head->splitCompletionMask   = 0;              // unused if not low/full speed && periodic schedule
    head->hubAddr               = 0;              // unused if high speed (Split transfer)
    head->portNumber            = 0;              // unused if high speed (Split transfer)
    head->mult                  = 1;              // 1-3 transaction per micro-frame, 0 means undefined results
    if (firstQTD == 0)
        head->qtd.next = 0x1;
    else
        head->qtd.next = paging_getPhysAddr(firstQTD);
}

/////////////////////////////////////////////
// Queue Element Transfer Descriptor (qTD) //
/////////////////////////////////////////////

static ehci_qtd_t* allocQTD(uintptr_t next)
{
    ehci_qtd_t* td = (ehci_qtd_t*)malloc_aligned(sizeof(ehci_qtd_t), 4096); // can be 32 byte alignment
    memset(td, 0, sizeof(ehci_qtd_t));

    if (next != 0x1)
        td->next = paging_getPhysAddr((void*)next);
    else
        td->next = 0x1;
    td->nextAlt            = 0x1;  // No alternate next, so T-Bit is set to 1
    td->token.status       = 0x80; // This will be filled by the Host Controller. Active bit set
    td->token.errorCounter = 0x3;  // 3 retries. The Host controller aborts if the error counter transitions from 1 to 0 and issues an error interrupt.
    td->token.currPage     = 0x0;  // Written by the Host Controller.
    td->token._interrupt    = 0x0;  // We want an interrupt after complete transfer, but not after each transaction

    return td;
}

static void* allocQTDbuffer(ehci_qtd_t* td, void** buffer, size_t size)
{
    if (size == 0)
        return 0;

    if(*buffer == 0)
    {
        *buffer = malloc_aligned(size, 4096);
        memset(*buffer, 0, size);
    }

    td->buffer0 = paging_getPhysAddr(*buffer);
    //20200330
    if ((PAGE_SIZE - td->buffer0%PAGE_SIZE) < size)
        td->buffer1 = alignDown(paging_getPhysAddr((int*)(*buffer)+1024), PAGE_SIZE);

    return *buffer;
}

ehci_qtd_t* ehci_createQTD_SETUP(uintptr_t next, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length, void** buffer)
{
    ehci_qtd_t* td = allocQTD(next);

    td->token.pid        = USB_TT_SETUP; // SETUP = 2
    td->token.bytes      = 8;            // Setup stage transfer length is always 8
    td->token.dataToggle = toggle;       // Should be toggled every list entry

    usb_request_t* request = (usb_request_t *)allocQTDbuffer(td, buffer, 8);
    request->type    = type;
    request->request = req;
    request->valueHi = hiVal;
    request->valueLo = loVal;
    request->index   = index;
    request->length  = length;

    return td;
}

ehci_qtd_t* ehci_createQTD_IO(uintptr_t next, uint8_t direction, bool toggle, uint16_t tokenBytes, void** buffer)
{
    ehci_qtd_t* td = allocQTD(next);

    td->token.pid        = direction;  // OUT = 0, IN = 1
    td->token.bytes      = tokenBytes; // dependent on transfer
    td->token.dataToggle = toggle;     // Should be toggled every list entry

    allocQTDbuffer(td, buffer, tokenBytes);

    return td;
}


////////////////////
// analysis tools //
////////////////////

uint8_t ehci_showStatusbyteQTD(const ehci_qtd_t* qTD)
{
    if (qTD->token.status != 0x00)
    {
        textColor(ERROR);
        if (qTD->token.status & BIT(6))
        {
            printf("\nHalted - serious error at the device/endpoint");
            printf("\nError Counter: %u", qTD->token.errorCounter);
        }
        if (qTD->token.status & BIT(5)) { printf("\nData Buffer Error (overrun or underrun)"); }
        if (qTD->token.status & BIT(4)) { printf("\nBabble - fatal error leads to Halted"); }
        if (qTD->token.status & BIT(3)) { printf("\nTransaction Err - no valid response, e.g. Timeout, CRC, Bad PID, etc."); }
        if (qTD->token.status & BIT(2)) { printf("\nMissed Micro-Frame"); }

      #ifdef _EHCI_DIAGNOSIS_
        textColor(IMPORTANT);
        if (qTD->token.status & BIT(7)) { printf("\nActive - HC transactions enabled"); }
        if (qTD->token.status & BIT(1)) { printf("\nDo Complete Split"); }
        if (qTD->token.status & BIT(0)) { printf("\nDo Ping"); }
      #endif

        textColor(TEXT);
    }
    return qTD->token.status;
}

uint8_t ehci_getNakCounter(const ehci_qhd_t* qh)
{
    return (qh->qtd.nextAlt & 0x1E)>>1;
}

///////////////////////////
// Asynchronous schedule //
///////////////////////////

static void enableAsyncScheduler(ehci_t* e)
{
    e->OpRegs->USBCMD |= CMD_ASYNCH_ENABLE;

    WAIT_FOR_CONDITION(e->OpRegs->USBSTS & STS_ASYNC_ENABLED, 7, 10); // wait until it is really on
}

void ehci_initializeAsyncScheduler(ehci_t* e)
{
    if (!e->idleQH)
    {
        e->idleQH = e->tailQH = (ehci_qhd*)malloc_aligned(sizeof(ehci_qhd_t), 4096);
    }
    ehci_createQH(e->idleQH, paging_getPhysAddr(e->idleQH), 0, 1, 0, 0, 0, 0);
    e->OpRegs->ASYNCLISTADDR = paging_getPhysAddr(e->idleQH);
    enableAsyncScheduler(e);
}

void ehci_addToAsyncScheduler(ehci_t* e, usb_transfer_t* transfer, uint8_t velocity)
{
    if (!(e->OpRegs->USBSTS & STS_ASYNC_ENABLED))
        enableAsyncScheduler(e); // Start async scheduler, when it is not running

    ehci_qhd_t* oldTailQH = e->tailQH; // save old tail QH
    e->tailQH = (ehci_qhd *)transfer->data; // new QH is now end of Queue

    //irq_resetCounter((ehci_qhd_t*)e->PCIdevice->irq);
    e->tailQH->horizontalPointer = paging_getPhysAddr(e->idleQH) | BIT(1); // Create ring. Link new QH with idleQH (always head of Queue)
    oldTailQH->horizontalPointer = paging_getPhysAddr(e->tailQH) | BIT(1); // Insert qh to Queue as element behind old queue head

    uint32_t timeout = 10 * velocity + 25 * 30; // Wait up to 250+100*velocity milliseconds for all transfers to be finished
    dlelement_t* dlE = transfer->transactions.head;
    while (timeout > 0)
    {
        ehci_transaction_t* eT = (ehci_transaction_t *)((usb_transaction_t*)dlE->data)->data;
        while (!(eT->qTD->token.status & BIT(7)))
        {
            dlE = dlE->next;
            if (dlE == 0)
                break;
            eT = (ehci_transaction_t *)((usb_transaction_t*)dlE->data)->data;
        }
        if (dlE == 0)
            break;
        
        //Syscall_Sleep(1);
        waitForIRQ(e->PCIdevice->irq, 1);
        //irq_resetCounter(e->PCIdevice->irq);
        timeout--;
    }
    if (timeout == 0)
    {
        printfe("\nEHCI: Timeout!");
        for (;;);
    }
    //printfe("\nEHCI: Pass!");
  
    e->idleQH->horizontalPointer = paging_getPhysAddr(e->idleQH) | BIT(1); // Restore link of idleQH to idleQH (endless loop)
    e->tailQH = e->idleQH; // qh done. idleQH is end of Queue again (ring structure of asynchronous schedule)

    e->USBasyncIntPending = true;
    e->OpRegs->USBCMD |= CMD_ASYNCH_INT_DOORBELL; // Activate Doorbell: We would like to receive an asynchronous schedule interrupt
}


///////////////////
// Periodic List //
///////////////////

// The periodic frame list is a 4K-page aligned array of Frame List Link pointers. Software must support all possible lenghths: 256, 512, or 1024 elements.
// The programmability of the periodic frame list is exported to system software via the HCCPARAMS register.
// If non-programmable, the length is 1024 elements.
// Programming the number of elements is done by writing the appropriate value into Frame List Size field in the USBCMD register.

// Interrupt Transfers
// System software sets a bit in a QH's S-Mask to indicate which micro-frame with-in a 1 millisecond period a transaction should be executed
// for the QH. Software must ensure that all QH in the periodic schedule have S-Mask set to a non-zero value.

static void enablePeriodicList(ehci_t* e)
{
    e->OpRegs->USBCMD |= CMD_PERIODIC_ENABLE;
    WAIT_FOR_CONDITION(e->OpRegs->USBSTS & STS_PERIODIC_ENABLED, 10, 5); // wait until it is really on
}

/*
static void disablePeriodicList(ehci_t* e)
{
    e->OpRegs->USBCMD &= ~CMD_PERIODIC_ENABLE;
    WAIT_FOR_CONDITION(!(e->OpRegs->USBSTS & STS_PERIODIC_ENABLED), 7, 100, "\nTimeout Error - STS_PERIODIC_ENABLED still set!"); // wait until it is really off
}
*/

uint32_t* memsetl(uint32_t* dest, uint32_t val, size_t dwords)
{
    uint32_t* retval = dest;

    //rep stosl : " + D"(dest), " + c"(dwords) : "a"(val) : "memory");
    __asm
    {
        cld
        mov eax, val
        mov ecx, dwords
        mov edi, dest
        rep stosd
    }
    return retval;
}

void ehci_initializePeriodicList(ehci_t* e)
{
    e->OpRegs->USBCMD = (e->OpRegs->USBCMD & CMD_FRAMELIST_SIZE) | CMD_FRAMELIST_1024;  // bit 3-2: framelistsize, default: 1024
    e->periodicList = (uint32_t*)malloc_aligned(sizeof(uint32_t)*1024, PAGE_SIZE);
    memsetl(e->periodicList, 0x1, 1024); // Start with empty list

    e->OpRegs->PERIODICLISTBASE = paging_getPhysAddr(e->periodicList);
}

void ehci_createiTD(ehci_t* e, ehci_iTD_t* iTD, uint8_t devNum, uint8_t epAddr, uint16_t packetSize, void* virtBuffer0)
{
    //static uint16_t counter = 0;
    //serial_log(SER_LOG_EHCI_ITD, "\nehci_createiTD[ %u ] at %X: devNum=%u ep=%u packetSize=%u virtBuffer=%X", counter, iTD, devNum, epAddr, packetSize, virtBuffer0);
    //counter++;

    memset(iTD, 0, sizeof(ehci_iTD_t));
    iTD->nextLP = 0x01; // 1= Link Pointer field is not valid. bit 2:1 : 0x00 = iTD

    uint32_t totLength = 0; // bytes to be transferred by all transactions

    for (uint8_t i=0; i<8; i++)
    {
        iTD->tsc[i].transactionLength = 3072;   // min(transfer->packetSize, 3072); // max value: 0xC00 (3*1024=3072)
        iTD->tsc[i].status_Active = 1;          // to be done by periodic list
        iTD->tsc[i].IOC = 0;                    // iTD chain, only last iTD will get IOC=1

        // TODO: calculate PG+Offset for all buffers
        iTD->tsc[i].PageSelect        = totLength / PAGE_SIZE; // 0-6 (bufferPtr)
        iTD->tsc[i].transactionOffset = totLength % PAGE_SIZE; // This field is concatenated onto the buffer page pointer
        totLength += iTD->tsc[i].transactionLength;
        //serial_log(SER_LOG_EHCI_ITD, "\ni=%u, PG=%u, offset=%u", i, iTD->tsc[i].PageSelect, iTD->tsc[i].transactionOffset);
    }

    //serial_log(SER_LOG_EHCI_ITD, "\n");

    for (uint8_t i=0; i<7; i++)
    {
        iTD->buffer[i] = paging_getPhysAddr((void*)((uintptr_t)virtBuffer0 + (i * PAGE_SIZE)));
        iTD->extend[i] = 0;
        //serial_log(SER_LOG_EHCI_ITD, "\niTD->buffer[%u]=%X", i, iTD->buffer[i]);
    }
    //serial_log(SER_LOG_EHCI_ITD, "\n");

    iTD->buffer[0] |= epAddr << 8 | devNum;
    iTD->buffer[1] |= MIN(packetSize, 1024); // "Software should not set a value larger than 1024"
    iTD->buffer[2] |= 3; // number of transactions to be issued for this endpoint per micro-frame (125µs)
}

void ehci_createiTDchain(usb_transfer_t* transfer, ehci_iTDchainTransfer_t* iTDchainTransfer)
{
    ehci_t* e = (ehci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    ehci_iTD_t* current_iTD = iTDchainTransfer->iTD;

    ehci_iTD_t* old_iTD = 0;

    iTDchainTransfer->iTDvirtBuffer0 = malloc_aligned(iTDchainTransfer->numChain * iTDchainTransfer->times * 7 * PAGE_SIZE, PAGE_SIZE );
    memset(iTDchainTransfer->iTDvirtBuffer0, 0, iTDchainTransfer->numChain * iTDchainTransfer->times * 7 * PAGE_SIZE);

    for (uint16_t k=0; k<iTDchainTransfer->numChain; k++)
    {
        for (uint16_t j=0; j<iTDchainTransfer->times; j++)
        {
            //20200330
            void* virtBuffer0 = (char*)iTDchainTransfer->iTDvirtBuffer0 + ((k * iTDchainTransfer->times + j ) * 7 * PAGE_SIZE);
            ehci_createiTD(e, current_iTD, transfer->device->num, transfer->endpoint->address, transfer->packetSize, virtBuffer0);

            old_iTD = current_iTD;
            current_iTD++;

            if (j == (iTDchainTransfer->times-1))
            {
                old_iTD->nextLP = 1; // signals last iTD in chain, no valid link
                //serial_log(SER_LOG_EHCI_ITD, "\nchain %u:\t last iTD: %X", k, old_iTD);
                old_iTD->tsc[7].IOC = 1;
            }
            else
            {
                old_iTD->nextLP = paging_getPhysAddr(current_iTD);
                //serial_log(SER_LOG_EHCI_ITD, "\nchain %u:\t iDT: %X nextLP: %X", k, old_iTD, current_iTD);
            }
        }
    }
    iTDchainTransfer->veryLast_iTD = old_iTD;
}

void ehci_show_iTD(ehci_iTD_t* iTD)
{
    serial_log(SER_LOG_EHCI_ITD,"\n\niTD:");
    serial_log(SER_LOG_EHCI_ITD,"\n----");

    serial_log(SER_LOG_EHCI_ITD,"\nNext Link Ptr: %X", iTD->nextLP);

    for (uint8_t i=0; i<8; i++)
    {
        serial_log(SER_LOG_EHCI_ITD,"\nT%u: sts: %u %u %u %u len: %u ioc: %u PG: %u off: %u \tlen: %u",
            i,
            iTD->tsc[i].status_Active,
            iTD->tsc[i].status_DataBufferError,
            iTD->tsc[i].status_Babble,
            iTD->tsc[i].status_TransactionError,
            iTD->tsc[i].transactionLength,
            iTD->tsc[i].IOC,
            iTD->tsc[i].PageSelect,
            iTD->tsc[i].transactionOffset,
            iTD->tsc[i].transactionLength);
    }

    for (uint8_t i=0; i<7; i++)
    {
        serial_log(SER_LOG_EHCI_ITD,"\nphys. bufPtr[%u]: %X", i, iTD->buffer[i]);
    }
}

/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
