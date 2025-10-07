// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>
#include <device/pci.h>

extern struct os_funcs *os;

uint8_t pci_config_read8(union pci_bdf bdf, uint32_t addr)
{
	os->outl(0x80000000 | (bdf.raw << 8) | (addr & 0xfc), PORT_PCI_CMD);
	return os->inb(PORT_PCI_DATA + (addr & 3));
}

uint16_t pci_config_read16(union pci_bdf bdf, uint32_t addr)
{
	os->outl(0x80000000 | (bdf.raw << 8) | (addr & 0xfc), PORT_PCI_CMD);
	return os->inw(PORT_PCI_DATA + (addr & 2));
}

uint32_t pci_config_read32(union pci_bdf bdf, uint32_t addr)
{
    os->outl(0x80000000 | (bdf.raw << 8) | (addr & 0xfc), PORT_PCI_CMD);
    return os->inl(PORT_PCI_DATA);
}

void pci_config_write32(union pci_bdf bdf, uint32_t addr, uint32_t val)
{
	os->outl(0x80000000 | (bdf.raw << 8) | (addr & 0xfc), PORT_PCI_CMD);
    os->outl(val, PORT_PCI_DATA);
}

uint32_t pci_read_vendor_device(union pci_bdf bdf)
{
	return pci_config_read32(bdf, PCI_VENDOR_ID);
}

OSSTATUS pci_read_cfg_space(union pci_bdf bdf, struct pci_device* out_pci_cfg_space_buf)
{
    MHV_STATUS mhv_sts = MHV_SUCCESS;

    if(!out_pci_cfg_space_buf)
        return E_INV_ARG;

    mhv_sts = (MHV_STATUS)os->hypercall_2args(NS_HC_X86_PIO_DUMP_PCI_CFG_SPACE, (ulong_t)bdf.raw, (ulong_t)out_pci_cfg_space_buf);
    if(MHV_FAIL(mhv_sts))
        return E_INTERNAL_ERROR;

    // On success
    return OS_SUCCESS;
}

void pci_read_barinfo(union pci_bdf bdf, uint32_t addr, pci_bar_t* bar, size_t* bar_size)
{
	uint32_t old_value, new_value;

    old_value = pci_config_read32(bdf, addr);
	*bar = (pci_bar_t)old_value;
	
	pci_config_write32(bdf, addr, 0xFFFFFFFF);
	new_value = pci_config_read32(bdf, addr);

	if((*bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY)
	{
		new_value &= PCI_BASE_ADDRESS_MEM_MASK;

        if(bar_size)
		    *bar_size = (size_t)((~(new_value))+1);
	}
	else if((*bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO)
	{
		new_value &= PCI_BASE_ADDRESS_IO_MASK;

        if(bar_size)
		    *bar_size = (uint16_t)((~(new_value))+1);
	}
	
	pci_config_write32(bdf, addr, old_value);
}

void pci_write_barinfo(union pci_bdf bdf, uint32_t addr, uint32_t value)
{
    pci_config_write32(bdf, addr, value);
}

paddr_t pci_device_bar_to_spaddr(pci_bar_t bar_low32, pci_bar_t bar_high32, bool* out_64bit_mmio_addr)
{
    paddr_t result = INVALID_PADDR;

    if((bar_low32 & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO)
    {
        // The BAR describes Port I/O, not MMIO
        return INVALID_PADDR;
    }
    else if((bar_low32 & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64)
    {
        // BAR contains a 64-bit address.
        //// A 64-bit base address register consumes 2 of the base address registers available.

        if(out_64bit_mmio_addr)
            *out_64bit_mmio_addr = true;

        result = (paddr_t)UINT32_TO_64((uint32_t)bar_high32, (uint32_t)(bar_low32 & PCI_BASE_ADDRESS_MEM_MASK));
    }
    else if((bar_low32 & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_32)
    {
        // BAR contains a 32-bit address.
        if(out_64bit_mmio_addr)
            *out_64bit_mmio_addr = false;

        result = (paddr_t)(bar_low32 & PCI_BASE_ADDRESS_MEM_MASK);
    }

    return result;
}

bool pci_bdf_equal(union pci_bdf addr1, union pci_bdf addr2)
{
    if(addr1.raw == addr2.raw)
        return true;
    else
        return false;
}