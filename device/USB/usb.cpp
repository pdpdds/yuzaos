/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f? die Verwendung dieses Sourcecodes siehe unten
*/

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "usb.h"
#include "usb_hc.h"
//#include "usb_hub.h"
//#include "hid/usb_hid.h"
#include "usb_msd.h"
#include <math.h>
#include <systemcall_impl.h>
#include "ehciqhqtd.h"

#define printfe printf
#define textColor(a)

list_t usb_interruptTransfers = list_init();


static void analyzeDeviceDescriptor(const struct usb_deviceDescriptor* d, usb_device_t* usbDev);
static void showDevice(const usb_device_t* usbDev, bool first);
static void showConfigurationDescriptor(const struct usb_configurationDescriptor* d);
static void showInterfaceAssociationDescriptor(const struct usb_interfaceAssociationDescriptor* d);
static void showInterfaceDescriptor(const struct usb_interfaceDescriptor* d);
static void showEndpointDescriptor(const struct usb_endpointDescriptor* d);
static void showStringDescriptor(const struct usb_stringDescriptor* d);
static void showUnicodeStringDescriptor(const struct usb_stringDescriptorUnicode* d, usb_device_t* device, uint32_t stringIndex);


usb_device_t* usb_createDevice(port_t* port, usb_speed_t speed)
{
    usb_device_t* device = (usb_device_t*)malloc_aligned(sizeof(usb_device_t), 4096);
    memset(device, 0, sizeof(usb_device_t));
    device->port = port;
    device->speed = speed;
    list_construct(&device->interfaces);
    list_construct(&device->endpoints);
    dlelement_t* ep0_elem = list_alloc_elem(sizeof(usb_endpoint_t), "usb_endpoint 0");
    usb_endpoint_t* ep0 = (usb_endpoint_t *)ep0_elem->data;
    ep0->address = 0;
    switch (speed)
    {
        case USB_LOWSPEED: case USB_FULLSPEED:
            ep0->mps = 8; // fullspeed can have MPS between 8 and 64 - we will find out later, start with 8
            break;
        case USB_HIGHSPEED:
            ep0->mps = 64;
            break;
        case USB_SUPERSPEED:
            ep0->mps = 512;
            break;
    }
    ep0->dir = EP_BIDIR;
    ep0->type = EP_CONTROL;
    ep0->toggle = false;
    ep0->interval = 0;
    list_append_elem(&device->endpoints, ep0_elem);
    memset(device->serialNumber, 0, 13);
    return (device);
}

void usb_destroyDevice(usb_device_t* device)
{
    if (!device)
        return;

    //if (device->usbClass == 0x09) // Hub
        //usb_destroyHub(device);

    for (dlelement_t* el = device->interfaces.head; el; el = el->next)
    {
        usb_interface_t* interface = (usb_interface_t *)el->data;
        if (interface->descriptor.interfaceClass == 0x08 && (interface->descriptor.interfaceSubclass == 0x06 || // MSD (SCSI transparent command set)
                                                             interface->descriptor.interfaceSubclass == 0x04))  // UFI (usb-Floppy)
            usb_destroyMSD(interface);
        //else if (interface->descriptor.interfaceClass == 0x03) // HID
           // usb_destroyHID(interface);
        //else if (interface->descriptor.interfaceClass == 0x0E && interface->descriptor.interfaceSubclass == 0x01) // Video Control
           // usb_destroyVideo(interface);
    }
    list_destruct(&device->interfaces);
    list_destruct(&device->endpoints);
    free(device);
}

void usb_setupDevice(usb_device_t* device, uint8_t address)
{
    device->num = 0; // device number has to be set to 0
    device->hub = 0;
    device->totalConfigDescriptor = 0;
    bool success = false;

    bool startWith8 = device->speed == USB_FULLSPEED; // Start with 8 bytes, since MPS is unknown
    success = usb_getDeviceDescriptor(device, startWith8?8:18, true);

    if (!success)
    {
        success = usb_getDeviceDescriptor(device, startWith8?8:18, true);
    }
    if (!success)
    {
        printfe("\nGet Device Descriptor failed!");
        return;
    }

    device->num = usb_setDeviceAddress(device, address);

    if (startWith8)
        usb_getDeviceDescriptor(device, 18, false); // Get all 18 bytes

 #ifdef _USB_DIAGNOSIS_
    printf("\ndevice address: %u", device->num);
 #endif

    success = usb_getConfigDescriptor(device);
    if (!success)
    {
        success = usb_getConfigDescriptor(device);
    }

    if (!success)
    {
        printfe("\nConfigDescriptor could not be read! Setup Device interrupted!");
        return;
    }
  
    usb_setConfiguration(device, 1); // set first configuration

  #ifdef _USB_DIAGNOSIS_
    printf("\nconfiguration: %u", usb_getConfiguration(device)); // check configuration
    //waitForKeyStroke();
  #endif

    /*if (device->usbClass == 0x09) // Hub
        usb_setupHub(device);
    else*/
    {
        bool foundSomething = false;
        for (dlelement_t* el = device->interfaces.head; el; el = el->next)
        {
            usb_interface_t* interface = (usb_interface_t *)el->data;
            if (interface->descriptor.interfaceClass == 0x08 && (interface->descriptor.interfaceSubclass == 0x06 || // MSD (SCSI transparent command set)
                                                                 interface->descriptor.interfaceSubclass == 0x04))  // UFI (usb-Floppy)
            {
                foundSomething = true;
                usb_setupMSD(interface);
            }
            /*else if (interface->descriptor.interfaceClass == 0x03) // HID
            {
                foundSomething = true;
                usb_setupHID(interface);
            }
            else if (interface->descriptor.interfaceClass == 0x0E && interface->descriptor.interfaceSubclass == 0x01) // Video Control
            {
                foundSomething = true;
                usb_setupVideo(interface);
            }*/
        }
        if (!foundSomething)
        {
            printfe("\nUnsupported device type!");
        }
    }

    free(device->totalConfigDescriptor);
    device->totalConfigDescriptor = 0;
}

bool usb_controlIn(usb_device_t* device, void* buffer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
    usb_transfer_t transfer;
    usb_constructTransfer(device, &transfer, USB_CONTROL, (usb_endpoint_t *)device->endpoints.head->data);
    usb_setupTransaction(&transfer, type, req, hiVal, loVal, index, length);
    usb_inTransaction(&transfer, false, buffer, length);
    usb_outTransaction(&transfer, true, 0, 0);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
    return transfer.success;
}

bool usb_controlOut(usb_device_t* device, const void* buffer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
    usb_transfer_t transfer;
    usb_constructTransfer(device, &transfer, USB_CONTROL, (usb_endpoint_t*)device->endpoints.head->data);
    usb_setupTransaction(&transfer, type, req, hiVal, loVal, index, length);
    usb_outTransaction(&transfer, false, buffer, length);
    usb_inTransaction(&transfer, true, 0, 0);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
    return transfer.success;
}

bool usb_controlSet(usb_device_t* device, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index)
{
    usb_transfer_t transfer;
    usb_constructTransfer(device, &transfer, USB_CONTROL, (usb_endpoint_t*)device->endpoints.head->data);
    usb_setupTransaction(&transfer, type, req, hiVal, loVal, index, 0);
    usb_inTransaction(&transfer, true, 0, 0);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
    return transfer.success;
}

uint8_t usb_setDeviceAddress(usb_device_t* device, uint8_t num)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: SET_ADDRESS: %u", num);
    textColor(TEXT);
  #endif

    uint8_t new_address = num; // indicated port number

    usb_transfer_t transfer;
    usb_constructTransfer(device, &transfer, USB_CONTROL, (usb_endpoint_t*)device->endpoints.head->data);
    new_address = usb_setupTransaction(&transfer, 0x00, SET_ADDRESS, 0, new_address, 0, 0);
    usb_inTransaction(&transfer, true, 0, 0);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);

  #ifdef _USB_TRANSFER_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\nnew address: %u", new_address);
    textColor(TEXT);
    waitForKeyStroke();
  #endif

    return new_address;
}

bool usb_getDeviceDescriptor(usb_device_t* device, uint8_t length, bool first)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_DESCRIPTOR Device");
    textColor(TEXT);
  #endif

    //static usb_deviceDescriptor descriptor;
    usb_deviceDescriptor* descriptor = (usb_deviceDescriptor *)malloc(sizeof(usb_deviceDescriptor));

    bool success = usb_controlIn(device, descriptor, 0x80, GET_DESCRIPTOR, 1, 0, 0, length);
    
    if (success)
    {
        analyzeDeviceDescriptor(descriptor, device);
        showDevice(device, first);
    
    }

    free(descriptor);

    return success;
}

bool usb_getConfigDescriptor(usb_device_t* device)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_DESCRIPTOR Config");
    textColor(TEXT);
  #endif

    usb_configurationDescriptor* cfgdesc = (usb_configurationDescriptor*)malloc_aligned(sizeof(usb_configurationDescriptor), 4096);;
    bool success = usb_controlIn(device, cfgdesc, 0x80, GET_DESCRIPTOR, 2, 0, 0, sizeof(cfgdesc));

    if (!success)
        return false;
    printf("ggggggg\n");
   
    const uint16_t bufferSize = cfgdesc->totalLength;
    char* buffer = (char*)malloc_aligned(bufferSize , 4096);
   
    success = usb_controlIn(device, buffer, 0x80, GET_DESCRIPTOR, 2, 0, 0, bufferSize);
   
    if (success)
    {
        uint16_t configSize = MIN(bufferSize, *(uint16_t*)(buffer + 2));

      #ifdef _USB_TRANSFER_DIAGNOSIS_
        textColor(LIGHT_GRAY);
        printf("\n---------------------------------------------------------------------\n");
        textColor(GREEN);
        printf("\nconfig descriptor - total length: %u", configSize);
        waitForKeyStroke();
      #endif
        device->totalConfigDescriptor = malloc_aligned(configSize, 4096);
        memcpy(device->totalConfigDescriptor, buffer, configSize);

      #ifdef _USB_TRANSFER_DIAGNOSIS_
        memshow(buffer, configSize, false);
        putch('\n');
      #endif
        
        //20200330
        char* addr     = buffer;
        void* lastByte = addr + configSize; // totalLength (WORD)

        // Analyze descriptors
        while ((uintptr_t)addr < (uintptr_t)lastByte)
        {
            uint8_t length  = *(uint8_t*)(addr+0);
            uint8_t type    = *(uint8_t*)(addr+1);

            if (length == 9 && type == CONFIGURATION) // CONFIGURATION descriptor
            {
                struct usb_configurationDescriptor* descriptor = (struct usb_configurationDescriptor*)addr;
                showConfigurationDescriptor(descriptor);
            }
            else if (length == 8 && type == 0x0B) // INTERFACE ASSOCIATION Descriptor
            {
                struct usb_interfaceAssociationDescriptor* descriptor = (struct usb_interfaceAssociationDescriptor*)addr;
                showInterfaceAssociationDescriptor(descriptor);
            }
            else if (length == 9 && type == INTERFACE) // INTERFACE descriptor
            {
                struct usb_interfaceDescriptor* descriptor = (struct usb_interfaceDescriptor*)addr;
                showInterfaceDescriptor(descriptor);

                dlelement_t* elem = list_alloc_elem(sizeof(usb_interface_t), "usb_interface_t");
                usb_interface_t* interface = (usb_interface_t * )elem->data;
                memcpy(&interface->descriptor, descriptor, sizeof(struct usb_interfaceDescriptor));
                interface->device = device;
                interface->data   = 0;
                list_append_elem(&device->interfaces, elem);
            }
            else if (length == 7 && type == ENDPOINT) // ENDPOINT descriptor
            {
                struct usb_endpointDescriptor* descriptor = (struct usb_endpointDescriptor * )addr;
                showEndpointDescriptor(descriptor);

                dlelement_t* elem = list_alloc_elem(sizeof(usb_endpoint_t), "usb_endpoint_t");
                usb_endpoint_t* ep = (usb_endpoint_t*)elem->data;

                if (descriptor->endpointAddress & 0x80)
                    ep->dir = EP_IN;
                else
                    ep->dir = EP_OUT;

                ep->address  = descriptor->endpointAddress & 0xF;
                ep->mps      = descriptor->maxPacketSize;
                ep->interval = descriptor->interval;
                ep->toggle   = false;
                ep->type     = (usb_endpointType_t)(descriptor->attributes & 0x3);

              #ifdef _USB_TRANSFER_DIAGNOSIS_
                textColor(DATA);
                printf("\nep address: %u", ep->address);
                textColor(TEXT);
              #endif

                list_append_elem(&device->endpoints, elem);
            }
            else
            {
              #ifdef _USB_TRANSFER_DIAGNOSIS_
                printf("\nUnknown part of ConfigDescriptor: length: %u type: %u\n", length, type);
                waitForKeyStroke();
              #endif
            }
            addr += length;
        }//while

        hc_updateEndpointInformation((hc_port_t *)device->port->data); // We now have information about all EPs
    }//if
    
    free(buffer);
    return success;
}

void usb_getStringDescriptor(usb_device_t* device)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_DESC string, dev: %X endpoint: 0 languageIDs", device);
    textColor(TEXT);
  #endif

    struct usb_stringDescriptor descriptor;
    usb_controlIn(device, &descriptor, 0x80, GET_DESCRIPTOR, 3, 0, 0, sizeof(descriptor));

  #ifdef _USB_TRANSFER_DIAGNOSIS_
    memshow(&descriptor, sizeof(descriptor), false);
    putch('\n');
  #endif
    showStringDescriptor(&descriptor);
}

void usb_getUnicodeStringDescriptor(usb_device_t* device, uint8_t stringIndex)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_DESC unicode string, dev: %X endp.: 0 strIndex: %u", device, stringIndex);
    textColor(TEXT);
  #endif

    char buffer[64];
    usb_controlIn(device, buffer, 0x80, GET_DESCRIPTOR, 3, stringIndex, 0x0409, 64);

  #ifdef _USB_TRANSFER_DIAGNOSIS_
    memshow(buffer, 64, false);
    putch('\n');
  #endif

    showUnicodeStringDescriptor((struct usb_stringDescriptorUnicode*)buffer, device, stringIndex);
}

// http://www.lowlevel.eu/wiki/USB#SET_CONFIGURATION
void usb_setConfiguration(usb_device_t* device, uint8_t configuration)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: SET_CONFIGURATION %u", configuration);
    textColor(TEXT);
  #endif

    usb_controlSet(device, 0x00, SET_CONFIGURATION, 0, configuration, 0);
}

uint8_t usb_getConfiguration(usb_device_t* device)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_CONFIGURATION");
    textColor(TEXT);
  #endif

    uint8_t configuration;
    usb_controlIn(device, &configuration, 0x80, GET_CONFIGURATION, 0, 0, 0, 1);
    return configuration;
}

void usb_setInterface(usb_device_t* device, uint8_t alternateInterface, uint16_t interfaceID)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: SET_INTERFACE %u alternate Interface: %u", interfaceID, alternateInterface);
    textColor(TEXT);
  #endif

    usb_controlSet(device, 0x01, SET_INTERFACE, 0, alternateInterface, interfaceID);
}

uint8_t usb_getInterface(usb_device_t* device, uint16_t interfaceID)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_INTERFACE %u", interfaceID);
    textColor(TEXT);
  #endif

    uint8_t alternateInterface;
    usb_controlIn(device, &alternateInterface, 0x81, GET_INTERFACE, 0, 0, interfaceID, 1);
    return alternateInterface;
}

// seems not to work correct, does not set HALT ???
void usb_setFeatureHALT(usb_device_t* device, usb_endpoint_t* endpoint)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: usbSetFeatureHALT, endpoint: %u", endpoint);
    textColor(TEXT);
  #endif

    usb_controlSet(device, 0x02, SET_FEATURE, 0, 0, endpoint->address);

  #ifdef _USB_TRANSFER_DIAGNOSIS_
    printf("\nset HALT at dev: %X endpoint: %u", device, endpoint);
  #endif
}

void usb_clearFeatureHALT(usb_device_t* device, usb_endpoint_t* endpoint)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: usbClearFeatureHALT, endpoint: %u", endpoint);
    textColor(TEXT);
  #endif

    usb_controlSet(device, 0x02, CLEAR_FEATURE, 0, 0, endpoint->address);

  #ifdef _USB_DIAGNOSIS_
    printf("\nclear HALT at dev: %X endpoint: %u", device, endpoint);
  #endif
}

uint16_t usb_getStatus(usb_device_t* device, usb_endpoint_t* endpoint)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: usbGetStatus at device: %X endp.: %u", device, endpoint);
    textColor(TEXT);
  #endif

    uint16_t status;
    usb_controlIn(device, &status, 0x02, GET_STATUS, 0, 0, 0, 2);
    return status;
}

static void analyzeDeviceDescriptor(const struct usb_deviceDescriptor* d, usb_device_t* usbDev)
{
    usbDev->usbSpec              = d->bcdUSB;
    usbDev->usbClass             = d->deviceClass;
    usbDev->usbSubclass          = d->deviceSubclass;
    usbDev->usbProtocol          = d->deviceProtocol;
    usb_endpoint_t* ep0 = (usb_endpoint_t *)usbDev->endpoints.head->data;
    bool changedMPS = ep0->mps != d->maxPacketSize;
    if (changedMPS)
    {
        ep0->mps = d->maxPacketSize;
        hc_updateEndpointInformation((hc_port_t *)usbDev->port->data);
    }
    if (d->length > 8)
    {
        usbDev->vendor               = d->idVendor;
        usbDev->product              = d->idProduct;
        usbDev->releaseNumber        = d->bcdDevice;
        usbDev->manufacturerStringID = d->manufacturer;
        usbDev->productStringID      = d->product;
        usbDev->serNumberStringID    = d->serialNumber;
        usbDev->numConfigurations    = d->numConfigurations;
    }
}

static void showDevice(const usb_device_t* usbDev, bool first)
{
    if (usbDev->usbSpec == 0x0100 || usbDev->usbSpec == 0x0110 ||
        usbDev->usbSpec == 0x0200 || usbDev->usbSpec == 0x0201 || usbDev->usbSpec == 0x0210 || usbDev->usbSpec == 0x0213 ||
        usbDev->usbSpec == 0x0300 || usbDev->usbSpec == 0x0310)
    {
        if (first)
        {
            textColor(SUCCESS);
            printf("\nUSB %y.%y\t", BYTE2(usbDev->usbSpec), BYTE1(usbDev->usbSpec)); // e.g. 0x0210 means 2.10
            textColor(TEXT);
        }
    }
    else
    {
        printfe("\nInvalid USB version %y.%y!", BYTE2(usbDev->usbSpec), BYTE1(usbDev->usbSpec));
        return;
    }

    if (first)
    {
        if (usbDev->usbClass == 0x09)
        {
            switch (usbDev->usbProtocol)
            {
            case 0:
                printf(" - Full speed USB hub");
                break;
            case 1:
                printf(" - Hi-speed USB hub with single TT");
                break;
            case 2:
                printf(" - Hi-speed USB hub with multiple TTs");
                break;
            }
        }

        usb_endpoint_t* ep0 = (usb_endpoint_t *)usbDev->endpoints.head->data;
        printf("\nendpoint 0 mps: %u bytes.", ep0->mps);
    }
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    printf("\nDevice: Class %yh, subclass: %yh, protocol %yh.\n", usbDev->usbClass, usbDev->usbSubclass, usbDev->usbProtocol);
    printf("vendor:            %xh\n",   usbDev->vendor);
    printf("product:           %xh\t",   usbDev->product);
    printf("release number:    %y.%y\n", BYTE2(usbDev->releaseNumber), BYTE1(usbDev->releaseNumber));
    printf("manufacturer:      %xh\t",   usbDev->manufacturerStringID);
    printf("product:           %xh\n",   usbDev->productStringID);
    printf("serial number:     %xh\t",   usbDev->serNumberStringID);
    printf("number of config.: %u\n",    usbDev->numConfigurations); // number of possible configurations
  #endif
}

static void showConfigurationDescriptor(const struct usb_configurationDescriptor* d)
{
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    if (d->length)
    {
        textColor(IMPORTANT);
        printf("length:               %u\t\t", d->length);
        printf("descriptor type:      %u\n", d->descriptorType);
        textColor(LIGHT_GRAY);
        printf("total length:         %u\t", d->totalLength);
        textColor(IMPORTANT);
        printf("\nNumber of interfaces: %u", d->numInterfaces);
        printf("ID of config:         %xh\t", d->configurationValue);
        printf("ID of config name     %xh\n", d->configuration);
        printf("remote wakeup:        %s\t", (d->attributes & BIT(5)) ? "yes" : "no");
        printf("self-powered:         %s\n", (d->attributes & BIT(6)) ? "yes" : "no");
        printf("max power (mA):       %u\n", d->maxPower*2); // 2 mA steps used
        textColor(TEXT);
    }
  #endif
}

static void showInterfaceAssociationDescriptor(const struct usb_interfaceAssociationDescriptor* d)
{
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    printf("\n\nInterfaceAssociationDescriptor:");
    printf("\n-------------------------------");
    printf("\nfirst interface:   %u",   d->firstInterface  );
    printf("\tinterface count:   %u",   d->interfaceCount  );
    printf("\nfunction class:    %u",   d->functionClass   );
    printf("\tfunction subclass: %u",   d->functionSubclass);
    printf("\nfunction protocol: %u",   d->functionProtocol);
    printf("\tIndex stringDesc:  %u\n", d->function        );
  #endif
}

static void showInterfaceDescriptor(const struct usb_interfaceDescriptor* d)
{
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    if (d->length)
    {
        putch('\n');

        textColor(LIGHT_GRAY);
        printf("---------------------------------------------------------------------\n");
        printf("length:               %u\t\t", d->length);          // 9
        printf("descriptor type:      %u\n",   d->descriptorType);  // 4

        switch (d->numEndpoints)
        {
            case 0:
                printf("Interface %u has no endpoint and belongs to class:\n", d->interfaceNumber);
                break;
            case 1:
                printf("Interface %u has only one endpoint and belongs to class:\n", d->interfaceNumber);
                break;
            default:
                printf("Interface %u has %u endpoints and belongs to class:\n", d->interfaceNumber, d->numEndpoints);
                break;
        }

        textColor(IMPORTANT);
        switch (d->interfaceClass)
        {
            case 0x01:
                printf("Audio");
                break;
            case 0x02:
                printf("Communications and CDC Control");
                break;
            case 0x03:
                printf("HID (Human Interface Device)");
                break;
            case 0x05:
                printf("Physical");
                break;
            case 0x06:
                printf("Image");
                break;
            case 0x07:
                printf("Printer");
                break;
            case 0x08:
                printf("Mass Storage, ");
                switch (d->interfaceSubclass)
                {
                    case 0x01:
                        printf("Reduced Block Commands, ");
                        break;
                    case 0x02:
                        printf("SFF-8020i or MMC-2(ATAPI), ");
                        break;
                    case 0x03:
                        printf("QIC-157 (tape device), ");
                        break;
                    case 0x04:
                        printf("UFI (e.g. Floppy Disk), ");
                        break;
                    case 0x05:
                        printf("SFF-8070i (e.g. Floppy Disk), ");
                        break;
                    case 0x06:
                        printf("SCSI transparent command set, ");
                        break;
                }
                switch (d->interfaceProtocol)
                {
                    case 0x00:
                        printf("CBI protocol with command completion interrupt.");
                        break;
                    case 0x01:
                        printf("CBI protocol without command completion interrupt.");
                        break;
                    case 0x50:
                        printf("Bulk-Only Transport protocol.");
                        break;
                }
                break;
            case 0x0A:
                printf("CDC-Data");
                break;
            case 0x0B:
                printf("Smart Card");
                break;
            case 0x0D:
                printf("Content Security");
                break;
            case 0x0E:
                printf("Video, ");
                switch (d->interfaceSubclass)
                {
                    case 0x00:
                        printf("Subclass: Undefined, ");
                        break;
                    case 0x01:
                        printf("Subclass: Video Control, ");
                        break;
                    case 0x02:
                        printf("Subclass: Video Streaming, ");
                        break;
                    case 0x03:
                        printf("Subclass: Video Interface Collection, ");
                        break;
                }
                switch (d->interfaceProtocol)
                {
                    case 0x00:
                        printf("Protocol: Undefined.");
                        break;
                    case 0x01:
                        printf("Protocol: Protocol_15.");
                        break;
                }
                break;
            case 0x0F:
                printf("Personal Healthcare");
                break;
            case 0xDC:
                printf("Diagnostic Device");
                break;
            case 0xE0:
                printf("Wireless Controller, subclass: %yh protocol: %yh.", d->interfaceSubclass, d->interfaceProtocol);
                break;
            case 0xEF:
                printf("Miscellaneous");
                break;
            case 0xFE:
                printf("Application Specific");
                break;
            case 0xFF:
                printf("Vendor Specific");
                break;
        }
        textColor(TEXT);

        printf("\nalternate Setting:  %u\n",  d->alternateSetting);
        printf("interface class:      %u\n",  d->interfaceClass);
        printf("interface subclass:   %u\n",  d->interfaceSubclass);
        printf("interface protocol:   %u\n",  d->interfaceProtocol);
        printf("interface:            %xh\n", d->interface);
    }
  #endif
}

static void showEndpointDescriptor(const struct usb_endpointDescriptor* d)
{
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    if (d->length)
    {
        textColor(LIGHT_GRAY);
        printf("\n---------------------------------------------------------------------\n");
        textColor(IMPORTANT);
        //printf("length:      %u\t\t",   d->length);         // 7
        //printf("descriptor type: %u\n", d->descriptorType); // 5
        printf("endpoint %u: %s, ",     d->endpointAddress & 0xF, (d->endpointAddress & 0x80) ? "IN " : "OUT");
        printf("attributes: %yh\t",     d->attributes);
        // bit 1:0 00 control    01 isochronous    10 bulk                         11 interrupt
        // bit 3:2 00 no sync    01 async          10 adaptive                     11 sync (only if isochronous)
        // bit 5:4 00 data endp. 01 feedback endp. 10 explicit feedback data endp. 11 reserved (Iso Mode)

        if (d->attributes == 2)
        {
            printf("\nbulk data,");
        }
        printf(" mps: %u byte",  d->maxPacketSize);
        printf(" interval: %u\n",  d->interval);
        textColor(TEXT);
    }
  #endif
}

static void showStringDescriptor(const struct usb_stringDescriptor* d)
{
    if (d->length)
    {
        textColor(IMPORTANT);

      #ifdef _USB_TRANSFER_DIAGNOSIS_
        printf("\nlength:          %u\t", d->length);         // 12
        printf("\tdescriptor type: %u\n", d->descriptorType); //  3
      #endif

        printf("\n\nlanguages: ");
        for (uint8_t i=0; i<10; i++)
        {
            if (d->wstring[i] >= 0x0400 && d->wstring[i] <= 0x0465)
            {
                switch (d->wstring[i])
                {
                    case 0x401:
                        printf("Arabic");
                        break;
                    case 0x404:
                        printf("Chinese");
                        break;
                    case 0x407:
                        printf("German");
                        break;
                    case 0x409:
                        printf("English");
                        break;
                    case 0x40A:
                        printf("Spanish");
                        break;
                    case 0x40C:
                        printf("French");
                        break;
                    case 0x410:
                        printf("Italian");
                        break;
                    case 0x411:
                        printf("Japanese");
                        break;
                    case 0x416:
                        printf("Portuguese");
                        break;
                    case 0x419:
                        printf("Russian");
                        break;
                    default:
                        printf("language code: %xh", d->wstring[i]);
                        break;
                }
            }
        }
        //putch('\n');
        textColor(TEXT);
    }
}

static void showUnicodeStringDescriptor(const struct usb_stringDescriptorUnicode* d, usb_device_t* device, uint32_t stringIndex)
{
    if (d->length)
    {
      #ifdef _USB_TRANSFER_DIAGNOSIS_
        textColor(IMPORTANT);
        printf("\nlength:          %u\t", d->length);
        printf("\tdescriptor type: %u", d->descriptorType);
        printf("\nstring:          ");
        textColor(DATA);
      #endif
        char asciichar[31] = {0};

        for (uint8_t i=0; i<MIN(60, (d->length-2)); i+=2) // show only low value of Unicode character
        {
            if (d->string[i])
            {
              #ifdef _USB_TRANSFER_DIAGNOSIS_
                putch(d->string[i]);
              #endif
                asciichar[i/2] = (char)d->string[i];
            }
        }
      #ifdef _USB_TRANSFER_DIAGNOSIS_
        putch('\t');
        textColor(TEXT);
      #endif

        if (stringIndex == 2) // product name
        {
            strncpy(device->productName, asciichar, 15);
            device->productName[15] = 0;

          #ifdef _USB_TRANSFER_DIAGNOSIS_
            printf(" product name: %s", device->productName);
          #endif
        }
        else if (stringIndex == 3) // serial number
        {
            // take the last 12 characters:

            size_t last = strlen(asciichar); // store last position
            size_t j = (size_t)MAX((int)(last - 12), 0); // step 12 characters backwards, but not below zero

            strncpy(device->serialNumber, asciichar + j, 12);
            device->serialNumber[12] = 0;
          #ifdef _USB_TRANSFER_DIAGNOSIS_
            printf(" serial: %s", device->serialNumber);
          #endif
        }
    }
}

void usb_startInterruptInTransfer(usb_transfer_t* transfer, usb_device_t* device, usb_endpoint_t* epInterrupt, void* buffer, size_t length, void(*handler)(usb_transfer_t*, void*), void* handlerData, uint8_t frequency)
{
    usb_constructTransfer(device, transfer, USB_INTERRUPT, epInterrupt);
    transfer->handler = handler;
    transfer->handlerData = handlerData;
    transfer->frequency = frequency;
    usb_inTransaction(transfer, false, buffer, length);
    usb_scheduleTransfer(transfer);
    list_append(&usb_interruptTransfers, transfer);
}

void usb_pollInterruptTransfers()
{
    for (dlelement_t* e = usb_interruptTransfers.head; e; e = e->next)
    {
        usb_transfer_t* transfer = (usb_transfer_t *)e->data;
        if (usb_pollTransfer(transfer))
        {
            if (transfer->handler)
                transfer->handler(transfer, transfer->handlerData);
        }
    }
}

void usb_IsochronousInTransfer(usb_device_t* device, usb_endpoint_t* epIsochronous, size_t length, void* buffer, uint16_t times, uint16_t numChains)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: usb_IsochronousTransfer, dev: %X", device);
    textColor(TEXT);
  #endif

    usb_transfer_t transfer;
    uint32_t numberITD = times<<16 | numChains;
    transfer.data = &numberITD;
    usb_constructTransfer(device, &transfer, USB_ISOCHRONOUS, epIsochronous);
    usb_inTransaction(&transfer, false, buffer, length);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
}


/*
* Copyright (c) 2010-2016 The PrettyOS Project. All rights reserved.
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
