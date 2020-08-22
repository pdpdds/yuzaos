
#include "audio.h"
#include "ac97_intel.h"
#include "wav.h"
#include <stdio.h>
#include <systemcall_impl.h>
#ifdef _AUDIO_ENABLE_
enum {AD_AC97_INTEL, AD_AC97_VIA, AD_SB16, AD_END};

static const audio_driver_t drivers[AD_END] =
{
    { &ac97Intel_install },
   // { &ac97Via_install },
   // { &install_SB16 }
};

static pciDev_t* firstDev = 0;
#endif


void audio_installDevice(pciDev_t* device)
{
  #ifdef _AUDIO_ENABLE_
    if (!firstDev)
        firstDev = device;

    if (device->vendorID == 0x8086 /* && device->deviceID ==  */) // Intel (TODO: We check only vendor here. Verify that its really an AC97)
    {
        drivers[AD_AC97_INTEL].install(device);
    }
  #endif
}

DWORD WINAPI audio_test(LPVOID parameter)
{
  #ifdef _AUDIO_ENABLE_
    if (firstDev)
    {
        // HACK. Support other devices than Intel-AC97 here.
        //ac97Intel_start((ac97Intel_t *)firstDev->data);
       // textColor(IMPORTANT);
		//Syscall_Sleep(15000);
       // printf("Press key to stop ac97 test sound!\n");
		
        //textColor(TEXT);
		//Syscall_GetChar();
       //ac97Intel_stop((ac97Intel_t*)firstDev->data);

        wav_playStartWav((ac97Intel_t*)firstDev->data); // start sound
    }
    else
    {
        printf("No audio device found.");
    }
    Syscall_GetChar();
  #endif
	return 0;
}
