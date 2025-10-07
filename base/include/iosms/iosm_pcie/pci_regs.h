// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
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

#ifndef IOSM_PCIE_REGS_H
#define IOSM_PCIE_REGS_H

#define PCI_CONFIG_SPACE_SIZE   (256)

#ifndef __ASSEMBLY__

/* PCI config space register offsets */
typedef enum {
    PCI_VENDOR_ID        = 0x00,
    PCI_DEVICE_ID        = 0x02,
    PCI_COMMAND          = 0x04,
    PCI_CLASS_REVISION   = 0x08,
    PCI_HEADER_TYPE      = 0x0e,

    PCI_BASE_ADDRESS_0   = 0x10,
    PCI_BASE_ADDRESS_1   = 0x14,
    PCI_BASE_ADDRESS_2   = 0x18,
    PCI_BASE_ADDRESS_3   = 0x1c,
    PCI_BASE_ADDRESS_4   = 0x20,
    PCI_BASE_ADDRESS_5   = 0x24,

	PCI_SUBSYSTEM_VENDOR_ID = 0x2c,
	PCI_CAPABILITY_LIST = 0x34, 
} pci_cfg_off_t;

/* PCI header type values & masks (read from PCI_HEADER_TYPE) */
typedef enum {
    PCI_HEADER_TYPE_MASK     = 0x7f,
    PCI_HEADER_TYPE_NORMAL   = 0,
    PCI_HEADER_TYPE_BRIDGE   = 1,
    PCI_HEADER_TYPE_CARDBUS  = 2,
    PCI_HEADER_TYPE_MF_MASK  = 0x80,  /* multifunction bit */
} pci_header_type_t;

/* BAR interpretation flags (read from PCI_BASE_ADDRESS_n) */
typedef enum {
    PCI_BASE_ADDRESS_SPACE        = 0x01,     /* 1 = I/O space, 0 = memory */
    PCI_BASE_ADDRESS_SPACE_IO     = 0x01,
    PCI_BASE_ADDRESS_SPACE_MEMORY = 0x00,

    PCI_BASE_ADDRESS_MEM_TYPE_MASK = 0x06,
    PCI_BASE_ADDRESS_MEM_TYPE_32   = 0x00,    /* 32-bit BAR */
    PCI_BASE_ADDRESS_MEM_TYPE_64   = 0x04,    /* 64-bit BAR */

    PCI_BASE_ADDRESS_MEM_MASK     = (int)~0x0fU,  /* addr bits for mem BAR */
    PCI_BASE_ADDRESS_IO_MASK      = (int)~0x03U,  /* addr bits for I/O BAR */
} pci_bar_flags_t;

#endif // __ASSEMBLY__

#endif // IOSM_PCIE_REGS_H