#include <stdio.h>
#include "usb_hc.h"
#include <math.h>
#include <string.h>
#include "ehci.h"
#include <systemcall_impl.h>

#define _EHCI_ENABLE_


void usb_hc_install(pciDev_t* PCIdev)
{
    switch (PCIdev->interfaceID)
    {
        case XHCI:
          #ifdef _XHCI_ENABLE_
            xhci_install(PCIdev);
          #endif
            break;
        case EHCI:
          #ifdef _EHCI_ENABLE_
            ehci_install(PCIdev);
          #endif
            break;
        case OHCI:
          #ifdef _OHCI_ENABLE_
            ohci_install(PCIdev);
          #endif
            break;
        case UHCI:
          #ifdef _UHCI_ENABLE_
            uhci_install(PCIdev);
          #endif
            break;
    }
}


void usb_constructTransfer(usb_device_t* usbDevice, usb_transfer_t* transfer, usb_transferType_t type, usb_endpoint_t* endpoint)
{
    transfer->device       = usbDevice;
    transfer->endpoint     = endpoint;
    transfer->type         = type;
    transfer->packetSize   = endpoint->mps;
    transfer->handler      = 0;
    transfer->success      = false;
    list_construct(&transfer->transactions);

    if (transfer->device->port->type == &USB_EHCI)
    {
        ehci_setupTransfer(transfer);
    }
   
    else
    {
        printf("\nusb_constructTransfer - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }
}

uint8_t usb_setupTransaction(usb_transfer_t* transfer, uint8_t type, uint8_t req, uint8_t hiVal, uint8_t loVal, uint16_t index, uint16_t length)
{
    dlelement_t* elem = list_alloc_elem(sizeof(usb_transaction_t), "usb_transaction_t");
    usb_transaction_t* transaction = (usb_transaction_t * )elem->data;
    transaction->type = USB_TT_SETUP;
    uint8_t retVal = loVal;

	if (transfer->device->port->type == &USB_EHCI)
    {
        ehci_setupTransaction(transfer, transaction, false, type, req, hiVal, loVal, index, length);
    }
    
    else
    {
        printf("\nusb_setupTransaction - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }

    list_append_elem(&transfer->transactions, elem);

    transfer->endpoint->toggle = true;
    return retVal;
}

void usb_inTransaction(usb_transfer_t* transfer, bool controlHandshake, void* buffer, size_t length)
{
    size_t clampedLength = 0;
    uint16_t remainingTransactions = 0;

    if (transfer->type == USB_ISOCHRONOUS)
    {
        // Do not use the clamped length system!
        clampedLength = length;
        length -= clampedLength; // 0
        remainingTransactions = 0;
    }
    else
    {

     
        clampedLength = MIN(transfer->packetSize, length);
        length -= clampedLength;

        if (length != 0)
        {
            remainingTransactions = length / transfer->packetSize;
            if (length % transfer->packetSize != 0)
                remainingTransactions++;
        }
    }

    dlelement_t* elem = list_alloc_elem(sizeof(usb_transaction_t), "usb_transaction_t");
    usb_transaction_t* transaction = (usb_transaction_t * )elem->data;
    transaction->type = USB_TT_IN;

    if (controlHandshake) // Handshake transaction of control transfers have always set toggle to 1
    {
        transfer->endpoint->toggle = true;
    }
	if (transfer->device->port->type == &USB_EHCI)
    {
        ehci_inTransaction(transfer, transaction, transfer->endpoint->toggle, buffer, clampedLength);
    }
    else
    {
        printf("\nusb_inTransaction - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }

    list_append_elem(&transfer->transactions, elem);

    transfer->endpoint->toggle = !transfer->endpoint->toggle; // Switch toggle

    if (remainingTransactions > 0)
    {
        usb_inTransaction(transfer, transfer->endpoint->toggle, (char*)buffer + clampedLength, length);
    }

    
}

void usb_outTransaction(usb_transfer_t* transfer, bool controlHandshake, const void* buffer, size_t length)
{
    size_t clampedLength = MIN(transfer->packetSize, length);
    length -= clampedLength;
    uint16_t remainingTransactions = length / transfer->packetSize;
    if (length % transfer->packetSize != 0)
        remainingTransactions++;

    dlelement_t* elem = list_alloc_elem(sizeof(usb_transaction_t), "usb_transaction_t");
    usb_transaction_t* transaction = (usb_transaction_t *)elem->data;
    transaction->type = USB_TT_OUT;

    if (controlHandshake) // Handshake transaction of control transfers have always set toggle to 1
    {
        transfer->endpoint->toggle = true;
    }
	if (transfer->device->port->type == &USB_EHCI)
    {
        ehci_outTransaction(transfer, transaction, transfer->endpoint->toggle, buffer, clampedLength);
    }
    else
    {
        printf("\nusb_outTransaction - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }

    list_append_elem(&transfer->transactions, elem);

    transfer->endpoint->toggle = !transfer->endpoint->toggle; // Switch toggle

    if (remainingTransactions > 0)
    {
		//20191112
       // printf("%x %x %x\n", buffer, clampedLength, (char*)buffer + clampedLength);
       // for (;;);
        usb_outTransaction(transfer, transfer->endpoint->toggle, (char*)buffer + clampedLength, length);
    }
}

void usb_scheduleTransfer(usb_transfer_t* transfer)
{
    if (transfer->device->port->type == &USB_EHCI)
        ehci_scheduleTransfer(transfer);
    
    else
    {
        printf("\nusb_scheduleTransfer - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }
}

bool usb_pollTransfer(usb_transfer_t* transfer)
{
  if (transfer->device->port->type == &USB_EHCI)
        return ehci_pollTransfer(transfer);

    printf("\nusb_pollTransfer - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
        transfer, transfer->device, transfer->device->port, transfer->device->port->type);

    return false;
}

void usb_waitForTransfer(usb_transfer_t* transfer)
{
    
    if (transfer->device->port->type == &USB_EHCI)
        ehci_waitForTransfer(transfer);
    
    else
    {
        printf("\nusb_waitForTransfer - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }
}

void usb_destructTransfer(usb_transfer_t* transfer)
{
	if (transfer->device->port->type == &USB_EHCI)
        ehci_destructTransfer(transfer);
   
    else
    {
        printf("\nusb_destructTransfer - Unknown port type: Xfer: %X, dev: %X, port: %X, type: %X",
            transfer, transfer->device, transfer->device->port, transfer->device->port->type);
    }

    if (transfer->type == USB_INTERRUPT)
    {
        extern list_t usb_interruptTransfers;
        list_delete(&usb_interruptTransfers, list_find(&usb_interruptTransfers, transfer));
    }

    list_destruct(&transfer->transactions);
}

void hc_setupUSBDevice(hc_t* hc, uint8_t portNumber, usb_speed_t speed)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    //textColor(DATA);
    printf("\nhc_setupUSBDevice, portNumber: %d, usb speed: %d", portNumber, speed);
    //textColor(TEXT);
  #endif

    hc_port_t* port = hc_getPort(hc, portNumber);
    port->connected = true;
    port->device = usb_createDevice(&port->port, speed);

    usb_setupDevice(port->device, portNumber+1);
  
}

void hc_updateEndpointInformation(hc_port_t* port)
{
	
}

static void hc_constructPort(hc_t* hc, uint8_t num, const portType_t* type)
{
    hc_port_t* port = hc_getPort(hc, num);

    port->connected = false;
    port->hc = hc;
    port->device = 0;
    port->port.data = port;
    port->port.insertedDisk = 0;
    port->port.type = type;

	if (type == &USB_EHCI)
    {
        snprintf(port->port.name, 15, "EHCI-Port %u", num+1);
    }
    else
    {
        printf("\nhc_constructPort - Unknown port type");
    }

    deviceManager_attachPort(&port->port);
}

void hc_constructRootPorts(hc_t* hc, uint8_t rootPortCount, const portType_t* type)
{
    array_construct(&hc->ports);
	//20191112
    //hc_port_t* rootPorts = malloc(sizeof(hc_port_t)*rootPortCount, 0, "rootPorts");
	hc_port_t* rootPorts = (hc_port_t *)malloc_aligned(sizeof(hc_port_t) * rootPortCount, 4096);
	memset(rootPorts, 0, sizeof(hc_port_t) * rootPortCount);
    array_resize(&hc->ports, rootPortCount);
    for (uint8_t j = 0; j < rootPortCount; j++)
    {
        hc->ports.data[j] = rootPorts+j;
        hc_constructPort(hc, j, type);
    }
    hc->rootPortCount = rootPortCount;
}

static uint8_t hc_aquirePort(hc_t* hc, hc_port_t* data)
{
    // Find next free list element
    size_t i = hc->rootPortCount;
    for (; i < hc->ports.size; i++)
    {
        if (hc->ports.data[i] == 0)
        {
            hc->ports.data[i] = data;
            return i;
        }
    }

    array_resize(&hc->ports, hc->ports.size+1);
    hc->ports.data[i] = data;
    return i;
}

uint8_t hc_addPort(usb_device_t* usbDevice)
{
    hc_t* hc = ((hc_port_t*)usbDevice->port->data)->hc;
    //20191112
	//hc_port_t* port = malloc(sizeof(hc_port_t), 0, "hc_port");
	hc_port_t* port = (hc_port_t * )malloc_aligned(sizeof(hc_port_t), 4096);
	memset(port, 0, sizeof(hc_port_t));

    uint8_t num = hc_aquirePort(hc, port);
    hc_constructPort(hc, num, usbDevice->port->type);

  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nhc_addPort: %u", num+1);
  #endif

    return num;
}

hc_port_t* hc_getPort(hc_t* hc, uint8_t num)
{
    if (num < hc->ports.size)
        return hc->ports.data[num];
    return 0;
}

void hc_destroyPort(hc_t* hc, uint8_t num)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nhc_destroyPort: %u", num+1);
  #endif

    hc_port_t* port = hc_getPort(hc, num);
    if (port->device)
        usb_destroyDevice(port->device);
    hc->ports.data[num] = 0;
    deviceManager_destructPort(&port->port);
    free(port);
}

