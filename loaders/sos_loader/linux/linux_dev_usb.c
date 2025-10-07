// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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