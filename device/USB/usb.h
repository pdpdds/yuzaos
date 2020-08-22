#ifndef USB_H
#define USB_H

#include "windef.h"
#include <stdint.h>
#include "_pci.h"
#include "devicemanager.h"




// Table 9-4. Standard Request Codes
#define GET_STATUS          0
#define CLEAR_FEATURE       1
#define SET_FEATURE         3
#define SET_ADDRESS         5
#define GET_DESCRIPTOR      6
#define SET_DESCRIPTOR      7
#define GET_CONFIGURATION   8
#define SET_CONFIGURATION   9
#define GET_INTERFACE      10
#define SET_INTERFACE      11
#define SYNCH_FRAME        12

// Table 9-5. Descriptor Types
#define DEVICE                      1
#define CONFIGURATION               2
#define STRING                      3
#define INTERFACE                   4
#define ENDPOINT                    5
#define DEVICE_QUALIFIER            6
#define OTHER_SPEED_CONFIGURATION   7
#define INTERFACE_POWER1            8

// Table 9-6. Standard Feature Selectors
#define DEVICE_REMOTE_WAKEUP        1
#define ENDPOINT_HALT               0
#define TEST_MODE                   2

#pragma pack (push, 1)
typedef struct
{
    uint8_t   type;
    uint8_t   request;
    uint8_t   valueLo;
    uint8_t   valueHi;
    uint16_t  index;
    uint16_t  length;
} usb_request_t;
#pragma pack (pop)

typedef enum { EP_OUT, EP_IN, EP_BIDIR } usb_endpointDir_t;
typedef enum { EP_CONTROL, EP_ISOCHRONOUS, EP_BULK, EP_INTERRUPT } usb_endpointType_t;

typedef struct
{
    usb_endpointDir_t   dir;
    usb_endpointType_t  type;
    uint8_t             address;
    uint16_t            mps;
    bool                toggle;
    uint8_t             interval;
} usb_endpoint_t;

typedef enum { USB_FULLSPEED = 1, USB_LOWSPEED = 2, USB_HIGHSPEED = 3, USB_SUPERSPEED = 4 } usb_speed_t;
typedef struct
{
    port_t*  port;
    void*    hub; // Hubs are not an interface of the device, but the device itself

    list_t   endpoints;
    list_t   interfaces;

    usb_speed_t speed;
    uint16_t vendor;
    uint16_t product;
    uint16_t releaseNumber;
    uint16_t usbSpec;
    uint8_t  usbClass;
    uint8_t  usbSubclass;
    uint8_t  usbProtocol;
    uint8_t  manufacturerStringID;
    uint8_t  productStringID;
    uint8_t  serNumberStringID;
    uint8_t  numConfigurations;
    uint8_t  num;
    char     productName[16];
    char     serialNumber[13];
    void*    totalConfigDescriptor;
} usb_device_t;


#pragma pack (push, 1)
struct usb_deviceDescriptor
{
    uint8_t  length;            // 18
    uint8_t  descriptorType;    // 1
    uint16_t bcdUSB;            // e.g. 0x0210 means 2.10
    uint8_t  deviceClass;
    uint8_t  deviceSubclass;
    uint8_t  deviceProtocol;
    uint8_t  maxPacketSize;     // MPS0, must be 8,16,32,64
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;         // release of the device
    uint8_t  manufacturer;
    uint8_t  product;
    uint8_t  serialNumber;
    uint8_t  numConfigurations; // number of possible configurations
} ;

struct usb_configurationDescriptor
{
    uint8_t  length;            // 9
    uint8_t  descriptorType;    // 2
    uint16_t totalLength;
    uint8_t  numInterfaces;
    uint8_t  configurationValue;
    uint8_t  configuration;
    uint8_t  attributes;
    uint8_t  maxPower;
} ;

struct usb_interfaceAssociationDescriptor
{
    uint8_t  length;            // 8
    uint8_t  descriptorType;    // 11
    uint8_t  firstInterface;
    uint8_t  interfaceCount;
    uint8_t  functionClass;
    uint8_t  functionSubclass;
    uint8_t  functionProtocol;
    uint8_t  function;
} ;

struct usb_interfaceDescriptor
{
    uint8_t  length;            // 9
    uint8_t  descriptorType;    // 4
    uint8_t  interfaceNumber;
    uint8_t  alternateSetting;
    uint8_t  numEndpoints;
    uint8_t  interfaceClass;
    uint8_t  interfaceSubclass;
    uint8_t  interfaceProtocol;
    uint8_t  interface;
} ;

struct usb_endpointDescriptor
{
    uint8_t  length;            // 7
    uint8_t  descriptorType;    // 5
    uint8_t  endpointAddress;
    uint8_t  attributes;
    uint16_t maxPacketSize;
    uint8_t  interval;
} ;

struct usb_stringDescriptor
{
    uint8_t  length;         // ?
    uint8_t  descriptorType; // 3
    uint16_t wstring[10];    // n = 10 test-wise
} ;

struct usb_stringDescriptorUnicode
{
    uint8_t  length;         // 2 + 2 * numUnicodeCharacters
    uint8_t  descriptorType; // 3
    uint8_t  string[60];     // n = 30 test-wise (60, because we use uint8_t as type)
} ;
#pragma pack (pop)

typedef struct
{
    void*                          data;
    usb_device_t*                  device;
    struct usb_interfaceDescriptor descriptor;
} usb_interface_t;


typedef enum
{
    USB_BULK, USB_CONTROL, USB_INTERRUPT, USB_ISOCHRONOUS
} usb_transferType_t;

typedef struct usb_transfer
{
    usb_device_t*      device;
    usb_endpoint_t*    endpoint;
    usb_transferType_t type;
    uint32_t           packetSize;
    list_t             transactions;
    void             (*handler)(struct usb_transfer*, void*); // Handler to be called on successful completion of transfer
    void*              handlerData;
    void*              data;
    uint8_t            frequency;
    bool               success;
} usb_transfer_t;

typedef enum
{
    USB_TT_OUT = 0, USB_TT_IN = 1, USB_TT_SETUP = 2
} usb_transactionType_t;

typedef struct
{
    void*                 data; // Contains pointer to *hci_transaction_t
    usb_transactionType_t type;
} usb_transaction_t;


usb_device_t* usb_createDevice(port_t* port, usb_speed_t speed);
void usb_destroyDevice(usb_device_t* device);
void usb_setupDevice(usb_device_t* device, uint8_t address);
uint8_t usb_setDeviceAddress(usb_device_t* device, uint8_t num);
bool usb_getDeviceDescriptor(usb_device_t* device, uint8_t length, bool first);
bool usb_getConfigDescriptor(usb_device_t* device);
void usb_getStringDescriptor(usb_device_t* device);
void usb_getUnicodeStringDescriptor(usb_device_t* device, uint8_t stringIndex);
void usb_setConfiguration(usb_device_t* device, uint8_t configuration);
void usb_setInterface(usb_device_t* device, uint8_t alternateInterface, uint16_t interfaceID);
uint8_t usb_getConfiguration(usb_device_t* device);
uint8_t usb_getInterface(usb_device_t* device, uint16_t interfaceID);
uint16_t usb_getStatus(usb_device_t* device, usb_endpoint_t* endpoint);

void usb_setFeatureHALT(usb_device_t* device, usb_endpoint_t* endpoint);
void usb_clearFeatureHALT(usb_device_t* device, usb_endpoint_t* endpoint);

bool usb_controlIn(usb_device_t* device, void* buffer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length);
bool usb_controlOut(usb_device_t* device, const void* buffer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length);
bool usb_controlSet(usb_device_t* device, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index);

void usb_startInterruptInTransfer(usb_transfer_t* transfer, usb_device_t* device, usb_endpoint_t* epInterrupt, void* buffer, size_t length, void(*handler)(usb_transfer_t*, void*), void* handlerData, uint8_t frequency);
void usb_pollInterruptTransfers(void);
void usb_IsochronousInTransfer(usb_device_t* device, usb_endpoint_t* epIsochronous, size_t length, void* buffer, uint16_t times, uint16_t numChains);

//extern const portType_t FDD, USB_UHCI, USB_OHCI, USB_EHCI, USB_XHCI, RAM, HDD;
//extern const diskType_t FLOPPYDISK, USB_MSD, RAMDISK, HDDDISK;


#endif
