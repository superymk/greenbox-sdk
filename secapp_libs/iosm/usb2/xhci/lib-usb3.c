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

#include "lib-usb3.h"
#include "iosm_usb3_calls.h"
#include "../util.h"


static shandle_t h_usbbuf_obj = INVALID_HANDLE;
static vaddr_t usbbuf_addr = INVALID_VADDR;
static size_t usbbuf_size = 0;
static uint32_t io_pid = IO_PID_INVALID;

SAP_STATUS libusb3_init(spid_t iosm_usb3_spid, size_t usbbuf_4k_pages)
{
	SAP_STATUS status = SUCCESS;

	// Sanity checks
	if ((iosm_usb3_spid == INVALID_SP_ID) || (!usbbuf_4k_pages))
		return EINVAL;

	usbbuf_size = PAGE_4K_TO_BYTES(usbbuf_4k_pages);

	// Create communication channel with the IOSM-USB3 service.
	status = comm_iosm_usb3_init(iosm_usb3_spid);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB3 error: Failed to init Inter-SecProcess communication channel with IOSM-USB3 (id:%u)! status:%lu\n", iosm_usb3_spid, status);
		return EINTERNAL;
	}

	// Create USB I/O partition
	status = iosm_usb3_register_usbdrv(usbbuf_4k_pages, &io_pid, &h_usbbuf_obj, &usbbuf_addr);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB3 error: Failed to create USB I/O partition! status:%lu\n", status);
		return EINTERNAL;
	}

	// Create USB buffer pool
	status = usbbuf_create_pool(usbbuf_addr, usbbuf_size);
	if (FAIL(status))
	{
		USB_ERR("Init Lib-USB3 error: Failed to create USB buffer pool! status:%lu\n", status);
		return EINTERNAL;
	}

	USB_INFO("Initial Lib-USB3 Success: Created USB I/O partition (id:%u)\n", io_pid);

    // On success
	return SUCCESS;
}

void libusb3_fini(void)
{
	SAP_STATUS status = SUCCESS;

	// Destroy USB buffer pool
	usbbuf_destroy_pool();

	// Destroy USB I/O partition
	if (h_usbbuf_obj != INVALID_HANDLE)
	{
		status = iosm_usb3_unregister_usbdrv(h_usbbuf_obj, usbbuf_addr, usbbuf_size);
		if (FAIL(status))
			USB_ERR("Finalize Lib-USB3 error: Failed to destroy USB I/O partition (id:%u)! status:%lu\n", io_pid, status);
	}

	// Destroy communication channel with the IOSM-USB3 service.
	comm_iosm_usb3_fini();

	h_usbbuf_obj = INVALID_HANDLE;
	io_pid = IO_PID_INVALID;
}

// /// @brief Activate the given USB peripheral device <usbpdev_addr>
SAP_STATUS libusb3_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot)
{
	SAP_STATUS status = SUCCESS;
	uint32_t usbpdev_slot = INVALID_SLOT;

	// Check: Parameters must be valid
	if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

	if(!out_usbpdev_slot)
		return EINVAL;

	// Activate USBPDev
	status = iosm_usb3_activate_usbpdev(usbpdev_addr, &usbpdev_slot);
	if (FAIL(status))
	{
		USB_ERR("Activate USBPDev error: Failed to activate USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				status);
		status = EINTERNAL;
		goto usbpdev_activate_err;
	}

	// On success
	*out_usbpdev_slot = usbpdev_slot;
	return SUCCESS;

usbpdev_activate_err:
	return status;
}

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
SAP_STATUS libusb3_deactivate_usbpdev(USBPDEV_ADDR usbpdev_addr)
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
	status = iosm_usb3_deactivate_usbpdev(usbpdev_slot);
	if (FAIL(status))
	{
		USB_ERR("Deactivate USBPDev error: Failed to deactivate USB physical device (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u) with USBPDev slot:%u! status:%lu\n",
				usbpdev_addr.dev_usbaddr, usbpdev_addr.hub_port, usbpdev_addr.hub_usbaddr, usbpdev_addr.bus_id,
				usbpdev_slot, status);
		return EINTERNAL;
	}

	return SUCCESS;
}

// /// @brief Activate an eEHCI
// SAP_STATUS libusb3_activate_eehci(uint32_t *out_eehci_slot)
// {
// 	SAP_STATUS status = SUCCESS;
// 	SAP_STATUS err_status = SUCCESS;
// 	union eEHCI_ID eehci_id;
// 	uint32_t eehci_handle;

// 	if (!out_eehci_slot)
// 		return EINVAL;

// 	// Activate eEHCI
// 	status = __iosm_usb3_activate_eehci(&eehci_id, &eehci_handle, out_eehci_slot);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Activate eEHCI error: Failed to activate eEHCI! status:%lu\n", status);
// 		status = EINTERNAL;
// 		goto eehci_activate_err;
// 	}

// 	// Register eEHCI and its slot
// 	status = eehci_register(*out_eehci_slot, eehci_id, eehci_handle);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Activate eEHCI error: Failed to register eEHCI with eEHCI slot:%u! status:%lu\n",
// 				*out_eehci_slot, status);
// 		status = EINTERNAL;
// 		goto eehci_register_err;
// 	}

// 	return SUCCESS;

// eehci_register_err:
// 	err_status = __iosm_usb3_deactivate_eehci(*out_eehci_slot);
// 	if (FAIL(err_status))
// 		USB_ERR("Activate eEHCI error: Failed to deactivate eEHCI on activation error! status:%lu\n", err_status);
// eehci_activate_err:
// 	return status;
// }

// /// @brief Deactivate the given eEHCI
// SAP_STATUS libusb3_deactivate_eehci(uint32_t eehci_slot)
// {
// 	SAP_STATUS status = SUCCESS;

// 	// Unregister eEHCI
// 	status = eehci_unregister(eehci_slot);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Deactivate eEHCI error: Failed to unregister eEHCI with eEHCI slot:%u! status:%lu\n",
// 				eehci_slot, status);
// 		return EINTERNAL;
// 	}

// 	// Deactivate eEHCI
// 	status = __iosm_usb3_deactivate_eehci(eehci_slot);
// 	if (FAIL(status))
// 	{
// 		USB_ERR("Deactivate eEHCI error: Failed to deactivate eEHCI with eEHCI slot:%u! status:%lu\n",
// 				eehci_slot, status);
// 		return EINTERNAL;
// 	}

// 	return SUCCESS;
// }