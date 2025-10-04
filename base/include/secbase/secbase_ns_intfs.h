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

#ifndef SECBASE_NS_INTERFACES_H
#define SECBASE_NS_INTERFACES_H

#ifndef __ASSEMBLY__



/********* Parameters for Hypercall <NS_HC_X86_PIO_DUMP_PCI_CFG_SPACE> *********/

#ifndef _PCI_BDF_UNION
    #define _PCI_BDF_UNION
    #ifdef _MSC_VER
        #pragma pack(push, 1)
        union pci_bdf
        {
            struct {
                unsigned short	fn:3,
                    dev:5,
                    bus:8;
            };
            unsigned short 	raw;
        };
        #pragma pack(pop)
    #else
        union pci_bdf
        {
            struct {
                unsigned short	fn:3,
                    dev:5,
                    bus:8;
            } __attribute__ ((packed));
            unsigned short 	raw;
        };
    #endif // _MSC_VER
#endif // _PCI_BDF_UNION

/// @brief Number of PCI BAR
#define PCI_DEV_NUM_BARS				6

typedef 	uint32_t 	pci_bar_t;

struct pci_device
{
    union pci_bdf 		bdf;

    // Configuration space device information
    uint16_t 			vendor, device;
    uint16_t 			class;
    uint8_t 			prog_if, revision;
    uint8_t 			header_type;
    uint8_t 			secondary_bus;

	uint16_t 			sub_class;
	uint16_t 			sub_vendor;
	
	// MMIO Info
	pci_bar_t 			pci_bar[PCI_DEV_NUM_BARS];
	size_t 				pci_bar_size[PCI_DEV_NUM_BARS];
};

#endif // __ASSEMBLY__
#endif // SECBASE_NS_INTERFACES_H