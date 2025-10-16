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

#ifndef IOSM_PCIE_PCI_H
#define IOSM_PCIE_PCI_H

/// @brief Number of PCI BAR
#define PCI_DEV_NUM_BARS 6

/// @brief Invalid raw value of <union pci_bdf>.
/// [NOTE] Though some platforms do have PCI devices at 00:00.0, these devices often are host controllers but are not
/// peripheral devices.
#define INVALID_PCI_BDF_RAW 0

#ifndef __ASSEMBLY__
#include <pci_ids.h>
#include <pci_regs.h>

typedef uint32_t pci_bar_t;

union pci_bdf
{
    struct
    {
        unsigned short fn : 3,
            dev : 5,
            bus : 8;
    } __attribute__((packed));
    unsigned short raw;
};

#define PCI_BDF(b, d, f) \
    ((union pci_bdf){.bus = b, .dev = d, .fn = f})

struct pci_device
{
    union pci_bdf bdf;

    // Configuration space device information
    uint16_t vendor, device;
    uint16_t command, status;
    uint16_t class;
    uint8_t prog_if, revision;
    uint8_t header_type;
    uint8_t secondary_bus;

    uint16_t sub_class;
    uint16_t sub_vendor;

    // MMIO Info
    pci_bar_t pci_bar[PCI_DEV_NUM_BARS];
    size_t pci_bar_size[PCI_DEV_NUM_BARS];

    // Capability list
    uint8_t cap_ptr;

    // IRQ
    irq_t irq;

    // PCI config space >= 0x40
    uint8_t raw_above_40b[256 - 0x40];
};

#endif // __ASSEMBLY__
#endif // IOSM_PCIE_PCI_H
