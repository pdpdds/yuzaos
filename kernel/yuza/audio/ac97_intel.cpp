
#include "ac97_intel.h"
#include "wav.h"
#include "timer.h"
#include <stdio.h>
#include <memory.h>
#include <Hal.h>
#include <string.h>
#include <systemcall_impl.h>
#include <intrinsic.h>
#include <Constants.h>
#include <InterruptHandler.h>
#include "AC97Handler.h"

static void AC97_handler(pciDev_t* device);

extern "C" int rand(void);

void ac97Intel_install(pciDev_t* device)
{
	unsigned int pa = 0;
	
    ac97Intel_t* ac97 = (ac97Intel_t*)malloc(sizeof(ac97Intel_t));
    ac97->device = device;

    ac97->device->data = ac97;

    // ICH6 specification requires Bit 0 to be set before PCI_CMD_IO can be set
    uint8_t CFG = pci_configRead(device, 0x41, 1);
    if (!(CFG & BIT(0)))
        pci_configWrite_byte(device, 0x41, CFG|BIT(0));

    uint16_t pciCommandRegister = pci_configRead(device, PCI_COMMAND, 2);
    pci_configWrite_word(device, PCI_COMMAND, pciCommandRegister | PCI_CMD_BUSMASTER | PCI_CMD_IO);

     //irq_installPCIHandler(device->irq, AC97_handler, device);
	InterruptHandler* pHandler = new AC97Handler(device->irq, AC97_handler, device, "AC97");
	Syscall_kObserveInterrupt(device->irq, pHandler);

    // first and second address room
    ac97->nambar = (uint16_t)device->bar[0].baseAddress;  // NAM-BAR (Mixer)
    ac97->nabmbar = (uint16_t)device->bar[1].baseAddress; // NABM-BAR (Player)
    printf("\nnambar: %X nabmbar: %X  ", ac97->nambar, ac97->nabmbar);

    // reset
    OutPortWord(ac97->nambar + PORT_NAM_RESET, 42);         // Each value is possible
	OutPortByte(ac97->nabmbar + PORT_NABM_PICONTROL, 0x02); // 0x02 enforces reset
	OutPortByte(ac97->nabmbar + PORT_NABM_POCONTROL, 0x02); // 0x02 enforces reset
    OutPortByte(ac97->nabmbar + PORT_NABM_MCCONTROL, 0x02); // 0x02 enforces reset
    Syscall_Sleep(100);

    // volume
    uint8_t volume = 0; //Am lautesten!
	OutPortWord(ac97->nambar + PORT_NAM_MASTER_VOLUME, (volume << 8) | volume);  // General volume left and right
	OutPortWord(ac97->nambar + PORT_NAM_MONO_VOLUME, volume);                    // Volume for Mono
	OutPortWord(ac97->nambar + PORT_NAM_PC_BEEP_VOLUME, volume);                 // Volume for PC speaker
	OutPortWord(ac97->nambar + PORT_NAM_PCM_OUT_VOLUME, (volume << 8) | volume); // Volume for PCM left and right
	Syscall_Sleep(10);

    // sample rate
    if (!(InPortWord(ac97->nambar + PORT_NAM_EXT_AUDIO_ID) & 1))
    {
        // sample rate is fixed to 48 kHz
    }
    else
    {
        OutPortWord(ac97->nambar + PORT_NAM_EXT_AUDIO_STS_CTRL, InPortWord(ac97->nambar + PORT_NAM_EXT_AUDIO_STS_CTRL) | 1); // Activate variable rate audio
        Syscall_Sleep(10);
		OutPortWord(ac97->nambar + PORT_NAM_FRONT_DAC_RATE, 44100); // General sample rate
		OutPortWord(ac97->nambar + PORT_NAM_LR_ADC_RATE, 44100); // Stereo  sample rate
		Syscall_Sleep(10);
    }

    // Actual sample rate can be read here:
    printf("\nsample rate: %u Hz\n", InPortWord(ac97->nambar + PORT_NAM_FRONT_DAC_RATE));

	bool tick = false;

    //Generate beep (test)
    ac97->NUM = 6553;

	uint16_t* temp = ac97->buffer;
	uint16_t temp2 = *temp;
    ac97->buffer = (uint16_t*)malloc(sizeof(temp2) * ac97->NUM);
    for (size_t i = 0; i < ac97->NUM; i++)
    {
        if (i % 100 == 0)
            tick = !tick;
        if (tick)
            ac97->buffer[i] = 0x7FFF;
        else
            ac97->buffer[i] = rand();
    }

    ac97->numDesc = 3;
	ac97->descs = (ac97Intel_bufDesc_t*)malloc(sizeof(ac97Intel_bufDesc_t) * ac97->numDesc);
}

void ac97Intel_start(ac97Intel_t* ac97)
{
	printf("ac97Intel_start!\n");
    for (uint32_t i = 0; i < ac97->numDesc; i++)
    {
        ac97->descs[i].buf = Syscall_GetPAFromVM(ac97->buffer);
        ac97->descs[i].len = ac97->NUM;
        ac97->descs[i].ioc = 1;
        ac97->descs[i].bup = 0;

		//printf("buffer address %x %x\n", ac97->descs[i].buf, ac97->buffer);
		//for (;;);
    }
    ac97->descs[ac97->numDesc - 1].bup = 1;

    OutPortDWord(ac97->nabmbar + PORT_NABM_POBDBAR, Syscall_GetPAFromVM(ac97->descs));
    OutPortByte(ac97->nabmbar + PORT_NABM_POLVI, ac97->numDesc - 1);
	OutPortByte(ac97->nabmbar + PORT_NABM_POCONTROL, 0x15); // play and generate interrupt afterwards
}

void ac97Intel_stop(ac97Intel_t* ac97)
{
    for (int i=0; i<ac97->numDesc; i++)
    {
        ac97->descs[i].buf = 0;
        ac97->descs[i].len = 0;
        ac97->descs[i].ioc = 0;
        ac97->descs[i].bup = 0;
    }
	OutPortDWord(ac97->nabmbar + PORT_NABM_POBDBAR, Syscall_GetPAFromVM(ac97->descs));
	OutPortByte(ac97->nabmbar + PORT_NABM_POLVI, ac97->numDesc - 1);
	OutPortByte(ac97->nabmbar + PORT_NABM_POCONTROL, 0);
}

static void AC97_handler(pciDev_t* device)
{
    ac97Intel_t* ac97 = (ac97Intel_t*)device->data;
	
    uint8_t pi = InPortByte(ac97->nabmbar + PORT_NABM_PISTATUS) & 0x1C;
    uint8_t po = InPortByte(ac97->nabmbar + PORT_NABM_POSTATUS) & 0x1C;
    uint8_t mc = InPortByte(ac97->nabmbar + PORT_NABM_MCSTATUS) & 0x1C;

    OutPortByte(ac97->nabmbar + PORT_NABM_PISTATUS, pi);
	OutPortByte(ac97->nabmbar + PORT_NABM_POSTATUS, po);
	OutPortByte(ac97->nabmbar + PORT_NABM_MCSTATUS, mc);
}

