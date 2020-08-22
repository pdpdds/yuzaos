#ifndef AC97_INTEL_H
#define AC97_INTEL_H

#include "windef.h"
#include "_pci.h"

////////////////////////////////////////////
// First Address space
// NAM - Native Audio Mixer

// "Audio Codec ?7" Revision 2.3 Revision 1.0 April, 2002, Intel
// chapter 5.7, "Baseline Audio Register Set"
// Table 16: "Baseline Audio Register Map"

/*
Appendix A. AC ?7 Register Set Summary
The AC ?7 Register space supports 64 16-bit registers using even 7-bit addresses.
?Registers 00h ?26h are dedicated to the baseline audio feature set
?Registers 28h ?3Ah are dedicated to the extended audio feature set
?Registers 3Ch ?58h are dedicated to the standardized modem feature set
?Registers 5Ah ?7Ah are reserved for vendor-specific functionality
?Registers 7Ch and 7Eh are dedicated to Microsoft Plug and Play Vendor ID** (3 bytes) and vendor specific
device ID (8-bits)
*/

#define PORT_NAM_RESET                0x0000
#define PORT_NAM_MASTER_VOLUME        0x0002
#define PORT_NAM_AUX_OUT_VOLUME       0x0004
#define PORT_NAM_MONO_VOLUME          0x0006
#define PORT_NAM_MASTER_TONE          0x0008
#define PORT_NAM_PC_BEEP_VOLUME       0x000A
#define PORT_NAM_PHONE_VOLUME         0x000C
#define PORT_NAM_MIC_VOLUME           0x000E
#define PORT_NAM_LINE_IN_VOLUME       0x0010
#define PORT_NAM_CD_VOLUME            0x0012
#define PORT_NAM_VIDEO_VOLUME         0x0014
#define PORT_NAM_AUX_IN_VOLUME        0x0016
#define PORT_NAM_PCM_OUT_VOLUME       0x0018
#define PORT_NAM_RECORD_SELECT        0x001A
#define PORT_NAM_RECORD_GAIN          0x001C
#define PORT_NAM_RECORD_GAIN_MIC      0x001E
#define PORT_NAM_GENERAL_PURPOSE      0x0020
#define PORT_NAM_3D_CONTROL           0x0022
#define PORT_NAM_AUDIO_INT_PAGING     0x0024
#define PORT_NAM_POWERDOWN_CTRL_STS   0x0026

#define PORT_NAM_EXT_AUDIO_ID         0x0028
#define PORT_NAM_EXT_AUDIO_STS_CTRL   0x002A
#define PORT_NAM_FRONT_DAC_RATE       0x002C
#define PORT_NAM_SURR_DAC_RATE        0x002E
#define PORT_NAM_LFE_DAC_RATE         0x0030
#define PORT_NAM_LR_ADC_RATE          0x0032
#define PORT_NAM_MIC_ADC_RATE         0x0034
#define PORT_NAM_CENTER_LFE_VOLUME    0x0036
#define PORT_NAM_SURR_VOLUME          0x0038
#define PORT_NAM_S_PDIF_CTRL          0x003A

#define PORT_NAM_EXTENDED_MODEM_ID    0x003C
// for details of the following modem regs cf. spec table 68

#define PORT_NAM_VENDOR_RESERVED_1    0x005A
#define PORT_NAM_PAGE_REGISTERS       0x0060
#define PORT_NAM_VENDOR_RESERVED_2    0x0070
#define PORT_NAM_VENDOR_ID_1          0x007C
#define PORT_NAM_VENDOR_ID_2          0x007E

////////////////////////////////////////////
// Second Address space
// NABM - Native Audio Bus Mastering

// source: wiki at lowlevel.eu about ac97
// http://www.lowlevel.eu/wiki/AC97#Anhang
//
#define PORT_NABM_POBDBAR             0x0010
#define PORT_NABM_POLVI               0x0015
#define PORT_NABM_PICONTROL           0x000B
#define PORT_NABM_POCONTROL           0x001B
#define PORT_NABM_MCCONTROL           0x002B
#define PORT_NABM_PISTATUS            0x0006
#define PORT_NABM_POSTATUS            0x0016
#define PORT_NABM_MCSTATUS            0x0026

#pragma pack(push, 1)
typedef struct
{
    uint32_t buf;
    uint16_t len;
    uint16_t reserved : 14;
    uint16_t bup :       1;
    uint16_t ioc :       1;
} ac97Intel_bufDesc_t;

typedef struct
{
    pciDev_t*   device;
    uint16_t    nambar;  // NAM-BAR  // Mixer
    uint16_t    nabmbar; // NABM-BAR // Player
    uint16_t*   buffer;  // sample buffer
    uint32_t    numDesc;
    ac97Intel_bufDesc_t* descs;
    uint32_t    NUM;
} ac97Intel_t;
#pragma pack(pop)

void ac97Intel_install(pciDev_t* device);
void ac97Intel_start(ac97Intel_t* ac97);
void ac97Intel_stop(ac97Intel_t* ac97);


#endif
