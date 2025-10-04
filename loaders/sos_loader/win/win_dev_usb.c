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

#include <sl_headers.h>

#include <iosm_pcie_app_intfs.h>
#include <dev/pci.h>

#include <stdio.h>

union pci_bdf os_usb_pci_addr_string_to_bdf(char* usb_hc_pci_bdf)
{
	int items = 0;
	uint32_t domain, bus, device, function;
	union pci_bdf result = {.raw = INVALID_PCI_BDF_RAW};

	// On Linux, lsusb always encode USB host controller's PCI address as "XXXX:XX:XX.X"
	items = sscanf(usb_hc_pci_bdf, "%04x:%02x:%02x.%u", &domain, &bus, &device, &function);
	
	// Check: the format of <usb_hc_pci_bdf> must be "XXXX:XX:XX.X" (i.e., the input must have four sub strings)
	if(items != 4) 
		return result;

	return pci_to_bdf(bus, device, function);
}