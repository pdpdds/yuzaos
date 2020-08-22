#ifndef USB_HC_H
#define USB_HC_H

#include "usb.h"
#include "_pci.h"
//#include "util/array.h"

#define UHCI    0x00
#define OHCI    0x10
#define EHCI    0x20
#define XHCI    0x30
#define NO_HCI  0x80
#define ANY_HCI 0xFE


struct hc;

typedef struct
{
    port_t        port;
    struct hc*    hc;
    usb_device_t* device;
    void*         data;
    bool          connected;
} hc_port_t;

typedef struct hc
{
    array(hc_port_t*) ports;
    uint8_t           rootPortCount; // number of rootports
} hc_t;


void usb_hc_install(pciDev_t* PCIdev);

void usb_constructTransfer(usb_device_t* usbDevice, usb_transfer_t* transfer, usb_transferType_t type, usb_endpoint_t* endpoint);
uint8_t usb_setupTransaction(usb_transfer_t* transfer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length);
void usb_inTransaction(usb_transfer_t* transfer, bool controlHandshake, void* buffer, size_t length);
void usb_outTransaction(usb_transfer_t* transfer, bool controlHandshake, const void* buffer, size_t length);
void usb_scheduleTransfer(usb_transfer_t* transfer);
bool usb_pollTransfer(usb_transfer_t* transfer);
void usb_waitForTransfer(usb_transfer_t* transfer);
void usb_destructTransfer(usb_transfer_t* transfer);

void hc_setupUSBDevice(hc_t* hc, uint8_t portNumber, usb_speed_t speed);
void hc_updateEndpointInformation(hc_port_t* port);
void hc_constructRootPorts(hc_t* hc, uint8_t rootPortCount, const portType_t* type);
uint8_t hc_addPort(usb_device_t* usbDevice);
hc_port_t* hc_getPort(hc_t* hc, uint8_t num);
void hc_destroyPort(hc_t* hc, uint8_t num);


#endif
