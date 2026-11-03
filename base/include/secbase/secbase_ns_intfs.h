// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header - Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
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