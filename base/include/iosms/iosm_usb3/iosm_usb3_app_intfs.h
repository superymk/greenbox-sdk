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

#ifndef IOSM_USB3_APP_INTFS_H
#define IOSM_USB3_APP_INTFS_H


#define MAX_TP_PATH_USBNODES 			16
#define NUM_ASSIGN_USBPDEVS				8

#define NONEXIST_UPSTREAM_HUB_ADDR		0x7F 	// The address must not equal to USB_DEFAULT_ADDR
#define NONEXIST_UPSTREAM_HUB_PORT		0x7F

#ifndef __ASSEMBLY__
#include <iosm_pcie_app_intfs.h>
#include <common/intfs.h>

/// @brief A USB node represents a host controller, hub, or a USB peripheral device.
/// [NOTE] Though XHCI assigns USB addresses by itself, IOSM-USB3 must control ALL parent hubs of SecProcess' USB 
/// devices. Otherwise, untrusted OSes/Apps can reset/re-configure those devices by misconfiguring these hubs.
struct usb_node
{
	USBPDEV_ADDR			node_addr;

	// /// @brief Vendor ID of the USB device. The field is ignored for root hubs (is_root_hub == true)
	// uint16_t 				dev_vendor_id;

	// /// @brief Product ID of the USB device. The field is ignored for root hubs (is_root_hub == true)
	// uint16_t 				dev_product_id;

	///	@brief True iff the USB node describes a USB peripheral device that can be assigned to SecApps (aka., "USBPDev").
	/// [NOTE] SecApps can only use a USBPDev if this flag is set, but not any other USB nodes.
	bool					tp_usbpdev;

	///	@brief True iff the USB node describes a root hub
	bool					is_root_hub;

	///	@brief True iff the USB node describes a USB hub
	bool					is_hub;
};

#define ROOTHUB_PORTS_NEED_RESUME_NUM (8)

/// @brief Configurations of USB device separation
struct usb_conf
{	
    union pci_bdf			xhci_bdf;

	/// @brief USB paths from XHCI to the USB peripheral devices to be assigned to IOSM-USB3.
	/// [NOTE] All USB nodes must under the same USB bus (i.e., <node_addr>.bus_id of <usb_nodes> are same).
	/// [NOTE] If a USB device A connects to a USB hub B, then B must be defined first than A in <usb_nodes>.
	/// [NOTE] If different paths share the same USB node, the node should be described only once.
	/// [NOTE] One should not describe any other USB paths.
	/// [NOTE] Only the USB paths to the USBPDevs are alive. All other paths and USB nodes must be suspended.
	struct usb_node			usb_nodes[MAX_TP_PATH_USBNODES];
	uint32_t				num_usb_nodes;

    /// @brief The USB address of the host controller's roothub set up by the untrusted OS. 
    uint32_t                os_hc_roothub_usb_addr;
};


#endif // __ASSEMBLY__
#endif // IOSM_USB3_APP_INTFS_H