// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOS_LOADER_DEV_PCI_H
#define SOS_LOADER_DEV_PCI_H

#ifndef __ASSEMBLY__

/// @brief Invalid raw value of <union pci_bdf>.
/// [NOTE] Though some platforms do have PCI devices at 00:00.0, these devices often are host controllers but are not 
/// peripheral devices.
#define INVALID_PCI_BDF_RAW 			0

static inline union pci_bdf pci_to_bdf(int bus, int dev, int fn) {
    union pci_bdf result;
    result.fn = fn;
    result.dev = dev;
    result.bus = bus;
    return result;
}

#endif // __ASSEMBLY__
#endif // SOS_LOADER_DEV_PCI_H