// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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