/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f?r die Verwendung dieses Sourcecodes siehe unten
*/
#include <minwindef.h>
#include <stdio.h>
#include <memory.h>
#include "uhci.h"
#include <string.h>
#include <systemcall_impl.h>
#include <InterruptHandler.h>
#include "UHCIHandler.h"

#define PAGE_SIZE 4096 

//20191112
#define WAIT_FOR_CONDITION(condition, runs, wait, message)\
    for (unsigned int timeout_ = 0; !(condition); timeout_++) {\
        if (timeout_ >= runs) {\
             printf(message);\
             break;\
        }\
        Syscall_Sleep(wait);\
    }

static list_t uhci = list_init();

DWORD WINAPI uhci_start(LPVOID parameter);
//static void uhci_start(uhci_t* u);
static void uhci_analysePortStatus(uhci_t* u, uint8_t j, uint16_t val);
static void uhci_handler(pciDev_t* device);
static void uhci_initHC(uhci_t* u);
static void uhci_resetHC(uhci_t* u);
static void uhci_enablePorts(uhci_t* u);
static void uhci_resetPort(uhci_t* u, uint8_t port);
static uhciTD_t* uhci_createTD_SETUP(hc_port_t* port, uintptr_t next, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length, uint32_t device, uint8_t endpoint);
static uhciTD_t* uhci_createTD_IO(hc_port_t* port, uintptr_t next, uint8_t direction, bool toggle, uint16_t tokenBytes, uint32_t device, uint8_t endpoint, void* buffer, size_t duplicates);
static void      uhci_fillQH(uhci_t* u, uhciQH_t* head, const uhciTD_t* firstTD, bool terminate);


void uhci_install(pciDev_t* PCIdev)
{
  #ifdef _UHCI_DIAGNOSIS_
    printf("\n>>>uhci_install<<<");
  #endif

    static uint8_t numHC = 0;
    dlelement_t* elem = list_alloc_elem(sizeof(uhci_t), "uhci");
    uhci_t* u = (uhci_t *)elem->data;
    u->PCIdevice  = PCIdev;
    u->PCIdevice->data = u;
    u->num = numHC++;
    u->enabledPortFlag = false;
    u->periodicScheduled = false;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (PCIdev->bar[i].memoryType == PCI_IO)
        {
            u->bar = PCIdev->bar[i].baseAddress;
            u->memSize = PCIdev->bar[i].memorySize;
            break;
        }
    }

    list_append_elem(&uhci, elem);

    char str[10];
    snprintf(str, 10, "UHCI %u", u->num+1);
//20191112
	Syscall_CreateThread(uhci_start, "uhci_start", u, 16, 0);
    //task_t* thread = create_cthread((void*)&uhci_start, str);
    //task_passStackParameter(thread, &u, sizeof(u));
    //scheduler_insertTask(thread);
}

DWORD WINAPI uhci_start(LPVOID parameter)
//static void uhci_start(uhci_t* u)
{
  #ifdef _UHCI_DIAGNOSIS_
    printf("\n>>>startUHCI<<<\n");
  #endif
	uhci_t* u = (uhci_t *)parameter;
    uhci_initHC(u);

    printf("\n\n>>> Press key to close this console. <<<");
    //getch();
	Syscall_GetChar();

	return 0;
}

static void uhci_initHC(uhci_t* u)
{
   // textColor(HEADLINE);
    printf("Initialize UHCI Host Controller:");
    //textColor(TEXT);

    // prepare PCI command register
    uint16_t pciCommandRegister = pci_configRead(u->PCIdevice, PCI_COMMAND, 2);
    pci_configWrite_word(u->PCIdevice, PCI_COMMAND, pciCommandRegister | PCI_CMD_IO | PCI_CMD_BUSMASTER); // resets status register, sets command register

  #ifdef _UHCI_DIAGNOSIS_
    printf("\nPCI Command Register before:          %xh", pciCommandRegister);
    printf("\nPCI Command Register plus bus master: %xh", pci_configRead(u->PCIdevice, PCI_COMMAND, 2));
 #endif
	//20191112
    //irq_installPCIHandler(u->PCIdevice->irq, uhci_handler, u->PCIdevice);
	
	InterruptHandler* pHandler = new UHCIHandler(u->PCIdevice->irq, uhci_handler, u->PCIdevice, "uhci");
	Syscall_kObserveInterrupt(u->PCIdevice->irq, pHandler);
    uhci_resetHC(u);
}

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

static void uhci_resetHC(uhci_t* u)
{
  #ifdef _UHCI_DIAGNOSIS_
    printf("\n\n>>>uhci_resetHostController<<<\n");
  #endif

    // http://www.lowlevel.eu/wiki/Universal_Host_Controller_Interface#Informationen_vom_PCI-Treiber_holen

    uint16_t legacySupport = pci_configRead(u->PCIdevice, UHCI_PCI_LEGACY_SUPPORT, 2);

	
    Syscall_OutPortWord(u->bar + UHCI_USBCMD, UHCI_CMD_GRESET);
	Syscall_Sleep(1);
	//sleepMilliSeconds(100); // at least 50 msec
    Syscall_OutPortWord(u->bar + UHCI_USBCMD, 0);

    // get number of valid root ports
    uint8_t rootPortCount = (u->memSize - UHCI_PORTSC1) / 2; // each port has a two byte PORTSC register
    for (uint8_t i=2; i<rootPortCount; i++)
    {
        uint16_t portVal = Syscall_InPortDWord(u->bar + UHCI_PORTSC1 + i * 2);
        if (((portVal & UHCI_PORT_VALID) == 0) || // reserved bit 7 is already read as 1
             (portVal == 0xFFFF))
        {
            rootPortCount = i;
            break;
        }
    }

    if (rootPortCount > UHCIPORTMAX)
    {
        rootPortCount = UHCIPORTMAX;
    }

    hc_constructRootPorts(&u->hc, rootPortCount, &USB_UHCI);

  #ifdef _UHCI_DIAGNOSIS_
    textColor(IMPORTANT);
    printf("\nUHCI root ports: %u", u->hc.rootPortCount);
    textColor(TEXT);
  #endif

    uint16_t uhci_usbcmd = Syscall_InPortWord(u->bar + UHCI_USBCMD);
    if ((legacySupport & ~(UHCI_PCI_LEGACY_SUPPORT_STATUS | UHCI_PCI_LEGACY_SUPPORT_NO_CHG | UHCI_PCI_LEGACY_SUPPORT_PIRQ)) ||
         (uhci_usbcmd & UHCI_CMD_RS)   ||
         (uhci_usbcmd & UHCI_CMD_CF)   ||
        !(uhci_usbcmd & UHCI_CMD_EGSM) ||
         (Syscall_InPortDWord(u->bar + UHCI_USBINTR) & UHCI_INT_MASK))
    {
        Syscall_OutPortWord(u->bar + UHCI_USBSTS, UHCI_STS_MASK);    // reset all status bits
        //sleepMilliSeconds(1);                             // wait one frame
		Syscall_Sleep(1);
		pci_configWrite_word(u->PCIdevice, UHCI_PCI_LEGACY_SUPPORT, UHCI_PCI_LEGACY_SUPPORT_STATUS); // resets support status bits in Legacy support register
        Syscall_OutPortWord(u->bar + UHCI_USBCMD, UHCI_CMD_HCRESET); // reset hostcontroller

        WAIT_FOR_CONDITION(!(Syscall_InPortWord(u->bar + UHCI_USBCMD) & UHCI_CMD_HCRESET), 50, 10, "USBCMD_HCRESET timed out!");

        Syscall_OutPortWord(u->bar + UHCI_USBINTR, 0); // switch off all interrupts
        Syscall_OutPortWord(u->bar + UHCI_USBCMD,  0); // switch off the host controller

        for (uint8_t i=0; i<u->hc.rootPortCount; i++) // switch off the valid root ports
        {
            Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + i*2, 0);
        }
    }

    // We work with a single QH for bulk and control transfers
    uhciQH_t* qh = (uhciQH_t *)malloc_aligned(sizeof(uhciQH_t), 16);
	
    uhci_fillQH(u, qh, 0, true);
    u->qhPointerVirt = qh;

    // frame list
    u->framelistAddrVirt = (frPtr_t*)malloc_aligned(PAGE_SIZE, PAGE_SIZE);

	//memsetl(u->framelistAddrVirt->frPtr, paging_getPhysAddr(qh) | BIT_QH, 1024);
	
	unsigned int pa = Syscall_GetPAFromVM(qh);
	memsetl(u->framelistAddrVirt->frPtr, pa | BIT_QH, 1024);

    // define each millisecond one frame, provide physical address of frame list, and start at frame 0
    Syscall_OutPortByte(u->bar + UHCI_SOFMOD, 0x40); // SOF cycle time: 12000. For a 12 MHz SOF counter clock input, this produces a 1 ms Frame period.

	pa =Syscall_GetPAFromVM((void*)u->framelistAddrVirt->frPtr);
    Syscall_OutPortDWord(u->bar + UHCI_FRBASEADD, pa);
    Syscall_OutPortWord(u->bar + UHCI_FRNUM, 0);

    // set PIRQ
    pci_configWrite_word(u->PCIdevice, UHCI_PCI_LEGACY_SUPPORT, UHCI_PCI_LEGACY_SUPPORT_PIRQ);

    // start hostcontroller and mark it configured with a 64-byte max packet
    Syscall_OutPortWord(u->bar + UHCI_USBCMD, UHCI_CMD_RS | UHCI_CMD_CF | UHCI_CMD_MAXP);
    Syscall_OutPortWord(u->bar + UHCI_USBINTR, UHCI_INT_MASK);  // switch on all interrupts

    for (uint8_t i=0; i<u->hc.rootPortCount; i++) // reset the CSC of the valid root ports
    {
        Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + i*2, UHCI_PORT_CS_CHANGE);
    }

    Syscall_OutPortWord(u->bar + UHCI_USBCMD, UHCI_CMD_RS | UHCI_CMD_CF | UHCI_CMD_MAXP | UHCI_CMD_FGR);
   // sleepMilliSeconds(20);
	Syscall_Sleep(2);
    Syscall_OutPortWord(u->bar + UHCI_USBCMD, UHCI_CMD_RS | UHCI_CMD_CF | UHCI_CMD_MAXP);
   // sleepMilliSeconds(100);
	Syscall_Sleep(10);

  #ifdef _UHCI_DIAGNOSIS_
    printf("\n\nRoot-Ports   port1: %xh  port2: %xh\n", InPortWord(u->bar + UHCI_PORTSC1), InPortWord(u->bar + UHCI_PORTSC2));
  #endif

    bool run = Syscall_InPortWord(u->bar + UHCI_USBCMD) & UHCI_CMD_RS;

    if (!(Syscall_InPortWord(u->bar + UHCI_USBSTS) & UHCI_STS_HCHALTED))
    {
       // textColor(SUCCESS);
        printf("\n\nRunStop bit: %u\n", run);
        //textColor(TEXT);
        uhci_enablePorts(u); // attaches the ports
    }
    else
    {
        printf("\nFatal Error: UHCI - HCHalted. Ports will not be enabled.");
        printf("\nRunStop Bit: %u  Frame Number: %u", run, Syscall_InPortWord(u->bar + UHCI_FRNUM));
    }
}

// ports

static void uhci_enablePorts(uhci_t* u)
{
  #ifdef _UHCI_DIAGNOSIS_
    printf("\n\n>>>uhci_enablePorts<<<\n");
  #endif

    for (uint8_t j=0; j < u->hc.rootPortCount; j++)
    {
        uhci_resetPort(u, j);
    }
    u->enabledPortFlag = true;
    for (uint8_t j=0; j < u->hc.rootPortCount; j++)
    {
        uint16_t val = Syscall_InPortWord(u->bar + UHCI_PORTSC1 + 2*j);
        printf("\nport %u: %xh ", j+1, val);
        uhci_analysePortStatus(u, j, val);
    }
}

static void uhci_resetPort(uhci_t* u, uint8_t port)
{
    Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + 2*port, UHCI_PORT_RESET);
    //sleepMilliSeconds(50); // do not delete this wait
	Syscall_Sleep(50);
    Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + 2*port, Syscall_InPortWord(u->bar + UHCI_PORTSC1 + 2*port) & ~UHCI_PORT_RESET); // clear reset bit

    // wait and check, whether reset bit is really zero
    //WAIT_FOR_CONDITION((InPortWord(u->bar + UHCI_PORTSC1 + 2 * port) & UHCI_PORT_RESET) == 0, 20, 20, "\nTimeour Error: Port %u did not reset! ", port + 1);
	WAIT_FOR_CONDITION((Syscall_InPortWord(u->bar + UHCI_PORTSC1 + 2 * port) & UHCI_PORT_RESET) == 0, 20, 20, "\nTimeour Error: Port %u did not reset! ");

    //sleepMilliSeconds(10);
	Syscall_Sleep(10);

    // Enable
    Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + 2 * port, UHCI_PORT_CS_CHANGE | UHCI_PORT_ENABLE_CHANGE // clear Change-Bits Connected and Enabled
             | UHCI_PORT_ENABLE);      // set Enable-Bit
    //sleepMilliSeconds(10);
	Syscall_Sleep(10);

  #ifdef _UHCI_DIAGNOSIS_
    printf("Port Status: %Xh", InPortWord(u->bar + UHCI_PORTSC1 + 2 * port));
    waitForKeyStroke();
  #endif
}


/*******************************************************************************************************
*                                                                                                      *
*                                              uhci handler                                            *
*                                                                                                      *
*******************************************************************************************************/

static void uhci_handler(pciDev_t* device)
{
    // Check if an UHCI controller issued this interrupt
    uhci_t* u = (uhci_t *)device->data;
    bool found = false;
    for (dlelement_t* el = uhci.head; el != 0; el = el->next)
    {
        if (el->data == u)
        {
            //textColor(TEXT);
            found = true;
            break;
        }
    }

    if (!found || u == 0) // Interrupt did not came from UHCI device
    {
      #ifdef _UHCI_DIAGNOSIS_
        printf("Interrupt did not came from UHCI device!\n");
      #endif
        return;
    }

    uint16_t reg = u->bar + UHCI_USBSTS;
    uint16_t val = Syscall_InPortWord(reg);

    if (val==0) // Interrupt came from another UHCI device
    {
      #ifdef _UHCI_DIAGNOSIS_
        printf("Interrupt came from another UHCI device!\n");
      #endif
        return;
    }

    if (!(val & UHCI_STS_USBINT))
    {
        printf("\nUSB UHCI %u: ", u->num);
    }

    //textColor(IMPORTANT);

    if (val & UHCI_STS_USBINT)
    {
      #ifdef _UHCI_DIAGNOSIS_
        printf("Frame: %u - USB transaction completed", InPortWord(u->bar + UHCI_FRNUM));
      #endif
        Syscall_OutPortWord(reg, UHCI_STS_USBINT); // reset interrupt
    }

    if (val & UHCI_STS_RESUME_DETECT)
    {
        printf("Resume Detect");
        Syscall_OutPortWord(reg, UHCI_STS_RESUME_DETECT); // reset interrupt
    }

    if (val & UHCI_STS_HCHALTED)
    {
        printf("Host Controller Halted");
        Syscall_OutPortWord(reg, UHCI_STS_HCHALTED); // reset interrupt
    }

    if (val & UHCI_STS_HC_PROCESS_ERROR)
    {
        printf("Host Controller Process Error");
        Syscall_OutPortWord(reg, UHCI_STS_HC_PROCESS_ERROR); // reset interrupt
    }

    if (val & UHCI_STS_USB_ERROR)
    {
        printf("USB Error");
        Syscall_OutPortWord(reg, UHCI_STS_USB_ERROR); // reset interrupt
    }

    if (val & UHCI_STS_HOST_SYSTEM_ERROR)
    {
        printf("Host System Error");
        Syscall_OutPortWord(reg, UHCI_STS_HOST_SYSTEM_ERROR); // reset interrupt
        pci_analyzeHostSystemError(u->PCIdevice);
    }
}


/*******************************************************************************************************
*                                                                                                      *
*                                              PORT CHANGE                                             *
*                                                                                                      *
*******************************************************************************************************/

static void uhci_showPortState(uint16_t val)
{
  #ifdef _UHCI_DIAGNOSIS_
    if (val & UHCI_PORT_RESET)           {printf(" RESET");}

    if (val & UHCI_SUSPEND)              {printf(" SUSPEND");}
    if (val & UHCI_PORT_RESUME_DETECT)   {printf(" RESUME DETECT");}

    if (val & UHCI_PORT_LOWSPEED_DEVICE) {printf(" LOWSPEED DEVICE");}
    else                                 {printf(" FULLSPEED DEVICE");}
    if (val & BIT(5))                    {printf(" Line State: D-");}
    if (val & BIT(4))                    {printf(" Line State: D+");}

    if (val & UHCI_PORT_ENABLE_CHANGE)   {printf(" ENABLE CHANGE");}
    if (val & UHCI_PORT_ENABLE)          {printf(" ENABLED");}

    if (val & UHCI_PORT_CS_CHANGE)       {printf(" DEVICE CHANGE");}
    if (val & UHCI_PORT_CS)              {printf(" DEVICE ATTACHED");}
    else                                 {printf(" NO DEVICE ATTACHED");}
  #endif
}

static void uhci_analysePortStatus(uhci_t* u, uint8_t j, uint16_t val)
{
    hc_port_t* port = hc_getPort(&u->hc, j);

    if (val & UHCI_PORT_LOWSPEED_DEVICE)
        printf("Lowspeed");
    else
        printf("Fullspeed");

    if ((val & UHCI_PORT_CS) && !port->connected)
    {
        printf(" device attached.");
        port->connected = true;
        //sleepMilliSeconds(100); // Wait 100ms until power is stable (USB 2.0, 9.1.2)
		Syscall_Sleep(100);
		uhci_resetPort(u, j);   // reset on attached
      #ifdef _UHCI_DIAGNOSIS_
        waitForKeyStroke();
      #endif

        hc_setupUSBDevice(&u->hc, j, (val&UHCI_PORT_LOWSPEED_DEVICE)?USB_LOWSPEED:USB_FULLSPEED);
    }
    else if (!(val & UHCI_PORT_CS) && port->connected)
    {
        printf(" device removed.");
        port->connected = false;

        if (port->device)
        {
            usb_destroyDevice(port->device);
            port->device = 0;
        }
    }
    uhci_showPortState(val);
}

void uhci_pollDisk(port_t* dev)
{
    uhci_t* u = (uhci_t*)((hc_port_t*)dev->data)->hc;

    for (uint8_t j=0; j < u->hc.rootPortCount; j++)
    {
        uint16_t val = Syscall_InPortWord(u->bar + UHCI_PORTSC1 + 2*j);

        if (val & UHCI_PORT_CS_CHANGE)
        {
            Syscall_OutPortWord(u->bar + UHCI_PORTSC1 + 2*j, UHCI_PORT_CS_CHANGE);
            printf("\nUHCI %u: Port %u changed: ", u->num, j+1);
            uhci_analysePortStatus(u, j, val);
        }
    }
}


/*******************************************************************************************************
*                                                                                                      *
*                                            Transactions                                              *
*                                                                                                      *
*******************************************************************************************************/

static bool isTransactionSuccessful(uhciTD_t* uTD);
static void uhci_showStatusbyteTD(uhciTD_t* uTD);

void uhci_setupTransfer(usb_transfer_t* transfer)
{
    uhci_t* u = (uhci_t*)((hc_port_t*)transfer->device->port->data)->hc;
	if (transfer->type == USB_INTERRUPT)
	{
		//"UHCI interrupt QH"
		//transfer->data = malloc(sizeof(uhciQH_t), 16 | HEAP_CONTINUOUS);
		transfer->data = (void*)malloc_aligned(sizeof(uhciQH_t), 16);
	}
	else
        transfer->data = u->qhPointerVirt; // Use main QH
}

void uhci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length)
{
    hc_port_t* port = (hc_port_t*)transfer->device->port->data;

	usbTransaction->data = (void*)uhci_createTD_SETUP(port, BIT_T, toggle, type, req, hiVal, loVal, i, length, transfer->device->num, transfer->endpoint->address);
	uhciTD_t* uTD = (uhciTD_t * )usbTransaction->data;

  #ifdef _UHCI_DIAGNOSIS_
    usb_request_t* request = (usb_request_t*)uTD->virtBuffer;
    textColor(IMPORTANT);
    printf("\ntype: %yh req: %u valHi: %u valLo: %u index: %u len: %u", request->type, request->request, request->valueHi, request->valueLo, request->index, request->length);
    textColor(LIGHT_GRAY);
    printf("\nuhci_setup - \ttoggle: %u \tlength: %u \tdev: %u \tendp: %u", toggle, length, transfer->device->num, transfer->endpoint->address);
    textColor(TEXT);
  #endif

    if (transfer->transactions.tail)
    {
        uhciTD_t* uTD_last = (uhciTD_t * )((usb_transaction_t*)transfer->transactions.tail->data)->data;
		
		unsigned int pa = Syscall_GetPAFromVM(uTD);

        uTD_last->next = (pa & 0xFFFFFFF0) | BIT_Vf; // build vertical TD queue // BIT_Vf=1 Depth first, BIT_Vf=0 Breadth first
    }
}

void uhci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, void* buffer, size_t length)
{
    hc_port_t* port = (hc_port_t*)transfer->device->port->data;

    usbTransaction->data = uhci_createTD_IO(port, BIT_T, UHCI_TD_IN, toggle, length, transfer->device->num, transfer->endpoint->address, buffer, transfer->type == USB_INTERRUPT ? 1 : 0);
	uhciTD_t* uTD = (uhciTD_t*)usbTransaction->data;
  #ifdef _UHCI_DIAGNOSIS_
    textColor(LIGHT_BLUE);
    printf("\nuhci_in - \ttoggle: %u \tlength: %u \tdev: %u \tendp: %u", toggle, length, transfer->device->num, transfer->endpoint->address);
    textColor(TEXT);
  #endif

    if (transfer->transactions.tail)
    {
        uhciTD_t* uTD_last = (uhciTD_t * )((usb_transaction_t*)transfer->transactions.tail->data)->data;
		
		unsigned int pa = Syscall_GetPAFromVM(uTD);

        uTD_last->next = (pa & 0xFFFFFFF0) | BIT_Vf; // build vertical TD queue // BIT_Vf=1 Depth first, BIT_Vf=0 Breadth first
        if (transfer->type == USB_INTERRUPT) // Refresh duplicate
            uTD_last[1].next = uTD_last[0].next;
    }
}

void uhci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* usbTransaction, bool toggle, const void* buffer, size_t length)
{
    hc_port_t* port = (hc_port_t*)transfer->device->port->data;

    usbTransaction->data = uhci_createTD_IO(port, BIT_T, UHCI_TD_OUT, toggle, length, transfer->device->num, transfer->endpoint->address, 0, transfer->type == USB_INTERRUPT ? 1 : 0);
	uhciTD_t* uTD = (uhciTD_t*)usbTransaction->data;
    if (buffer != 0 && length != 0)
    {
        memcpy(uTD->virtBuffer, buffer, length);
    }

  #ifdef _UHCI_DIAGNOSIS_
    textColor(LIGHT_GREEN);
    printf("\nuhci_out - \ttoggle: %u \tlength: %u \tdev: %u \tendp: %u", toggle, length, transfer->device->num, transfer->endpoint->address);
    textColor(TEXT);
  #endif

    if (transfer->transactions.tail)
    {
        uhciTD_t* uTD_last = (uhciTD_t *)((usb_transaction_t*)transfer->transactions.tail->data)->data;
		
		unsigned int pa = Syscall_GetPAFromVM(uTD);
        uTD_last->next = (pa & 0xFFFFFFF0) | BIT_Vf; // build vertical TD queue // BIT_Vf=1 Depth first, BIT_Vf=0 Breadth first
        if (transfer->type == USB_INTERRUPT) // Refresh duplicate
            uTD_last[1].next = uTD_last[0].next;
    }
}

void uhci_scheduleTransfer(usb_transfer_t* transfer)
{
  #ifdef _UHCI_DIAGNOSIS_
    printf("\n\nuhci_issue_Transfer");
  #endif

    uhci_t* u = (uhci_t*)((hc_port_t*)transfer->device->port->data)->hc; // HC
    uhciTD_t* firstTD = (uhciTD_t *)((usb_transaction_t*)transfer->transactions.head->data)->data;
    uhci_fillQH(u, (uhciQH_t *)transfer->data, firstTD, transfer->type != USB_INTERRUPT);
    if (transfer->type == USB_INTERRUPT)
    {
        u->periodicScheduled = true;
		for (size_t i = 0; i < 1024; i += transfer->frequency)
		{
			
			unsigned int pa = Syscall_GetPAFromVM(transfer->data);
			u->framelistAddrVirt->frPtr[i] = pa | BIT_QH;
		}
    }

    transfer->success = true;

    // start scheduler, if necessary
    if (Syscall_InPortWord(u->bar + UHCI_USBSTS) & UHCI_STS_HCHALTED)
    {
        Syscall_OutPortWord(u->bar + UHCI_FRNUM, 0);
        Syscall_OutPortWord(u->bar + UHCI_USBCMD, Syscall_InPortWord(u->bar + UHCI_USBCMD) | UHCI_CMD_RS);
    }
}

bool uhci_pollTransfer(usb_transfer_t* transfer)
{
    // check completion
    transfer->success = true;
    bool completed = true;
    for (dlelement_t* elem = transfer->transactions.head; elem; elem = elem->next)
    {
        uhciTD_t* uTD = (uhciTD_t *)((usb_transaction_t*)elem->data)->data;
        if (!uTD->active)
            uhci_showStatusbyteTD(uTD);

        transfer->success = transfer->success && isTransactionSuccessful(uTD);
        completed = completed && !uTD->active;
    }
    if (!completed)
        return false;

    // Schedule it again
    for (dlelement_t* elem = transfer->transactions.head; elem; elem = elem->next)
    {
        static bool toggle = false;
        toggle = !toggle;
        uhciTD_t* uTD = (uhciTD_t * )((usb_transaction_t*)elem->data)->data;
        memcpy(uTD, uTD + 1, sizeof(*uTD));
        uTD->dataToggle = transfer->endpoint->toggle;
        transfer->endpoint->toggle = !transfer->endpoint->toggle;
    }
    uhci_t* u = (uhci_t*)((hc_port_t*)transfer->device->port->data)->hc; // HC
    uhciTD_t* firstTD = (uhciTD_t *)((usb_transaction_t*)transfer->transactions.head->data)->data;
    uhci_fillQH(u, (uhciQH_t *)transfer->data, firstTD, transfer->type != USB_INTERRUPT);

    return transfer->success;
}

void uhci_waitForTransfer(usb_transfer_t* transfer)
{
    uhci_t* u = (uhci_t*)((hc_port_t*)transfer->device->port->data)->hc; // HC

    // wait for completion
    uint32_t timeout = 150; // Wait up to 150*10ms = 1.5 seconds
    dlelement_t* dlE = transfer->transactions.head;
    while (timeout > 0)
    {
        uhciTD_t* uTD = (uhciTD_t * )((usb_transaction_t*)dlE->data)->data;

        while (!uTD->active)
        {
            dlE = dlE->next;
            if (dlE == 0)
                break;
            uTD = (uhciTD_t * )((usb_transaction_t*)dlE->data)->data;
        }
        if (dlE == 0)
            break;
        //sleepMilliSeconds(10);
		Syscall_Sleep(10);
		timeout--;
    }
    if (timeout == 0)
    {
        printf("\nUHCI: Timeout!");
    }

    // stop scheduler, if nothing more to do
    if (!u->periodicScheduled)
        Syscall_OutPortWord(u->bar + UHCI_USBCMD, Syscall_InPortWord(u->bar + UHCI_USBCMD) & ~UHCI_CMD_RS);

    // check conditions and save data
    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        uhciTD_t* uTD = (uhciTD_t*)((usb_transaction_t*)elem->data)->data;
        uhci_showStatusbyteTD(uTD);
        transfer->success = transfer->success && isTransactionSuccessful(uTD); // executed w/o error
    }

    #ifdef _UHCI_DIAGNOSIS_
    printf("\nQH:%X,QH->tfer:%X,", paging_getPhysAddr(transfer->data), ((uhciQH_t*)transfer->data)->transfer);

    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        uhciTD_t* uTD = ((usb_transaction_t*)elem->data)->data;

        if (elem == transfer->transactions.head)
        {
            printf("\nTD:%X->%X,", paging_getPhysAddr(uTD), uTD->next);
        }
        else
        {
            printf(" TD:%X->%X,", paging_getPhysAddr(uTD), uTD->next);
        }
    }
    #endif

    if (transfer->success)
    {
        #ifdef _UHCI_DIAGNOSIS_
        textColor(SUCCESS);
        printf("\nTransfer successful.");
        textColor(TEXT);
        #endif
    }
    else
    {
        printf("\nTransfer failed.");
    }
}

void uhci_destructTransfer(usb_transfer_t* transfer)
{
    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        uhciTD_t* uTD = (uhciTD_t *)((usb_transaction_t*)elem->data)->data;

        free(uTD->virtBuffer);
        free(uTD);
    }

  #ifdef _UHCI_DIAGNOSIS_
    waitForKeyStroke();
  #endif
}


/*******************************************************************************************************
*                                                                                                      *
*                                            uhci QH TD functions                                      *
*                                                                                                      *
*******************************************************************************************************/

static uhciTD_t* uhci_allocTD(uintptr_t next, hc_port_t* port, size_t duplicates)
{
    size_t size = (1 + duplicates) * sizeof(uhciTD_t);
	//20191112 "uhciTD"
    //uhciTD_t* td = (uhciTD_t*)malloc(size, 16 | HEAP_CONTINUOUS); // 16 byte alignment
	unsigned int pa = 0;
	uhciTD_t* td = (uhciTD_t*)malloc_aligned(size, 16); // 16 byte alignment
    memset(td, 0, size);

	if (next != BIT_T)
	{
		unsigned int pa = Syscall_GetPAFromVM((void*)next);
		td->next = (pa & 0xFFFFFFF0) | BIT_Vf;
	}
	else
        td->next = BIT_T;

    td->errorCounter       = 3;                // Stop after three errors
    td->lowSpeedDevice     = (port->device->speed==USB_LOWSPEED)?1:0;
    td->active             = 1;                // to be executed
    td->intOnComplete      = 0;                // We want an interrupt after complete transfer

    return td;
}

static void* uhci_allocTDbuffer(uhciTD_t* td, size_t length)
{
    if (length)
    {
        td->virtBuffer = malloc_aligned(length, 4096);
		unsigned int pa = Syscall_GetPAFromVM(td->virtBuffer);
        td->buffer = pa;
    }
    else
    {
        td->virtBuffer = 0;
        td->buffer = 0;
    }

    return td->virtBuffer;
}


static uhciTD_t* uhci_createTD_SETUP(hc_port_t* port, uintptr_t next, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t i, uint16_t length, uint32_t device, uint8_t endpoint)
{
    uhciTD_t* td = uhci_allocTD(next, port, 0);

    td->PacketID      = UHCI_TD_SETUP;

    td->dataToggle    = toggle; // Should be toggled every list entry

    td->deviceAddress = device;
    td->endpoint      = endpoint;
    td->maxLength     = 8-1;

    usb_request_t* request = (usb_request_t *)uhci_allocTDbuffer(td, sizeof(usb_request_t));
    request->type    = type;
    request->request = req;
    request->valueHi = hiVal;
    request->valueLo = loVal;
    request->index   = i;
    request->length  = length;

    return (td);
}

static uhciTD_t* uhci_createTD_IO(hc_port_t* port, uintptr_t next, uint8_t direction, bool toggle, uint16_t tokenBytes, uint32_t device, uint8_t endpoint, void* buffer, size_t duplicates)
{
    uhciTD_t* td = uhci_allocTD(next, port, duplicates);

    td->PacketID      = direction; // UHCI_TD_IN or UHCI_TD_OUT

    if (tokenBytes)
        td->maxLength = (tokenBytes-1U) & 0x7FFU;
    else
        td->maxLength = 0x7FF;

    td->dataToggle    = toggle; // Should be toggled every list entry

    td->deviceAddress = device;
    td->endpoint      = endpoint;

	if (buffer)
	{
		 unsigned int pa = Syscall_GetPAFromVM(buffer);
		td->buffer = pa;
	}
	else
        uhci_allocTDbuffer(td, tokenBytes);

    for(size_t i = 0; i < duplicates; i++)
    {
        memcpy(td + i + 1, td, sizeof(*td));
    }
    return (td);
}

static void uhci_fillQH(uhci_t* u, uhciQH_t* head, const uhciTD_t* firstTD, bool terminate)
{
	if (terminate)
		head->next = BIT_T;
	else
	{
		unsigned int pa = Syscall_GetPAFromVM(u->qhPointerVirt);
		head->next = pa | BIT_QH;
	}
	if (firstTD == 0)
		head->transfer = BIT_T;
	else
	{

		unsigned int pa = Syscall_GetPAFromVM(firstTD);
		head->transfer = (pa & 0xFFFFFFF0);

	}
}


////////////////////
// analysis tools //
////////////////////

static void uhci_showStatusbyteTD(uhciTD_t* uTD)
{
    if (uTD->bitstuffError)     printf("\nBitstuff Error ");          // receive data stream contained a sequence of more than 6 ones in a row
    if (uTD->crc_timeoutError)  printf("\nNo Response from Device "); // no response from the device (CRC or timeout)
    if (uTD->nakReceived)       printf("\nNAK received ");            // NAK handshake
    if (uTD->babbleDetected)    printf("\nBabble detected ");         // Babble (fatal error), e.g. more data from the device than MAXP
    if (uTD->dataBufferError)   printf("\nData Buffer Error ");       // HC cannot keep up with the data  (overrun) or cannot supply data fast enough (underrun)
    if (uTD->stall)             printf("\tStalled ");                 // can be caused by babble, error counter (0) or STALL from device
    if (uTD->active)            printf("\tHC still active ");         // 1: HC will execute   0: set by HC after excution (HC will not excute next time)
  #ifdef _UHCI_DIAGNOSIS_
    textColor(IMPORTANT);
    if (uTD->intOnComplete)     printf("\ninterrupt on complete ");   // 1: HC issues interrupt on completion of the frame in which the TD is executed
    if (uTD->isochrSelect)      printf("\nisochronous TD ");          // 1: Isochronous TD
    if (uTD->lowSpeedDevice)    printf("\nLowspeed Device ");         // 1: LS      0: FS
    if (uTD->shortPacketDetect) printf("\nShortPacketDetect ");       // 1: enable  0: disable
    textColor(TEXT);
  #endif
}

static bool isTransactionSuccessful(uhciTD_t* uTD)
{
    return
    (
        // no error
        (uTD->bitstuffError  == 0) && (uTD->crc_timeoutError == 0) && (uTD->nakReceived == 0) &&
        (uTD->babbleDetected == 0) && (uTD->dataBufferError  == 0) && (uTD->stall       == 0) &&
        // executed
        (uTD->active == 0)
    );
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
