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