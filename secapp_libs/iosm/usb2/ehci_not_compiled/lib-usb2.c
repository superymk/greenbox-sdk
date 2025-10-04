/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 *
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 *
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 *
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#include <palddk.h>
#include "../lib-iosm-usb-internal.h"

// #include "usb.h"
#include "lib-usb2.h"
#include "ehci.h"
#include "iosm_usb2_calls.h"
#include "../util.h"


static shandle_t h_usbbuf_obj = INVALID_HANDLE;
static vaddr_t usbbuf_addr = INVALID_VADDR;
static size_t usbbuf_size = 0;
static uint32_t io_pid = IO_PID_INVALID;

// static SAP_STATUS _usb_get_devdesc_full(struct sp_usbpdev_s *dev, struct usb_device_descriptor *ddesc)
// {
// 	SAP_STATUS status = SUCCESS;
// 	struct usb_device_request *req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));

	// req->bmRequestType = UT_READ_DEVICE;
    // req->bRequest = UR_GET_DESCRIPTOR;
    // USETW(req->wValue, UDESC_DEVICE << 8);
    // USETW(req->wIndex, 0);
    // USETW(req->wLength, sizeof(struct usb_device_descriptor));

// 	status = usb_send_default_control(dev, req, ddesc, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
// 	usbbuf_dealloc(USBBUF_MEM_DMA, req);
// 	return status;
// }

// static SAP_STATUS _usb_get_devdesc_8bytes(struct sp_usbpdev_s *dev, struct usb_device_descriptor *ddesc)
// {
// 	SAP_STATUS status = SUCCESS;
// 	struct usb_device_request *req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));

// req->bmRequestType = UT_READ_DEVICE;
// req->bRequest = UR_GET_DESCRIPTOR;
// USETW(req->wValue, UDESC_DEVICE << 8);
// USETW(req->wIndex, 0);
// USETW(req->wLength, 8);

// 	status = usb_send_default_control(dev, req, ddesc, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
// 	usbbuf_dealloc(USBBUF_MEM_DMA, req);
// 	return status;
// }

// static struct usb_config_descriptor *_usb_get_config_desc(struct sp_usbpdev_s *dev)
// {
// 	SAP_STATUS status = SUCCESS;
// 	struct usb_config_descriptor *cfg = NULL;
//     struct usb_device_request *req = NULL;
// 	size_t config_sz = 0;
// 	struct usb_config_descriptor *config = NULL;

// 	cfg = (struct usb_config_descriptor *)smalloc(MEM_DMA, sizeof(struct usb_config_descriptor));
//     if (!cfg)
//     {
//         goto err;
//     }

//     req = (struct usb_device_request *)smalloc(MEM_DMA, sizeof(struct usb_device_request));
//     if (!req)
//     {
//         goto err;
//     }

// 	// Read a partial of config descriptor in order to know the <wTotalLength>.
//     req->bmRequestType = UT_READ_DEVICE;
//     req->bRequest = UR_GET_DESCRIPTOR;
//     USETW(req->wValue, UDESC_CONFIG << 8);
//     USETW(req->wIndex, 0);
//     USETW(req->wLength, sizeof(cfg));

// 	status = usb_send_default_control(dev, req, cfg, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("USB Read Device Config Descriptor: Host controller reports error! status:%lu\n", status);
// 		goto err;
// 	}

// 	// Read all the rest descriptor together
// 	config_sz = UGETW(cfg->wTotalLength);
// 	config = (struct usb_config_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, config_sz);
// 	if (!config)
// 	{
// 		goto err;
// 	}

// 	USETW(req->wLength, config_sz);
// 	status = usb_send_default_control(dev, req, config, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("USB Read Device Config Descriptor: Host controller failed to read the full descriptor! status:%lu\n", status);
//         goto err;
// 	}

// 	usbbuf_dealloc(USBBUF_MEM_DMA, cfg);
// 	usbbuf_dealloc(USBBUF_MEM_DMA, req);
// 	return config;

// err:
// 	if (config)
// 		usbbuf_dealloc(USBBUF_MEM_DMA, config);
// 	if (cfg)
// 		usbbuf_dealloc(USBBUF_MEM_DMA, cfg);
// 	if (req)
// 		usbbuf_dealloc(USBBUF_MEM_DMA, req);
// 	return NULL;
// }

// /* helper functions */
// static void pal_print_endpoint(struct usb_endpoint_descriptor *endpoint)
// {
// 	USB_DEBUG("    Endpoint Descriptor:\n");
// 	USB_DEBUG("      bLength:          %d\n", endpoint->bLength);
// 	USB_DEBUG("      bDescriptorType:  %d\n", endpoint->bDescriptorType);
// 	USB_DEBUG("      bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
// 	USB_DEBUG("      bmAttributes:     %02xh\n", endpoint->bmAttributes);
// 	USB_DEBUG("      wMaxPacketSize:   %d\n", endpoint->wMaxPacketSize);
// 	USB_DEBUG("      bInterval:        %d\n", endpoint->bInterval);
// 	// printf("      bRefresh:         %d\n", endpoint->bRefresh);
// 	// printf("      bSynchAddress:    %d\n", endpoint->bSynchAddress);
// }

// static void pal_print_altsetting(struct usb_interface_descriptor *interface)
// {
// 	// int i;

// 	USB_DEBUG("  Interface Descriptor:\n");
// 	USB_DEBUG("    bLength:            %d\n", interface->bLength);
// 	USB_DEBUG("    bDescriptorType:    %d\n", interface->bDescriptorType);
// 	USB_DEBUG("    bInterfaceNumber:   %d\n", interface->bInterfaceNumber);
// 	USB_DEBUG("    bAlternateSetting:  %d\n", interface->bAlternateSetting);
// 	USB_DEBUG("    bNumEndpoints:      %d\n", interface->bNumEndpoints);
// 	USB_DEBUG("    bInterfaceClass:    %d\n", interface->bInterfaceClass);
// 	USB_DEBUG("    bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
// 	USB_DEBUG("    bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
// 	USB_DEBUG("    iInterface:         %d\n", interface->iInterface);

// 	//  for (i = 0; i < interface->bNumEndpoints; i++)
// 	//    print_endpoint(&interface->endpoint[i]);
// }

// static void pal_print_configuration(struct usb_config_descriptor *config)
// {
// 	// int i;

// 	USB_DEBUG("Configuration Descriptor:\n");
// 	USB_DEBUG("  bLength:              %d\n", config->bLength);
// 	USB_DEBUG("  bDescriptorType:      %d\n", config->bDescriptorType);
// 	USB_DEBUG("  wTotalLength:         %d\n", config->wTotalLength);
// 	USB_DEBUG("  bNumInterfaces:       %d\n", config->bNumInterfaces);
// 	USB_DEBUG("  bConfigurationValue:  %d\n", config->bConfigurationValue);
// 	USB_DEBUG("  iConfiguration:       %d\n", config->iConfiguration);
// 	USB_DEBUG("  bmAttributes:         %02xh\n", config->bmAttributes);
// 	USB_DEBUG("  MaxPower:             %d\n", config->bMaxPower);

// 	// for (i = 0; i < config->bNumInterfaces; i++)
// 	//   print_interface(&config->interface[i]);
// }

// static void pal_print_configuration_total(struct usb_config_descriptor *config)
// {
// 	int i, k, iface_num = 0, ep_num = 0;
// 	struct usb_interface_descriptor *tmp_ifdesc;
// 	struct usb_endpoint_descriptor *tmp_epdesc;
// 	char *current;

// 	/* FIXME: haven't handled altsettings yet */
// 	pal_print_configuration(config);
// 	iface_num = config->bNumInterfaces;
// 	current = (char *)(config) + config->bLength;
// 	for (i = 0; i < iface_num; i++)
// 	{
// 		tmp_ifdesc = (struct usb_interface_descriptor *)current;
// 		if (tmp_ifdesc->bDescriptorType == USB_DT_INTERFACE)
// 			pal_print_altsetting(tmp_ifdesc);
// 		ep_num = tmp_ifdesc->bNumEndpoints;
// 		current += tmp_ifdesc->bLength;
// 		for (k = 0; k < ep_num; k++)
// 		{
// 			tmp_epdesc = (struct usb_endpoint_descriptor *)current;
// 			if (tmp_epdesc->bDescriptorType == USB_DT_ENDPOINT)
// 				pal_print_endpoint(tmp_epdesc);
// 			current += tmp_epdesc->bLength;
// 		}
// 	}
// }

// static void pal_print_device(struct usb_device_descriptor *ddesc)
// {
// 	// int i;

// 	USB_DEBUG("Device Descriptor:\n");
// 	USB_DEBUG("bcdUSB:                 %02xh\n", ddesc->bcdUSB);
// 	USB_DEBUG("bDeviceClass:             %d\n", ddesc->bDeviceClass);
// 	USB_DEBUG("bMaxPacketSize0:          %d\n", ddesc->bMaxPacketSize0);
// 	USB_DEBUG("idVendor:               %02xh\n", ddesc->idVendor);
// 	USB_DEBUG("idProduct:              %02xh\n", ddesc->idProduct);
// 	USB_DEBUG("bNumConfigurations:       %d\n", ddesc->bNumConfigurations);
// 	// for (i = 0; i < config->bNumInterfaces; i++)
// 	//  print_interface(&config->interface[i]);
// }

// int pal_usbdev_get_desc(struct sp_usbpdev_s *dev)
// {
// 	SAP_STATUS status = SUCCESS;

// 	// read device descriptor
// 	if (usb2_usbdev_alloc_defpipe(dev))
// 	{
// 		warn_noalloc();
// 		return -1;
// 	}

// 	dev->ddesc = (struct usb_device_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, 64);
// 	if (!dev->ddesc)
// 	{
// 		warn_noalloc();
// 		return -1;
// 	}

// 	status = _usb_get_devdesc_8bytes(dev, dev->ddesc);
// 	if (FAIL(status))
// 	{
// 		warn_internalerror();
// 		goto dinfo_error;
// 	}

// 	if (dev->ddesc->bMaxPacketSize0 < 8 || dev->ddesc->bMaxPacketSize0 > 64)
// 	{
// 		warn_internalerror();
// 		goto dinfo_error;
// 	}

// 	// Reallocate the default pipe
// 	status = usb2_usbdev_free_defpipe(dev);
// 	if(FAIL(status))
// 	{
// 		USB_ERR("Failed to free default pipe after getting USB device descriptor! status:%lu\n", status);
// 		goto free_pipe_error;
// 	}
// 	dev->defpipe = NULL;

// 	if(usb2_usbdev_alloc_defpipe(dev))
// 	{
// 		USB_ERR("Failed to re-allocate default pipe after getting USB device descriptor!\n");
// 		warn_noalloc();
// 		goto realloc_pipe_err;
// 	}

// 	status = _usb_get_devdesc_full(dev, dev->ddesc);
// 	if (FAIL(status))
// 	{
// 		warn_internalerror();
// 		goto dinfo_error2;
// 	}
// 	pal_print_device(dev->ddesc);

// 	/* read configuration descriptor, and others */
// 	dev->config = _usb_get_config_desc(dev);
// 	if (!dev->config)
// 	{
// 		warn_internalerror();
// 		goto get_config_desc_err;
// 	}

// 	dev->iface = (struct usb_interface_descriptor *)(&dev->config[1]);
// 	dev->imax = dev->config->wTotalLength - dev->config->bLength;
// 	pal_print_configuration_total(dev->config);
// 	return 0;


// get_config_desc_err:
// dinfo_error2:
// realloc_pipe_err:
// free_pipe_error:
// dinfo_error:
// 	if(dev->ddesc)
// 	{
// 		usbbuf_dealloc(USBBUF_MEM_DMA, dev->ddesc);
// 		dev->ddesc = NULL;
// 	}
// 	return -1;
// }

// /// @brief Init the general USB configuration of the given USBPDev <dev_addr>.
// struct sp_usbpdev_s *libusb2_init_usbpdev(USBPDEV_ADDR dev_addr)
// {
// 	SAP_STATUS status = SUCCESS;
// 	struct sp_usbpdev_s *dev = NULL;
// 	uint32_t usbpdev_slot = INVALID_SLOT;

// 	// Sanity checks
// 	if (usbpdev_addr_equal(&dev_addr, &USBADDR_INVALID))
// 		return NULL;

// 	// Find USBPDev slot
// 	usbpdev_slot = usbpdev_get_slot(dev_addr);
// 	if (usbpdev_slot == INVALID_SLOT)
// 	{
// 		USB_ERR("Init USBPDev Error: The USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) has not been activated!\n",
// 				dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id);
// 		return NULL;
// 	}

// 	// Allocate device structure
// 	dev = (struct sp_usbpdev_s *)smalloc(sizeof(struct sp_usbpdev_s));
// 	if (!dev)
// 	{
// 		warn_noalloc();
// 		return NULL;
// 	}

// 	dev->devaddr = dev_addr;
// 	dev->usbpdev_slot = usbpdev_slot;

// 	// Get device speed
// 	status = iosm_usb2_get_usbpdev_speed(dev_addr, &dev->speed);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Init USBPDev Error: Failed to get speed of the USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
// 				dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id,
// 				status);
// 		goto dev_error;
// 	}

// 	// USB_DEBUG("<iosm_usb_init_usbpdev> device No.%d is in port No.%d of its parent hub!\n", dev->devaddr.dev_usbaddr, dev->devaddr.hub_port);

// 	/* directly read device, config and interface descriptors
// 	 * from the USB devices/hubs, for root hub, just the
// 	 * hardcoded descriptors (shamelessly borrowed from Linux
// 	 * kernel) */
// 	if (pal_usbdev_get_desc(dev))
// 	{
// 		warn_internalerror();
// 		goto dev_error;
// 	}

// 	USB_INFO("Initialized USB Device (addr:%u) with speed:%u\n", dev->devaddr.dev_usbaddr, dev->speed);

// 	return dev;


// dev_error:
// 	iosm_usb_fini_usbpdev(dev);
// 	return NULL;
// }

// SAP_STATUS iosm_usb_fini_usbpdev(struct sp_usbpdev_s *dev)
// {
// 	SAP_STATUS status = SUCCESS;

// 	// Sanity checks
// 	if (!dev)
// 		return EINVAL;

// 	// Free <ddesc> and <config>
// 	if (dev->ddesc)
// 	{
// 		usbbuf_dealloc(USBBUF_MEM_DMA, dev->ddesc);
// 	}
// 	if (dev->config)
// 	{
// 		usbbuf_dealloc(USBBUF_MEM_DMA, dev->config);
// 	}

// 	// Free the pipe
// 	status = usb2_usbdev_free_defpipe(dev);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Finalize USBPDev Error: Failed to finalize the default pipe of USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
// 				dev->devaddr.dev_usbaddr, dev->devaddr.hub_port, dev->devaddr.hub_usbaddr, dev->devaddr.bus_id,
// 				status);
// 	}

// 	sfree(dev, sizeof(struct sp_usbpdev_s));

// 	return SUCCESS;
// }

SAP_STATUS libusb2_init(spid_t iosm_usb2_spid, size_t usbbuf_4k_pages)
{
	SAP_STATUS status = SUCCESS;

	// Sanity checks
	if ((iosm_usb2_spid == INVALID_SP_ID) || (!usbbuf_4k_pages))
		return EINVAL;

	usbbuf_size = PAGE_4K_TO_BYTES(usbbuf_4k_pages);

	// Create communication channel with the IOSM-USB2 service.
	status = comm_iosm_usb2_init(iosm_usb2_spid);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB2 error: Failed to init Inter-SecProcess communication channel with IOSM-USB2 (id:%u)! status:%lu\n", iosm_usb2_spid, status);
		return EINTERNAL;
	}

	// Init the manager of eEHCIs (Num of registers: eEHCI_INSTANCE_NUM)
	eehci_mgr_init();

	// Init the manager of USBPDevs (Num of registers: WimpUSBPDev_Info_ENTRIES)
	usbpdev_mgr_init();

	// Init the manager of USB TD registers (Num of registers: USBTD_REG_NUMS)
	usbtd_reg_mgr_init();

	// Create USB I/O partition
	status = iosm_usb2_register_usbdrv(usbbuf_4k_pages, &io_pid, &h_usbbuf_obj, &usbbuf_addr);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB2 error: Failed to create USB I/O partition! status:%lu\n", status);
		return EINTERNAL;
	}

	// Create USB buffer pool
	status = usbbuf_create_pool(usbbuf_addr, usbbuf_size);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB2 error: Failed to create USB buffer pool! status:%lu\n", status);
		return EINTERNAL;
	}

	USB_INFO("Initial Lib-USB2 Success: Created USB I/O partition (id:%u)\n", io_pid);
	return SUCCESS;
}

void libusb2_fini(void)
{
	SAP_STATUS status = SUCCESS;

	// Destroy USB buffer pool
	usbbuf_destroy_pool();

	// Destroy USB I/O partition
	if (h_usbbuf_obj != INVALID_HANDLE)
	{
		status = iosm_usb2_unregister_usbdrv(h_usbbuf_obj, usbbuf_addr, usbbuf_size);
		if (FAIL(status))
			USB_ERR("Finalize Lib-USB2 error: Failed to destroy USB I/O partition (id:%u)! status:%lu\n", io_pid, status);
	}

	// Finalize the manager of USB TD registers
	usbtd_reg_mgr_fini();

	// Finalize the manager of USBPDevs
	usbpdev_mgr_fini();

	// Finalize the manager of eEHCIs
	eehci_mgr_fini();

	// Destroy communication channel with the IOSM-USB2 service.
	comm_iosm_usb2_fini();

	h_usbbuf_obj = INVALID_HANDLE;
	io_pid = IO_PID_INVALID;
}

// /// @brief Activate the given USB peripheral device <usbpdev_addr>
SAP_STATUS libusb2_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot)
{
	SAP_STATUS status = SUCCESS;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Check: Parameters must be valid
	if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

	if(!out_usbpdev_slot)
		return EINVAL;

	// Activate USBPDev
	status = iosm_usb2_activate_usbpdev(usbpdev_addr, &usbpdev_slot);
	if (FAIL(status))
	{
		USB_ERR("Activate USBPDev error: Failed to activate USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				status);
		status = EINTERNAL;
		goto usbpdev_activate_err;
	}

	// On success
	return SUCCESS;

usbpdev_activate_err:
	return status;
}

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
SAP_STATUS libusb2_deactivate_usbpdev(USBPDEV_ADDR usbpdev_addr)
{
	SAP_STATUS status = SUCCESS;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Find the USBPDev slot corresponds to <usbpdev_addr>
	usbpdev_slot = usbpdev_get_slot(usbpdev_addr);
	if (usbpdev_slot == INVALID_SLOT)
	{
		USB_ERR("Deactivate USBPDev error: USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) is not registered!\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id);
		return EINTERNAL;
	}

	// Deactivate USBPDev
	status = iosm_usb2_deactivate_usbpdev(usbpdev_slot);
	if (FAIL(status))
	{
		USB_ERR("Deactivate USBPDev error: Failed to deactivate USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				usbpdev_slot, status);
		return EINTERNAL;
	}

	return SUCCESS;
}

/// @brief Activate an eEHCI
SAP_STATUS libusb2_activate_eehci(uint32_t *out_eehci_slot)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS err_status = SUCCESS;
	union eEHCI_ID eehci_id;
	uint32_t eehci_handle;

	if (!out_eehci_slot)
		return EINVAL;

	// Activate eEHCI
	status = __iosm_usb2_activate_eehci(&eehci_id, &eehci_handle, out_eehci_slot);
	if (FAIL(status))
	{
		USB_ERR("Activate eEHCI error: Failed to activate eEHCI! status:%lu\n", status);
		status = EINTERNAL;
		goto eehci_activate_err;
	}

	// Register eEHCI and its slot
	status = eehci_register(*out_eehci_slot, eehci_id, eehci_handle);
	if (FAIL(status))
	{
		USB_ERR("Activate eEHCI error: Failed to register eEHCI with eEHCI slot:%u! status:%lu\n",
				*out_eehci_slot, status);
		status = EINTERNAL;
		goto eehci_register_err;
	}

	return SUCCESS;

eehci_register_err:
	err_status = __iosm_usb2_deactivate_eehci(*out_eehci_slot);
	if (FAIL(err_status))
		USB_ERR("Activate eEHCI error: Failed to deactivate eEHCI on activation error! status:%lu\n", err_status);
eehci_activate_err:
	return status;
}

/// @brief Deactivate the given eEHCI
SAP_STATUS libusb2_deactivate_eehci(uint32_t eehci_slot)
{
	SAP_STATUS status = SUCCESS;

	// Unregister eEHCI
	status = eehci_unregister(eehci_slot);
	if (FAIL(status))
	{
		USB_ERR("Deactivate eEHCI error: Failed to unregister eEHCI with eEHCI slot:%u! status:%lu\n",
				eehci_slot, status);
		return EINTERNAL;
	}

	// Deactivate eEHCI
	status = __iosm_usb2_deactivate_eehci(eehci_slot);
	if (FAIL(status))
	{
		USB_ERR("Deactivate eEHCI error: Failed to deactivate eEHCI with eEHCI slot:%u! status:%lu\n",
				eehci_slot, status);
		return EINTERNAL;
	}

	return SUCCESS;
}