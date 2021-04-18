/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include <stdio.h>
#include <memory.h>
#include "ehci.h"
#include "ehciQHqTD.h"
#include <string.h>
#include <systemcall_impl.h>
#include "EHCIHandler.h"

#define printfe printf
#define textColor(a)
#define serial_log(a, ...)
#define sleepMilliSeconds Syscall_Sleep
#define writeInfo printf

#define SER_LOG_EHCI_ITD 0
#define PAGE_SIZE 4096

#define WAIT_FOR_CONDITION(condition, runs, wait)\
    for (unsigned int timeout_ = 0; !(condition); timeout_++) {\
        if (timeout_ >= runs) {\
             break;\
        }\
        Syscall_Sleep(wait);\
    }


unsigned int paging_getPhysAddr(void* va)
{
    return Syscall_GetPAFromVM((unsigned int)va);
}

static list_t ehci = list_init();


//static void ehci_handler(registers_t* r, pciDev_t* device);

static void ehci_handler(pciDev_t * device);
static void ehci_deactivateLegacySupport(ehci_t* e);
static void ehci_analyze(ehci_t* e);
static void ehci_checkPortLineStatus(ehci_t* e, uint8_t j);
static void ehci_detectDevice(ehci_t* e, uint8_t j);
static void ehci_start(ehci_t* e);
static void ehci_initHC(ehci_t* e);
static void ehci_startHC(ehci_t* e);
static void ehci_resetHC(ehci_t* e);
static void ehci_enablePorts(ehci_t* e);
static void ehci_portCheck(ehci_t* e);
static void ehci_showUSBSTS(ehci_t* e);


void ehci_install(pciDev_t* PCIdev)
{
    dlelement_t* elem = list_alloc_elem(sizeof(ehci_t), "ehci");
    ehci_t* e             = (ehci_t *)elem->data;
    e->PCIdevice          = PCIdev;
    e->PCIdevice->data    = e;
    e->enabledPortFlag    = false;
    e->USBasyncIntPending = false;
    e->idleQH = e->tailQH = 0;

    // Get MMIO space
    e->bar = 0;
    for (uint8_t i = 0; i < 6 && !e->bar; i++)
    {
        e->bar = (uintptr_t)pci_aquireMemoryForMMIO(PCIdev->bar + i);
        
    }
    
    e->CapRegs   = (ehci_CapRegs_t*) e->bar;
    e->OpRegs    = (ehci_OpRegs_t*)(e->bar + e->CapRegs->CAPLENGTH);
    hc_constructRootPorts(&e->hc, (e->CapRegs->HCSPARAMS & 0x000F), &USB_EHCI);
    e->isochronousSchedulingThreshold = (e->CapRegs->HCCPARAMS & 0xF0) >> 4;
    list_append_elem(&ehci, elem);

    ehci_analyze(e); // get data (capregs, opregs)

    static uint8_t numPorts = 0;
    char str[10];
    snprintf(str, 10, "EHCI %u", ++numPorts);

    //Syscall_CreateThread(ehci_start, str, e, 16);
    ehci_start(e);
    //task_t* thread = create_cthread((void*)&ehci_start, str);
    //task_passStackParameter(thread, &e, sizeof(e));
    //scheduler_insertTask(thread);
}

static void ehci_analyze(ehci_t* e)
{
  #ifdef _EHCI_DIAGNOSIS_
    printf("\nEHCI bar get_physAddress: %Xh\n", (uintptr_t)paging_getPhysAddr((void*)e->bar));
    printf("HCIVERSION: %xh ",  e->CapRegs->HCIVERSION);             // Interface Version Number
    printf("HCSPARAMS: %Xh ",   e->CapRegs->HCSPARAMS);              // Structural Parameters
    printf("Ports: %u",         e->hc.rootPortCount);                // Number of Ports
    printf("\nHCCPARAMS: %Xh ", e->CapRegs->HCCPARAMS);              // Capability Parameters
    if (BYTE2(e->CapRegs->HCCPARAMS)==0) printf("No ext. capabil."); // Extended Capabilities Pointer
    printf("\nOpRegs Address: %Xh", e->OpRegs);                      // Host Controller Operational Registers
  #endif
}

static void ehci_start(ehci_t* e)
{
    textColor(HEADLINE);
    printf("Start EHCI Host Controller:");
    textColor(TEXT);

    ehci_initHC(e);
    ehci_resetHC(e);
    ehci_startHC(e);
    ehci_initializeAsyncScheduler(e);

    if (!(e->OpRegs->USBSTS & STS_HCHALTED))
    {
        ehci_enablePorts(e);
    }
    else
    {
        printfe("\nFatal Error: HCHalted set. Ports cannot be enabled.");
        ehci_showUSBSTS(e);
    }

    textColor(LIGHT_MAGENTA);
    printf("\n\n>>> Press key to close this console22. <<<");
    //getch();
}

static void ehci_initHC(ehci_t* e)
{
    printf(" Initialize");

    // prepare PCI command register
    uint16_t pciCommandRegister = pci_configRead(e->PCIdevice, PCI_COMMAND, 2);
    pci_configWrite_word(e->PCIdevice, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER); // resets status register, sets command register

  #ifdef _EHCI_DIAGNOSIS_
    uint8_t pciCapabilitiesList = pci_configRead(e->PCIdevice, PCI_CAPLIST, 1);
    printf("\nPCI Command Register before:          %xh", pciCommandRegister);
    printf("\nPCI Command Register plus bus master: %xh", pci_configRead(e->PCIdevice, PCI_COMMAND, 2));
    printf("\nPCI Capabilities List: first Pointer: %yh", pciCapabilitiesList);

    if (pciCapabilitiesList) // pointer != 0
    {
        uint16_t nextCapability = 0;
        do
        {
            nextCapability = pci_configRead(e->PCIdevice, BYTE2(nextCapability), 2);
            printf("\nPCI Capabilities List: ID: %yh, next Pointer: %yh", BYTE1(nextCapability), BYTE2(nextCapability));
        } while (BYTE2(nextCapability)); // pointer to next capability != 0
    }
  #endif

    //20191112
    //irq_installPCIHandler(e->PCIdevice->irq, ehci_handler, e->PCIdevice);

    InterruptHandler* pHandler = new EHCIHandler(e->PCIdevice->irq, ehci_handler, e->PCIdevice, "ehci");
    Syscall_kObserveInterrupt(e->PCIdevice->irq, pHandler);
}

static void ehci_startHC(ehci_t* e)
{
    printf("ehci_startHC\n");

    /*
    Intel Intel® 82801EB (ICH5), 82801ER (ICH5R), and 82801DB (ICH4)
    Enhanced Host Controller Interface (EHCI) Programmer’s Reference Manual (PRM) April 2003:
    After the reset has completed, the system software must reinitialize the host controller so as to
    return the host controller to an operational state (See Section 4.3.3.3, Post-Operating System Initialization)

    ... software must complete the controller initialization by performing the following steps:
    */

    // 1. Claim/request ownership of the EHCI. This process is described in detail in Section 5 - EHCI Ownership.
    ehci_deactivateLegacySupport(e);

    // 2. Program the CTRLDSSEGMENT register. This value must be programmed since the ICH4/5 only uses 64bit addressing
    //    (See Section 4.3.3.1.2-HCCPARAMS – Host Controller Capability Parameters).
    //    This register must be programmed before the periodic and asynchronous schedules are enabled.
    e->OpRegs->CTRLDSSEGMENT = 0; // Program the CTRLDSSEGMENT register with 4-GiB-segment where all of the interface data structures are allocated.

    // TEST: periodic list
    ehci_initializePeriodicList(e);

    // 3. Determine which events should cause an interrupt. System software programs the USB2INTR register
    //    with the appropriate value. See Section 9 - Hardware Interrupt Routing - for additional details.
    e->OpRegs->USBSTS = 0x0000103F; // Note: Always insure that the USB2STS registers bits 5:0 are clear
    e->OpRegs->USBINTR = STS_ASYNC_INT|STS_HOST_SYSTEM_ERROR|STS_PORT_CHANGE|STS_USBERRINT|STS_USBINT/*|STS_FRAMELIST_ROLLOVER*/;

    // 4. Program the USB2CMD.InterruptThresholdControl bits to set the desired interrupt threshold
    e->OpRegs->USBCMD |= CMD_8_MICROFRAME;

    //    and turn the host controller ON via setting the USB2CMD.Run/Stop bit. Setting the Run/Stop
    //    bit with both the periodic and asynchronous schedules disabled will still allow interrupts and
    //    enabled port events to be visible to software
    if (e->OpRegs->USBSTS & STS_HCHALTED)
    {
        e->OpRegs->USBCMD |= CMD_RUN_STOP; // set Run-Stop-Bit
    }

    // 5. Write a 1 to CONFIGFLAG register to default-route all ports to the EHCI. The EHCI can temporarily release control
    //    of the port to a cHC by setting the PortOwner bit in the PORTSC register to a one
    e->OpRegs->CONFIGFLAG = CF; // if zero, EHCI is not enabled and all usb devices go to the cHC

  #ifdef _EHCI_DIAGNOSIS_
    // 60 bits = 15 nibble  for the 15 possible ports of the EHCI show number of cHC
    printf("\nHCSPPORTROUTE_Hi: %X  HCSPPORTROUTE_Lo: %X", (uint32_t)(e->CapRegs->HCSPPORTROUTE<<32), (uint32_t)(e->CapRegs->HCSPPORTROUTE));
    // There VMWare has a bug! You can write to this Read-Only register, and then it does not reset.
  #endif
}

static void ehci_resetHC(ehci_t* e)
{
    printf(" Reset");
    /*
    Intel Intel® 82801EB (ICH5), 82801ER (ICH5R), and 82801DB (ICH4)
    Enhanced Host Controller Interface (EHCI) Programmer’s Reference Manual (PRM) April 2003

    To initiate a host controller reset system software must:
    */

    // 1. Stop the host controller.
    //    System software must program the USB2CMD.Run/Stop bit to 0 to stop the host controller.
    e->OpRegs->USBCMD &= ~CMD_RUN_STOP;            // set Run-Stop-Bit to 0

    // 2. Wait for the host controller to halt.
    //    To determine when the host controller has halted, system software must read the USB2STS.HCHalted bit;
    //    the host controller will set this bit to 1 as soon as
    //    it has successfully transitioned from a running state to a stopped state (halted).
    //    Attempting to reset an actively running host controller will result in undefined behavior.
    while (!(e->OpRegs->USBSTS & STS_HCHALTED))
    {
        Syscall_Sleep(10); // wait at least 16 microframes (= 16*125 micro-sec = 2 ms)
    }

    // 3. Program the USB2CMD.HostControllerReset bit to a 1.
    //    This will cause the host controller to begin the host controller reset.
    e->OpRegs->USBCMD |= CMD_HCRESET;              // set Reset-Bit to 1

    // 4. Wait until the host controller has completed its reset.
    // To determine when the reset is complete, system software must read the USB2CMD.HostControllerReset bit;
    // the host controller will set this bit to 0 upon completion of the reset.
    WAIT_FOR_CONDITION((e->OpRegs->USBCMD & CMD_HCRESET) == 0, 30, 10);
}

static void ehci_deactivateLegacySupport(ehci_t* e)
{
    pciDev_t* dev = e->PCIdevice;
    uint8_t eecp = BYTE2(e->CapRegs->HCCPARAMS);

  #ifdef _EHCI_DIAGNOSIS_
    printf("\nDeactivateLegacySupport: eecp = %yh\n",eecp);
  #endif
    /*
    cf. EHCI 1.0 spec, 2.2.4 HCCPARAMS - Capability Parameters, Bit 15:8 (BYTE2)
    EHCI Extended Capabilities Pointer (EECP). Default = Implementation Dependent.
    This optional field indicates the existence of a capabilities list.
    A value of 00h indicates no extended capabilities are implemented.
    A non-zero value in this register indicates the offset in PCI configuration space
    of the first EHCI extended capability. The pointer value must be 40h or greater
    if implemented to maintain the consistency of the PCI header defined for this class of device.
    */
    // cf. http://wiki.osdev.org/PCI#PCI_Device_Structure

    //   eecp     // RO - This field identifies the extended capability.
                  //      01h identifies the capability as Legacy Support.
    if (eecp >= 0x40)
    {
        uint8_t eecp_id=0;

        while (eecp) // 00h indicates end of the ext. cap. list.
        {
          #ifdef _EHCI_DIAGNOSIS_
            printf("eecp = %yh, ",eecp);
          #endif
            eecp_id = pci_configRead(dev, eecp, 1);
          #ifdef _EHCI_DIAGNOSIS_
            printf("eecp_id = %xh\n",eecp_id);
          #endif
            if (eecp_id == 1)
            {
                break;
            }
            eecp = pci_configRead(dev, eecp + 1, 1);
        }
        uint8_t BIOSownedSemaphore = eecp + 2; // R/W - only Bit 16 (Bit 23:17 Reserved, must be set to zero)
        uint8_t OSownedSemaphore   = eecp + 3; // R/W - only Bit 24 (Bit 31:25 Reserved, must be set to zero)
        uint8_t USBLEGCTLSTS       = eecp + 4; // USB Legacy Support Control/Status (DWORD, cf. EHCI 1.0 spec, 2.1.8)

        // Legacy-Support-EC found? BIOS-Semaphore set?
        if (eecp_id == 1 && (pci_configRead(dev, BIOSownedSemaphore, 1) & 0x01))
        {
          #ifdef _EHCI_DIAGNOSIS_
            printf("set OS-Semaphore.\n");
          #endif
            pci_configWrite_byte(dev, OSownedSemaphore, 0x01);

            WAIT_FOR_CONDITION((pci_configRead(dev, BIOSownedSemaphore, 1) & 0x01) == 0, 250, 10);
            WAIT_FOR_CONDITION((pci_configRead(dev, OSownedSemaphore, 1) & 0x01) != 0, 250, 10);

          #ifdef _EHCI_DIAGNOSIS_
            printf("Check: BIOSownedSemaphore: %u OSownedSemaphore: %u\n",
                pci_configRead(dev, BIOSownedSemaphore, 1),
                pci_configRead(dev, OSownedSemaphore, 1));
          #endif

            // USB SMI Enable R/W. 0=Default.
            // The OS tries to set SMI to disabled in case that BIOS bit stays at one.
            pci_configWrite_dword(dev, USBLEGCTLSTS, 0x0); // USB SMI disabled
        }
      #ifdef _EHCI_DIAGNOSIS_
        else
        {
            textColor(SUCCESS);
            printf("\nBIOS did not own the EHCI. No action needed.\n");
            textColor(TEXT);
        }
    }
    else
    {
        printf("No valid eecp found.\n");
  #endif
    }
}

static void ehci_enablePorts(ehci_t* e)
{
    e->enabledPortFlag = true;
    for (uint8_t j=0; j<e->hc.rootPortCount; j++)
    {
        ehci_checkPortLineStatus(e, j);
    }
}

static void ehci_resetPort(ehci_t* e, uint8_t j)
{
  #ifdef _EHCI_DIAGNOSIS_
    printf("Reset port %u\n", j+1);
  #endif

    // This field is zero if Port Power is zero.
    e->OpRegs->PORTSC[j] |= PSTS_POWERON;

    /*
     The HCHalted bit in the USBSTS register should be a zero before software attempts to use this bit.
     The host controller may hold Port Reset asserted to a one when the HCHalted bit is a one.
    */
    if (e->OpRegs->USBSTS & STS_HCHALTED) // TEST
    {
        printfe("\nHCHalted set to 1 (Not OK!)");
        ehci_showUSBSTS(e);
    }

    /*
     When software writes a one to this bit (from a zero), the bus reset sequence as defined in the USB Specification Revision 2.0 is started.
     Software writes a zero to this bit to terminate the bus reset sequence.
     Software must keep this bit at a one long enough to ensure the reset sequence, as specified in the USB Specification Revision 2.0, completes.
     Note: when software writes this bit to a one, it must also write a zero to the Port Enable bit.
    */
    // start reset sequence
    e->OpRegs->PORTSC[j] = (e->OpRegs->PORTSC[j] & ~PSTS_ENABLED) | PSTS_PORT_RESET;
    Syscall_Sleep(200);                   // do not delete this wait (freeBSD: 250 ms, spec: 50 ms) <== 200 ms at 250 Hz!!!
    e->OpRegs->PORTSC[j] &= ~PSTS_PORT_RESET; // stop reset sequence

    // wait and check, whether really zero
    WAIT_FOR_CONDITION((e->OpRegs->PORTSC[j] & PSTS_PORT_RESET) == 0, 25, 5);
}



/*******************************************************************************************************
*                                                                                                      *
*                                              ehci handler                                            *
*                                                                                                      *
*******************************************************************************************************/

static void ehci_handler(pciDev_t* device)
{
    // Check if an EHCI controller issued this interrupt
    ehci_t* e = (ehci_t*)device->data;
    bool found = false;
    for (dlelement_t* el = ehci.head; el != 0; el = el->next)
    {
        if (el->data == e)
        {
            textColor(TEXT);
            found = true;
            break;
        }
    }

    if (!found || e == 0) // Interrupt did not come from EHCI device
    {
      #ifdef _EHCI_DIAGNOSIS_
        printf("Interrupt did not came from EHCI device!\n");
      #endif
        return;
    }

    uint32_t val = e->OpRegs->USBSTS;

    if (val==0) // Interrupt came from another EHCI device
    {
      #ifdef _EHCI_DIAGNOSIS_
        printf("Interrupt came from another EHCI device!\n");
      #endif
        return;
    }

    e->OpRegs->USBSTS = val; // reset interrupt


  #ifdef _EHCI_DIAGNOSIS_
    if (!(val & STS_FRAMELIST_ROLLOVER) && !(val & STS_USBINT))
    {
        textColor(LIGHT_BLUE);
        printf("\nehci_handler: ");
    }
  #endif

    if (val & STS_USBERRINT)
    {
        printf("\nUSB Error Interrupt\n");
    }

    if (val & STS_PORT_CHANGE)
    {
        textColor(LIGHT_BLUE);
        printf("Port Change");
        textColor(TEXT);

        if (e->enabledPortFlag && e->PCIdevice)
        {
            //20200330
            Syscall_CreateThread(ehci_portCheck, "EHCI Ports", e, 16, 0);
            /*task_t* thread = create_cthread((void*)&ehci_portCheck, "EHCI Ports");
            task_passStackParameter(thread, &e, sizeof(e));
            scheduler_insertTask(thread);*/
        }
    }

    if (val & STS_FRAMELIST_ROLLOVER)
    {
        //printf("Frame List Rollover Interrupt");
    }

    if (val & STS_HOST_SYSTEM_ERROR)
    {
        printfe("Host System Error");
        pci_analyzeHostSystemError(e->PCIdevice);
        textColor(IMPORTANT);
        printf("\n>>> Init EHCI after fatal error:           <<<");
        printf("\n>>> Press key for EHCI (re)initialization. <<<");
        //20200330
        Syscall_GetChar();
        Syscall_CreateThread(ehci_start, "ehci_start", e, 16, 0);
        /*getch();
        task_t* thread = create_cthread((void*)&ehci_start, "EHCI");
        task_passStackParameter(thread, &e, sizeof(e));
        scheduler_insertTask(thread);
        textColor(TEXT);*/
    }

    if (val & STS_ASYNC_INT)
    {
        e->USBasyncIntPending = false;
      #ifdef _EHCI_DIAGNOSIS_
        textColor(YELLOW);
        printf("Interrupt on Async Advance");
        textColor(TEXT);
      #endif
    }

    if (val & STS_USBINT)
    {
         //printf("USB Interrupt"); // Issued on completion of qTD/iTD if IOC is set
    }
}



/*******************************************************************************************************
*                                                                                                      *
*                                              PORT CHANGE                                             *
*                                                                                                      *
*******************************************************************************************************/

static void ehci_portCheck(ehci_t* e)
{
    //console_setProperties(CONSOLE_SHOWINFOBAR|CONSOLE_AUTOSCROLL|CONSOLE_AUTOREFRESH); // protect console against info area
    for (uint8_t j=0; j<e->hc.rootPortCount; j++)
    {
        if (e->OpRegs->PORTSC[j] & PSTS_CONNECTED_CHANGE)
        {
            e->OpRegs->PORTSC[j] |= PSTS_CONNECTED_CHANGE; // reset interrupt
            if (e->OpRegs->PORTSC[j] & PSTS_CONNECTED)
            {
                ehci_checkPortLineStatus(e, j);
            }
            else
            {
                writeInfo("Port: %u, no device attached", j+1);

                if (e->hc.ports.data[j]->device)
                {
                    usb_destroyDevice(e->hc.ports.data[j]->device);
                    e->hc.ports.data[j]->device = 0;
                    e->hc.ports.data[j]->connected = false;
                }

            }
        }
    }
    textColor(IMPORTANT);
    printf("\n>>> Press key to close this console11. <<<");
    textColor(TEXT);
   // getch();
}

static void ehci_checkPortLineStatus(ehci_t* e, uint8_t j)
{
    if (!(e->OpRegs->PORTSC[j] & PSTS_CONNECTED))
    {
        printf("\nline state: -");
        return;
    }

    sleepMilliSeconds(100); // Wait 100ms until power is stable (USB 2.0, 9.1.2)

    uint8_t lineStatus = (e->OpRegs->PORTSC[j]>>10) & 3; // bits 11:10

    static const char* const state[] = {"SE0", "K-state", "J-state", "undefined"};
  #ifdef _EHCI_DIAGNOSIS_
    // field lineStatus is valid only when the port enable bit is zero and the current connect status bit is set to a one
    printf("\nehci_checkPortLineStatus:  PSTS_ENABLED: %u  PSTS_CONNECTED: %u", e->OpRegs->PORTSC[j] & PSTS_ENABLED, e->OpRegs->PORTSC[j] & PSTS_CONNECTED);
    textColor(LIGHT_CYAN);
    printf("\nport %u: %xh, line: %yh (%s) ", j+1, e->OpRegs->PORTSC[j], lineStatus, state[lineStatus]);
  #else
    printf("\nline state: %s", state[lineStatus]);
  #endif

    switch (lineStatus)
    {
        case 1: // K-state, release ownership of port, because a low speed device is attached
            if(e->CapRegs->HCSPARAMS & 0xF000)
                e->OpRegs->PORTSC[j] |= PSTS_COMPANION_HC_OWNED; // release it to the cHC (if there is one)
            break;
        case 0: // SE0
        case 2: // J-state
        case 3: // undefined
            ehci_detectDevice(e, j); // cf. spec: reset necessary for SE0, J-state, undefined
            break;
    }
}

static void ehci_detectDevice(ehci_t* e, uint8_t j)
{
    ehci_resetPort(e,j);
    if (e->enabledPortFlag && (e->OpRegs->PORTSC[j] & PSTS_POWERON)) // power on
    {
        // ehci only set PSTS_ENABLED when the reset sequence determines that the attached device is a high-speed device
        if (e->OpRegs->PORTSC[j] & PSTS_ENABLED)
        {
            writeInfo("Port: %u, hi-speed device attached", j+1);
            hc_setupUSBDevice(&e->hc, j, USB_HIGHSPEED);
        }
        else // Full speed
        {
            if (e->CapRegs->HCSPARAMS & 0xF000)
                e->OpRegs->PORTSC[j] |= PSTS_COMPANION_HC_OWNED; // release it to the cHC (if there is one)
        }
    }
}


static void ehci_showUSBSTS(ehci_t* e)
{
  #ifdef _EHCI_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\nUSB status: ");
    textColor(IMPORTANT);
    printf("%Xh", e->OpRegs->USBSTS);
  #endif
    if (e->OpRegs->USBSTS & STS_HCHALTED)         { printfe("\nHCHalted");                    e->OpRegs->USBSTS |= STS_HCHALTED;         }
    textColor(IMPORTANT);
    if (e->OpRegs->USBSTS & STS_RECLAMATION)      { printf("\nReclamation");                  e->OpRegs->USBSTS |= STS_RECLAMATION;      }
    if (e->OpRegs->USBSTS & STS_PERIODIC_ENABLED) { printf("\nPeriodic Schedule Status");     e->OpRegs->USBSTS |= STS_PERIODIC_ENABLED; }
    if (e->OpRegs->USBSTS & STS_ASYNC_ENABLED)    { printf("\nAsynchronous Schedule Status"); e->OpRegs->USBSTS |= STS_ASYNC_ENABLED;    }
    textColor(TEXT);
}



/*******************************************************************************************************
*                                                                                                      *
*                                            Transactions                                              *
*                                                                                                      *
*******************************************************************************************************/


void ehci_setupTransfer(usb_transfer_t* transfer)
{
    if (transfer->type == USB_ISOCHRONOUS)
    {
        ehci_iTDchainTransfer_t* iTDchainTransfer = (ehci_iTDchainTransfer_t *)malloc_aligned(sizeof(ehci_iTDchainTransfer_t), 4096);
        iTDchainTransfer->times = WORD2(*(uint32_t*)transfer->data);
        iTDchainTransfer->numChain = WORD1(*(uint32_t*)transfer->data);
        //20200330
        iTDchainTransfer->iTD = (ehci_iTD_t*)malloc_aligned(iTDchainTransfer->times * iTDchainTransfer->numChain * sizeof(ehci_iTD_t), 4096);
        memset(iTDchainTransfer->iTD, 0, iTDchainTransfer->times * iTDchainTransfer->numChain * sizeof(ehci_iTD_t));
        transfer->data = iTDchainTransfer;

        serial_log(SER_LOG_EHCI_ITD, "\n\n>>>>>> number of chains: %u, chain length: %u ==> %u iTD <<<<<<", iTDchainTransfer->numChain, iTDchainTransfer->times, (uint32_t)iTDchainTransfer->numChain * (uint32_t)iTDchainTransfer->times * 8);
        serial_log(SER_LOG_EHCI_ITD, "\nmax Transfer: %u bytes", (uint32_t)iTDchainTransfer->numChain * (uint32_t)iTDchainTransfer->times * 8 * 3072);

        ehci_createiTDchain(transfer, iTDchainTransfer);
    }
    else
    {
        //20200330
        transfer->data = malloc_aligned(sizeof(ehci_qhd_t), 4096);
        memset(transfer->data, 0, sizeof(ehci_qhd_t));
    }
}

void ehci_setupTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
    uTransaction->data = (ehci_transaction_t*)malloc_aligned(sizeof(ehci_transaction_t), 4096);
    ehci_transaction_t* eTransaction = (ehci_transaction_t * )uTransaction->data;
    eTransaction->qTDBuffer = 0;
    eTransaction->qTD = ehci_createQTD_SETUP(1, toggle, type, req, hiVal, loVal, index, length, &eTransaction->qTDBuffer);
    if (transfer->transactions.tail)
    {
        ehci_transaction_t* eLastTransaction = (ehci_transaction_t *)((usb_transaction_t*)transfer->transactions.tail->data)->data;
        eLastTransaction->qTD->next = paging_getPhysAddr(eTransaction->qTD);
    }
}

void ehci_inTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, void* buffer, size_t length)
{
    if (transfer->type == USB_ISOCHRONOUS)
    {
        // serial_log(SER_LOG_EHCI_ITD, "\nehci_inTransaction");

        ehci_iTDchainTransfer_t* ehci_iTDchainTransfer = (ehci_iTDchainTransfer_t*)transfer->data;
        ehci_iTD_t* iTD = ehci_iTDchainTransfer->iTD;

        if (buffer)
        {
            uTransaction->data = buffer;

            for (uint16_t k=0; k<ehci_iTDchainTransfer->numChain; k++)
            {
                for (uint16_t j=0; j<ehci_iTDchainTransfer->times; j++)
                {
                     iTD[j].buffer[1] |= BIT(11); // Direction (I/O). 0 = OUT; 1 = IN
                     iTD++;
                }
            }
        }
    }
    else
    {
       
         uTransaction->data = malloc_aligned(sizeof(ehci_transaction_t), 4096);
         ehci_transaction_t* eTransaction = (ehci_transaction_t *)uTransaction->data;
        eTransaction->qTDBuffer = buffer;
        eTransaction->qTD = ehci_createQTD_IO(1, USB_TT_IN, toggle, length, &eTransaction->qTDBuffer);
        if (buffer)
            eTransaction->qTDBuffer = 0;
        if (transfer->transactions.tail)
        {
            ehci_transaction_t* eLastTransaction = (ehci_transaction_t *)((usb_transaction_t*)transfer->transactions.tail->data)->data;
            eLastTransaction->qTD->next = paging_getPhysAddr(eTransaction->qTD);
        }
    }
}

void ehci_outTransaction(usb_transfer_t* transfer, usb_transaction_t* uTransaction, bool toggle, const void* buffer, size_t length)
{
    if (transfer->type == USB_ISOCHRONOUS)
    {
        // serial_log(SER_LOG_EHCI_ITD, "\nehci_inTransaction");

        ehci_iTDchainTransfer_t* ehci_iTDchainTransfer = (ehci_iTDchainTransfer_t*)transfer->data;
        ehci_iTD_t* iTD = ehci_iTDchainTransfer->iTD;

        if (buffer != 0)
        {
            uTransaction->data = (void*)buffer;

            for (uint16_t k=0; k<ehci_iTDchainTransfer->numChain; k++)
            {
                for (uint16_t j=0; j<ehci_iTDchainTransfer->times; j++)
                {
                     iTD[j].buffer[1] &= ~BIT(11); // Direction (I/O). 0 = OUT; 1 = IN
                     iTD++;
                }
            }
        }
    }
    else
    {
         
        uTransaction->data = malloc_aligned(sizeof(ehci_transaction_t), 4096);
        ehci_transaction_t* eTransaction = (ehci_transaction_t*)uTransaction->data;
        eTransaction->qTDBuffer = 0;
        eTransaction->qTD = ehci_createQTD_IO(1, USB_TT_OUT, toggle, length, &eTransaction->qTDBuffer);
        if (buffer != 0 && length != 0)
            memcpy(eTransaction->qTDBuffer, buffer, length);
        if (transfer->transactions.tail)
        {
            ehci_transaction_t* eLastTransaction = (ehci_transaction_t *)((usb_transaction_t*)transfer->transactions.tail->data)->data;
            eLastTransaction->qTD->next = paging_getPhysAddr(eTransaction->qTD);
        }
    }
}

void ehci_scheduleTransfer(usb_transfer_t* transfer)
{
    ehci_t* e = (ehci_t*)((hc_port_t*)transfer->device->port->data)->hc;

    ehci_transaction_t* firstTransaction = (ehci_transaction_t *)((usb_transaction_t*)transfer->transactions.head->data)->data;

    uint8_t s_mask = 0;
    if (transfer->type == USB_INTERRUPT)
        s_mask = BIT(7);

    if (transfer->type != USB_ISOCHRONOUS)
    {
        ehci_createQH((ehci_qhd_t *)transfer->data, BIT(0), firstTransaction->qTD, 0, transfer->device->num, transfer->endpoint->address, transfer->packetSize, s_mask);
        ehci_transaction_t* lastTransaction = (ehci_transaction_t *)((usb_transaction_t*)transfer->transactions.tail->data)->data;
        if (transfer->type != USB_INTERRUPT)
            lastTransaction->qTD->token._interrupt = 0x1; // Receive interrupt from last qTD
    }

    if (transfer->type == USB_CONTROL)
    {
        ehci_addToAsyncScheduler(e, transfer, 0);
    }
    else if (transfer->type == USB_BULK)
    {
        ehci_addToAsyncScheduler(e, transfer, 1 + transfer->packetSize / 200);
    }
    else if (transfer->type == USB_INTERRUPT || transfer->type == USB_ISOCHRONOUS)
    {
        //serial_log(SER_LOG_EHCI_ITD, "\nehci_scheduleTransfer");
        ehci_addToPeriodicScheduler(e, transfer);
    }
}

bool ehci_pollTransfer(usb_transfer_t* transfer)
{
    return false; // Not implemented
}

void ehci_waitForTransfer(usb_transfer_t* transfer)
{
    transfer->success = true;
    for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
    {
        ehci_transaction_t* transaction = (ehci_transaction_t * )((usb_transaction_t*)elem->data)->data;

        if (transfer->type == USB_ISOCHRONOUS)
        {
            ehci_iTDchainTransfer_t* ehci_iTDchainTransfer = (ehci_iTDchainTransfer_t*)transfer->data;
            ehci_iTD_t* iTD = ehci_iTDchainTransfer->iTD;

            uint32_t bytesTransferred = 0;
            uint32_t used_iTDbuffer   = 0;
            uint32_t unused_iTDbuffer = 0;

            for (uint16_t k=0; k<ehci_iTDchainTransfer->numChain; k++)
            {
                serial_log(SER_LOG_EHCI_ITD, "\n\n>>> chain %u <<<\n", k);

                for (uint16_t j=0; j<ehci_iTDchainTransfer->times; j++)
                {
                    for (uint8_t i=0; i<8; i++)
                    {
                        if ((iTD->tsc[i].status_Active==0) && (iTD->tsc[i].status_DataBufferError  || iTD->tsc[i].status_Babble  ||
                            iTD->tsc[i].status_TransactionError || (iTD->tsc[i].transactionLength > 0)))
                        {
                            serial_log(SER_LOG_EHCI_ITD, "\n%u:%u:%u: A: %u Buf: %u Bab: %u Tr: %u L: %u",
                            k, j, i,
                            iTD->tsc[i].status_Active, iTD->tsc[i].status_DataBufferError,
                            iTD->tsc[i].status_Babble, iTD->tsc[i].status_TransactionError,
                            iTD->tsc[i].transactionLength);

                            bytesTransferred += iTD->tsc[i].transactionLength;
                            used_iTDbuffer++;
                        }

                        if (iTD->tsc[i].status_Active)
                        {
                            serial_log(SER_LOG_EHCI_ITD, "\n%u:%u:%u: A: %u", k, j, i, iTD->tsc[i].status_Active);
                            unused_iTDbuffer++;
                        }
                    }

                    void*  transactionBuffer = ((usb_transaction_t*)(transfer->transactions.head->data))->data;
                    size_t size   = 8 * 3 * 1024;

                    if (((usb_transaction_t*)elem->data)->type == USB_TT_IN)
                    {
                        //20200330
                        void*  dest   = (char*)transactionBuffer + (k * ehci_iTDchainTransfer->times + j) * size;
                        void*  source = (char*)ehci_iTDchainTransfer->iTDvirtBuffer0 + (k * ehci_iTDchainTransfer->times + j) * 7 * PAGE_SIZE;
                        memcpy(dest, source, size);
                    }

                    if (((usb_transaction_t*)elem->data)->type == USB_TT_OUT)
                    {
                        // TODO
                        serial_log(SER_LOG_EHCI_ITD, "\nOUT: Copy process not yet implemented!");
                    }

                    iTD++;
                }
            }
            
            transfer->success = true;
            uint32_t cntBuf = ehci_iTDchainTransfer->times * ehci_iTDchainTransfer->numChain * 8;
            serial_log(SER_LOG_EHCI_ITD, "\nBytes transferred: %u, iTD buffer with data: %u (%u percent), still active iTD buffer: %u (%u percent)",
                bytesTransferred,
                used_iTDbuffer,
                (100 * used_iTDbuffer) / cntBuf,
                unused_iTDbuffer,
                (100 * unused_iTDbuffer) / cntBuf);
        }
        else
        {
            uint8_t status = ehci_showStatusbyteQTD(transaction->qTD);
            transfer->success = transfer->success && (status == 0 || status == BIT(0));
        }
    }
    if (transfer->type == USB_INTERRUPT)
    {
        printf("\nTransaction Nak Counter: %u", ehci_getNakCounter((const ehci_qhd_t *)transfer->data));
    }
}

void ehci_destructTransfer(usb_transfer_t* transfer)
{
    
    ehci_t* e = (ehci_t*)((hc_port_t*)transfer->device->port->data)->hc;
    if (transfer->type == USB_ISOCHRONOUS)
    {
        printf("ppppppppp\n");
        ehci_iTDchainTransfer_t* ehci_iTDchainTransfer = (ehci_iTDchainTransfer_t*)transfer->data;
        free(ehci_iTDchainTransfer->iTDvirtBuffer0);
        free(transfer->data);
    }
    else
    {
        if (transfer->type != USB_INTERRUPT)
        {
            // Wait until we may release the memory
            WAIT_FOR_CONDITION(!e->USBasyncIntPending, 25, 10);
        }
        
        free(transfer->data);
        
        for (dlelement_t* elem = transfer->transactions.head; elem != 0; elem = elem->next)
        {
            ehci_transaction_t* transaction = (ehci_transaction_t * )((usb_transaction_t*)elem->data)->data;
            free(transaction->qTDBuffer);
            free(transaction->qTD);
            free(transaction);
        }
    }
   
    if (transfer->success)
    {
      #ifdef _EHCI_DIAGNOSIS_
        textColor(SUCCESS);
        printf("\nTransfer successful.");
        textColor(TEXT);
      #endif
    }
    else
    {
        printfe("\nTransfer failed.");
    }
}


/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
