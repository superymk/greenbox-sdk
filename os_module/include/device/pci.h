// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SA_DEVICE_PCI_H
#define SA_DEVICE_PCI_H

#include <os_types.h>

/// @brief PCI BAR selectors
#define PCI_DEV_BAR_0                   (0)
#define PCI_DEV_BAR_1                   (1)
#define PCI_DEV_BAR_2                   (2)
#define PCI_DEV_BAR_3                   (3)
#define PCI_DEV_BAR_4                   (4)
#define PCI_DEV_BAR_5                   (5)

#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_COMMAND		    0x04	/* 16 bits */

#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE		0x01	/* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO	0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK	0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK	(~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK	(~0x03UL)

// IO Ports in PCI
#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_REBOOT        0x0cf9
#define PORT_PCI_DATA          0x0cfc

#ifndef __ASSEMBLY__

// typedef 	uint32_t 	pci_bar_t;

// union pci_bdf
// {
// 	struct {
// 		unsigned short	fn:3,
// 			dev:5,
// 			bus:8;
// 	} __attribute__ ((packed));
// 	unsigned short 	raw;
// };

#define pci_to_bdf(bus, dev, fn) 							(uint16_t)(((bus)<<8) | ((dev)<<3) | (fn))
#define pci_dev_vendor_id_to_dev_id(dev_vendor_id)			(uint16_t)(((dev_vendor_id) >> 16) & 0xFFFF)
#define pci_dev_vendor_id_to_vendor_id(dev_vendor_id)		(uint16_t)((dev_vendor_id) & 0xFFFF)

extern uint8_t pci_config_read8(union pci_bdf bdf, uint32_t addr);
extern uint16_t pci_config_read16(union pci_bdf bdf, uint32_t addr);
extern uint32_t pci_config_read32(union pci_bdf bdf, uint32_t addr);
extern void pci_config_write32(union pci_bdf bdf, uint32_t addr, uint32_t val);

extern uint32_t pci_read_vendor_device(union pci_bdf bdf);

/// @brief Read the configuration space (excluding the offset 30h and onwards) of the given PCI device.
/// [NOTE] Commodity OSes may forbid SecAgent and other drivers to read PCI configuration space by issuing Port I/O  
/// accesses to avoid the contention of PCI cfg and data PIO registers.
/// Thus, this function calls SecBase to dump the PCI configuration space "atomically" from the commodity OSes' 
/// perspectives.
/// @param bdf 
/// @param out_pci_cfg_space_buf 
/// @return 
extern OSSTATUS pci_read_cfg_space(union pci_bdf bdf, struct pci_device* out_pci_cfg_space_buf);

extern void pci_read_barinfo(union pci_bdf bdf, uint32_t addr, pci_bar_t* bar, size_t* bar_size);
extern void pci_write_barinfo(union pci_bdf bdf, uint32_t addr, uint32_t value);

/// @brief Get the MMIO spaddr from the given PCI BAR registers.
/// @param pci 
/// @param bar_sel 
/// @param out_64bit_mmio_addr Optional.
/// @return 
extern paddr_t pci_device_bar_to_spaddr(pci_bar_t bar_low32, pci_bar_t bar_high32, bool* out_64bit_mmio_addr);

extern bool pci_bdf_equal(union pci_bdf addr1, union pci_bdf addr2);
#endif // __ASSEMBLY__

#endif // SA_DEVICE_PCI_H