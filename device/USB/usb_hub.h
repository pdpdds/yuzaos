#ifndef USB_HUB_H
#define USB_HUB_H

#include "windef.h"
#include "usb.h"

// Table 11-15. Hub Class Request Type

#define SET_HUB              0x20
#define SET_PORT             0x23
#define GET_HUB              0xA0
#define GET_PORT             0xA3


// Table 11-16. Hub Class Request Codes
#define GET_STATUS              0
#define CLEAR_FEATURE           1
#define SET_FEATURE             3
#define GET_DESCRIPTOR          6
#define SET_DESCRIPTOR          7
#define CLEAR_TT_BUFFER         8
#define RESET_TT                9
#define GET_TT_STATE           10
#define STOP_TT                11


// Table 11-17. Hub Class Feature Selectors
// recipient: Hub
#define C_HUB_LOCAL_POWER       0
#define C_HUB_OVER_CURRENT      1
// recipient: Port
#define PORT_CONNECTION         0
#define PORT_ENABLE             1
#define PORT_SUSPEND            2
#define PORT_OVER_CURRENT       3
#define PORT_RESET              4
#define PORT_POWER              8
#define PORT_LOW_SPEED          9
#define C_PORT_CONNECTION      16
#define C_PORT_ENABLE          17
#define C_PORT_SUSPEND         18
#define C_PORT_OVER_CURRENT    19
#define C_PORT_RESET           20
#define PORT_TEST              21
#define PORT_INDICATOR         22

#pragma pack(push, 1)
typedef struct // cf. 11.23.2.1 Hub Descriptor, Table 11-13
{
    // The hub descriptor itself (7 stable and 2 or more variable fields)
    uint8_t  length;            // descriptor length
    uint8_t  descriptorType;    // descriptor type, value: 29H for hub descriptor
    uint8_t  numberPorts;       // number of ports this hub supports

    uint16_t logicalPowerSwitchingMode  :  2;
    uint16_t compoundDevice             :  1;
    uint16_t overCurrentProtectionMode  :  2;
    uint16_t ttThinkTime                :  2;
    uint16_t portIndicatorsSupported    :  1;
    uint8_t  reserved;

    uint8_t  pwrOn2PwrGood;
    uint8_t  hubControllerCurrent;

    uint8_t  deviceRemovable[8]; // bit0 = reserved, bit1 = port1, bit2 = port2, ... (field size depends on numberPorts)
} usb_hubDescriptor_t;


// 11.24.2.7.1 Port Status Bits
typedef struct
{
    // status
    uint32_t currentConnectStatus      :  1; // bit0
    /* bit0 is reset to zero when the port is in the Powered-off state or the Disconnected states.
       It is set to one when the port is in the Powered state, a device attach is detected,
       and the port transitions from the Disconnected state to the Disabled state.*/
    uint32_t portEnabledDisabled       :  1; // bit1
    uint32_t suspend                   :  1; // bit2
    uint32_t overCurrent               :  1; // bit3
    uint32_t reset                     :  1; // bit4
    uint32_t reserved1                 :  3; // bit5-7
    uint32_t portPower                 :  1; // bit8
    uint32_t lowSpeedDevAttached       :  1; // bit9  // 0=fullspeed 1=lowspeed
    uint32_t hiSpeedDevAttached        :  1; // bit10 // 0=fullspeed 1=hispeed
    uint32_t portTestMode              :  1; // bit11
    uint32_t portIndicatorControl      :  1; // bit12
    uint32_t reserved2                 :  3; // bit13-15

    // change
    uint32_t connectStatusChange       :  1; // bit0
    uint32_t portEnableDisableChange   :  1; // bit1
    uint32_t suspendChange             :  1; // bit2
    uint32_t overCurrentChange         :  1; // bit3
    uint32_t resetChange               :  1; // bit4
    uint32_t reserved3                 : 11; // bit5-15
} portStatusAndChange_t;

// Hub Status Field and Hub Change Field // Tables 11-19 and 11-20
typedef struct
{
    uint32_t localPowerSource           :  1;
    uint32_t overCurrent                :  1;
    uint32_t reserved1                  : 14;
    uint32_t localPowerStatusChange     :  1;
    uint32_t overCurrentChange          :  1;
    uint32_t reserved2                  : 14;
} hubStatusAndChange_t;

typedef struct
{
    usb_device_t*        device;
    usb_endpoint_t*      endpointInterrupt;
    usb_hubDescriptor_t  hubDesc;
    hubStatusAndChange_t status;
    uint8_t              portNumber[16];
} usb_hub_t;
#pragma pack(pop)

void usb_setupHub(usb_device_t* device);
void usb_destroyHub(usb_device_t* device);

bool usb_clearHubFeature(usb_device_t* device, uint8_t feature);
bool usb_clearPortFeature(usb_device_t* device, uint8_t feature, uint8_t port);
bool usb_clearTTBuffer(usb_device_t* device, uint8_t port);
bool usb_getHubStatus(usb_device_t* device);
bool usb_getPortStatus(usb_device_t* device, uint8_t port, portStatusAndChange_t* portStatusAndChange);
bool usb_isHubPortConnected(usb_device_t* device, uint8_t port);
bool usb_getTTState(usb_device_t* device, uint8_t TT_flags, uint8_t port);
bool usb_resetTT(usb_device_t* device, uint8_t port);
bool usb_setHubDescriptor(usb_device_t* device, uint8_t* hdData, uint8_t hdLength);
bool usb_setHubFeature(usb_device_t* device, uint8_t feature);
bool usb_setPortFeature(usb_device_t* device, uint8_t feature, uint8_t selector, uint8_t port);
bool usb_stopTT(usb_device_t* device, uint8_t port);

void usb_hubWatchdog(void);

#endif
