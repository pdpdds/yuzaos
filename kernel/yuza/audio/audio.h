#ifndef AUDIO_H
#define AUDIO_H

#include "_pci.h"


typedef struct {
    void (*install)(pciDev_t*);
} audio_driver_t;


void audio_installDevice(pciDev_t* device);
DWORD WINAPI audio_test(LPVOID parameter);


#endif
