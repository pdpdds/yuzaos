/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/
#include <stdio.h>
#include "xhci.h"
#include <memory.h>
#include <string.h>
#include <systemcall_impl.h>
#include <InterruptHandler.h>
#include <math.h>
#include "XHCIHandler.h"

#define PAGE_SIZE 4096

int g_j = 0;
#define WAIT_FOR_CONDITION(condition, runs, wait, message)\
    for (unsigned int timeout_ = 0; !(condition); timeout_++) {\
        if (timeout_ >= runs) {\
             printf(message);\
             break;\
        }\
        Syscall_Sleep(wait);\
    }

static list_t xhci = list_init();

static void xhci_handler(pciDev_t* device);
static void xhci_analyze(xhci_t* x);
static void xhci_start(xhci_t* x);
static void xhci_initHC(xhci_t* x);
static void xhci_resetHC(xhci_t* x);
static void xhci_startHC(xhci_t* x);
static void xhci_prepareSlotsForControlTransfers(xhci_t* x);
static void xhci_resetSlot(xhci_t* x, uint8_t slotNr);
static void xhci_portCheck(xhci_t* x);
static void xhci_detectDevice(xhci_t* x);
//static void xhci_showPortStatus(xhci_t* x, uint8_t j);
//static void xhci_showSlotStatus(xhci_t* x, uint8_t firstPort, uint8_t lastPort);
static void xhci_prepareEventRing(xhci_t* x);
static void xhci_showStatus(xhci_t* x);
static void xhci_deactivateLegacySupport(xhci_t* x);
static void xhci_parseEvents(xhci_t* x);
static void xhci_showCompletionCodeError(uint8_t CompletionCode);
#ifdef _XHCI_DIAGNOSIS_
static void xhci_showTRBType(uint8_t trbType);
#endif
static void xhci_ringDoorbellAndWait_HC(xhci_t* x);
static void xhci_ringDoorbell_Device(xhci_t* x, uint8_t slotNr, uint8_t target, uint16_t streamID);
static void xhci_showStates(xhci_t* x, uint8_t slotNr, uint8_t portNr);
static void showCounter(xhci_t* x, uint8_t slotNr);
static xhci_LinkTRB_t* xhci_Enable_Slot_Command(xhci_t* x);
static void xhci_Disable_Slot_Command(xhci_t* x, uint8_t slotNr);
static void xhci_Address_Device_Command(xhci_t* x, uint8_t slotNr, bool BSR);
static void xhci_Configure_Endpoint_Command(xhci_t* x, uint8_t slotNr, bool DC);
static void xhci_Evaluate_Context_Command(xhci_t* x, uint8_t slotNr);
static void xhci_Reset_Device_Command(xhci_t* x, uint8_t slotNr);
static void xhci_Reset_Endpoint_Command(xhci_t* x, uint8_t slotNr, uint8_t DCI, bool TransferStatePreserve);
static void xhci_Set_TR_DeqPointer_Command(xhci_t* x, uint8_t slotNr, uint8_t DCI, uintptr_t NewTRDeqPtrLo, uint16_t StreamID);
static uint8_t calculateDCI(usb_endpoint_t* endpoint);


void xhci_install(pciDev_t* PCIdev)
{
    dlelement_t* elem = list_alloc_elem(sizeof(xhci_t), "xhci");
    xhci_t* x             = (xhci_t *)elem->data;
    x->PCIdevice          = PCIdev;
    x->PCIdevice->data    = x;
    x->CmdPending         = 0;

    // Get MMIO space
    x->bar = 0;
    for (uint8_t i = 0; i < 6 && !x->bar; i++)
    {
        x->bar = pci_aquireMemoryForMMIO(PCIdev->bar + i);
    }

    x->CapRegs     = (xhci_CapRegs_t*)x->bar;
    x->OpRegs      = (xhci_OpRegs_t*)((char*)x->bar + x->CapRegs->caplength);

  #ifdef _XHCI_DIAGNOSIS_
    uint8_t maxEventRingSegmentTableEntries = BYTE1(x->CapRegs->hcsparams2)>>4;
    printf("\nmaxEventRingSegmentTableEntries: 2 ^ %u", maxEventRingSegmentTableEntries);
    printf("\nruntime base offset: %Xh", x->CapRegs->rtsoff);
  #endif

    x->RuntimeRegs = (xhci_RuntimeRegs_t*)((char*)x->bar + x->CapRegs->rtsoff);

  #ifdef _XHCI_DIAGNOSIS_
    printf("\tdoorbell offset:     %Xh", x->CapRegs->dboff);
  #endif

    x->DoorbellRegs = (xhci_DoorbellRegs_t*)((char*)x->bar + x->CapRegs->dboff);

    hc_constructRootPorts(&x->hc, BYTE4(x->CapRegs->hcsparams1), &USB_XHCI); // Number of Ports 24:31
    printf("\nx->hc.rootPortCount: %u", x->hc.rootPortCount);

    list_append_elem(&xhci, elem);

    xhci_analyze(x); // print data (capregs, opregs, runtime base)

    static uint8_t numPorts = 0;
    char str[10];
    snprintf(str, 10, "xHCI %u", ++numPorts);
	
	Syscall_CreateThread(xhci_start, str, x, 16, 0);
    //task_t* thread = create_cthread((void*)&xhci_start, str);
    //task_passStackParameter(thread, &x, sizeof(x));
    //scheduler_insertTask(thread);
}

static void xhci_start(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("Start xHC:");
	//textColor(TEXT);
  #endif

    for (uint8_t j=0; j<MAX_HC_PORTS; j++)
        x->portsEnabled[j] = false;
    x->CmdCounter = 0;
    x->EvtCounter = 0;

    xhci_initHC(x);
    xhci_resetHC(x);
	
    xhci_startHC(x);
	
    if (!(x->OpRegs->status & STS_HCH)) // HC not Halted
    {
        xhci_prepareSlotsForControlTransfers(x);
    }
    else // HC Halted
    {
        printf("\nFatal Error: HCHalted set. Ports cannot be enabled.");
        xhci_showStatus(x);
    }

	//textColor(LIGHT_MAGENTA);
    printf("\n\n>>> Press key to close this console. <<<");
	Syscall_GetChar();
}

static void xhci_initHC(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\ninitHC");
	//textColor(TEXT);
  #endif

    // prepare PCI command register
    uint16_t pciCommandRegister = pci_configRead(x->PCIdevice, PCI_COMMAND, 2);
    pci_configWrite_word(x->PCIdevice, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER); // resets status register, sets command register
}

static void xhci_resetHC(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nresetHC");
	//textColor(TEXT);
  #endif

    x->OpRegs->command &= ~CMD_RUN;  // clear Run-Stop-Bit

    WAIT_FOR_CONDITION(x->OpRegs->status & STS_HCH, 30, 10, "Timeout Error: HC Halted-Bit still cleared.\n");

    x->OpRegs->command |= CMD_RESET; // set Reset-Bit

    WAIT_FOR_CONDITION((x->OpRegs->command & CMD_RESET) == 0, 30, 10, "Timeout Error: xHC Reset-Bit still set.\n");
}

static void xhci_configureInterrupts(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nconfigureInterrupts");
	//textColor(TEXT);
  #endif

    // MSI (We do not support MSI-X)
    x->msiCapEnabled = pci_trySetMSIVector(x->PCIdevice, (IRQ_NUM_t)APICIRQ);

    if (x->msiCapEnabled)
        x->irq = APICIRQ;
    else
        x->irq = x->PCIdevice->irq;

    //irq_installPCIHandler(x->irq, xhci_handler, x->PCIdevice);
	InterruptHandler* pHandler = new XHCIHandler(x->PCIdevice->irq, xhci_handler, x->PCIdevice, "ehci");
	Syscall_kObserveInterrupt(x->PCIdevice->irq, pHandler);
}

static void xhci_startHC(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nstartHC");
	//textColor(TEXT);
  #endif

    xhci_deactivateLegacySupport(x); // 7.1

    x->OpRegs->devnotifctrl = 0x2;   // device notification control register (5.4.4)
                                     // only for debugging a value other than 0x2 (cf. note below Table 35)

    // Program the Max Device Slots Enabled (MaxSlotsEn) field in the CONFIG register (5.4.7) to enable the device slots that system software is going to use.
    x->OpRegs->config |= MAX_HC_SLOTS; // table 34

    // Program the Device Context Base Address Array Pointer (DCBAAP) register (5.4.6) with a 64-bit address pointing to where the Device Context Base Address Array is located.
    x->virt_deviceContextPointerArrayBase = (xhci_DeviceContextArray_t *)malloc(sizeof(xhci_DeviceContextArray_t)); // Heap is contiguous below 4K. Alignment see Table 54
    x->OpRegs->dcbaap = (uint64_t)Syscall_GetPAFromVM(x->virt_deviceContextPointerArrayBase);

    // 6.1 - Device Context Base Address Array

    uint8_t MaxScratchpadBuffers = ((x->CapRegs->hcsparams2 >> 27) & 0x1F) | ((x->CapRegs->hcsparams2 >> 16) & 0xE0);
    if (MaxScratchpadBuffers > 0) // Max Scratchpad Buffers
    {
        printf("\nscratchpad buffer created! Max Scratchpad Buffers = %u", MaxScratchpadBuffers);
        uint64_t* ScratchpadBuffersPtr = (uint64_t * )malloc(sizeof(uint64_t)*MaxScratchpadBuffers);
        for (uint8_t i=0; i<MaxScratchpadBuffers; i++)
        {
            ScratchpadBuffersPtr[i] = Syscall_GetPAFromVM(malloc(PAGE_SIZE));
        }
        x->virt_deviceContextPointerArrayBase->scratchpadBufferArrBase = (uint64_t)Syscall_GetPAFromVM(ScratchpadBuffersPtr); // Ptr to scratchpad buffer array
    }
    else // Max Scratchpad Buffers = 0
    {
        x->virt_deviceContextPointerArrayBase->scratchpadBufferArrBase = 0;
    }

    // Device Contexts
    for (uint16_t i=0; i<MAX_HC_SLOTS; i++)
    {
        x->devContextPtr[i] = (xhci_DeviceContext_t*)malloc(sizeof(xhci_DeviceContext_t)); // Alignment see Table 54
        memset(x->devContextPtr[i], 0, sizeof(xhci_DeviceContext_t));
        x->virt_deviceContextPointerArrayBase->devContextPtr[i] = (uintptr_t)Syscall_GetPAFromVM(x->devContextPtr[i]);
    }

    // Input Device Contexts
    for (uint16_t i=0; i<MAX_HC_SLOTS; i++)
    {
        x->devInputContextPtr[i] = (xhci_InputContext_t*)malloc(sizeof(xhci_InputContext_t)); // Alignment see Table 54
        memset(x->devInputContextPtr[i], 0, sizeof(xhci_InputContext_t));
    }

    // Transfer Rings
    for (uint16_t slotNr = 0; slotNr < MAX_HC_SLOTS; slotNr++)
    {
        x->slots[slotNr] = (xhci_slot_t*)malloc(sizeof(xhci_slot_t));

        for (uint16_t i=0; i<NUM_ENDPOINTS; i++)
        {
            xhci_xfer_NormalTRB_t* trb = (xhci_xfer_NormalTRB_t*)malloc(256*sizeof(xhci_xfer_NormalTRB_t)); // Alignment see Table 54. Use PAGESIZE to ensure that the memory is within one page.
            memset(trb, 0, 256 * sizeof(xhci_xfer_NormalTRB_t));
            x->trb[slotNr][i] = trb;

            // Software uses and maintains private copies of the Enqueue and Dequeue Pointers for each Transfer Ring.
            x->slots[slotNr]->endpoints[i].virtEnqTransferRingPtr = x->slots[slotNr]->endpoints[i].virtDeqTransferRingPtr = trb;
            x->slots[slotNr]->endpoints[i].TransferRingProducerCycleState = true; // PCS
            x->slots[slotNr]->endpoints[i].TransferCounter = 0; // Reset Transfer Counter
            x->slots[slotNr]->endpoints[i].timeEvent = 0;
            x->slots[slotNr]->endpoints[i].TransferRingbase = trb;

            // LinkTRB
            xhci_LinkTRB_t* linkTrb = (xhci_LinkTRB_t*)(trb + 255);
            linkTrb->RingSegmentPtrLo = Syscall_GetPAFromVM(trb); // segment pointer
            linkTrb->IntTarget = 0;                              // intr target
            linkTrb->TC = 1;                                     // Toggle Cycle !! 4.11.5.1 Link TRB
            linkTrb->TRBtype = TRB_TYPE_LINK;                    // ID of link TRB, cf. table 131
        }
    }

	
    // Command Ring
    // Define the Command Ring Dequeue Pointer by programming the Command Ring Control Register (5.4.5) with a 64-bit address pointing to the starting address of the first TRB of the Command Ring.
    x->CmdRingbase = (xhci_LinkTRB_t*)malloc(256*sizeof(xhci_LinkTRB_t)); // Alignment see Table 54
    x->virtEnqCmdRingPtr = x->CmdRingbase;
    memset(x->CmdRingbase, 0, 256*sizeof(xhci_LinkTRB_t));

    // LinkTRB
    x->CmdRingbase[255].RingSegmentPtrLo = Syscall_GetPAFromVM(x->CmdRingbase); // segment pointer
    x->CmdRingbase[255].TC = 1;                                                // Toggle Cycle
    x->CmdRingbase[255].TRBtype = TRB_TYPE_LINK;                               // ID of link TRB, cf. table 131

    // Command Ring Control Register (5.4.5), "crcr"
    x->CmdRingProducerCycleState = true; // PCS
    x->OpRegs->crcr = Syscall_GetPAFromVM(x->CmdRingbase) | (uintptr_t)x->CmdRingProducerCycleState; // command ring control register, Table 32, 5.4.5, CCS is Bit 0. Bit5:4 are RsvdP, but cannot be read.

    xhci_configureInterrupts(x);
    xhci_prepareEventRing(x);
	
    // Write the USBCMD (5.4.1) to turn the host controller ON via setting the Run/Stop (R/S) bit to ‘1’. This operation allows the xHC to begin accepting doorbell references.
    x->OpRegs->command |= CMD_RUN; // set Run/Stop bit
    Syscall_Sleep(100); //  IMPORTANT

    xhci_showStatus(x);
}


/*******************************************************************************************************
*                                                                                                      *
*                                              PORT CHANGE                                             *
*                                                                                                      *
*******************************************************************************************************/

static void xhci_detectDevice(xhci_t* x)
{
	uint8_t j = g_j;
    //console_setProperties(CONSOLE_SHOWINFOBAR | CONSOLE_AUTOSCROLL | CONSOLE_AUTOREFRESH); // protect console against info area
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\ndetectDevice");
	//textColor(TEXT);
  #endif

    uint32_t timeout=80;
    while (!x->portsEnabled[j]) // wait
    {
        timeout--;
        if (timeout>0)
        {
			//Syscall_Sleep(12); // do not use delay here!
        }
        else
        {
            /*
            //textColor(IMPORTANT);
            printf("\nTimeout Error at detectDevice! portsEnabled[%u] = false. Enable needed.", j+1);
            //textColor(TEXT);
            */
            break;
        }
    }

    if(x->portSlotLink[j].slotNr==0xFF || x->devContextPtr[x->portSlotLink[j].slotNr-1]->slotContext.slotState == 0) // TODO: check, if correct
    {
        if(x->portsEnabled[j] == false)
        {
          #ifdef _XHCI_DIAGNOSIS_
			//textColor(IMPORTANT);
            printf("\nxhci Enable Slot");
			//textColor(TEXT);
          #endif

            x->portSlotLink[j].cmdPtr = xhci_Enable_Slot_Command(x); // port hopefully gets new device slot
            xhci_ringDoorbellAndWait_HC(x);
            if (x->portSlotLink[j].slotNr == 0xFF)
                return;

            xhci_resetSlot(x, x->portSlotLink[j].slotNr);

            x->portsEnabled[j] = true;
        }
      #ifdef _XHCI_DIAGNOSIS_
        else
        {
			//textColor(IMPORTANT);
            printf("\nline: %u New \"Enable Slot\" not needed.", __LINE__);
			//textColor(TEXT);
        }
      #endif
    }
    else
    {
		//textColor(IMPORTANT);
        printf("\nline: %u No xhci_Enable_Slot_Command, no xhci_Reset_Slot!\nslotNr: %u slotState: %u", __LINE__,
                   x->portSlotLink[j].slotNr, x->devContextPtr[x->portSlotLink[j].slotNr-1]->slotContext.slotState);
		//textColor(TEXT);
    }

    x->OpRegs->PortReg[j].portsc |= PORT_PR; // inits port reset sequence

    WAIT_FOR_CONDITION(!(x->OpRegs->PortReg[j].portsc & PORT_PR), 20, 10, "\nTimeout Error - detectDevice! Waiting for end of Port Reset");

	
    if (x->devContextPtr[x->portSlotLink[j].slotNr-1]->slotContext.slotState > 1)
    {
        xhci_Reset_Device_Command(x, x->portSlotLink[j].slotNr);
        xhci_ringDoorbellAndWait_HC(x);
    }
    else
    {
      #ifdef _XHCI_DIAGNOSIS_
		//textColor(IMPORTANT);
        printf("\nReset Device Cmd not needed.");
		//textColor(TEXT);
      #endif
    }

    if (x->OpRegs->PortReg[j].portsc & PORT_PP) // power on
    {
        if (x->OpRegs->PortReg[j].portsc & PORT_PED) // enabled
        {
            // port speed can be read from Bits 10-13 of PORTSC after port reset (5.4.8)
            usb_speed_t speed = (usb_speed_t)((x->OpRegs->PortReg[j].portsc & 0x3C00) >> 10);

            //writeInfo(0, "xhci Port: %u, device attached and enabled, xHC slot: %u, speed: %u", j + 1, x->portSlotLink[j].slotNr, speed);

            xhci_InputContext_t* inputContext = x->devInputContextPtr[x->portSlotLink[j].slotNr-1];
            inputContext->ICC.A = 0;
            inputContext->ICC.A |= BIT(0); // 4.3.3 Device Slot Initialization
            inputContext->ICC.A |= BIT(1); // These flags indicate that the Slot Context and the Endpoint 0 Context
            inputContext->ICC.D = ~inputContext->ICC.A;

            // of the Input Context are affected by the command.
            inputContext->DC.slotContext.speed = speed;
            inputContext->DC.slotContext.usbDeviceAddress = 0;
            xhci_endpointContext_t* pEP = inputContext->DC.endpointContext;

            switch (speed)
            {
                case USB_LOWSPEED: case USB_FULLSPEED:
                    pEP[0].maxPacketSize = 8; // fullspeed can have MPS between 8 and 64 - we will find out later, start with 8
                    break;
                case USB_HIGHSPEED:
                    pEP[0].maxPacketSize = 64;
                    break;
                case USB_SUPERSPEED: default:
                    pEP[0].maxPacketSize = 512;
                    break;
            }

            xhci_Address_Device_Command(x, x->portSlotLink[j].slotNr, 1); // BSR=1
            xhci_ringDoorbellAndWait_HC(x);

            hc_setupUSBDevice(&x->hc, j, speed);

			//textColor(DATA);
            printf("\nspeed in the slot context: %u", x->devContextPtr[x->portSlotLink[j].slotNr-1]->slotContext.speed);
            printf("\nmps in the ep0 context: %u",    x->devContextPtr[x->portSlotLink[j].slotNr-1]->endpointContext->maxPacketSize);
			//textColor(TEXT);
        }
        else
        {
            printf("\nPP, but no PED");
            //writeInfo(0, "xhci Port: %u, device attached, but not enabled! xHC slot: %u", j+1, x->portSlotLink[j].slotNr);
            // xhci_showStates(x, x->portSlotLink[j].slotNr, j);
        }
    }
    Syscall_GetChar();
}

static void xhci_portCheck(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nportCheck");
	//textColor(TEXT);
  #endif

    for (uint8_t j=0; j<x->hc.rootPortCount; j++)
    {
        if (x->OpRegs->PortReg[j].portsc & PORT_CSC) // TODO: clarify: ports or slots?
        {
            x->OpRegs->PortReg[j].portsc |= PORT_CSC; // clear CSC

            if (x->OpRegs->PortReg[j].portsc & PORT_CCS) // attached
            {
                // xhci_showPortStatus(x, j);
				g_j = j;
				
				Syscall_CreateThread(xhci_detectDevice, "xhci_detectDevice", x, 16, 0);
                //task_t* thread = create_cthread((void*)&xhci_detectDevice, "xhci_detectDevice");
                //task_passStackParameter(thread, &x, sizeof(x));
                //task_passStackParameter(thread, &j, sizeof(j));
                //scheduler_insertTask(thread);
            }
            else // detached
            {
                //writeInfo(0, "Port: %u, no device attached", j+1);

                // diasable slot (state: configured -> disabled)
				//textColor(IMPORTANT);
                printf("\nDisableSlot port: %u slot: %u", j+1, x->portSlotLink[j].slotNr);
				//textColor(TEXT);

                xhci_Disable_Slot_Command(x, x->portSlotLink[j].slotNr);
                xhci_ringDoorbellAndWait_HC(x);

                x->portsEnabled[j]=false;

                // released devContext (slot) to be filled by zero in Disabled State (owner: software)
                memset(x->devContextPtr[x->portSlotLink[j].slotNr-1], 0, sizeof(xhci_DeviceContext_t));

                // printf("\nslot %u state: %u", x->portSlotLink[j].slotNr, x->devContextPtr[j]->slotContext.slotState);

                // Reset Slot/EP-Transferring
                for (uint16_t i=0; i<NUM_ENDPOINTS; i++)
                {
                    uint8_t slotNr = x->portSlotLink[j].slotNr;
                    xhci_xfer_NormalTRB_t* trb = x->trb[slotNr-1][i];
                    memset(trb, 0, 256 * sizeof(xhci_xfer_NormalTRB_t));
                    // Software uses and maintains private copies of the Enqueue and Dequeue Pointers for each Transfer Ring.
                    x->slots[slotNr-1]->endpoints[i].virtEnqTransferRingPtr = x->slots[slotNr-1]->endpoints[i].virtDeqTransferRingPtr = trb;
                    x->slots[slotNr-1]->endpoints[i].TransferRingProducerCycleState = true; // PCS
                    x->slots[slotNr-1]->endpoints[i].TransferCounter = 0; // Reset Transfer Counter
                    x->slots[slotNr-1]->endpoints[i].timeEvent = 0;
                    x->slots[slotNr-1]->endpoints[i].TransferRingbase = trb;
                }

                // destroy port-slot-Link
                x->portSlotLink[j].slotNr = 0xFF; // no valid slot

                if (x->hc.ports.data[j]->device)
                {
                    // destroy device
                    usb_destroyDevice(x->hc.ports.data[j]->device);
                    x->hc.ports.data[j]->device = 0;
                    x->hc.ports.data[j]->connected = false;
                }
            }
        }
    }
}


/*******************************************************************************************************
*                                                                                                      *
*                                  Command & Transfer Ring Management                                  *
*                                                                                                      *
*******************************************************************************************************/

static xhci_LinkTRB_t* xhci_enqueueCommand(xhci_t* x, xhci_LinkTRB_t* pCmd)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nenqueueCommand at phys: ");
	//textColor(LIGHT_BLUE);
    printf("%X", Syscall_GetPAFromVM(x->virtEnqCmdRingPtr));
	//textColor(TEXT);
  #endif

    pCmd->cycle = x->CmdRingProducerCycleState;
    memcpy(x->virtEnqCmdRingPtr, pCmd, sizeof(xhci_LinkTRB_t));

    xhci_LinkTRB_t* CmdRingPtr = x->virtEnqCmdRingPtr;

    x->virtEnqCmdRingPtr++;
    x->CmdCounter++;
    x->CmdPending++;
    showCounter(x, 0);

    // Found end of ring, goto start
    if (x->virtEnqCmdRingPtr->TRBtype == TRB_TYPE_LINK)
    {
        x->virtEnqCmdRingPtr->cycle = x->CmdRingProducerCycleState;
        x->virtEnqCmdRingPtr = x->CmdRingbase;
        x->CmdRingProducerCycleState = !x->CmdRingProducerCycleState;

        //writeInfo(2,"LinkTRB Cmd found - CmdCounter: %u", x->CmdCounter);
        printf("LinkTRB Cmd found - CmdCounter: %u", x->CmdCounter);
    }
    return CmdRingPtr;
}

// 4.9.3 Command Ring Management
// Software is responsible for advancing the Enqueue pointer.
// It does this by toggling the Cycle bit each pass through the Command Ring as it writes Command TRBs.
static void xhci_setEnqueueCommandPtr(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nsetEnqueueCommandPtr");
	//textColor(TEXT);
  #endif

    xhci_LinkTRB_t cmd = { 0 };
    cmd.cycle = !x->CmdRingProducerCycleState; // The Enq Ptr is defined by a Cycle bit transition.
    memcpy(x->virtEnqCmdRingPtr, &cmd, sizeof(xhci_LinkTRB_t));
}

/*
5.6: Note: Software shall not write the Doorbell of an endpoint until after it has issued a Configure Endpoint
Command for the endpoint and received a successful Command Completion Event.*/
static void xhci_ringDoorbell_Device(xhci_t* x, uint8_t slotNr, uint8_t target, uint16_t streamID)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nringDoorbell_Device slot: %u target: %u streamID: %u time: %u", slotNr, target, streamID, Syscall_GetTickCount());
	//textColor(TEXT);
  #endif

    x->DoorbellRegs[slotNr].dword = (target | (streamID<<16));
}

static void xhci_ringDoorbellAndWait_HC(xhci_t* x)
{
    //irq_resetCounter(x->irq);
    xhci_ringDoorbell_Device(x, 0, 0, 0); // 4.7, 4.6.1, 5.6

    uint16_t timeout = 20;
    while (x->CmdPending > 0)
    {
		Syscall_Sleep(20);
       // waitForIRQ(x->irq, 20);
        //irq_resetCounter(x->irq);
        if (timeout == 0)
        {
            printf("\nxHCI: Timeout; command seems to be not finished.");
            x->CmdPending = 0;
            break;
        }
        timeout--;
    }
}

static xhci_xfer_NormalTRB_t* xhci_enqueueTransfer(xhci_t* x, xhci_xfer_NormalTRB_t* pTransfer, uint8_t slotNr, uint8_t DCI)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nsetEnqueueTransfer DCI: %u EnqPtr: %X (%X)", DCI, x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr, Syscall_GetPAFromVM(x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr));
	//textColor(TEXT);
  #endif

    pTransfer->cycle = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;
    xhci_xfer_NormalTRB_t* destination = x->slots[slotNr-1]->endpoints[DCI - 1].virtEnqTransferRingPtr;
    memcpy(destination, pTransfer, sizeof(xhci_xfer_NormalTRB_t));

    x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr++;
    x->slots[slotNr-1]->endpoints[DCI-1].TransferCounter++;
    showCounter(x, slotNr);

    // Found end of ring, goto start
    if (x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr->TRBtype == TRB_TYPE_LINK)
    {
        x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr->cycle = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;
        x->slots[slotNr-1]->endpoints[DCI-1].virtEnqTransferRingPtr = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingbase;
        x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState = !x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;
        //writeInfo(2, "slot: %u DCI: %u LinkTRB found - TransferCounter: %u", slotNr, DCI, x->slots[slotNr-1]->endpoints[DCI-1].TransferCounter);
    }
    return destination;
}

static void xhci_setEnqueueTransferPtr(xhci_t* x, uint8_t slotNr, uint8_t DCI)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nsetEnqueueTransferPtr");
	//textColor(TEXT);
  #endif

    xhci_xfer_NormalTRB_t transfer = {0};
    transfer.cycle = !x->slots[slotNr - 1]->endpoints[DCI - 1].TransferRingProducerCycleState; // Enq Ptr is marked in the Transfer Ring itself by a transition of the Cycle bit.
    memcpy(x->slots[slotNr - 1]->endpoints[DCI - 1].virtEnqTransferRingPtr, &transfer, sizeof(xhci_xfer_NormalTRB_t));
}


/*******************************************************************************************************
*                                                                                                      *
*                                           PREPARE SLOTS & ENDPOINTS                                  *
*                                                                                                      *
*******************************************************************************************************/

static void xhci_resetSlot(xhci_t* x, uint8_t slotNr)
{
	//textColor(DATA);
    printf("\nxhci_resetSlot");
	//textColor(TEXT);

    // 6.2.2.1 Address Device Command Usage
    /*The Input Slot Context is considered “valid” by the Address Device Command if:
    1) the Route String field defines a valid route string,
    2) the Speed field identifies the speed of the device,
    3) the Context Entries field is set to ‘1’ (i.e. Only the Control Endpoint Context is valid),
    4) the value of the Root Hub Port Number field is between 1 and MaxPorts,
    5) if the device is LS/FS and connected through a HS hub, then the TT Hub Slot ID field references a Device Slot that is assigned to the HS hub,
    the MTT field indicates whether the HS hub supports Multi-TTs, and the TT Port Number field indicates the correct TT port number on the HS hub,
    else these fields are cleared to ‘0’,
    6) the Interrupter Target field set to a valid value, and
    7) all other fields are cleared to ‘0’.*/

    xhci_InputContext_t* inputContext = x->devInputContextPtr[slotNr-1];
    inputContext->ICC.A = 0;
    inputContext->ICC.A |= BIT(0); // 4.3.3 Device Slot Initialization
    inputContext->ICC.A |= BIT(1); // These flags indicate that the Slot Context and the Endpoint 0 Context
                                                    // of the Input Context are affected by the command.
    inputContext->ICC.D = ~inputContext->ICC.A;

    xhci_slotContext_t* pSlot = &(inputContext->DC.slotContext);
    if (pSlot)
    {
        // To access a device attached directly to a Root Hub port, the Route String shall equal ‘0’,
        // and the Root Hub Port Number shall indicate the specific Root Hub port to use.
        pSlot->routeString       = 0;      // 1)
        pSlot->speed             = 3;      // 2)
        pSlot->contextEntries    = 1;      // 3)
        pSlot->rootHubPortNumber = slotNr; // 4)
        pSlot->interrupterTarget = 0;      // 6)

        pSlot->MTT = 0, // MTT is set to '1' by software if this is a High-speed hub (Speed = ‘3’ and Hub = ‘1’)
        pSlot->hub = 0; // set to '1' by software if this device is a USB hub, or '0' if it is a USB function.

        pSlot->usbDeviceAddress = 0;    // As Input, software shall initialize the field to ‘0’.
    }

    /* 6.2.3.1 Address Device Command Usage (rev. 1.1)
    The Input Endpoint 0 Context is considered “valid” by the Address Device Command if:
    1) the EP Type field = Control,
    2) the values of the Max Packet Size, Max Burst Size, and the Interval are considered within range for endpoint type and the speed of the device,
    3) the TR Dequeue Pointer field points to a valid Transfer Ring,
    4) the DCS field = ‘1’,
    5) the MaxPStreams field = ‘0’, and
    6) all other fields are within the valid range of values.
    */

    xhci_endpointContext_t* pEP = inputContext->DC.endpointContext; // pEP[0] is for EP0
    if (pEP)
    {
        pEP[0].epState = 0;        // As Input, this field is initialized to ‘0’ by software.
        pEP[0].epType = 4;         // Control
        pEP[0].maxPacketSize = 8;  // not 512! The default maximum packet size for the Default Control Endpoint, as function of the PORTSC Port Speed field. HS = 64, SS = 512.
        pEP[0].maxBurstSize = 0;   // max number of consecutive USB transactions that should be executed per scheduling opportunity.
                                    // This is a “zero-based” value, where 0 to 15 represents burst sizes of 1 to 16, respectively.
        pEP[0].TRdequeuePtrLo = Syscall_GetPAFromVM(x->slots[slotNr-1]->endpoints[0].virtDeqTransferRingPtr) >> 4; // Start address of first segment of the Default Control EP Transfer (bits 63:4 !!)
        pEP[0].TRdequeuePtrHi = 0;
        pEP[0].interval = 0;       // The Interval field defines the Interval for polling endpoint for data transfers, expressed in 125 μs units.
                                    // For SuperSpeed bulk and control endpoints, the Interval field shall not be used by the xHC.
        pEP[0].maxPStreams = 0;    // ‘0’ indicates that Streams are not supported by this endpoint and the Endpoint Context TR Dequeue Pointer field references a Transfer Ring.
        pEP[0].mult = 0;           // This field shall be ‘0’ for all endpoint types except for SS Isochronous.
        pEP[0].errCount = 3;       // 3;
        pEP[0].dcs = 1;            // This field shall be ‘0’ if MaxPStreams > ‘0’.
        pEP[0].averageTRBLenghth = 8; // ??? The value of this field shall be greater than ‘0’!
    }
}

static void xhci_prepareSlotsForControlTransfers(xhci_t* x) // TODO: necessary to do that for all slots before attach? Idea: portNr. <==> slotNr.
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nprepareSlotsForControlTransfers");
	//textColor(TEXT);
  #endif

    for (uint8_t j=0; j<MAX_HC_PORTS; j++)
    {
        x->portSlotLink[j].cmdPtr = xhci_Enable_Slot_Command(x);
        xhci_resetSlot(x, j+1); // slotNr = j+1
        x->portsEnabled[j] = true;
    }

    xhci_ringDoorbellAndWait_HC(x);
}

static void updateEndpointInformation(xhci_t* x, uint8_t slotNr, usb_endpoint_t* ep)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nendpoint %u: mps=%u, interval=%u, type=%u", ep->address, ep->mps, ep->interval, ep->type);
  #endif
    uint8_t DCI = calculateDCI(ep);
    x->devInputContextPtr[slotNr-1]->ICC.A |= BIT(DCI);

    xhci_endpointContext_t* pEP = x->devInputContextPtr[slotNr-1]->DC.endpointContext;
    if (pEP) // cf. 4.8.2.3 Bulk Endpoints
    {
        pEP[DCI - 1].epState = 0; // As Input, this field is initialized to ‘0’ by software.
        switch (ep->type)
        {
            case EP_CONTROL:
                pEP[DCI - 1].epType = 4;
                break;
            case EP_ISOCHRONOUS:
                pEP[DCI - 1].epType = 1;
                break;
            case EP_BULK:
                pEP[DCI - 1].epType = 2;
                break;
            case EP_INTERRUPT:
                pEP[DCI - 1].epType = 3;
                break;
        }
        if (ep->dir == EP_IN)
            pEP[DCI - 1].epType += 4;
        pEP[DCI - 1].maxPacketSize = ep->mps;
        // This is a “zero-based” value, where 0 to 15 represents burst sizes of 1 to 16, respectively.
        pEP[DCI - 1].maxBurstSize = 0; // max number of consecutive USB transactions that should be executed per scheduling opportunity.
        pEP[DCI - 1].TRdequeuePtrLo = Syscall_GetPAFromVM(x->slots[slotNr-1]->endpoints[DCI - 1].virtDeqTransferRingPtr) >> 4; // Start address of first segment of ... EP Transfer (bits 63:4 !!)

      #ifdef _XHCI_DIAGNOSIS_
		//textColor(IMPORTANT);
        printf("\npEP[%u].TRdequeuePtrLo (32bit): virt: %X phys: %X", DCI-1, Syscall_GetPAFromVM(pEP[DCI-1].TRdequeuePtrLo<<4), pEP[DCI-1].TRdequeuePtrLo<<4);
		//textColor(TEXT);
      #endif

        pEP[DCI - 1].TRdequeuePtrHi = 0;
        // For SuperSpeed bulk and control endpoints, the Interval field shall not be used by the xHC.
        pEP[DCI - 1].interval = ep->interval; // The Interval field defines the Interval for polling endpoint for data transfers, expressed in 125 μs units.
        pEP[DCI - 1].maxPStreams = 0; // ‘0’ indicates that Streams are not supported by this endpoint and the Endpoint Context TR Dequeue Pointer field references a Transfer Ring.
        pEP[DCI - 1].mult = 0;        // This field shall be ‘0’ for all endpoint types except for SS Isochronous.
        pEP[DCI - 1].errCount = 3;    // 3;
        pEP[DCI - 1].dcs = 1;         // This field shall be ‘0’ if MaxPStreams > ‘0’.
        pEP[DCI - 1].averageTRBLenghth = ep->mps / 2; // The value of this field shall be greater than ‘0’!
    }
}

void xhci_updateEndpointInformation(hc_port_t* hc_port)
{
    xhci_t* x = (xhci_t*)hc_port->hc;
    uint8_t portNr = (uint8_t)(size_t)hc_port->data; // TODO: check
    uint8_t slotNr = x->portSlotLink[portNr-1].slotNr;

    xhci_InputContext_t* inputContext = x->devInputContextPtr[slotNr-1];
    inputContext->DC.slotContext.contextEntries = 0;
    inputContext->DC.slotContext.interrupterTarget = 0;

    /*
    The Add Context flag A1 and Drop Context flags D0 and D1 of the Input Control Context (in the Input
    Context) shall be cleared to ‘0’. Endpoint 0 Context does not apply to the Configure Endpoint Command
    and shall be ignored by the xHC. A0 shall be set to ‘1’ and refer to section 6.2.2.2 for the Slot Context fields
    used by the Configure Endpoint Command.
    */
    inputContext->ICC.A = 0;
    inputContext->ICC.A |= BIT(0);
    inputContext->ICC.A &= ~BIT(1); //EP0

    usb_device_t* dev = hc_port->device;
    if (dev)
    {
        for (dlelement_t* el = dev->endpoints.head; el; el = el->next)
        {
            usb_endpoint_t* ep = (usb_endpoint_t * )(el->data);
            inputContext->DC.slotContext.contextEntries = MAX(inputContext->DC.slotContext.contextEntries, calculateDCI(ep));

            updateEndpointInformation(x, slotNr, ep);
        }
    }

    inputContext->ICC.D = ~inputContext->ICC.A;
    xhci_Evaluate_Context_Command(x, slotNr);
    xhci_ringDoorbellAndWait_HC(x);
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Event-Ring and Interrupters                               *
*                                                                                                      *
*******************************************************************************************************/

static void xhci_prepareEventRing(xhci_t* x)
{
    // Initialize each active interrupter by:
    // Defining the Event Ring: (4.9.4), allocate and initialize the Event Ring Segment(s).
    x->evtSegmentSize    = 256; // at least 16!
    x->evtNumberSegments =   1;

    void* EvtRingSegment = malloc(x->evtSegmentSize*sizeof(xhci_eventTRB_t)); // Alignment see Table 54
    memset(EvtRingSegment, 0, x->evtSegmentSize*sizeof(xhci_eventTRB_t));
    x->EvtRingbase = (xhci_eventTRB_t*)EvtRingSegment;

    // Allocate the Event Ring Segment Table (ERST) (6.5).
    // Initialize ERST table entries to point to and to define the size (in TRBs) of the respective Event Ring Segment.
    xhci_eventRingSegmentTableEntry_t* EvtRingSegmentEntry = (xhci_eventRingSegmentTableEntry_t*)malloc(sizeof(xhci_eventRingSegmentTableEntry_t)); // Alignment see Table 54
    EvtRingSegmentEntry->physBaseAddressLo = Syscall_GetPAFromVM(EvtRingSegment);
    EvtRingSegmentEntry->physBaseAddressHi = 0;
    EvtRingSegmentEntry->ringSegmentSize   = x->evtSegmentSize;
    EvtRingSegmentEntry->reserved2         = 0;

    x->virtDeqEvtRingPtr = x->EvtRingbase;

    // Program the Interrupter Event Ring Segment Table Size (ERSTSZ) register (5.5.2.3.1) with the number of segments described by the Event Ring Segment Table.
    x->RuntimeRegs->IRS[0].eventRingSegmentTableSize = x->evtNumberSegments;

    // Program the Interrupter Event Ring Dequeue Pointer (ERDP) register (5.5.2.3.3) with the starting address of the first segment described by the Event Ring Segment Table.
    x->RuntimeRegs->IRS[0].eventRingDequeuePointerLo = EvtRingSegmentEntry->physBaseAddressLo | BIT(3); // Table 47 // // clear EHB, cf. 5.5.2.3.3 Event Ring Dequeue Pointer Register (ERDP)
    x->RuntimeRegs->IRS[0].eventRingDequeuePointerHi = EvtRingSegmentEntry->physBaseAddressHi;

    // Program the Interrupter Event Ring Segment Table Base Address (ERSTBA) register (5.5.2.3.2) with a 64-bit address pointer to where the Event Ring Segment Table is located.
    // Note that writing the ERSTBA enables the Event Ring. Refer to section 4.9.4 for more information on the Event Ring registers and their initialization.
    x->RuntimeRegs->IRS[0].eventRingSegmentTableBaseAddressLo = Syscall_GetPAFromVM(EvtRingSegmentEntry);
    x->RuntimeRegs->IRS[0].eventRingSegmentTableBaseAddressHi = 0;

    // Software CCS
    x->EvtRingConsumerCycleState = true;

    // Switch to MSI interrupt mechanism if available
    if (x->msiCapEnabled)
        pci_switchToMSI(x->PCIdevice);

    // Initializing the Interval field of the Interrupt Moderation register (5.5.2.2) with the target interrupt moderation rate.
    x->RuntimeRegs->IRS[0].interrupterModerationInterval = 4000; // default ~1ms
    x->RuntimeRegs->IRS[0].interrupterModerationCounter  =    0; // default = undefined

    // Enable system bus interrupt generation by writing a ‘1’ to the Interrupter Enable (INTE) flag of the USBCMD register (5.4.1).
    x->OpRegs->command |= CMD_INTE; // Interrupter Enable

    // Enable the Interrupter by writing a ‘1’ to the Interrupt Enable (IE) field of the Interrupter Management register (5.5.2.1).
    if (!x->msiCapEnabled)
    {
        x->RuntimeRegs->IRS[0].IP = 1; // In systems that do not support MSI or MSI-X, the IP bit may be cleared by writing a ‘1’ to it (5.5.2.1, Table 43)
    }
    x->RuntimeRegs->IRS[0].IE = 1; // 1: interrupter can generate an interrupt, 0: generating interrupts is prohibited (5.5.2.1, Table 43)
    Syscall_Sleep(20); //  IMPORTANT
}

static void xhci_parseEvents(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nparseEvents time: %u", Syscall_GetTickCount());
  #endif

    xhci_eventTRB_t* events = x->virtDeqEvtRingPtr;

    while (events->cycle == x->EvtRingConsumerCycleState)
    {
        x->virtDeqEvtRingPtr++;
        x->EvtCounter++;
        showCounter(x, 0);

        xhci_showCompletionCodeError(events->completionCode);

        switch (events->TRBtype)
        {
            case TRB_EVENT_TRANSFER:
                x->slots[events->slot-1]->endpoints[(events->byte3 & 0x1F) - 1].pendingTransfer = false; // Feedback to doorbell
                x->slots[events->slot-1]->endpoints[(events->byte3 & 0x1F) - 1].timeEvent = Syscall_GetTickCount();
                x->slots[events->slot-1]->endpoints[(events->byte3 & 0x1F) - 1].transferError = events->completionCode;

              #ifdef _XHCI_DIAGNOSIS_
                printf("\tslot:%u EP:%u ED:%u Addr: %X len:%u type:%s time: %u",
                    events->slot,
                    events->byte3 & 0x1F, // bit 16-20
                    events->ED, /* ED = 1: event generated by an Event Data TRB,
                                            TRB Pointer points to a 64-bit value provided by the Event Data TRB.
                                    ED = 0: TRB Pointer points to the TRB that generated this event.
                                */
                    events->eventDataLo,
                    events->transferLength, // cf. Table 91: Offset 08h – Transfer Event TRB Field Definitions
                                            // This field shall reflect the residual number of bytes not transferred.
                    "Transfer",
                    x->slots[events->slot-1]->endpoints[(events->byte3 & 0x1F) - 1].timeEvent);
              #endif

                /*
                4.8.3 Endpoint Context State
                A Halt condition, e.g. a Stall Error, Invalid Stream Type Error, Invalid Stream ID Error, Babble Detected Error, Event Lost Error,
                USB Transaction Error, or a Split Transaction Error detected on a USB pipe shall cause a Running Endpoint to transition to the Halted state.
                A Reset Endpoint Command shall be used to clear the Halt condition on the endpoint and transition the endpoint to the Stopped state.
                */
                if ( ((events->completionCode == 3) || (events->completionCode == 6))      // babble or stall
                        && (x->devContextPtr[events->slot-1]->endpointContext[0].epState == 1)  // EP0 running
                        && (x->devContextPtr[events->slot-1]->slotContext.slotState == 3) )     // slot configured
                {
                    xhci_showStates(x, events->slot, events->slot - 1);

                    printf("\nReset_Endpoint_Command after HALTED, EP state: 2=>3");

                    xhci_Reset_Endpoint_Command(x, events->slot, (events->byte3 & 0x1F), 0); // Transfer State Preserve (TSP) = 0
                    // Cleared to ‘0’ by software if the Reset operation resets the current transfer state of the endpoint,
                    // i.e. The Data Toggle of a USB2 device or the Sequence Number of a USB3 device is cleared to ‘0’. cf 4.6.8.1.
                    xhci_ringDoorbellAndWait_HC(x);

                    xhci_showStates(x, events->slot, events->slot - 1);

                    uintptr_t ptr = events->eventDataLo + sizeof(xhci_xfer_NormalTRB_t); //HACK  TODO: we need function regarding linkTRB
                    xhci_Set_TR_DeqPointer_Command(x, events->slot, (events->byte3 & 0x1F), ptr, 0);

                    printf("\nringDoorbell_Device, EP state: 3=>1");
                    xhci_ringDoorbell_Device(x, events->slot, (events->byte3 & 0x1F), 0);
                    Syscall_Sleep(20);

                    xhci_showStates(x, events->slot, events->slot - 1);
                }
                break;

            case TRB_EVENT_CMD_COMPLETE:    // 6.4.2.2 - Command Completion Event TRB
             #ifdef _XHCI_DIAGNOSIS_
                printf("\tCmd: ");
				//textColor(LIGHT_BLUE);
                printf("%X ", events->eventDataLo);
				//textColor(IMPORTANT);
                printf("slot: %u ", events->slot);
				//textColor(TEXT);
              #endif

                if (x->CmdPending > 0)
                    x->CmdPending--;
                for (uint8_t j=0; j<MAX_HC_PORTS; j++)
                {
                    if (Syscall_GetPAFromVM((unsigned int)(x->portSlotLink[j].cmdPtr)) == events->eventDataLo)
                    {
                        x->portSlotLink[j].slotNr = events->slot;

                      #ifdef _XHCI_DIAGNOSIS_
						//textColor(DATA);
                        printf("\nLink: port=%u slot=%u ", j+1, x->portSlotLink[j].slotNr);
						//textColor(TEXT);
                      #endif
                    }
                }

              #ifdef _XHCI_DIAGNOSIS_
                xhci_showTRBType(((xhci_LinkTRB_t*)Syscall_GetPAFromVM(events->eventDataLo))->TRBtype);

                printf("slot: %u", events->slot);
              #endif
                break;

          #ifdef _XHCI_DIAGNOSIS_
            case TRB_EVENT_PORT_STS_CHANGE: // 6.4.2.3 - Port Status Change Event TRB
                printf("\tPort: %u type: %s", BYTE4(events->eventDataLo), "Port Status Change");
                break;

            case TRB_EVENT_HOST_CTRL:       // 6.4.2.6 Host Controller Event TRB (xHC state changes and errors)
                printf("\tc:%u type:%s", events->cycle ? 1 : 0, "xHC Event");
                break;
          #endif

            default:
              #ifdef _XHCI_DIAGNOSIS_
                printf("\tLo:%Xh Hi:%Xh Compl.:%u type:%yh", events->eventDataLo, events->eventDataHi, events->completionCode, events->TRBtype);
              #endif
                break;
        }//switch

        if (x->virtDeqEvtRingPtr == x->EvtRingbase + x->evtSegmentSize)
        {
            events = x->virtDeqEvtRingPtr = x->EvtRingbase;

            //writeInfo(2, "EvtRing reset - EvtCounter: %u DeqEvtRingPtr: %X EvtBase: %X", x->EvtCounter, x->virtDeqEvtRingPtr, x->EvtRingbase);

            x->EvtRingConsumerCycleState = !x->EvtRingConsumerCycleState;
        }
        else
            events++;
    }//while

    // System software shall write the Event Ring Dequeue Pointer (ERDP) register to inform the xHC
    // that it has completed the processing of Event TRBs up to and including the Event TRB referenced by the ERDP.
    x->RuntimeRegs->IRS[0].eventRingDequeuePointerLo = Syscall_GetPAFromVM((unsigned int)events) | BIT(3); // cycle bit mismatch has to be counted. // clear EHB, cf. 5.5.2.3.3 Event Ring Dequeue Pointer Register (ERDP)
    x->RuntimeRegs->IRS[0].eventRingDequeuePointerHi = 0;
  #ifdef _XHCI_DIAGNOSIS_
    //Syscall_GetChar();
  #endif
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Commands                                                  *
*                                                                                                      *
*******************************************************************************************************/

/*
static void xhci_NoOp_Command(xhci_t* x)
{
    xhci_cmd_linkTRB_t cmdNoOp = {0};
    cmdNoOp.TRBtype = TRB_TYPE_NOOP; // 6.4.3.1 No Op Command TRB
    xhci_enqueueCommand(x, &cmdNoOp);
    xhci_setEnqueueCommandPtr(x);
}
*/

static xhci_LinkTRB_t* xhci_Enable_Slot_Command(xhci_t* x)
{
    ////textColor(DATA);
    printf("\nxhci_Enable_Slot_Command");
	//textColor(TEXT);

    xhci_LinkTRB_t cmdEnSlot = { 0 };
    cmdEnSlot.TRBtype = TRB_TYPE_ENABLE_SLOT; // 6.4.3.2 Enable Slot Command TRB

    xhci_LinkTRB_t* cmdPtr = xhci_enqueueCommand(x, &cmdEnSlot);
    xhci_setEnqueueCommandPtr(x);
    return cmdPtr;
}

static void xhci_Disable_Slot_Command(xhci_t* x, uint8_t slotNr)
{
    xhci_cmd_DeviceTRB_t cmdDisSlot = {0};
    cmdDisSlot.TRBtype = TRB_TYPE_DISABLE_SLOT; // 6.4.3.3 Disable Slot Command TRB
    cmdDisSlot.SlotID = slotNr;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdDisSlot);
    xhci_setEnqueueCommandPtr(x);
}

static void xhci_Address_Device_Command(xhci_t* x, uint8_t slotNr, bool BSR)
{
	//textColor(DATA);
    printf("\nxhci_Address_Device_Command BSR=%u", BSR);
	//textColor(TEXT);

    xhci_cmd_DeviceTRB_t cmdAddr = {0};
    cmdAddr.InputContextPtrLo = Syscall_GetPAFromVM(x->devInputContextPtr[slotNr-1]);
    cmdAddr.InputContextPtrHi = 0;
    cmdAddr.TRBtype = TRB_TYPE_ADDRESS_DEVICE; // 6.4.3.4 Address Device Command TRB
    cmdAddr.SlotID = slotNr;
    cmdAddr.BSR_DC = BSR; // BSR = ‘0’: USB SET_ADDRESS request, '1': no request

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdAddr);
    xhci_setEnqueueCommandPtr(x);
}

static void xhci_Configure_Endpoint_Command(xhci_t* x, uint8_t slotNr, bool DC)
{
    xhci_cmd_DeviceTRB_t cmdConfigEP = {0};
    cmdConfigEP.InputContextPtrLo = Syscall_GetPAFromVM(x->devInputContextPtr[slotNr-1]);
    cmdConfigEP.InputContextPtrHi = 0;
    cmdConfigEP.TRBtype = TRB_TYPE_CONFIGURE_EP; // 6.4.3.5 Configure Endpoint Command TRB
    cmdConfigEP.SlotID = slotNr;
    cmdConfigEP.BSR_DC = DC; // not deconfigure

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdConfigEP);
    xhci_setEnqueueCommandPtr(x);
}

static void xhci_Evaluate_Context_Command(xhci_t* x, uint8_t slotNr)
{
    xhci_cmd_DeviceTRB_t cmdEvalContext = {0};
    cmdEvalContext.InputContextPtrLo = Syscall_GetPAFromVM(x->devInputContextPtr[slotNr-1]);
    cmdEvalContext.InputContextPtrHi = 0;
    cmdEvalContext.TRBtype = TRB_TYPE_EVALUATE_CTX; // 6.4.3.6 Evaluate Context Command TRB
    cmdEvalContext.SlotID = slotNr;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdEvalContext);
    xhci_setEnqueueCommandPtr(x);
}

static void xhci_Reset_Endpoint_Command(xhci_t* x, uint8_t slotNr, uint8_t DCI, bool TransferStatePreserve)
{
    xhci_cmd_ResetEndpointTRB_t cmdReset = {0};
    cmdReset.TRBtype = TRB_TYPE_RESET_EP; // 6.4.3.7 Reset Endpoint Command TRB
    cmdReset.SlotID = slotNr;
    cmdReset.EndpointID = DCI;
    cmdReset.TSP = TransferStatePreserve;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdReset);
    xhci_setEnqueueCommandPtr(x);
}

/*
static void xhci_Stop_Endpoint_Command(xhci_t* x, uint8_t slotNr, uint8_t DCI, bool Suspend)
{
    xhci_cmd_StopEndpointTRB_t cmdStop = {0};
    cmdStop.TRBtype = TRB_TYPE_STOP_EP; // 6.4.3.8 Stop Endpoint Command TRB
    cmdStop.SlotID = slotNr;
    cmdStop.EndpointID = DCI;
    cmdStop.SP = Suspend;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdStop);
    xhci_setEnqueueCommandPtr(x);
}
*/

static void xhci_Set_TR_DeqPointer_Command(xhci_t* x, uint8_t slotNr, uint8_t DCI, uintptr_t NewTRDeqPtrLo, uint16_t StreamID)
{
    xhci_TR_DequeuePointerTRB_t cmdTRDeqPtr = {0};
    cmdTRDeqPtr.NewTRDeqPtrLo = NewTRDeqPtrLo;
    cmdTRDeqPtr.NewTRDeqPtrHi = 0;
    cmdTRDeqPtr.StreamID = StreamID;
    cmdTRDeqPtr.TRBtype = TRB_TYPE_SET_TR_DEQUEUE; // 6.4.3.9 Set TR Dequeue Pointer Command TRB
    cmdTRDeqPtr.SlotID = slotNr;
    cmdTRDeqPtr.EndpointID = DCI;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdTRDeqPtr);
    xhci_setEnqueueCommandPtr(x);
}

static void xhci_Reset_Device_Command(xhci_t* x, uint8_t slotNr)
{
    xhci_cmd_DeviceTRB_t cmdReset = {0};
    cmdReset.TRBtype = TRB_TYPE_RESET_DEVICE; // 6.4.3.10 Reset Device Command TRB
    cmdReset.SlotID = slotNr;

    xhci_enqueueCommand(x, (xhci_LinkTRB_t*)&cmdReset);
    xhci_setEnqueueCommandPtr(x);
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Transactions                                              *
*                                                                                                      *
*******************************************************************************************************/

typedef struct
{
    xhci_xfer_NormalTRB_t* TD;
    void*  TDBuffer;
    void*  inBuffer;
    size_t inLength;
} xhci_transaction_t;

void xhci_setupTransfer(usb_transfer_t* transfer)
{
    transfer->data = 0;

  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_setupTransfer");
    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t slotNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data;
    printf("\nbefore Xfer: ");
    xhci_showStates(x, slotNr, slotNr-1);
  #endif
}

uint8_t xhci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_setupTransaction. req = %u hiVal = %u loVal = %u", req, hiVal, loVal);
  #endif

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data;
    uint8_t slotNr = x->portSlotLink[portNr - 1].slotNr;
    if (slotNr == 0xFF || slotNr == 0x00) // no valid slot
        return loVal;

    transfer->data = (void*)(uintptr_t)req;

    if (req == 5) // SET_ADDRESS
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci Enumeration with BSR = 0");
      #endif

        xhci_Address_Device_Command(x, slotNr, 0); // SlotNr for usb Device Address
        xhci_ringDoorbellAndWait_HC(x);

        // wait for xHC Addressed State (2)
        if (x->devContextPtr[slotNr-1]->slotContext.slotState < 2)
        {
            printf("\nAddress Device Command failed, current xHc slot state: %u", x->devContextPtr[slotNr-1]->slotContext.slotState);
        }

      #ifdef _XHCI_DIAGNOSIS_
		//textColor(DATA);
        printf("\nAfter Address Dev Cmd - slotContext.usbDeviceAddress: %u", x->devContextPtr[slotNr-1]->slotContext.usbDeviceAddress);
		//textColor(TEXT);
      #endif

        return x->devContextPtr[slotNr-1]->slotContext.usbDeviceAddress;
    }

    else if (req == 9) // SET_CONFIGURE
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci: SET CONFIGURE");
      #endif
        xhci_Configure_Endpoint_Command(x, slotNr, false);
        xhci_ringDoorbellAndWait_HC(x);

        // wait for xHC Configured State (3)
        if (x->devContextPtr[slotNr-1]->slotContext.slotState < 3)
        {
            printf("\nSwitching to xHC Configured State failed! Current xHC State: %u", x->slots[slotNr-1]->slotState);
        }

      #ifdef _XHCI_DIAGNOSIS_
        xhci_showStates(x, slotNr, slotNr-1);
        xhci_endpointContext_t* pEPo = x->devContextPtr[slotNr-1]->endpointContext;
        for (uint8_t i=0; i<5;i++)
        {
            //printf("\nEP %u Context-Input:  type %u TRDeq %X dcs %u", i, pEP[i].epType,  pEP[i].TRdequeuePtrLo<<4,  pEP[i].dcs);
            printf("\nEP %u Context-Output: type %u TRDeq %X dcs %u", i, pEPo[i].epType, pEPo[i].TRdequeuePtrLo<<4, pEPo[i].dcs);
        }
      #endif
    }

	uTransaction->data = malloc(sizeof(xhci_transaction_t));
	xhci_transaction_t* xTransaction = (xhci_transaction_t*)(uTransaction->data);
    xTransaction->inBuffer = 0;
    xTransaction->inLength = 0;
    xTransaction->TDBuffer = 0;

    xhci_xfer_SetupStageTRB_t TD = { 0 };
    TD.TRBtype = TRB_TYPE_SETUP_STAGE;
    TD.transferLength = 8;
    TD.IOC = 0; // We want an interrupt only from last transaction
    TD.IDT = 1; // This bit shall be set to ‘1’ in a Setup Stage TRB. It specifies that the Parameter component of this TRB contains Setup Data.

    TD.bmRequestType = type;
    TD.bRequest = req;
    TD.wValue = (hiVal << 8) | loVal;
    TD.wIndex = index;
    TD.wLength = length;

    // EP0
    uint8_t DCI = calculateDCI(transfer->endpoint);

    TD.cycle = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;

    TD.IntTarget = 0;
    TD.TRT = 3; // IN Data Stage (assumed. Fixed in xhci_outTransaction, if wrong)

    xTransaction->TD = xhci_enqueueTransfer(x, (xhci_xfer_NormalTRB_t*)&TD, slotNr, DCI);

    return loVal;
}

void xhci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, void* buffer, size_t length, uint16_t remainingIn)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_inTransaction len=%u remaining=%u", length, remainingIn);
  #endif

    if ((uintptr_t)transfer->data == 5) // SET_ADDRESS
        return;

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data; // TODO: check
    uint8_t slotNr = x->portSlotLink[portNr - 1].slotNr;
    if (slotNr == 0xFF || slotNr == 0x00) // no valid slot
        return;

    uTransaction->data = (usb_transaction_t*)malloc(sizeof(xhci_transaction_t));
	xhci_transaction_t* xTransaction = (xhci_transaction_t *)uTransaction->data;
    xTransaction->inBuffer = buffer;
    xTransaction->inLength = length;
    xTransaction->TDBuffer = 0;

    xhci_xfer_DataStageTRB_t TD = { 0 };
    TD.DIR = 1; // 0 = OUT, 1 = IN
    TD.IOC = 0; // We want an interrupt only from last transaction
    TD.ISP = 1;
    TD.CH  = 0;
    TD.IDT = 0;
    TD.IntTarget = 0;
    TD.TDsize = remainingIn;
    TD.transferLength = length;
    TD.NS = 0;
    TD.ENT = 0;

    if (buffer != 0) // In transaction
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci_inTransaction - In transaction");
      #endif

        TD.TRBtype = TRB_TYPE_DATA_STAGE;

        xTransaction->TDBuffer = malloc(length); // Alignment see section 6.4.1.1

        TD.dataBufferPtrLo = Syscall_GetPAFromVM(xTransaction->TDBuffer);
        TD.dataBufferPtrHi = 0;
    }
    else // Status transaction
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci_inTransaction - Status transaction");
      #endif

        TD.TRBtype = TRB_TYPE_STATUS_STAGE;
    }

    uint8_t DCI = calculateDCI(transfer->endpoint);
    TD.cycle = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;
    xTransaction->TD = xhci_enqueueTransfer(x, (xhci_xfer_NormalTRB_t*)&TD, slotNr, DCI);
}

void xhci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, const void* buffer, size_t length, uint16_t remainingOut)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_outTransaction");
  #endif

    if ((uintptr_t)transfer->data == 5) // SET_ADDRESS
        return;

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data;
    uint8_t slotNr = x->portSlotLink[portNr - 1].slotNr;
    if (slotNr == 0xFF || slotNr == 0x00) // no valid slot
        return;

	uTransaction->data = malloc(sizeof(xhci_transaction_t));
	xhci_transaction_t* xTransaction = (xhci_transaction_t*)(uTransaction->data);
    xTransaction->inBuffer = 0;
    xTransaction->inLength = 0;
    xTransaction->TDBuffer = 0;

    if (transfer->transactions.tail)
    {
        usb_transaction_t* prevTransaction = (usb_transaction_t*)(transfer->transactions.tail->data);
        if (prevTransaction->type == USB_TT_SETUP) // Control followed by Out means: OUT Data Stage
        {
            xhci_transaction_t* prevXTransaction = (xhci_transaction_t*)(prevTransaction->data);
            ((xhci_xfer_SetupStageTRB_t*)prevXTransaction->TD)->TRT = 2; // OUT
        }
    }

    xhci_xfer_DataStageTRB_t TD = { 0 };
    TD.DIR = 0; // 0 = OUT, 1 = IN
    TD.IOC = 0; // We want an interrupt only from last transaction
    TD.ISP = 1;
    TD.CH  = 0;
    TD.IDT = 0;
    TD.IntTarget = 0;
    TD.TDsize = remainingOut;

    if (buffer != 0 && length != 0) // Out transaction
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci_outTransaction - Out transaction");
      #endif
        xTransaction->TDBuffer = malloc(length); // Alignment see section 6.4.1.1
        memcpy(xTransaction->TDBuffer, buffer, length);

        TD.TRBtype = TRB_TYPE_DATA_STAGE;
        TD.transferLength = length;
        TD.dataBufferPtrLo = Syscall_GetPAFromVM(xTransaction->TDBuffer);
    }
    else // Status transaction
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\nxhci_outTransaction - Status transaction");
      #endif

        TD.TRBtype = TRB_TYPE_STATUS_STAGE;
    }

    uint8_t DCI = calculateDCI(transfer->endpoint);
    TD.cycle = x->slots[slotNr-1]->endpoints[DCI-1].TransferRingProducerCycleState;
    xTransaction->TD = xhci_enqueueTransfer(x, (xhci_xfer_NormalTRB_t*)&TD, slotNr, DCI);
}

void xhci_scheduleTransfer(usb_transfer_t* transfer)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_scheduleTransfer");
  #endif

    if ((uintptr_t)transfer->data == 5) // SET_ADDRESS
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\tleft at once due to transfer->data == 5 <== SET_ADDRESS");
      #endif

      return;
    }

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t) (size_t) ((hc_port_t*) transfer->device->port->data)->data;
    uint8_t slotNr = x->portSlotLink[portNr-1].slotNr;

    if (slotNr == 0xFF || slotNr == 0x00) // no valid slot
    {
        transfer->success = false;
        printf("\nNo valid slot!");
        return;
    }

    uint8_t DCI = calculateDCI(transfer->endpoint);

    // Request interrupt from last transaction
    usb_transaction_t* lastTransaction = (usb_transaction_t*)(transfer->transactions.tail->data);
    xhci_transaction_t* lastXTransaction = (xhci_transaction_t*)(lastTransaction->data);
    lastXTransaction->TD->IOC = 1;

    //irq_resetCounter(x->irq);
    x->slots[slotNr-1]->endpoints[DCI-1].pendingTransfer = true;
    xhci_setEnqueueTransferPtr(x, slotNr, DCI);

  #ifdef _XHCI_DIAGNOSIS_
    if (transfer->type == USB_CONTROL) printf("\nDoorbell Device at slotNr.: %u DCI(ctrl): %u", slotNr, DCI);
    if (transfer->type == USB_BULK)    printf("\nDoorbell Device at slotNr.: %u DCI(bulk): %u type: %s",
                                                slotNr, DCI, "TODO: correct code!"/*transfer->device->endpoints[transfer->endpoint]->type ? "IN" : "OUT"*/);
  #endif

    x->slots[slotNr-1]->endpoints[DCI-1].timeTransfer = Syscall_GetTickCount();
    xhci_ringDoorbell_Device(x, slotNr, DCI, 0);
}

bool xhci_pollTransfer(usb_transfer_t* transfer)
{
    return false; // Not implemented
}

void xhci_waitForTransfer(usb_transfer_t* transfer)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_waitForTransfer");
  #endif

    if ((uintptr_t)transfer->data == 5) // SET_ADDRESS
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("\tleft at once due to transfer->data == 5 <== SET_ADDRESS");
      #endif

      return;
    }

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data;
    uint8_t slotNr = x->portSlotLink[portNr - 1].slotNr;
    if (slotNr == 0xFF || slotNr == 0x00) // no valid slot
        return;
    uint8_t DCI = calculateDCI(transfer->endpoint);

    transfer->success = true;
    uint16_t timeoutCounter = 75;

    while (x->slots[slotNr-1]->endpoints[DCI-1].pendingTransfer)
    {
        if (timeoutCounter>0)
        {
			Syscall_Sleep(20);
            //waitForIRQ(x->irq, 20);
            //irq_resetCounter(x->irq);
        }
        else
        {
            printf("\nTimeout Error: waiting for transfer completion.");
            transfer->success = false;
            break;
        }
        timeoutCounter--;
    }

    if (!((x->slots[slotNr-1]->endpoints[DCI-1].transferError ==  1) ||  // success
          (x->slots[slotNr-1]->endpoints[DCI-1].transferError == 13) ||  // short packet
          (x->slots[slotNr-1]->endpoints[DCI-1].transferError == 21)))   // event ring full
    {
        transfer->success = false;
    }
    else if (x->slots[slotNr-1]->endpoints[DCI-1].transferError == 21)
    {
        printf("\nevent ring full");
    }

    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        xhci_transaction_t* transaction = (xhci_transaction_t*)(((usb_transaction_t*) elem->data)->data);

        if (transaction->inBuffer != 0 && transaction->inLength != 0)
        {
          #ifdef _XHCI_DIAGNOSIS_
			//textColor(GREEN);
            printf("\nIN transaction->inLength: ");
			//textColor(IMPORTANT);
            printf("%u ", transaction->inLength);
			//textColor(TEXT);
          #endif
            memcpy(transaction->inBuffer, transaction->TDBuffer, transaction->inLength);
        }
    }
}

void xhci_destructTransfer(usb_transfer_t* transfer)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\nxhci_destructTransfer");
  #endif

    if ((uintptr_t)transfer->data == 5) // SET_ADDRESS
        return;

    xhci_t* x = (xhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    uint8_t portNr = (uint8_t)(size_t)((hc_port_t*)transfer->device->port->data)->data;
    uint8_t slotNr = x->portSlotLink[portNr - 1].slotNr;
    uint8_t DCI = calculateDCI(transfer->endpoint);

    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        xhci_transaction_t* transaction = (xhci_transaction_t *)(((usb_transaction_t*)elem->data)->data);
        free(transaction->TDBuffer);
        free(transaction);
    }

  #ifdef _XHCI_DIAGNOSIS_
    printf("\nafter Xfer: ");
    xhci_showStates(x, slotNr, slotNr-1);
    printf("\nDCI: %u transfer: %u ms event: %u ms", DCI, x->slots[slotNr-1]->endpoints[DCI-1].timeTransfer, x->slots[slotNr-1]->endpoints[DCI-1].timeEvent);
  #endif

    x->slots[slotNr-1]->endpoints[DCI-1].timeTransfer = x->slots[slotNr-1]->endpoints[DCI-1].timeEvent = 0; // reset
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Helpers                                                   *
*                                                                                                      *
*******************************************************************************************************/

static void xhci_analyze(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
    printf("\n--------------");
    printf("\nxHCI bar physical address: %Xh", (uintptr_t)Syscall_GetPAFromVM(x->bar));
    printf("\nHCIVERSION:  %y.%y", BYTE2(x->CapRegs->hciversion), BYTE1(x->CapRegs->hciversion)); // Interface Version Number
    printf("\nHCSPARAMS 1: %Xh", x->CapRegs->hcsparams1);             // Structural Parameters
    printf("\nHCSPARAMS 2: %Xh", x->CapRegs->hcsparams2);             // Structural Parameters
    printf("\tHCSPARAMS 3: %Xh", x->CapRegs->hcsparams3);             // Structural Parameters
    printf("\nPorts:       %u",  x->hc.rootPortCount);                // Number of Ports
    printf("\nHCCPARAMS:   %Xh", x->CapRegs->hccparams1);             // Capability Parameters
    printf("\nOpRegs Address: %Xh", x->OpRegs);                       // Host Controller Operational Registers
    printf("\nRuntime Base: %Xh", x->RuntimeRegs);                    // Host Controller Runtime Registers
    printf("\n--------------\n");
  #endif
}

/*
static void xhci_showPortStatus(xhci_t* x, uint8_t j) // TODO: clarify: ports or slots???
{
    printf("  Port: %u PORTSC: %X\nCCS:%u CSC:%u PED:%u PEC:%u PP:%u PLS:%u PLC:%u WRC:%u PRC:%u",
            j+1, x->OpRegs->PortReg[j].portsc,
            (x->OpRegs->PortReg[j].portsc & PORT_CCS) ? 1 : 0,  // Current Connect Status
            (x->OpRegs->PortReg[j].portsc & PORT_CSC) ? 1 : 0,  // Connect Status Change, ‘1’ = Change in CCS. ‘0’ = No change.
            (x->OpRegs->PortReg[j].portsc & PORT_PED) ? 1 : 0,  // Port Enabled/Disabled, ‘1’ = Change in PED. ‘0’ = No change.
            (x->OpRegs->PortReg[j].portsc & PORT_PEC) ? 1 : 0,  // Port Enabled Change
            (x->OpRegs->PortReg[j].portsc & PORT_PP)  ? 1 : 0,  // Port Power
            (x->OpRegs->PortReg[j].portsc >> 5) & 0xF,          // Port Link State
            (x->OpRegs->PortReg[j].portsc & PORT_PLC) ? 1 : 0,  // Port Link State Change
            (x->OpRegs->PortReg[j].portsc & PORT_WRC) ? 1 : 0,  // Warm Port Reset Change
            (x->OpRegs->PortReg[j].portsc & PORT_PRC) ? 1 : 0); // Port Reset Change
}

static void xhci_showSlotStatus(xhci_t* x, uint8_t firstPort, uint8_t lastPort)
{
    for (uint8_t j = firstPort-1; j<lastPort; j++)
    {
        uint8_t slotNr = x->portSlotLink[j].slotNr;
        xhci_slotContext_t* pSlot = &(x->devContextPtr[slotNr-1]->slotContext);

        printf("\nPort: %u  Slot: %u  Addr(slot): %u  Addr(usb): %u  slotState: %u",
               j+1,
               slotNr,
               pSlot->usbDeviceAddress,
               x->hc.rootPorts->device->num,
               pSlot->slotState);
    }
}
*/

static void xhci_showStatus(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
	//textColor(HEADLINE);
    printf("\nxHCI status: ");
	//textColor(IMPORTANT);
    printf("%Xh", x->OpRegs->status);
  #endif
	//textColor(ERROR);
    if (x->OpRegs->status & STS_HCH)  { printf("\nStatus: HC Halted");                      x->OpRegs->status |= STS_HCH; }
    if (x->OpRegs->status & STS_HSE)  { printf("\nStatus: Host System Error");              x->OpRegs->status |= STS_HSE; }
    if (x->OpRegs->status & STS_HCE)  { printf("\nStatus: Internal Host Controller Error"); x->OpRegs->status |= STS_HCE; }
    if (x->OpRegs->status & STS_SRE)  { printf("\nStatus: Save/Restore Error");             x->OpRegs->status |= STS_SRE; }
	//textColor(IMPORTANT);
    if (x->OpRegs->status & STS_EINT) { printf("\nStatus: Event Interrupt");                x->OpRegs->status |= STS_EINT;}
    if (x->OpRegs->status & STS_PCD)  { printf("\nStatus: Port Change Detect");             x->OpRegs->status |= STS_PCD; }
    if (x->OpRegs->status & STS_SSS)  { printf("\nStatus: Save State Status");              x->OpRegs->status |= STS_SSS; }
    if (x->OpRegs->status & STS_RSS)  { printf("\nStatus: Restore State Status");           x->OpRegs->status |= STS_RSS; }
    if (x->OpRegs->status & STS_CNR)  { printf("\nStatus: Controller Not Ready");           x->OpRegs->status |= STS_CNR; }
	//textColor(TEXT);
}

static void xhci_showStates(xhci_t* x, uint8_t slotNr, uint8_t portNr)
{
	//textColor(IMPORTANT);
    // EP state
    // uint8_t DCI = ((endpoint + 1) * 2)-1; // Control EP
    // uint8_t DCI_bulk_out = endpoint * 2;
    // uint8_t DCI_bulk_in  = DCI_bulk_out + 1;

    xhci_endpointContext_t* pEP = x->devContextPtr[slotNr-1]->endpointContext;
    printf("\nslot: %u, states EP0: %u EP1: %u %u EP2: %u %u",
        slotNr, pEP[0].epState, pEP[1].epState, pEP[2].epState, pEP[3].epState, pEP[4].epState);

    // Port PLS
    uint8_t pls = ((x->OpRegs->PortReg[portNr].portsc >> 5) & 0xF);
    printf(" pls: %u ", pls);

    //  PLS - Read Value Meaning
    //  0 Link is in the U0 State                   1 Link is in the U1 State
    //  2 Link is in the U2 State                   3 Link is in the U3 State (Device Suspended)
    //  4 Link is in the Disabled State             5 Link is in the RxDetect State
    //  6 Link is in the Inactive State             7 Link is in the Polling State
    //  8 Link is in the Recovery State             9 Link is in the Hot Reset State
    // 10 Link is in the Compliance Mode State     11 Link is in the Test Modei State  12-14 Reserved
    // 15 Link is in the Resume State

    // slotState
    xhci_slotContext_t* pSlot = &(x->devContextPtr[slotNr-1]->slotContext);
    printf("slot st.: %u, ", pSlot->slotState);
	//textColor(TEXT);
}

static void showCounter(xhci_t* x, uint8_t slotNr)
{
    if (slotNr)
    {
        /*writeInfo(0, "slot: %u Cmd: %u, Evt0: %u, Xfer: %u  %u %u  %u %u",
            slotNr,
            x->CmdCounter,
            x->EvtCounter,
            x->slots[slotNr-1]->endpoints[0].TransferCounter,
            x->slots[slotNr-1]->endpoints[1].TransferCounter,
            x->slots[slotNr-1]->endpoints[2].TransferCounter,
            x->slots[slotNr-1]->endpoints[3].TransferCounter,
            x->slots[slotNr-1]->endpoints[4].TransferCounter);*/
    }
    else // xHC (slotNr == 0)
    {
       // writeInfo(1, "xHC Cmd: %u Evt0: %u", x->CmdCounter, x->EvtCounter);
    }
}

static uint8_t calculateDCI(usb_endpoint_t* endpoint)
{
    if (endpoint->dir == EP_BIDIR || endpoint->dir == EP_IN) // Assumption: EP_BIDIR calculated like EP_IN
        return endpoint->address * 2 + 1;
    if (endpoint->dir == EP_OUT)
        return endpoint->address * 2;
    return (0); // Should not happen
}

void xhci_showCompletionCodeError(uint8_t CompletionCode)
{
    // 6.4.5 TRB Completion Codes
    static const char* completionCodes[] =
    {
        "Invalid              ",
        "",
        "Data Buffer          ",
        "Babble Detected      ",
        "USB Transaction      ",
        "TRB                  ",
        "Stall                ",
        "Resource             ",
        "Bandwidth            ",
        "No Slots Available   ",
        "Invalid Stream Type  ",
        "Slot Not Enabled     ",
        "EP Not Enabled       ",
        "Short Packet         ",
        "Ring Underrun        ",
        "Ring Overrun         ",
        "VF Event Ring Full   ",
        "Parameter            ",
        "Bandwidth Overrun    ",
        "Context State        ",
        "No Ping Response     ",
        "Event Ring Full      ",
        "Incompatible Device  ",
        "Missed Service       ",
        "Command Ring Stopped ",
        "Command Aborted      ",
        "Stopped              ",
        "Stop: Length Invalid ",
        "Reserved             ",
        "MaxExitLat Too Large ",
        "Reserved             ",
        "Isoch Buffer Overrun ",
        "Event Lost           ",
        "Undefined            ",
        "Invalid Stream ID    ",
        "Secondary Bandwidth  ",
        "Split Transaction    "
    };

    if (CompletionCode == 1)
    {
      #ifdef _XHCI_DIAGNOSIS_
		//textColor(SUCCESS);
		printf("\nSuccess              ");
		//textColor(TEXT);
      #endif
        return;
    }
    printf("\n");
	//textColor(ERROR);
    if (CompletionCode < 37)
		printf(completionCodes[CompletionCode]);
    else
		printf("???                  ");
	//textColor(TEXT);
}

#ifdef _XHCI_DIAGNOSIS_
static void xhci_showTRBType(uint8_t trbType)
{
	//textColor(IMPORTANT);
    switch (trbType)
    {
        case  0: printf(" RESERVED        ");  break;
        case  1: printf(" NORMAL          ");  break;
        case  2: printf(" SETUP_STAGE     ");  break;
        case  3: printf(" DATA_STAGE      ");  break;
        case  4: printf(" STATUS_STAGE    ");  break;
        case  5: printf(" ISOCH           ");  break;
        case  6: printf(" LINK            ");  break;
        case  7: printf(" EVENT_DATA      ");  break;
        case  8: printf(" NOOP            ");  break;
        case  9: printf(" ENABLE_SLOT     ");  break;
        case 10: printf(" DISABLE_SLOT    ");  break;
        case 11: printf(" ADDRESS_DEVICE  ");  break;
        case 12: printf(" CONFIGURE_EP    ");  break;
        case 13: printf(" EVALUATE_CTX    ");  break;
        case 14: printf(" RESET_EP        ");  break;
        case 15: printf(" STOP_EP         ");  break;
        case 16: printf(" SET_TR_DEQUEUE  ");  break;
        case 17: printf(" RESET_DEVICE    ");  break;
        case 18: printf(" FORCE_EVENT     ");  break;
        case 19: printf(" NEGOTIATE_BW    ");  break;
        case 20: printf(" SET_LATENCY_TOL ");  break;
        case 21: printf(" GET_PORT_BW     ");  break;
        case 22: printf(" FORCE_HEADER    ");  break;
        case 23: printf(" NOOP_CMD        ");  break;
        default: printf(" ???             ");  break;
    }
	//textColor(TEXT);
}
#endif

static void xhci_deactivateLegacySupport(xhci_t* x)
{
    uint16_t xECP = ((x->CapRegs->hccparams1 >> 16) & 0xFFFF) << 2;

  #ifdef _XHCI_DIAGNOSIS_
    printf("\nDeactivateLegacySupport: xECP = %xh\n", xECP);
  #endif

    // xECP: This field identifies the extended capability. Base + (xECP << 2) provides beginning of the first extended capablity.
    if (xECP != 0x00)
    {
        uint8_t xECP_id=0;

        while (xECP) // 00h indicates end of the ext. cap. list.
        {
            xECP_id = *(uint8_t*)((char*)x->bar + xECP);
          #ifdef _XHCI_DIAGNOSIS_
            printf("xECP_id = %yh, ", xECP_id);
          #endif
            if (xECP_id == 1)
            {
                break;
            }
            uint8_t offset = *(uint8_t*)((char*)x->bar + xECP + 1);
          #ifdef _XHCI_DIAGNOSIS_
            printf("offset = %yh\n", xECP_id);
          #endif
            if (!offset)
                xECP = 0;
            else
                xECP += offset << 2;
        }
        uint16_t BIOSownedSemaphore = xECP + 2; // R/W - only Bit 16 (Bit 23:17 Reserved, must be set to zero)
        uint16_t OSownedSemaphore = xECP + 3;   // R/W - only Bit 24 (Bit 31:25 Reserved, must be set to zero)
        uint16_t USBLEGCTLSTS = xECP + 4;       // USB Legacy Support Control/Status

        // Legacy-Support-EC found? BIOS-Semaphore set?
        if (xECP_id == 1 && *(uint8_t*)((char*)x->bar + BIOSownedSemaphore) & 0x01)
        {
          #ifdef _XHCI_DIAGNOSIS_
            printf("\nSet OS-Semaphore: ");
          #endif
            *(uint8_t*)((char*)x->bar + OSownedSemaphore) = 0x01;

            WAIT_FOR_CONDITION((*(uint8_t*)((char*)x->bar + BIOSownedSemaphore) & 0x01) == 0, 250, 10, "\nBIOS-Semaphore still set.");
            WAIT_FOR_CONDITION((*(uint8_t*)((char*)x->bar + OSownedSemaphore) & 0x01) != 0, 250, 10, "\nOS-Semaphore still cleared.");

          #ifdef _XHCI_DIAGNOSIS_
            printf("\nCheck: BIOSownedSemaphore: %u OSownedSemaphore: %u",
                *(uint8_t*)((char*)x->bar + BIOSownedSemaphore), *(uint8_t*)((char*)x->bar + OSownedSemaphore));
          #endif

            // USB SMI Enable R/W. 0=Default. The OS tries to set SMI to disabled in case that BIOS bit stays at one.
            *(uint32_t*)((char*)x->bar + USBLEGCTLSTS) = 0x00000000; // USB SMI disabled
        }
      #ifdef _XHCI_DIAGNOSIS_
        else
        {
			//textColor(SUCCESS);
            printf("\nBIOS did not own the XHCI. No action needed.");
			//textColor(TEXT);
        }
      #endif
    }
  #ifdef _XHCI_DIAGNOSIS_
    else
    {
        printf("\nNo valid xECP found.");
    }
  #endif
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Interrupt Handler                                         *
*                                                                                                      *
*******************************************************************************************************/

static void xhci_handler(pciDev_t* device)
{
    // Check if an xHCI controller issued this interrupt
    xhci_t* x = (xhci_t *)device->data;
    bool found = false;
    for (dlelement_t* el = xhci.head; el != 0; el = el->next)
    {
        if (el->data == x)
        {
            found = true;
            break;
        }
    }

    if (!found || x == 0) // Interrupt did not come from xHCI device
    {
      #ifdef _XHCI_DIAGNOSIS_
        printf("Interrupt did not come from xHCI device!\n");
      #endif
        return;
    }

    uint32_t val = x->OpRegs->status;

    if ((val & STS_INTMASK) == 0) // Interrupt came from another xHCI device
        return;

    // Software sets a bit to ‘0’ in this register by writing a ‘1’ to it (RW1C).
    x->OpRegs->status = val; // reset interrupt

    if (val & STS_HCH)
    {
        printf("\nxhci-handler: HC Halted (RO)");
    }

    if (val & STS_HSE)
    {
		
        printf("\nxhci-handler: Host System Error");
		pci_analyzeHostSystemError(x->PCIdevice);
    }

    if (val & STS_EINT)
    {
      #ifdef _XHCI_DIAGNOSIS_
        static uint32_t count = 0;
        count++;

		//textColor(LIGHT_BLUE);
        printf("\nxhci-handler: Event Interrupt No. %u", count);
		//textColor(TEXT);
      #endif
        xhci_parseEvents(x);
    }

    if (val & STS_PCD)
    {
		//textColor(LIGHT_BLUE);
        printf("\nxhci-handler: Port Change Detect");
		//textColor(TEXT);
        if (!(val & STS_HCH))
        {
            xhci_portCheck(x);
        }
        else
        {
            printf(" - not reset");
        }
    }

    if (val & STS_SSS)
    {
		//textColor(LIGHT_BLUE);
        printf("xhci-handler: Save State Status (RO)");
		//textColor(TEXT);
    }

    if (val & STS_RSS)
    {
		//textColor(LIGHT_BLUE);
        printf("xhci-handler: Restore State Status (RO)");
		//textColor(TEXT);
    }

    if (val & STS_SRE)
    {
		//textColor(LIGHT_BLUE);
        printf("xhci-handler: Save/Restore Error");
		//textColor(TEXT);
    }

    if (val & STS_CNR)
    {
		//textColor(LIGHT_BLUE);
        printf("xhci-handler: Controller not ready (RO)");
		//textColor(TEXT);
    }

    if (val & STS_HCE)
    {
		//textColor(LIGHT_BLUE);
        printf("xhci-handler: Host Controller Error (RO)");
		//textColor(TEXT);
    }
}

void xhci_pollDisk(port_t* dev)
{
    xhci_t* x = (xhci_t*)((hc_port_t*)dev->data)->hc;
    uint32_t val = x->OpRegs->status;

    if (val & STS_PCD)
    {
		//textColor(LIGHT_BLUE);
        printf("\nxhci_pollDisk: Port Change Detect");
		//textColor(TEXT);

        if (!(val & STS_HCH))
        {
            xhci_portCheck(x);
            x->OpRegs->status |= STS_PCD; // reset STS_PCD
        }
        else
        {
            printf("- but xHC is not running!");
        }
    }
}

/*
* Copyright (c) 2013-2016 The PrettyOS Project. All rights reserved.
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
