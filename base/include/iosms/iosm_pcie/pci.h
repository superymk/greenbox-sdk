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

#ifndef IOSM_PCIE_PCI_H
#define IOSM_PCIE_PCI_H

/// @brief Number of PCI BAR
#define PCI_DEV_NUM_BARS				6

/// @brief Invalid raw value of <union pci_bdf>.
/// [NOTE] Though some platforms do have PCI devices at 00:00.0, these devices often are host controllers but are not 
/// peripheral devices.
#define INVALID_PCI_BDF_RAW 			0


#ifndef __ASSEMBLY__
#include <pci_ids.h>
#include <pci_regs.h>

typedef 	uint32_t 	pci_bar_t;



union pci_bdf
{
	struct {
		unsigned short	fn:3,
			dev:5,
			bus:8;
	} __attribute__ ((packed));
	unsigned short 	raw;
};

#define PCI_BDF(b,d,f) \
    ((union pci_bdf){ .bus = b, .dev = d, .fn = f })


struct pci_device {
    union pci_bdf 		bdf;

    // Configuration space device information
    uint16_t 			vendor, device;
    uint16_t            command, status;
    uint16_t 			class;
    uint8_t 			prog_if, revision;
    uint8_t 			header_type;
    uint8_t 			secondary_bus;

	uint16_t 			sub_class;
	uint16_t 			sub_vendor;
	
	// MMIO Info
	pci_bar_t 			pci_bar[PCI_DEV_NUM_BARS];
	size_t 				pci_bar_size[PCI_DEV_NUM_BARS];

    // Capability list
    uint8_t             cap_ptr;

	// IRQ
	irq_t				irq;

    // PCI config space >= 0x40
    uint8_t             raw_above_40b[256 - 0x40];
};

#endif // __ASSEMBLY__
#endif // IOSM_PCIE_PCI_H
