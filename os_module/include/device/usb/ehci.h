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

#ifndef EHCI_H
#define EHCI_H
#include <device/pci.h>

#define EHCI_PCI_DEVICE_ID     (0x3B34)
#define EHCI_PCI_VENDOR_ID     (0x8086)

#ifndef __ASSEMBLY__

#define CMD_ASE         (1<<5)
#define CMD_PSE         (1<<4)

#define STS_ASS         (1<<15)
#define STS_PSS         (1<<14)

#define ehci_disable_async_list(ehci_regs) ehci_disable_list(ehci_regs, CMD_ASE, STS_ASS)
#define ehci_enable_async_list(ehci_regs) ehci_enable_list(ehci_regs, CMD_ASE, STS_ASS)
#define ehci_disable_periodic_list(ehci_regs) ehci_disable_list(ehci_regs, CMD_PSE, STS_PSS)
#define ehci_enable_periodic_list(ehci_regs) ehci_enable_list(ehci_regs, CMD_PSE, STS_PSS)

// extern void* ehci_map_mmio(union pci_bdf ehci_bdf);
// extern void ehci_unmap_mmio(union pci_bdf ehci_bdf);

// extern OSSTATUS ehci_disable_list(vaddr_t ehci_regs, uint32_t cmd_mask, uint32_t sts_mask);
// extern OSSTATUS ehci_enable_list(vaddr_t ehci_regs, uint32_t cmd_mask, uint32_t sts_mask);

#endif // __ASSEMBLY__
#endif // EHCI_H