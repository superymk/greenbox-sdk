// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include "lib-iosm-usb-internal.h"

#include "usb.h"
// #include "./ehci/lib-usb2.h"
// #include "./ehci/ehci.h"
// #include "./ehci/iosm_usb2_calls.h"
#include "./xhci/lib-usb3.h"
#include "./xhci/xhci.h"
#include "./xhci/iosm_usb3_calls.h"
#include "util.h"

static pid_t iosm_usb2_spid = INVALID_SP_ID;
static pid_t iosm_usb3_spid = INVALID_SP_ID;

/// @brief If true, the library will be in the debug mode.
static bool _enable_debug = false;

static SAP_STATUS _usb_get_devdesc_full(struct sp_usbpdev_s *dev, struct usb_device_descriptor *ddesc)
{
	SAP_STATUS status = SUCCESS;

	struct usb_device_request *req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));
	
	req->bmRequestType = UT_READ_DEVICE;
    req->bRequest = UR_GET_DESCRIPTOR;
    USETW(req->wValue, UDESC_DEVICE << 8);
    USETW(req->wIndex, 0);
    USETW(req->wLength, sizeof(struct usb_device_descriptor));

	status = usb_send_default_control(dev, req, ddesc, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
	usbbuf_dealloc(USBBUF_MEM_DMA, req);
	return status;
}

static SAP_STATUS _usb_get_devdesc_8bytes(struct sp_usbpdev_s *dev, struct usb_device_descriptor *ddesc)
{
	SAP_STATUS status = SUCCESS;
	struct usb_device_request *req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));

	req->bmRequestType = UT_READ_DEVICE;
    req->bRequest = UR_GET_DESCRIPTOR;
    USETW(req->wValue, UDESC_DEVICE << 8);
    USETW(req->wIndex, 0);
    USETW(req->wLength, 8);

	status = usb_send_default_control(dev, req, ddesc, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
	usbbuf_dealloc(USBBUF_MEM_DMA, req);
	return status;
}

static struct usb_config_descriptor *_usb_get_config_desc(struct sp_usbpdev_s *dev)
{
	SAP_STATUS status = SUCCESS;
	struct usb_config_descriptor *cfg = NULL;
    struct usb_device_request *req = NULL;
	size_t config_sz = 0;
	struct usb_config_descriptor *config = NULL;

	cfg = (struct usb_config_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_config_descriptor));
    if (!cfg)
    {
        goto err;
    }

    req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));
    if (!req)
    {
        goto err;
    }

	// Read a partial of config descriptor in order to know the <wTotalLength>.
    req->bmRequestType = UT_READ_DEVICE;
    req->bRequest = UR_GET_DESCRIPTOR;
    USETW(req->wValue, UDESC_CONFIG << 8);
    USETW(req->wIndex, 0);
    USETW(req->wLength, sizeof(cfg));

	status = usb_send_default_control(dev, req, cfg, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
	if (FAIL(status))
	{
		USB_ERR("USB Read Device Config Descriptor: Host controller reports error! status:%lu\n", status);
		goto err;
	}

	// Read all the rest descriptor together
	config_sz = UGETW(cfg->wTotalLength);
	config = (struct usb_config_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, config_sz);
	if (!config)
	{
		goto err;
	}

	USETW(req->wLength, config_sz);
	status = usb_send_default_control(dev, req, config, USB_TRANSFER_DEFAULT_TIMEOUT_MS, _enable_debug);
	if (FAIL(status))
	{
		USB_ERR("USB Read Device Config Descriptor: Host controller failed to read the full descriptor! status:%lu\n", status);
        goto err;
	}

	usbbuf_dealloc(USBBUF_MEM_DMA, cfg);
	usbbuf_dealloc(USBBUF_MEM_DMA, req);
	return config;

err:
	if (config)
		usbbuf_dealloc(USBBUF_MEM_DMA, config);
	if (cfg)
		usbbuf_dealloc(USBBUF_MEM_DMA, cfg);
	if (req)
		usbbuf_dealloc(USBBUF_MEM_DMA, req);
	return NULL;
}

/* helper functions */
static void _usbdev_print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
	USB_DEBUG("    Endpoint Descriptor:\n");
	USB_DEBUG("      bLength:          %d\n", endpoint->bLength);
	USB_DEBUG("      bDescriptorType:  %d\n", endpoint->bDescriptorType);
	USB_DEBUG("      bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
	USB_DEBUG("      bmAttributes:     %02xh\n", endpoint->bmAttributes);
	USB_DEBUG("      wMaxPacketSize:   %d\n", UGETW(endpoint->wMaxPacketSize));
	USB_DEBUG("      bInterval:        %d\n", endpoint->bInterval);
	// printf("      bRefresh:         %d\n", endpoint->bRefresh);
	// printf("      bSynchAddress:    %d\n", endpoint->bSynchAddress);
}

static void _usbdev_print_altsetting(struct usb_interface_descriptor *interface)
{
	// int i;

	USB_DEBUG("  Interface Descriptor:\n");
	USB_DEBUG("    bLength:            %d\n", interface->bLength);
	USB_DEBUG("    bDescriptorType:    %d\n", interface->bDescriptorType);
	USB_DEBUG("    bInterfaceNumber:   %d\n", interface->bInterfaceNumber);
	USB_DEBUG("    bAlternateSetting:  %d\n", interface->bAlternateSetting);
	USB_DEBUG("    bNumEndpoints:      %d\n", interface->bNumEndpoints);
	USB_DEBUG("    bInterfaceClass:    %d\n", interface->bInterfaceClass);
	USB_DEBUG("    bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
	USB_DEBUG("    bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
	USB_DEBUG("    iInterface:         %d\n", interface->iInterface);

	//  for (i = 0; i < interface->bNumEndpoints; i++)
	//    print_endpoint(&interface->endpoint[i]);
}

static void _usbdev_print_configuration(struct usb_config_descriptor *config)
{
	// int i;

	USB_DEBUG("Configuration Descriptor:\n");
	USB_DEBUG("  bLength:              %d\n", config->bLength);
	USB_DEBUG("  bDescriptorType:      %d\n", config->bDescriptorType);
	USB_DEBUG("  wTotalLength:         %d\n", UGETW(config->wTotalLength));
	USB_DEBUG("  bNumInterfaces:       %d\n", config->bNumInterfaces);
	USB_DEBUG("  bConfigurationValue:  %d\n", config->bConfigurationValue);
	USB_DEBUG("  iConfiguration:       %d\n", config->iConfiguration);
	USB_DEBUG("  bmAttributes:         %02xh\n", config->bmAttributes);
	USB_DEBUG("  MaxPower:             %d\n", config->bMaxPower);

	// for (i = 0; i < config->bNumInterfaces; i++)
	//   print_interface(&config->interface[i]);
}

static void _usbdev_print_configuration_total(struct usb_config_descriptor *config)
{
	int i, k, iface_num = 0, ep_num = 0;
	struct usb_interface_descriptor *tmp_ifdesc;
	struct usb_endpoint_descriptor *tmp_epdesc;
	char *current;

	/* FIXME: haven't handled altsettings yet */
	_usbdev_print_configuration(config);
	iface_num = config->bNumInterfaces;
	current = (char *)(config) + config->bLength;
	for (i = 0; i < iface_num; i++)
	{
		tmp_ifdesc = (struct usb_interface_descriptor *)current;
		if (tmp_ifdesc->bDescriptorType == USB_DT_INTERFACE)
			_usbdev_print_altsetting(tmp_ifdesc);
		ep_num = tmp_ifdesc->bNumEndpoints;
		current += tmp_ifdesc->bLength;
		for (k = 0; k < ep_num; k++)
		{
			tmp_epdesc = (struct usb_endpoint_descriptor *)current;
			// if (tmp_epdesc->bDescriptorType == USB_DT_ENDPOINT)
			// 	_usbdev_print_endpoint(tmp_epdesc);
			// current += tmp_epdesc->bLength;

			// Skip non-endpoint descriptors (e.g., HID descriptors)
			if (tmp_epdesc->bDescriptorType != USB_DT_ENDPOINT)
			{
				USB_DEBUG("    Ignore printing descriptor (bDescriptorType:%u)...\n", tmp_epdesc->bDescriptorType);

				current += tmp_epdesc->bLength;
				k--; // We haven't found the endpoint descriptor yet
				continue;
			}
			else
			{
				// Print the endpoint descriptor
				_usbdev_print_endpoint(tmp_epdesc);

				current += tmp_epdesc->bLength;
			}
		}
	}
}

static void _usbdev_print(struct usb_device_descriptor *ddesc)
{
	// int i;

	USB_DEBUG("Device Descriptor:\n");
	USB_DEBUG("bLength:                %02xh\n", ddesc->bLength);
	USB_DEBUG("bDescriptorType:        %02xh\n", ddesc->bDescriptorType);
	USB_DEBUG("bcdUSB:                 %02xh\n", UGETW(ddesc->bcdUSB));
	USB_DEBUG("bDeviceClass:             %d\n", ddesc->bDeviceClass);
	USB_DEBUG("bDeviceSubClass:          %d\n", ddesc->bDeviceSubClass);
	USB_DEBUG("bDeviceProtocol:          %d\n", ddesc->bDeviceProtocol);
	USB_DEBUG("bMaxPacketSize:          %d\n", ddesc->bMaxPacketSize);

	USB_DEBUG("idVendor:               %02xh\n", UGETW(ddesc->idVendor));
	USB_DEBUG("idProduct:              %02xh\n", UGETW(ddesc->idProduct));
	USB_DEBUG("bcdDevice:              %02xh\n", UGETW(ddesc->bcdDevice));
	USB_DEBUG("iManufacturer:            %d\n", ddesc->iManufacturer);
	USB_DEBUG("iProduct:                 %d\n", ddesc->iProduct);
	USB_DEBUG("iSerialNumber:            %d\n", ddesc->iSerialNumber);

	USB_DEBUG("bNumConfigurations:       %d\n", ddesc->bNumConfigurations);

	// for (i = 0; i < config->bNumInterfaces; i++)
	//  print_interface(&config->interface[i]);
}

static SAP_STATUS _pal_usbdev_alloc_defpipe(struct sp_usbpdev_s *dev)
{
	if(dev->bus_type == USB2)
	{
		// return usb2_usbdev_alloc_defpipe(dev);

        USB_ERR("<_pal_usbdev_alloc_defpipe>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (dev->bus_type == USB3)
	{
		return usb3_usbdev_alloc_defpipe(dev);
	}

	return EINVAL;
}

static SAP_STATUS _pal_usbdev_free_defpipe(struct sp_usbpdev_s* dev)
{
	if(dev->bus_type == USB2)
	{
		// return usb2_usbdev_free_defpipe(dev);

        USB_ERR("<_pal_usbdev_free_defpipe>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (dev->bus_type == USB3)
	{
		return usb3_usbdev_free_defpipe(dev);
	}

	return EINVAL;
}

static SAP_STATUS _usb_parse_interfaces(struct sp_usbpdev_s *dev)
{
	struct usb_interface_descriptor *iface = NULL;
	uint32_t num_ifaces = 0;
	uint32_t next_idx = 0;

	// Check: Parameters must be valid
	if(!dev || !dev->config)
		return EINVAL;

	num_ifaces = dev->config->bNumInterfaces;
	if(num_ifaces > USBPDEV_MAX_DATA_INTERFACES_NUM)
		return EINVAL;

	dev->data_ifaces_num = num_ifaces;
	iface = (struct usb_interface_descriptor *)(&dev->config[1]);

	// Iterate all data interfaces
	while(next_idx < num_ifaces)
	{
		if (iface->bDescriptorType == USB_DT_INTERFACE)
		{
			// Found an interface
			dev->data_iface[next_idx] = iface;
			next_idx++;
		}

		iface = (void*)iface + iface->bLength;
	}

	// On success
	return SUCCESS;
}

/// @brief Parse <dev->config> and fill data endpoint descriptors in <dev->data_eps>
/// @param dev 
static void _usb_parse_data_eps(struct sp_usbpdev_s *dev)
{
	struct usb_config_descriptor *config = dev->config;
	struct usb_interface_descriptor *tmp_ifdesc;
	struct usb_endpoint_descriptor *tmp_epdesc;
	char *current;
	int i, k, iface_num = 0, iface_ep_num = 0;
	int ep_idx = 0;

	iface_num = config->bNumInterfaces;
	current = (char *)(config) + config->bLength;
	for (i = 0; i < iface_num; i++)
	{
		tmp_ifdesc = (struct usb_interface_descriptor *)current;
		iface_ep_num = tmp_ifdesc->bNumEndpoints;
		current += tmp_ifdesc->bLength;
		for (k = 0; k < iface_ep_num; k++)
		{
			tmp_epdesc = (struct usb_endpoint_descriptor *)current;
			
			// Skip non-endpoint descriptors (e.g., HID descriptors)
			if (tmp_epdesc->bDescriptorType != USB_DT_ENDPOINT)
			{
				current += tmp_epdesc->bLength;
				k--; // We haven't found the endpoint descriptor yet
				continue;
			}
			else
			{
				// Reference the endpoint descriptor in <dev>
				uint32_t cur_ep_idx = ep_idx++;

				dev->data_eps[cur_ep_idx].iface = dev->data_iface[i];
				dev->data_eps[cur_ep_idx].epdesc = tmp_epdesc;

				current += tmp_epdesc->bLength;
			}
		}
	}

	// Store the number of data endpoints in <dev->data_eps_num>
	dev->data_eps_num = ep_idx;
}





static SAP_STATUS _usbdev_get_desc_usb2(struct sp_usbpdev_s *dev)
{
	SAP_STATUS status = SUCCESS;

	// read device descriptor
	status = _pal_usbdev_alloc_defpipe(dev);
	if (FAIL(status))
	{
		USB_ERR("GET_USB_Descriptor of USB device error: Failed to allocate default pipe! status:%lu\n", status);
		return EINTERNAL;
	}

	dev->ddesc = (struct usb_device_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_descriptor));
	if (!dev->ddesc)
	{
		return ENOMEM;
	}

	status = _usb_get_devdesc_8bytes(dev, dev->ddesc);
	if (FAIL(status))
	{
		USB_ERR("GET_USB_Descriptor of USB device error: Failed to get the first 8 bytes! status:%lu\n", status);
		status = EINTERNAL;
		goto dinfo_error;
	}

	// Reallocate the default pipe
	status = _pal_usbdev_free_defpipe(dev);
	if (FAIL(status))
	{
		USB_ERR("Failed to free default pipe after getting USB device descriptor! status:%lu\n", status);
		status = EINTERNAL;
		goto free_pipe_error;
	}
	dev->defpipe = NULL;

	status = _pal_usbdev_alloc_defpipe(dev);
	if (FAIL(status))
	{
		USB_ERR("Failed to re-allocate default pipe after getting USB device descriptor! status:%lu\n", status);
		status = EINTERNAL;
		goto realloc_pipe_err;
	}

	status = _usb_get_devdesc_full(dev, dev->ddesc);
	if (FAIL(status))
	{
		USB_ERR("GET_USB_Descriptor of USB device error: Failed to read the full device descriptor! status:%lu\n", status);
		status = EINTERNAL;
		goto dinfo_error2;
	}
	_usbdev_print(dev->ddesc);

	// read configuration descriptor, and others
	dev->config = _usb_get_config_desc(dev);
	if (!dev->config)
	{
		USB_ERR("GET_USB_Descriptor of USB device error: Failed to read the device config descriptor!\n");
		status = EINTERNAL;
		goto get_config_desc_err;
	}

	// Parse the device's data interfaces
	status = _usb_parse_interfaces(dev);
	if(FAIL(status))
	{
		USB_ERR("Parse USB device interface error! status:%lu\n", status);
		status = EINTERNAL;
		goto parse_iface_err;
	}

	dev->imax = UGETW(dev->config->wTotalLength) - dev->config->bLength;
	_usbdev_print_configuration_total(dev->config);
	_usb_parse_data_eps(dev);

	// On success
	USB_DEBUG("USB device has %u interfaces and %u endpoints\n", dev->data_ifaces_num, dev->data_eps_num);
	return SUCCESS;

parse_iface_err:
get_config_desc_err:
dinfo_error2:
realloc_pipe_err:
free_pipe_error:
dinfo_error:
	if (dev->ddesc)
	{
		usbbuf_dealloc(USBBUF_MEM_DMA, dev->ddesc);
		dev->ddesc = NULL;
	}
	return status;
}

static SAP_STATUS _usbdev_get_desc_usb3(struct sp_usbpdev_s *dev)
{
	SAP_STATUS status = SUCCESS;

    dev->ddesc = (struct usb_device_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_descriptor));
    if (!dev->ddesc)
    {
        status = ENOMEM;
        goto alloc_err;
    }

    status = iosm_usb3_usbpdev_get_desc(dev->devaddr, dev->ddesc, &dev->config, false);
    if (FAIL(status))
	{
		USB_ERR("Get USB3 device descriptor error! status:%lu\n", status);
		goto dinfo_error;
	}

	status = _pal_usbdev_alloc_defpipe(dev);
    if (FAIL(status))
	{
		USB_ERR("Failed to re-allocate default pipe after getting USB3 device descriptor! status:%lu\n", status);
		status = EINTERNAL;
		goto realloc_pipe_err;
	}

    // Print device descriptor
    _usbdev_print(dev->ddesc);

	// Parse the device's data interfaces
	status = _usb_parse_interfaces(dev);
	if(FAIL(status))
	{
		USB_ERR("Parse USB3 device interface error! status:%lu\n", status);
		goto parse_iface_err;
	}

	dev->imax = UGETW(dev->config->wTotalLength) - dev->config->bLength;
	_usbdev_print_configuration_total(dev->config);
	_usb_parse_data_eps(dev);

	// On success
	USB_DEBUG("USB3 device has %u interfaces and %u endpoints\n", dev->data_ifaces_num, dev->data_eps_num);
	return SUCCESS;


parse_iface_err:
realloc_pipe_err:
dinfo_error:
alloc_err:
    if (dev->config)
	{
		usbbuf_dealloc(USBBUF_MEM_DMA, dev->config);
		dev->config = NULL;
	}

    if (dev->ddesc)
	{
		usbbuf_dealloc(USBBUF_MEM_DMA, dev->ddesc);
		dev->ddesc = NULL;
	}

    return status;
}

static SAP_STATUS _usbdev_get_desc(struct sp_usbpdev_s *dev)
{
    if(dev->bus_type == USB2)
    {
        return _usbdev_get_desc_usb2(dev);
    }
    else if (dev->bus_type == USB3)
    {
        return _usbdev_get_desc_usb3(dev);
    }

    // Unknown USB bus type
    return EINVAL;
}

static void _usb_dev_clear_data_eps(struct sp_usbpdev_s *dev)
{
	uint32_t i = 0;

	FOREACH_S(i, dev->data_eps_num, USBPDEV_MAX_DATA_ENDPOINTS_NUM, 0, 1)
	{
		dev->data_eps[i].iface = NULL;
		dev->data_eps[i].epdesc = NULL;
	}

	dev->data_eps_num = 0;
}

static void _usb_dev_clear_data_interfaces(struct sp_usbpdev_s *dev)
{
	uint32_t i = 0;

	FOREACH_S(i, dev->data_ifaces_num, USBPDEV_MAX_DATA_INTERFACES_NUM, 0, 1)
	{
		dev->data_iface[i] = NULL;
	}

	dev->data_ifaces_num = 0;
}




/********* Public functions *********/
/// @brief Init the general USB configuration of the given USBPDev <dev_addr>.
struct sp_usbpdev_s *iosm_usb_init_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR dev_addr)
{
	SAP_STATUS status = SUCCESS;
	struct sp_usbpdev_s *dev = NULL;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Sanity checks
	if(bus_type != USB2 && bus_type != USB3)
		return NULL;

	if (usbpdev_addr_equal(&dev_addr, &USBADDR_INVALID))
		return NULL;

	// Find USBPDev slot
	usbpdev_slot = usbpdev_get_slot(dev_addr);
	if (usbpdev_slot == INVALID_SLOT)
	{
		USB_ERR("Init USBPDev Error: The USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) has not been activated!\n",
				dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id);
		return NULL;
	}

	// Allocate device structure
	dev = (struct sp_usbpdev_s *)smalloc(sizeof(struct sp_usbpdev_s));
	if (!dev)
	{
		return NULL;
	}

	_usb_dev_clear_data_eps(dev);
	_usb_dev_clear_data_interfaces(dev);

	memcpy(&dev->devaddr, &dev_addr, sizeof(USBPDEV_ADDR));
	dev->bus_type = bus_type;
	dev->usbpdev_slot = usbpdev_slot;

	// Get device speed
	if(bus_type == USB2)
	{
		// status = iosm_usb2_get_usbpdev_speed(dev_addr, &dev->speed);
		// if (FAIL(status))
		// {
		// 	USB_ERR("Init USBPDev Error: Failed to get speed of the USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
		// 			dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id,
		// 			status);
		// 	goto dev_error;
		// }

        USB_ERR("<iosm_usb_init_usbpdev> Unsupported!\n");
        goto dev_error;
	}
	else if (bus_type == USB3)
	{
		status = iosm_usb3_get_usbpdev_speed(dev_addr, &dev->speed);
		if (FAIL(status))
		{
			USB_ERR("Init USBPDev Error: Failed to get speed of the USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
					dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id,
					status);
			goto dev_error;
		}
	}

	// USB_DEBUG("<iosm_usb_init_usbpdev> device No.%d is in port No.%d of its parent hub!\n", dev->devaddr.dev_usbaddr, dev->devaddr.hub_port);

	/* directly read device, config and interface descriptors
	 * from the USB devices/hubs, for root hub, just the
	 * hardcoded descriptors (shamelessly borrowed from Linux
	 * kernel) */
	status = _usbdev_get_desc(dev);
	if (FAIL(status))
	{
		USB_ERR("Init USBPDev Error: Failed to get descriptor of the USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
					dev_addr.dev_usbaddr, dev_addr.hub_port, dev_addr.hub_usbaddr, dev_addr.bus_id,
					status);
		goto dev_error;
	}

	USB_INFO("Initialized USB Device (addr:%u) with speed:%u\n", dev->devaddr.dev_usbaddr, dev->speed);

	return dev;

dev_error:
	iosm_usb_fini_usbpdev(dev);
	return NULL;
}

SAP_STATUS iosm_usb_fini_usbpdev(struct sp_usbpdev_s *dev)
{
	SAP_STATUS status = SUCCESS;

	// Sanity checks
	if (!dev)
		return EINVAL;

	// Free <ddesc> and <config>
	if (dev->ddesc)
	{
		usbbuf_dealloc(USBBUF_MEM_DMA, dev->ddesc);
	}
	if (dev->config)
	{
		usbbuf_dealloc(USBBUF_MEM_DMA, dev->config);
	}

	// Clear data endpoints
	_usb_dev_clear_data_eps(dev);

	// Clear data interfaces
	_usb_dev_clear_data_interfaces(dev);


	// Free the pipe
	status = _pal_usbdev_free_defpipe(dev);
	if (FAIL(status))
	{
		USB_ERR("Finalize USBPDev Error: Failed to finalize the default pipe of USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
				dev->devaddr.dev_usbaddr, dev->devaddr.hub_port, dev->devaddr.hub_usbaddr, dev->devaddr.bus_id,
				status);
	}

	sfree(dev, sizeof(struct sp_usbpdev_s));

	return SUCCESS;
}

SAP_STATUS iosm_usb_init(spid_t in_iosm_usb2_spid, size_t iosm_usb2_usbbuf_4k_pages, spid_t in_iosm_usb3_spid, size_t iosm_usb3_usbbuf_4k_pages)
{
	SAP_STATUS status = SUCCESS;

	if(in_iosm_usb2_spid != INVALID_SP_ID)
	{
		// status = libusb2_init(in_iosm_usb2_spid, iosm_usb2_usbbuf_4k_pages);
		// if (FAIL(status))
		// {
		// 	USB_ERR("Init Lib-IOSM-USB USB2 error! status:%lu\n", status);
		// 	status = EINTERNAL;
		// 	goto err;
		// }

		// iosm_usb2_spid = in_iosm_usb2_spid;

        USB_ERR("<iosm_usb_init>: Unsupported!\n");
        return EINTERNAL;
	}

	if(in_iosm_usb3_spid != INVALID_SP_ID)
	{
		status = libusb3_init(in_iosm_usb3_spid, iosm_usb3_usbbuf_4k_pages);
		if (FAIL(status))
		{
			USB_ERR("Init Lib-IOSM-USB USB3 error! status:%lu\n", status);
			status = EINTERNAL;
			goto err;
		}

		iosm_usb3_spid = in_iosm_usb3_spid;
	}

	// On success
	return SUCCESS;

err:
	iosm_usb_fini();
	return status;
}

void iosm_usb_fini(void)
{
	if(iosm_usb2_spid != INVALID_SP_ID)
	{
		// libusb2_fini();
		// iosm_usb2_spid = INVALID_SP_ID;

        USB_ERR("<iosm_usb_fini>: Unsupported!\n");
        return;
	}

	if(iosm_usb3_spid != INVALID_SP_ID)
	{
		libusb3_fini();
		iosm_usb3_spid = INVALID_SP_ID;
	}
}

SAP_STATUS iosm_usb_activate_eHC(USB_BUS_TYPE bus_type, uint32_t* out_eHC_slot)
{
	// Check: Parameters must be valid
	if(bus_type != USB2 && bus_type != USB3)
		return EINVAL;

	if(!out_eHC_slot)
		return EINVAL;

	// Activate an ephemeral USB host controller.
	if(bus_type == USB2)
	{
		// return libusb2_activate_eehci(out_eHC_slot);

        USB_ERR("<iosm_usb_activate_eHC>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (bus_type == USB3)
	{
		// Do nothing, because there is no eXHCI at the moment.
		*out_eHC_slot = INVALID_SLOT;
		return SUCCESS;
	}

	// Should not hit
	return EINTERNAL;
}

SAP_STATUS iosm_usb_deactivate_eHC(USB_BUS_TYPE bus_type, uint32_t eHC_slot)
{
	// Check: Parameters must be valid
	if(bus_type != USB2 && bus_type != USB3)
		return EINVAL;

	// Activate an ephemeral USB host controller.
	if(bus_type == USB2)
	{
		// return libusb2_deactivate_eehci(eHC_slot);

        USB_ERR("<iosm_usb_deactivate_eHC>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (bus_type == USB3)
	{
		// Do nothing, because there is no eXHCI at the moment.
		return SUCCESS;
	}

	// Should not hit
	return EINTERNAL;
}

/// @brief Activate the given USB peripheral device <usbpdev_addr>
SAP_STATUS iosm_usb_activate_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR usbpdev_addr)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS err_status = SUCCESS;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Check: Parameters must be valid
	if(bus_type != USB2 && bus_type != USB3)
		return EINVAL;

	if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

	// Activate USBPDev
	if(bus_type == USB2)
	{
		// status = libusb2_activate_usbpdev(usbpdev_addr, &usbpdev_slot);
		// if (FAIL(status))
		// {
		// 	USB_ERR("Activate USBPDev error: Failed to activate USB2 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
		// 			usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
		// 			status);
		// 	status = EINTERNAL;
		// 	goto usbpdev_activate_err;
		// }

        USB_ERR("<iosm_usb_activate_usbpdev>: Unsupported!\n");
        status = EINTERNAL;
        goto usbpdev_activate_err;
	}
	else if (bus_type == USB3)
	{
		status = libusb3_activate_usbpdev(usbpdev_addr, &usbpdev_slot);
		if (FAIL(status))
		{
			USB_ERR("Activate USBPDev error: Failed to activate USB3 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
					usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
					status);
			status = EINTERNAL;
			goto usbpdev_activate_err;
		}
	}

	// Register USBPDev and its slot
	status = usbpdev_register(usbpdev_slot, usbpdev_addr);
	if (FAIL(status))
	{
		USB_ERR("Activate USBPDev error: Failed to register USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				usbpdev_slot, status);
		status = EINTERNAL;
		goto usbpdev_register_err;
	}

	// On success
	return SUCCESS;

usbpdev_register_err:
	if(bus_type == USB2)
	{
		// err_status = libusb2_deactivate_usbpdev(usbpdev_addr);
		// if (FAIL(err_status))
		// 	USB_ERR("Activate USBPDev error: Failed to deactivate USB2 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) on activation error! status:%lu\n",
		// 			usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
		// 			err_status);

        USB_ERR("<iosm_usb_activate_usbpdev>: <libusb2_deactivate_usbpdev> Unsupported!\n");
	}
	else if (bus_type == USB3)
	{
		err_status = libusb3_deactivate_usbpdev(usbpdev_addr);
		if (FAIL(err_status))
			USB_ERR("Activate USBPDev error: Failed to deactivate USB3 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) on activation error! status:%lu\n",
					usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
					err_status);
	}
usbpdev_activate_err:
	return status;
}

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
SAP_STATUS iosm_usb_deactivate_usbpdev(USB_BUS_TYPE bus_type, USBPDEV_ADDR usbpdev_addr)
{
	SAP_STATUS status = SUCCESS;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Check: Parameters must be valid
	if(bus_type != USB2 && bus_type != USB3)
		return EINVAL;
		
	if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

	// Deactivate USBPDev
	if(bus_type == USB2)
	{
		// status = libusb2_deactivate_usbpdev(usbpdev_addr);
		// if (FAIL(status))
		// {
		// 	USB_ERR("Deactivate USBPDev error: Failed to deactivate USB2 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
		// 			usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
		// 			usbpdev_slot, status);
		// 	return EINTERNAL;
		// }

        USB_ERR("<iosm_usb_deactivate_usbpdev>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (bus_type == USB3)
	{
		status = libusb3_deactivate_usbpdev(usbpdev_addr);
		if (FAIL(status))
		{
			USB_ERR("Deactivate USBPDev error: Failed to deactivate USB3 physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
					usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
					usbpdev_slot, status);
			return EINTERNAL;
		}
	}

	// Find USBPDev slot
	usbpdev_slot = usbpdev_get_slot(usbpdev_addr);
	if (usbpdev_slot == INVALID_SLOT)
	{
		USB_ERR("Deactivate USBPDev Error: The USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) has not been activated!\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id);
		return EINVAL;
	}

	// Unregister USBPDev and its slot
	status = usbpdev_unregister(usbpdev_slot);
	if (FAIL(status))
	{
		USB_ERR("Deactivate USBPDev error: Failed to unregister USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				usbpdev_slot, status);
		return EINTERNAL;
	}

	// On success
	return SUCCESS;
}

SAP_STATUS iosm_usb_send_default_control(struct sp_usbpdev_s* dev, const struct usb_device_request *req, void *data, uint32_t timeout_ms)
{
    return usb_send_default_control(dev, req, data, timeout_ms, _enable_debug);
}

struct sp_usbpdev_pipe* iosm_usb_usbpdev_alloc_intr_pipe(
	struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
)
{
	struct sp_usbpdev_pipe* pipe = NULL;
	void* internal_pipe = NULL;

	// Check: Parameters must be valid
	if(!usbdev || !epdesc)
		return NULL;

	if(usbdev->bus_type != USB2 && usbdev->bus_type != USB3)
		return NULL;

	pipe = smalloc(sizeof(struct sp_usbpdev_pipe));
    if (!pipe) 
    {
        return NULL;
    }

	// Allocate internal interrupt pipe
	if(usbdev->bus_type == USB2)
	{
		// internal_pipe = libusb2_usbpdev_alloc_intr_pipe(usbdev, epdesc);

        USB_ERR("<iosm_usb_usbpdev_alloc_intr_pipe>: Unsupported!\n");
        return NULL;
	}
	else if (usbdev->bus_type == USB3)
	{
		internal_pipe = libusb3_usbpdev_alloc_intr_pipe(usbdev, epdesc);
	}

	if(!internal_pipe)
	{
		goto alloc_internal_pipe_err;
	}

	pipe->bus_type = usbdev->bus_type;
	pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	pipe->pipe = internal_pipe;
	pipe->dev = usbdev;

	// On success
	return pipe;

alloc_internal_pipe_err:
	if(pipe)
		sfree(pipe, sizeof(struct sp_usbpdev_pipe));
	return NULL;
}

SAP_STATUS iosm_usb_poll_intr(struct sp_usbpdev_pipe* pipe, void *data, size_t data_sz)
{
    // Check: Parameters must be valid
	if(!pipe || !data)
    {
		return EINVAL;
    }

	if(pipe->eptype != USB_ENDPOINT_XFER_INT)
    {
		return EINVAL;
    }

	if(pipe->bus_type != USB2 && pipe->bus_type != USB3)
    {
		return EINVAL;
    }

	// Poll interrupt pipe
	if(pipe->bus_type == USB2)
	{
		// return libusb2_poll_intr((struct usb_new_pipe*)pipe->pipe, data, data_sz);

        USB_ERR("<iosm_usb_poll_intr>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (pipe->bus_type == USB3)
	{
		return libusb3_poll_intr((struct usb3_pipe*)pipe->pipe, data, data_sz);
	}

	// Should not hit
	return EINTERNAL;
}

SAP_STATUS iosm_usb_usbpdev_free_pipe(struct sp_usbpdev_pipe* pipe)
{
	SAP_STATUS status = SUCCESS;

	// Check: Parameters must be valid
	if(!pipe)
		return EINVAL;

	if(pipe->bus_type != USB2 && pipe->bus_type != USB3)
		return EINVAL;

	// Deallocate internal pipe
	if(pipe->bus_type == USB2)
	{
		// status = libusb2_usbpdev_free_pipe((struct usb_new_pipe*)pipe->pipe);

        USB_ERR("<iosm_usb_usbpdev_free_pipe>: Unsupported!\n");
        return EINTERNAL;
	}
	else if (pipe->bus_type == USB3)
	{
		status = libusb3_usbpdev_free_pipe((struct usb3_pipe*)pipe->pipe);
	}

	sfree(pipe, sizeof(struct sp_usbpdev_pipe));

	return status;
}





/********* Debug Functions *********/
void iosm_usb_enable_debug(void)
{
    _enable_debug = true;
}

void iosm_usb_disable_debug(void)
{
    _enable_debug = false;
}