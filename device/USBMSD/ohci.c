/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f?r die Verwendung dieses Sourcecodes siehe unten
*/

#include "ohci.h"
#include "util/util.h"
#include "timer.h"
#include "kheap.h"
#include "tasking/task.h"
#include "irq.h"
#include "hid/keyboard.h"


static list_t ohci = list_init();


static void ohci_handler(registers_t* r, pciDev_t* device);
static void ohci_start(ohci_t* o);
static void ohci_initHC(ohci_t* o);
static void ohci_resetHC(ohci_t* o);
static void ohci_setupHC(ohci_t* o);
static void ohci_analyzePortStatus(ohci_t* o, uint8_t j);
static void ohci_resetPort(ohci_t* o, uint8_t j);
static void ohci_checkResetMempool(ohci_t* o);
static void ohci_toggleFrameInterval(ohci_t* o);
static ohciTD_t* ohci_createTD_SETUP(ohci_t* o, ohciED_t* oED, ohciTD_t* oTD, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length, void** buffer);
static void      ohci_initTD_IO(ohciTD_t* oTD, uint8_t direction, bool toggle, uint16_t tokenBytes, void* buffer);
static ohciTD_t* ohci_createTD_IO(ohci_t* o, ohciED_t* oED, ohciTD_t* oTD, uint8_t direction, bool toggle, uint16_t tokenBytes, void* buffer);
static void      ohci_initED(hc_port_t* port, ohciED_t* oED, ohciTD_t* head, ohciTD_t* tail, uint32_t device, usb_endpoint_t* endpoint);
static uint8_t   ohci_showStatusbyteTD(ohciTD_t* TD);


void ohci_install(pciDev_t* PCIdev)
{
  #ifdef _OHCI_DIAGNOSIS_
    printf("\n>>>ohci_install<<<\n");
  #endif

    static uint8_t numHC = 0;
    dlelement_t* elem = list_alloc_elem(sizeof(ohci_t), "ohci");
    ohci_t* o = elem->data;
    o->PCIdevice  = PCIdev;
    o->PCIdevice->data = o;
    o->num        = numHC++;
    o->enabledPortFlag = false;
    list_append_elem(&ohci, elem);

    // Get MMIO space
    void* bar = 0;
    for (uint8_t i = 0; i < 6 && !bar; i++)
    {
        bar = pci_aquireMemoryForMMIO(PCIdev->bar + i);
    }

    o->OpRegs = (ohci_OpRegs_t*)bar;

    char str[10];
    snprintf(str, 10, "OHCI %u", o->num+1);

    task_t* thread = create_cthread((void*)&ohci_start, str);
    task_passStackParameter(thread, &o, sizeof(o));
    scheduler_insertTask(thread);
}

static void ohci_start(ohci_t* o)
{
  #ifdef _OHCI_DIAGNOSIS_
    printf("\n>>> ohci_start <<<\n");
  #else
    textColor(HEADLINE);
    printf("Initialize OHCI Host Controller:\n");
    textColor(TEXT);
  #endif

    // prepare PCI command register
    uint16_t pciCommandRegister = pci_configRead(o->PCIdevice, PCI_COMMAND, 2);
    pci_configWrite_word(o->PCIdevice, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER); // resets status register, sets command register

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nPCI Command Register before: %xh", pciCommandRegister);
    printf("\nPCI Command Register after:  %xh", pci_configRead(o->PCIdevice, PCI_COMMAND, 2));
  #endif

    irq_installPCIHandler(o->PCIdevice->irq, ohci_handler, o->PCIdevice);

    ohci_initHC(o);
    ohci_resetHC(o);
    ohci_setupHC(o);

    printf("\n\n>>> Press key to close this console. <<<");
    getch();
}

static void ohci_initHC(ohci_t* o)
{
    // Check revision
    // When checking the Revision, the HC Driver must mask the rest of the bits in the HcRevision register
    // as they are used to specify which optional features that are supported by the HC.
    textColor(IMPORTANT);
    printf("\nOHCI: Revision %u.%u\n",
        BYTE1(o->OpRegs->HcRevision) >> 4,
        BYTE1(o->OpRegs->HcRevision) & 0xF);

    if (!((BYTE1(o->OpRegs->HcRevision)) == 0x10 || BYTE1(o->OpRegs->HcRevision) == 0x11))
    {
        printfe("Revision not valid!");
    }
    textColor(TEXT);

    // Take control of host controller (5.1.1.3)
    if (o->OpRegs->HcControl & OHCI_CTRL_IR) // SMM driver is active because the InterruptRouting bit is set
    {
        o->OpRegs->HcCommandStatus |= OHCI_STATUS_OCR; // ownership change request

        // monitor the IR bit to determine when the ownership change has taken effect
        uint16_t i;
        for (i=0; (o->OpRegs->HcControl & OHCI_CTRL_IR) && (i < 100); i++)
        {
            sleepMilliSeconds(10);
        }

        if (i < 100)
        {
          #ifdef _OHCI_DIAGNOSIS_
            // Once the IR bit is cleared, the HC driver may proceed to the setup of the HC.
            textColor(SUCCESS);
            printf("\nOHCI took control from SMM after %u loops.", i);
          #endif
        }
        else
        {
          #ifdef _OHCI_DIAGNOSIS_
            printfe("\nOwnership change request did not work. SMM has still control.");
          #endif

            o->OpRegs->HcControl &= ~OHCI_CTRL_IR; // we try to reset the IR bit
            for (i = 0; (o->OpRegs->HcControl & OHCI_CTRL_IR) && (i < 20); i++)
            {
                sleepMilliSeconds(10);
            }

            if (o->OpRegs->HcControl & OHCI_CTRL_IR) // SMM driver is still active
            {
                printfe("\nOHCI taking control from SMM did not work."); // evil
            }
          #ifdef _OHCI_DIAGNOSIS_
            else
            {
                textColor(SUCCESS);
                printf("\nSuccess in taking control from SMM.");
            }
          #endif
        }
        textColor(TEXT);
    }
    else // InterruptRouting bit is not set
    {
        if ((o->OpRegs->HcControl & OHCI_CTRL_HCFS) != OHCI_USB_RESET)
        {
            // there is an active BIOS driver, if the InterruptRouting bit is not set
            // and the HostControllerFunctionalState (HCFS) is not USBRESET
          #ifdef _OHCI_DIAGNOSIS_
            printf("\nThere is an active BIOS OHCI driver");
          #endif

            if ((o->OpRegs->HcControl & OHCI_CTRL_HCFS) != OHCI_USB_OPERATIONAL)
            {
                // If the HostControllerFunctionalState is not USBOPERATIONAL, the OS driver should set the HCFS to USBRESUME
              #ifdef _OHCI_DIAGNOSIS_
                printf("\nActivate RESUME");
              #endif
                o->OpRegs->HcControl = (o->OpRegs->HcControl & ~OHCI_CTRL_HCFS) | OHCI_USB_RESUME; // set specific HCFS bit for USBRESUME

                // and wait the minimum time specified in the USB Specification for assertion of resume on the USB (20ms in USB 1.1)
                sleepMilliSeconds(30);
            }
        }
        else // HCFS is USBRESET
        {
            // Neither SMM nor BIOS
            sleepMilliSeconds(30);
        }
    }
}

static void ohci_resetHC(ohci_t* o)
{
  #ifdef _OHCI_DIAGNOSIS_
    printf("\n\n>>> ohci_resetHC <<<\n");
  #endif
    // The HC Driver should now save the contents of the HcFmInterval register ...
    uint32_t saveHcFmInterval = o->OpRegs->HcFmInterval;

    // ... and then issue a software reset
    o->OpRegs->HcCommandStatus |= OHCI_STATUS_RESET;
    sleepMilliSeconds(10);

    // After the software reset is complete (a maximum of 10 µs), the Host Controller Driver
    // should restore the value of the HcFmInterval register
    o->OpRegs->HcFmInterval = saveHcFmInterval;
    ohci_toggleFrameInterval(o);

    // The HC is now in the USBSUSPEND state; it must not stay in this state more than 2 ms
    // or the USBRESUME state will need to be entered for the minimum time specified
    // in the USB Specification for the assertion of resume on the USB (20ms in USB 1.1).

    if ((o->OpRegs->HcControl & OHCI_CTRL_HCFS) == OHCI_USB_SUSPEND)
    {
        o->OpRegs->HcControl = (o->OpRegs->HcControl & ~OHCI_CTRL_HCFS) | OHCI_USB_RESUME; // set specific HCFS bit for USBRESUME
        sleepMilliSeconds(30);
    }
}

static void ohci_setupHC(ohci_t* o)
{
    // Setup of the Host Controller (5.1.1.4)
  #ifdef _OHCI_DIAGNOSIS_
    printf("\n\n>>> ohci_setupHC <<<\n");
  #endif

    // Initialize the device data HCCA block to match the current device data state;
    // i.e., all virtual queues are run and constructed into physical queues on the HCCA block
    // and other fields initialized accordingly.
    o->hcca = malloc(sizeof(ohci_HCCA_t), OHCI_HCCA_ALIGN, "ohci HCCA"); // HCCA must be minimum 256-byte aligned
    memset(o->hcca, 0, sizeof(ohci_HCCA_t));

    // Initialize the Operational Registers to match the current device data state;
    // i.e., all virtual queues are run and constructed into physical queues for HcControlHeadED and HcBulkHeadED

    // 5.2.7.2 - Periodic list (interrupt and isochronous)
    const uint32_t maxFrequencyIndex = 6;
    o->interrupt_baseEDs = malloc((sizeof(ohciED_t) + sizeof(ohciTD_t)) * maxFrequencyIndex, OHCI_DESCRIPTORS_ALIGN, "ohci_interruptEDs");
    memset(o->interrupt_baseEDs, 0, (sizeof(ohciED_t) + sizeof(ohciTD_t)) * maxFrequencyIndex);
    for (uint32_t i = 0; i < maxFrequencyIndex; i++)
    {
        o->interrupt_baseEDs[i].sKip = 1;
        o->interrupt_baseEDs[i].tdQueueHead = o->interrupt_baseEDs[i].tdQueueTail = paging_getPhysAddr(((ohciTD_t*)(o->interrupt_baseEDs + maxFrequencyIndex)) + i);
        list_construct(&o->interruptEDs[i]);
    }

    // Prepare frequencies between 2ms and 32ms
    uint32_t start = 0;
    uint32_t frequencyIndex = maxFrequencyIndex-1;
    for (uint32_t frequency = 32; frequency > 1; frequency /= 2)
    {
        for (uint32_t head = 0; head < 32/frequency; head++)
        {
            o->hcca->interrruptTable[start + head*frequency] = paging_getPhysAddr(o->interrupt_baseEDs + frequencyIndex);
            //printf("\nLink: %u -> %u ms", start + head*frequency, frequency);
        }
        start++;
        frequencyIndex--;
    }

    // Link all together (frequency of 1ms)
    for (uint32_t i = 0; i < 32; i++)
        if (!o->hcca->interrruptTable[i])
            o->hcca->interrruptTable[i] = paging_getPhysAddr(o->interrupt_baseEDs + 0);
    for (uint32_t i = 1; i < maxFrequencyIndex; i++)
        o->interrupt_baseEDs[i].nextED = paging_getPhysAddr(o->interrupt_baseEDs + 0);

    // Async list (bulk and control)
    // Pointers and indices to ED are part of ohci_t
    o->indexED_bulk = NUM_ED_BULK;
    o->indexED_control = NUM_ED_CONTROL;

    // ED pool: NUM_ED EDs
    o->pooledED = malloc(sizeof(ohciED_t)*NUM_ED, OHCI_DESCRIPTORS_ALIGN, "ohci_EDs");
    o->pooledTD = malloc(sizeof(ohciTD_t*)*NUM_ED, 0, "ohci_TDlist");
    memset(o->pooledED, 0, sizeof(ohciED_t)*NUM_ED);
    for (uint32_t i=0; i<NUM_ED; i++)
    {
        if ((i == NUM_ED_BULK - 1) || (i == NUM_ED - 1))
        {
            o->pooledED[i].nextED = 0; // no next ED, end of control or bulk list
        }
        else
        {
            o->pooledED[i].nextED = paging_getPhysAddr(&o->pooledED[i+1]);
        }
        o->pooledED[i].sKip = 1;
        o->pooledTD[i] = malloc(sizeof(ohciTD_t), OHCI_DESCRIPTORS_ALIGN, "ohciTD");
        memset(o->pooledTD[i], 0, sizeof(ohciTD_t));
        o->pooledTD[i]->nextTD = BIT(0);
        o->pooledED[i].tdQueueHead = o->pooledED[i].tdQueueTail = paging_getPhysAddr(o->pooledTD[i]); // Each ED has to point on a valid TD
    }
    o->OpRegs->HcControlHeadED = o->OpRegs->HcControlCurrentED = paging_getPhysAddr(&o->pooledED[NUM_ED_CONTROL]);
    o->OpRegs->HcBulkHeadED    = o->OpRegs->HcBulkCurrentED    = paging_getPhysAddr(&o->pooledED[NUM_ED_BULK]);

    // Set the HcHCCA to the physical address of the HCCA block
    o->OpRegs->HcHCCA = paging_getPhysAddr(o->hcca);

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nHCCA (phys. address): %X", o->OpRegs->HcHCCA);
  #endif

    // Set HcInterruptEnable to have all interrupt enabled except Start-of-Frame detect
    o->OpRegs->HcInterruptDisable = OHCI_INT_SF   | // start of frame
                                    OHCI_INT_MIE;   // deactivates interrupts
    o->OpRegs->HcInterruptStatus  = ~0U;
    o->OpRegs->HcInterruptEnable  = OHCI_INT_SO   | // scheduling overrun
                                    OHCI_INT_WDH  | // write back done head
                                    OHCI_INT_RD   | // resume detected
                                    OHCI_INT_UE   | // unrecoverable error
                                    OHCI_INT_FNO  | // frame number overflow
                                    OHCI_INT_RHSC | // root hub status change
                                    OHCI_INT_OC   | // ownership change
                                    OHCI_INT_MIE;   // activates interrupts

    o->OpRegs->HcControl |= (OHCI_CTRL_CLE | OHCI_CTRL_PLE | /*OHCI_CTRL_IE |*/ OHCI_CTRL_BLE);  // Set HcControl to have "all queues on" (5.1.1.4)

    o->OpRegs->HcControl |= OHCI_CTRL_RWE; // activate RemoteWakeup

    // Set HcPeriodicStart to a value that is 90% of the value in FrameInterval field of the HcFmInterval register
    // When HcFmRemaining reaches this value, periodic lists gets priority over control/bulk processing
    o->OpRegs->HcPeriodicStart = (o->OpRegs->HcFmInterval & 0x3FFF) * 90/100;

    // The HCD then begins to send SOF tokens on the USB by writing to the HcControl register with
    // the HostControllerFunctionalState set to USBOPERATIONAL and the appropriate enable bits set.
    // The Host Controller begins sending SOF tokens within one ms
    // (if the HCD needs to know when the SOFs it may unmask the StartOfFrame interrupt).
    printf("\nHC will be set to USB Operational.\n");
    o->OpRegs->HcControl = (o->OpRegs->HcControl & ~OHCI_CTRL_HCFS) | OHCI_USB_OPERATIONAL; // set HCFS bits

    // duration HCD has to wait before accessing a powered-on port of the Root Hub.
    // It is implementation-specific. Duration is calculated as POTPGT * 2 ms.
    uint16_t powerWait = max(20, 2 * BYTE4(o->OpRegs->HcRhDescriptorA));

    // Configure ports
    o->OpRegs->HcRhDescriptorA = (o->OpRegs->HcRhDescriptorA & ~(OHCI_RHA_DT | OHCI_RHA_NDP)) | OHCI_RHA_NPS; // No power switching
    o->OpRegs->HcRhDescriptorB = 0; // DR: devices removable; PPCM: PortPowerControlMask is set to global power switching mode

    o->OpRegs->HcRhStatus = OHCI_RHS_LPSC;           // SetGlobalPower: turn on power to all ports
    sleepMilliSeconds(powerWait);

    uint8_t rootPortCount = min(OHCIPORTMAX, BYTE1(o->OpRegs->HcRhDescriptorA)); // NumberDownstreamPorts
    textColor(IMPORTANT);
    printf("\nFound %u Rootports. Power wait: %u ms\n", rootPortCount, powerWait);
    textColor(TEXT);

    hc_constructRootPorts(&o->hc, rootPortCount, &USB_OHCI);
    for (uint8_t j = 0; j < o->hc.rootPortCount; j++)
    {
        if (o->OpRegs->HcRhPortStatus[j] & OHCI_PORT_CCS)
            ohci_analyzePortStatus(o, j);
    }
    o->enabledPortFlag = true;
}


/*******************************************************************************************************
*                                                                                                      *
*                                              PORTS                                                   *
*                                                                                                      *
*******************************************************************************************************/

static void ohci_attachSequence(ohci_t* o, uint8_t j)
{
    o->hc.ports.data[j]->connected = true;
    sleepMilliSeconds(100); // Wait 100ms until power is stable (USB 2.0, 9.1.2)
    ohci_resetPort(o, j);   // Reset on attached

    uint32_t val = o->OpRegs->HcRhPortStatus[j];
    if (val & OHCI_PORT_PES)
    {
      #ifdef _OHCI_DIAGNOSIS_
        textColor(SUCCESS);
        printf(" enabled  ");
      #endif
        if ((val & OHCI_PORT_PPS) && (val & OHCI_PORT_CCS)) // powered, device attached
        {
            hc_setupUSBDevice(&o->hc, j, (val & OHCI_PORT_LSDA)?USB_LOWSPEED:USB_FULLSPEED);
        }
    }
  #ifdef _OHCI_DIAGNOSIS_
    else
    {
        textColor(IMPORTANT);
        printf(" disabled ");
    }
    textColor(TEXT);

    if (val & OHCI_PORT_PSS)
        printf(" suspend   ");
    else
        printf(" not susp. ");
  #endif

    if (val & OHCI_PORT_POCI)
    {
        printfe(" overcurrent ");
    }

  #ifdef _OHCI_DIAGNOSIS_
    if (val & OHCI_PORT_PRS)
        printf(" reset ");

    if (val & OHCI_PORT_PPS)
        printf(" pow on  ");
    else
        printf(" pow off ");
  #endif

    if (val & OHCI_PORT_CSC)
    {
      #ifdef _OHCI_DIAGNOSIS_
        printf(" CSC -");
      #endif
        o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_CSC;
    }

    if (val & OHCI_PORT_PESC)
    {
        printf(" enable Change ");
        o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_PESC;
    }

    if (val & OHCI_PORT_PSSC)
    {
        printf(" resume compl. ");
        o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_PSSC;
    }

    if (val & OHCI_PORT_OCIC)
    {
        printf(" overcurrent Change ");
        o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_OCIC;
    }

    if (val & OHCI_PORT_PRSC)
    {
        printf(" Reset Complete ");
        o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_PRSC;
    }
    putch('\n');

    waitForKeyStroke();
}

static void ohci_analyzePortStatus(ohci_t* o, uint8_t j)
{
    uint32_t val = o->OpRegs->HcRhPortStatus[j];
    if ((val & OHCI_PORT_CCS) == o->hc.ports.data[j]->connected)
        return;

    textColor(HEADLINE);
    printf("\nport[%u]: ", j + 1);
    textColor(DATA);

    if (val & OHCI_PORT_LSDA)
    {
        printf("LowSpeed");
    }
    else
    {
        printf("FullSpeed");
    }
    textColor(TEXT);

    if (val & OHCI_PORT_CCS)
    {
        textColor(SUCCESS);
        printf(" device attached.");

        task_t* thread = create_cthread((void*)&ohci_attachSequence, "OHCI attached");
        task_passStackParameter(thread, &o, sizeof(o));
        task_passStackParameter(thread, &j, sizeof(j));
        scheduler_insertTask(thread);
    }
    else
    {
        printf(" device removed.");
        o->hc.ports.data[j]->connected = false;
        if (o->hc.ports.data[j]->device)
        {
            usb_destroyDevice(o->hc.ports.data[j]->device);
            o->hc.ports.data[j]->device = 0;
        }
    }

}

static void ohci_resetPort(ohci_t* o, uint8_t j)
{
    o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_PRS; // reset

    WAIT_FOR_CONDITION((o->OpRegs->HcRhPortStatus[j] & OHCI_PORT_PRS) == 0, 200, 10, "\nTimeout Error: ohci port reset bit still set to 1");

    o->OpRegs->HcRhPortStatus[j] = OHCI_PORT_PES; // enable
    sleepMilliSeconds(50);
}


/*******************************************************************************************************
*                                                                                                      *
*                                              ohci handler                                            *
*                                                                                                      *
*******************************************************************************************************/

static void ohci_handler(registers_t* r, pciDev_t* device)
{
    // Check if an OHCI controller issued this interrupt
    ohci_t* o = device->data;
    bool found = false;

    for (dlelement_t* el = ohci.head; el != 0; el = el->next)
    {
        if (el->data == o)
        {
            textColor(TEXT);
            found = true;
            break;
        }
    }

    if (!found || o == 0)
    {
      #ifdef _OHCI_DIAGNOSIS_
        printf("Interrupt did not came from OHCI device!\n");
      #endif
        return;
    }

    uint32_t val = o->OpRegs->HcInterruptStatus;

    if (val==0)
    {
      #ifdef _OHCI_DIAGNOSIS_
        printf("Interrupt came from another OHCI device!\n");
      #endif
        return;
    }

    if (val & OHCI_INT_WDH) // write back done head
    {
        //printf("Write back done head.");
    }

    o->OpRegs->HcInterruptStatus = val; // reset interrupts

    if (!((val & OHCI_INT_SF) || (val & OHCI_INT_RHSC)))
    {
        printf("\nUSB OHCI %u: ", o->num);
    }

    if (val & OHCI_INT_SO) // scheduling overrun
    {
        printf("Scheduling overrun.");
    }

    if (val & OHCI_INT_RD) // resume detected
    {
        printf("Resume detected.");
    }

    if (val & OHCI_INT_UE) // unrecoverable error
    {
        printf("Unrecoverable HC error.");
        o->OpRegs->HcCommandStatus |= OHCI_STATUS_RESET;
    }

    if (val & OHCI_INT_FNO) // frame number overflow
    {
        printf("Frame number overflow.");
    }

    if ((val & OHCI_INT_RHSC) && o->enabledPortFlag) // root hub status change
    {
        for (uint8_t j = 0; j < o->hc.rootPortCount; j++)
            if (o->OpRegs->HcRhPortStatus[j] & OHCI_PORT_CSC)
                ohci_analyzePortStatus(o, j);
    }

    if (val & OHCI_INT_OC) // ownership change
    {
        printf("Ownership change.");
    }
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Transactions                                              *
*                                                                                                      *
*******************************************************************************************************/

typedef struct
{
    ohciTD_t* TD;
    ohciTD_t* TD_copy;
    void*     TDBuffer;
    void*     inBuffer;
    size_t    inLength;
} ohci_transaction_t;


void ohci_setupTransfer(usb_transfer_t* transfer)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;

    if (transfer->type == USB_BULK || transfer->type == USB_CONTROL)
    {
        // recycle EDs
        ohci_checkResetMempool(o);

        // endpoint descriptor
        uint16_t indexED = 0xFFFF;
        if (transfer->type == USB_CONTROL)
            indexED = o->indexED_control;
        else if (transfer->type == USB_BULK)
            indexED = o->indexED_bulk;
        transfer->data = (void*)(size_t)indexED;

      #ifdef _OHCI_DIAGNOSIS_
        printf("\nsetupTransfer: indexED: %u", indexED);
      #endif
    }
    else if (transfer->type == USB_INTERRUPT)
    {
        ohciED_t* ed = transfer->data = malloc(sizeof(ohciED_t), OHCI_DESCRIPTORS_ALIGN, "ohci_interruptED");
        memset(ed, 0, sizeof(ohciED_t));
    }
}

void ohci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
    ohci_transaction_t* oTransaction = uTransaction->data = malloc(sizeof(ohci_transaction_t), 0, "ohci_transaction_t");
    oTransaction->inBuffer = 0;
    oTransaction->inLength = 0;

    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;

    size_t indexED = (size_t)transfer->data;
    oTransaction->TD = o->pooledTD[indexED];
    oTransaction->TD_copy = 0;
    o->pooledTD[indexED] = ohci_createTD_SETUP(o, o->pooledED + indexED, oTransaction->TD, toggle, type, req, hiVal, loVal, index, length, &oTransaction->TDBuffer);

  #ifdef _OHCI_DIAGNOSIS_
    usb_request_t* request = (usb_request_t*)oTransaction->TDBuffer;
    printf("\ntype: %u req: %u valHi: %u valLo: %u i: %u len: %u", request->type, request->request, request->valueHi, request->valueLo, request->index, request->length);
  #endif
}

void ohci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, void* buffer, size_t length)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    ohci_transaction_t* oTransaction = uTransaction->data = malloc(sizeof(ohci_transaction_t), 0, "ohci_transaction_t");
    oTransaction->inBuffer = buffer;
    oTransaction->inLength = length;
    oTransaction->TDBuffer = length ? malloc(length, 16 | HEAP_CONTINUOUS, "OHCI TDBuffer") : 0;

    if (transfer->type == USB_BULK || transfer->type == USB_CONTROL)
    {
        size_t indexED = (size_t)transfer->data;
        oTransaction->TD = o->pooledTD[indexED];
        oTransaction->TD_copy = 0;
        o->pooledTD[indexED] = ohci_createTD_IO(o, o->pooledED + indexED, oTransaction->TD, OHCI_TD_IN, toggle, length, oTransaction->TDBuffer);
    }
    else if (transfer->type == USB_INTERRUPT)
    {
        oTransaction->TD_copy = malloc(2 * sizeof(ohciTD_t), 0, "ohciTD_t: original");                  // Allocate TD for usage and dummy TD to be copied into TD_copy
        oTransaction->TD      = malloc(2 * sizeof(ohciTD_t), OHCI_DESCRIPTORS_ALIGN, "ohciTD_t: copy"); // Allocate TD for usage and dummy TD as copies to be accessed by the OHCI
        ohci_initTD_IO(oTransaction->TD_copy, OHCI_TD_IN, toggle, length, oTransaction->TDBuffer);
        oTransaction->TD_copy->nextTD = paging_getPhysAddr(oTransaction->TD + 1);
        (oTransaction->TD_copy + 1)->nextTD = BIT(0);
    }
}

void ohci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, const void* buffer, size_t length)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    ohci_transaction_t* oTransaction = uTransaction->data = malloc(sizeof(ohci_transaction_t), 0, "ohci_transaction_t");
    oTransaction->inBuffer = 0;
    oTransaction->inLength = 0;
    oTransaction->TDBuffer = length ? malloc(length, 16 | HEAP_CONTINUOUS, "OHCI TDBuffer") : 0;

    if (transfer->type == USB_BULK || transfer->type == USB_CONTROL)
    {
        size_t indexED = (size_t)transfer->data;
        oTransaction->TD = o->pooledTD[indexED];
        oTransaction->TD_copy = 0;
        o->pooledTD[indexED] = ohci_createTD_IO(o, o->pooledED + indexED, oTransaction->TD, OHCI_TD_OUT, toggle, length, oTransaction->TDBuffer);
    }
    else if (transfer->type == USB_INTERRUPT)
    {
        oTransaction->TD_copy = malloc(2 * sizeof(ohciTD_t), 0, "ohciTD_t: original");                  // Allocate TD for usage and dummy TD to be copied into TD
        oTransaction->TD      = malloc(2 * sizeof(ohciTD_t), OHCI_DESCRIPTORS_ALIGN, "ohciTD_t: copy"); // Allocate TD for usage and dummy TD to be accessed by the OHCI
        ohci_initTD_IO(oTransaction->TD_copy, OHCI_TD_OUT, toggle, length, oTransaction->TDBuffer);
        oTransaction->TD_copy->nextTD = paging_getPhysAddr(oTransaction->TD + 1);
        (oTransaction->TD_copy + 1)->nextTD = BIT(0);
    }

    if (buffer != 0 && length != 0)
    {
        memcpy(oTransaction->TDBuffer, buffer, length);
    }
}

void ohci_scheduleTransfer(usb_transfer_t* transfer)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    ohci_transaction_t* firstTransaction = ((usb_transaction_t*)transfer->transactions.head->data)->data;

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nohci_createED: devNum = %u endp = %u packetsize = %u", transfer->device->num, transfer->endpoint, transfer->packetSize);
  #endif

    if (transfer->type == USB_CONTROL || transfer->type == USB_BULK)
    {
        size_t indexED = (size_t)transfer->data;
        ohciED_t* ed = o->pooledED + indexED;
        ohci_initED(transfer->device->port->data, ed, firstTransaction->TD, o->pooledTD[indexED], transfer->device->num, transfer->endpoint);

      #ifdef _OHCI_DIAGNOSIS_
        textColor(MAGENTA);
        printf("\nHcControlCurrentED: %X", o->OpRegs->HcControlCurrentED);
        printf(" ED->skip = %u ED->Halted = %u", ed->sKip, ed->tdQueueHead & BIT(0));
        printf("\nHeadP = %X TailP = %X", ed->tdQueueHead, ed->tdQueueTail);
        textColor(MAGENTA);
        printf("\nHcCommandStatus: %X", o->OpRegs->HcCommandStatus);
        textColor(TEXT);
      #endif

        transfer->success = true;

        ed->sKip = 0;
        ed->tdQueueHead &= ~0x1U; // reset Halted Bit

        if (transfer->type == USB_CONTROL)
            o->OpRegs->HcCommandStatus |= OHCI_STATUS_CLF; // control list filled
        else if (transfer->type == USB_BULK)
            o->OpRegs->HcCommandStatus |= OHCI_STATUS_BLF; // bulk list filled
    }
    else if (transfer->type == USB_INTERRUPT)
    {
        ohciED_t* ed = transfer->data;
        memcpy(firstTransaction->TD, firstTransaction->TD_copy, sizeof(ohciTD_t) * 2);
        ohci_initED(transfer->device->port->data, ed, firstTransaction->TD, firstTransaction->TD + 1, transfer->device->num, transfer->endpoint);

        // Calculate frequency index wit bsr intrinsic
        uint8_t power = bsr(transfer->frequency);
        if (BIT(power) != transfer->frequency)
            power++;
        size_t index = min(5, power);

        // Insert new ED at beginning
        ed->nextED = o->interrupt_baseEDs[index].nextED;
        o->interrupt_baseEDs[index].nextED = paging_getPhysAddr(ed);
        list_insert(&o->interruptEDs[index], o->interruptEDs[index].head, ed);
    }

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nNumber of TD elements: %u", list_getCount(&transfer->transactions));
  #endif
}

bool ohci_pollTransfer(usb_transfer_t* transfer)
{
    // check completion
    transfer->success = true;
    bool completed = true;
    for (dlelement_t* elem = transfer->transactions.head; elem; elem = elem->next)
    {
        ohci_transaction_t* transaction = ((usb_transaction_t*)elem->data)->data;
        ohci_showStatusbyteTD(transaction->TD);

        transfer->success = transfer->success && (transaction->TD->cond == 0);
        completed = completed && (transaction->TD->cond != 15); // 15 means: not accessed

        if (completed && transaction->inBuffer != 0 && transaction->inLength != 0)
        {
            memcpy(transaction->inBuffer, transaction->TDBuffer, transaction->inLength);
        }
    }
    if (!completed)
        return false;

    // Schedule it again
    ohciED_t* ed = transfer->data;
    ohci_transaction_t* firstTransaction = ((usb_transaction_t*)transfer->transactions.head->data)->data;
    memcpy(firstTransaction->TD, firstTransaction->TD_copy, sizeof(ohciTD_t) * 2);
    ohci_initED(transfer->device->port->data, ed, firstTransaction->TD, firstTransaction->TD + 1, transfer->device->num, transfer->endpoint);

    return transfer->success;
}

void ohci_waitForTransfer(usb_transfer_t* transfer)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;

    // A transfer is completed when the Host Controller successfully transfers, to or from an endpoint, the byte pointed to by BufferEnd.
    // Upon successful completion, the Host Controller sets CurrentBufferPointer to zero, sets ConditionCode to NOERROR,
    // and retires the General TD to the Done Queue.

    // wait for completion
    uint32_t timeout = 300; // Wait up to 1,5 seconds
    dlelement_t* dlE = transfer->transactions.head;
    while (timeout > 0)
    {
        ohci_transaction_t* oT = ((usb_transaction_t*)dlE->data)->data;

        while (oT->TD->cond != 15) // 15 means: not accessed
        {
            dlE = dlE->next;
            if (dlE == 0)
                break;
            oT = ((usb_transaction_t*)dlE->data)->data;
        }
        if (dlE == 0)
            break;
        sleepMilliSeconds(5);
        timeout--;
    }
    if (timeout == 0)
    {
        printfe("\nOHCI: Timeout!");
    }

    // check conditions
    for (dlelement_t* elem = transfer->transactions.head; elem; elem = elem->next)
    {
        ohci_transaction_t* transaction = ((usb_transaction_t*)elem->data)->data;
        ohci_showStatusbyteTD(transaction->TD);

        transfer->success = transfer->success && (transaction->TD->cond == 0);
    }

  #ifdef _OHCI_DIAGNOSIS_
    textColor(IMPORTANT);
    printf("\n\nED-Index (control): %u, ED-Index (bulk): %u, Transfer->endpoint: %u, &o: %X", o->indexED_control, o->indexED_bulk, transfer->endpoint, o);
    textColor(TEXT);
  #endif

    if (transfer->type == USB_CONTROL || transfer->type == USB_BULK)
    {
        size_t indexED = (size_t)transfer->data;
        ohciED_t* ed = o->pooledED + indexED;
        ed->sKip = 1;
        ed->tdQueueHead = ed->tdQueueTail = paging_getPhysAddr(o->pooledTD[indexED]);
    }

    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        ohci_transaction_t* transaction = ((usb_transaction_t*)elem->data)->data;

        if (transaction->inBuffer != 0 && transaction->inLength != 0)
        {
            memcpy(transaction->inBuffer, transaction->TDBuffer, transaction->inLength);
        }
    }
}

void ohci_destructTransfer(usb_transfer_t* transfer)
{
    ohci_t* o = (ohci_t*)((hc_port_t*)transfer->device->port->data)->hc;

    if (transfer->type == USB_INTERRUPT)
    {
        // Calculate frequency index wit bsr intrinsic
        uint8_t power = bsr(transfer->frequency);
        if (BIT(power) != transfer->frequency)
            power++;
        size_t index = min(5, power);
        // Take ED out
        dlelement_t* elem = list_find(&o->interruptEDs[index], transfer->data);
        if (elem->prev)
            ((ohciED_t*)elem->prev->data)->nextED = ((ohciED_t*)elem->data)->nextED;
        else
            o->interrupt_baseEDs[index].nextED = ((ohciED_t*)elem->data)->nextED;
        list_delete(&o->interruptEDs[index], elem);
    }

    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        ohci_transaction_t* transaction = ((usb_transaction_t*)elem->data)->data;
        free(transaction->TD);
        free(transaction->TD_copy);
        free(transaction->TDBuffer);
        free(transaction);
    }
    if (transfer->type == USB_INTERRUPT)
        free(transfer->data);

    if (transfer->success)
    {
      #ifdef _OHCI_DIAGNOSIS_
        textColor(SUCCESS);
        printf("\nTransfer successful.");
        textColor(TEXT);
      #endif
    }
    else
    {
        printfe("\nTransfer failed.");
    }

    if (transfer->type == USB_BULK)
        o->indexED_bulk++;
    else if (transfer->type == USB_CONTROL)
        o->indexED_control++;
}


/*******************************************************************************************************
*                                                                                                      *
*                                            ohci ED TD functions                                      *
*                                                                                                      *
*******************************************************************************************************/

static ohciTD_t* ohci_appendTDtoED(ohci_t* o, ohciED_t* oED, ohciTD_t* oTD_at)
{
    // Create next TD (see 5.2.8.2)
    ohciTD_t* next = malloc(sizeof(ohciTD_t), OHCI_DESCRIPTORS_ALIGN, "ohciTD_t");
    memset(next, 0, sizeof(ohciTD_t));
    next->nextTD = BIT(0);

    if(oTD_at)
        oTD_at->nextTD = paging_getPhysAddr(next);

    return next;
}

static ohciTD_t* ohci_createTD_SETUP(ohci_t* o, ohciED_t* oED, ohciTD_t* oTD, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length, void** buffer)
{
    ohciTD_t* next = ohci_appendTDtoED(o, oED, oTD);

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nohci_createTD_SETUP: ED (control) = %u  toggle: %u", o->indexED_control, oTD->toggle);
  #endif

    oTD->direction    = OHCI_TD_SETUP;
    oTD->toggle       = toggle;
    oTD->toggleFromTD = 1;
    oTD->cond         = OHCI_TD_NOCC; // to be executed
    oTD->delayInt     = OHCI_TD_NOINT;
    oTD->errCnt       = 0;
    oTD->bufRounding  = 1;

    usb_request_t* request = *buffer = malloc(sizeof(usb_request_t), 16 | HEAP_CONTINUOUS, "OHCI usb request");
    request->type     = type;
    request->request  = req;
    request->valueHi  = hiVal;
    request->valueLo  = loVal;
    request->index    = i;
    request->length   = length;

    oTD->curBuffPtr   = paging_getPhysAddr(request);
    oTD->buffEnd      = oTD->curBuffPtr + sizeof(usb_request_t) - 1; // physical address of the last byte in the buffer

    oED->tdQueueTail = paging_getPhysAddr(oTD);

    return next;
}

static void ohci_initTD_IO(ohciTD_t* oTD, uint8_t direction, bool toggle, uint16_t tokenBytes, void* buffer)
{
    oTD->direction = direction;
    oTD->toggle = toggle;
    oTD->toggleFromTD = 1;
    oTD->cond = OHCI_TD_NOCC; // to be executed
    oTD->delayInt = OHCI_TD_NOINT;
    oTD->errCnt = 0;
    oTD->bufRounding = 1;

    oTD->curBuffPtr = paging_getPhysAddr(buffer);
    if (tokenBytes)
    {
        oTD->buffEnd = oTD->curBuffPtr + tokenBytes - 1; // BufferEnd contains physical address of the last byte in the buffer
    }
    else
    {
        oTD->buffEnd = oTD->curBuffPtr;
    }
}

static ohciTD_t* ohci_createTD_IO(ohci_t* o, ohciED_t* oED, ohciTD_t* oTD, uint8_t direction, bool toggle, uint16_t tokenBytes, void* buffer)
{
    ohciTD_t* next = ohci_appendTDtoED(o, oED, oTD);
    if (!oTD)
        oTD = next;

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nohci_createTD_IO: ED (control) = %u, ED (bulk) = %u, toggle: %u", o->indexED_control, o->indexED_bulk, oTD->toggle);
  #endif

    ohci_initTD_IO(oTD, direction, toggle, tokenBytes, buffer);

    oED->tdQueueTail = paging_getPhysAddr(oTD);

    return next;
}

static void ohci_initED(hc_port_t* port, ohciED_t* oED, ohciTD_t* head, ohciTD_t* tail, uint32_t device, usb_endpoint_t* endpoint)
{
  #ifdef _OHCI_DIAGNOSIS_
    printf("\nnext ED: %X", ed->nextED);
  #endif

    oED->endpNum = endpoint->address;

  #ifdef _OHCI_DIAGNOSIS_
    printf("\nendpoint: %u", oED->endpNum);
  #endif

    oED->devAddr = device;

  #ifdef _OHCI_DIAGNOSIS_
    printf("  device: %u", oED->devAddr);
  #endif

    oED->mps     = endpoint->mps;
    oED->dir     = OHCI_ED_TD ;     // 00b Get direction From TD
    oED->speed   = (port->device->speed==USB_LOWSPEED)?1:0;  // speed of the endpoint: full-speed (0), low-speed (1)
    oED->format  = 0;               // format of the TDs: Control, Bulk, or Interrupt Endpoint (0); Isochronous Endpoint (1)

    oED->tdQueueHead = paging_getPhysAddr(head); // head TD in queue
    oED->tdQueueTail = paging_getPhysAddr(tail); // tail TD in queue
  #ifdef _OHCI_DIAGNOSIS_
    printf("\nohci_createED: %X tdQueueHead = %X tdQueueTail = %X", paging_getPhysAddr(oED), ed->tdQueueHead, oED->tdQueueTail); // Tail is read-only
  #endif

    oED->sKip = 0;  // 1: HC continues on to next ED w/o attempting access to the TD queue or issuing any USB token for the endpoint
}


////////////////////
// analysis tools //
////////////////////

static uint8_t ohci_showStatusbyteTD(ohciTD_t* TD)
{
    static const char* texts[] =
    {
        0, // "Successful Completion."
        "Last data packet from EP contained a CRC error.",
        "Last data packet from EP contained a bit stuffing violation.",
        "Toggle PID from EP did not match the expected value.",
        "TD was moved to the Done Queue because the EP returned a STALL PID.",
        "Device: no response to token (IN) or no handshake (OUT)",
        "Check bits on PID from EP failed on data PID (IN) or handshake (OUT).",
        "Receive PID was not valid when encountered or PID value is not defined.",
        "DATAOVERRUN: Too many data returned by the EP.",
        0,
        0,
        0,
        "DATAUNDERRUN: EP returned less than MPS.",
        "BUFFERUNDERRUN",
        0, // "Not accessed."
        0  // "Not accessed."
    };
    if (TD->cond < 16 && texts[TD->cond])
        printfe("\nOHCI: %s", texts[TD->cond]);
    return TD->cond;
}

static void ohci_checkResetMempool(ohci_t* o)
{
    //if (o->indexED_control >= NUM_ED_BULK)
    {
        o->indexED_control = NUM_ED_CONTROL;
    }
    //if (o->indexED_bulk >= NUM_ED)
    {
        o->indexED_bulk = NUM_ED_BULK;
    }
}

static void ohci_toggleFrameInterval(ohci_t* o)
{
    o->OpRegs->HcFmInterval ^= BIT(31); // toggle FRT
}


/*
* Copyright (c) 2011-2017 The PrettyOS Project. All rights reserved.
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
