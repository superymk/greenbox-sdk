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

#ifndef XHCI_H
#define XHCI_H
#include <device/pci.h>

#define XHCI_REG_OFFSET_CAPLENGTH (0x0)

#define XHCI_PCI_DEVICE_ID     (0xa2af)
#define XHCI_PCI_VENDOR_ID     (0x8086)

#ifndef __ASSEMBLY__
#define XHCI_OB_OFFSET_USBCMD   (0x0)
#define XHCI_OB_OFFSET_USBSTS   (0x4)

#define XHCI_REG_CMD_RS          (1 << 0)
#define XHCI_REG_CMD_INTE        (1 << 2)
#define XHCI_REG_STS_RS          (1 << 0)



// extern void* xhci_map_mmio(union pci_bdf xhci_bdf);
// extern void xhci_unmap_mmio(union pci_bdf xhci_bdf);

// extern OSSTATUS xhci_disable_list(vaddr_t xhci_regs, uint32_t cmd_mask, uint32_t sts_mask);
// extern OSSTATUS xhci_enable_list(vaddr_t xhci_regs, uint32_t cmd_mask, uint32_t sts_mask);

#endif // __ASSEMBLY__
#endif // XHCI_H