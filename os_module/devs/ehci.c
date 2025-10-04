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

#include <sa_headers.h>
#include <device/usb/ehci.h>

// Disabling each list in <ehci_disable_list> maximum waits for 100ms
#define EHCI_DISABLE_LIST_TIMEOUT_MICROSEC      (100 * 1000)
// Each yield in <ehci_disable_list> is 100 us
#define EHCI_DISABLE_LIST_MICROSEC_PER_YIELD    (100)
#define EHCI_DISABLE_LIST_YIELDS                (EHCI_DISABLE_LIST_TIMEOUT_MICROSEC/EHCI_DISABLE_LIST_MICROSEC_PER_YIELD)

static void* ehci_mmio_ptr = NULL;
static size_t ehci_mmio_size = 0;
static bool os_alist_setting = false;
static bool os_plist_setting = false;

static void* ehci_map_mmio(union pci_bdf ehci_bdf, size_t* out_mmio_size)
{
    pci_bar_t bar0 = 0;
    pci_bar_t bar1 = 0;
    paddr_t pci_bar0_paddr = INVALID_PADDR;
    void* bar0_ptr = NULL;

    // Check: <out_mmio_size> must be a valid addr.
    if(!out_mmio_size)
        return NULL;

    // MMIO region is stored in BAR0
    pci_read_barinfo(ehci_bdf, PCI_BASE_ADDRESS_0, &bar0, out_mmio_size);
    pci_read_barinfo(ehci_bdf, PCI_BASE_ADDRESS_1, &bar1, NULL);
    // pci_bar0_paddr = bar0 & PCI_BASE_ADDRESS_MEM_MASK;
    pci_bar0_paddr = pci_device_bar_to_spaddr(bar0, bar1, NULL);

    bar0_ptr = os->mm_ioremap_nocache(pci_bar0_paddr, *out_mmio_size);
    DRV_DEBUG("EHCI BAR0 paddr:0x%lx, vaddr:0x%lX\n", (paddr_t)pci_bar0_paddr, (vaddr_t)bar0_ptr);

    return bar0_ptr;
}

static void ehci_unmap_mmio(void* mmio_ptr, size_t mmio_size)
{
    os->mm_iounmap(mmio_ptr, mmio_size);
}

static uint32_t ehci_read_usbcmd(vaddr_t ehci_regs)
{
    vaddr_t usbcmd = ehci_regs;
    return sa_read32(usbcmd);
}

static uint32_t ehci_read_usbsts(vaddr_t ehci_regs)
{
    vaddr_t usbsts = ehci_regs + 4;
    return sa_read32(usbsts);
}

static OSSTATUS ehci_disable_list(vaddr_t ehci_regs, uint32_t cmd_mask, uint32_t sts_mask)
{
    vaddr_t usbcmd = ehci_regs;
	uint32_t cmd = 0, sts = 0;
	uint32_t i = 0;

    // Sanity checks
    if(!ehci_regs)
        return E_INV_ARG;
	
    // clear bits of USBCMD
    cmd = ehci_read_usbcmd(ehci_regs);
    sa_write32(usbcmd, uint32_clear_bits(cmd, cmd_mask));
	
    for (i = 0; i < EHCI_DISABLE_LIST_YIELDS; i++)
    {
        cmd = ehci_read_usbcmd(ehci_regs);
        if (!uint32_get_bits(cmd, cmd_mask))
            break;
        if (i == EHCI_DISABLE_LIST_YIELDS - 1) 
        {
            DRV_ERR("Disable list error: Timeout to check usbcmd: cmd:%#X\n", cmd);
            return E_TIMEOUT;
        }
        os->timer_usleep(EHCI_DISABLE_LIST_MICROSEC_PER_YIELD);
    }

	// check bit of USBSTS util the relevant status bits are cleared or timeout
    for (i = 0; i < EHCI_DISABLE_LIST_YIELDS; i++)
    {
        sts = ehci_read_usbsts(ehci_regs);
        if (!uint32_get_bits(sts,sts_mask))
            break;
        if (i == EHCI_DISABLE_LIST_YIELDS - 1) 
        {
            DRV_ERR("Disable list error: Timeout to check usbsts: sts:%#X\n", sts);
            return E_TIMEOUT;
        }
        os->timer_usleep(EHCI_DISABLE_LIST_MICROSEC_PER_YIELD);
    }

	return OS_SUCCESS;
}

static OSSTATUS ehci_enable_list(vaddr_t ehci_regs, uint32_t cmd_mask, uint32_t sts_mask)
{
	vaddr_t usbcmd = ehci_regs;
	uint32_t cmd = 0, sts = 0;
	uint32_t i = 0;

	// Sanity checks
    if(!ehci_regs)
        return E_INV_ARG;

    // set bits of USBCMD
    cmd = ehci_read_usbcmd(ehci_regs);
    sa_write32(usbcmd, uint32_set_bits(cmd,cmd_mask));

    for (i = 0; i < EHCI_DISABLE_LIST_YIELDS; i++)
    {
        cmd = ehci_read_usbcmd(ehci_regs);
        if (uint32_get_bits(cmd,cmd_mask))
            break;
        if (i == EHCI_DISABLE_LIST_YIELDS - 1) 
        {
            DRV_ERR("Enable list error: Timeout to check usbcmd: cmd:%#X\n", cmd);
            return E_TIMEOUT;
        }
        os->timer_usleep(EHCI_DISABLE_LIST_MICROSEC_PER_YIELD);
    }
	
    for (i = 0; i < EHCI_DISABLE_LIST_YIELDS; i++)
    {
        sts = ehci_read_usbsts(ehci_regs);
        if (uint32_get_bits(sts,sts_mask))
            break;
        if (i == EHCI_DISABLE_LIST_YIELDS - 1) 
        {
            DRV_ERR("Enable list error: Timeout to check usbsts: sts:%#X\n", sts);
            return E_TIMEOUT;
        }
        os->timer_usleep(EHCI_DISABLE_LIST_MICROSEC_PER_YIELD);
    }

	return OS_SUCCESS;
}




/********* Public Functions *********/
OSSTATUS do_CMD_IOSM_USB2_EHCI_PROBE(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    union pci_bdf ehci_bdf = {.bus = 0, .dev = 0x1d, .fn = 0};
    uint32_t dev_vendor_id = 0;
    // uint32_t dev_id = 0;
    uint32_t vendor_id = 0;

    UNUSED(cmd_packet);

    DRV_INFO("Probe EHCI [00:1d:00]\n");
    dev_vendor_id = pci_read_vendor_device(ehci_bdf);
    // dev_id = pci_dev_vendor_id_to_dev_id(dev_vendor_id);
    vendor_id = pci_dev_vendor_id_to_vendor_id(dev_vendor_id);

    // Check: PCI device ID and vendor ID are good
    if(vendor_id != EHCI_PCI_VENDOR_ID)
    {
        DRV_ERR("Probe EHCI failed: Unrecongized EHCI!\n");
        status = E_CHKERR;
        goto probe_err;
    }

    // Map the MMIO of the EHCI
    if(!ehci_mmio_ptr)
    {
        ehci_mmio_ptr = ehci_map_mmio(ehci_bdf, &ehci_mmio_size);
        if(!ehci_mmio_ptr)
        {
            DRV_ERR("Probe EHCI failed: Cannot access EHCI!\n");
            status = E_IO;
            goto map_mmio_err;
        }
    }
    
    // Map EHCI's MMIO is succeed
    status = OS_SUCCESS;
    

map_mmio_err:
probe_err:
    return status;
}

OSSTATUS do_CMD_IOSM_USB2_EHCI_UNPROBE(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    // union pci_bdf ehci_bdf = {.bus = 0, .dev = 0x1d, .fn = 0};

    UNUSED(cmd_packet);

    DRV_INFO("Release Probe Info of EHCI [00:1d:00]\n");
    if(ehci_mmio_ptr)
    {
        ehci_unmap_mmio(ehci_mmio_ptr, ehci_mmio_size);
        ehci_mmio_ptr = NULL;
        ehci_mmio_size = 0;
    }

    return status;
}

OSSTATUS do_CMD_IOSM_USB2_PAUSE_EHCI(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    vaddr_t usb_regs = INVALID_VADDR;
    uint32_t cmd = 0;

    UNUSED(cmd_packet);
    DRV_INFO("Pause EHCI [00:1d:00]\n");
    usb_regs = (vaddr_t)ehci_mmio_ptr + 0x20;
    
    // Pause EHCI
    // [NOTE] We do not clear Run/Stop bit in USBCMD, but instead stop Async list and Periodic List.  
    // This is because of an issue: if we clear Run/Stop bit and then set the bit in IOSM-USB2, then the EHCI
    // cannot talk to its root hub.

    //// Check the USBCMD of the Async list and Periodic List.
    cmd = ehci_read_usbcmd(usb_regs);
    if(cmd & CMD_ASE)
        os_alist_setting = true;
    if(cmd & CMD_PSE)
        os_plist_setting = true;

    //// Pause the lists accordingly.
    if(os_alist_setting)
    {
        status = ehci_disable_async_list(usb_regs);
        if(SA_FAIL(status))
        {
            DRV_ERR("Pause EHCI failed: Cannot stop async list!\n");
            status = E_IO;
            goto pause_ehci_err;
        }

        DRV_INFO("EHCI async list paused\n");
    }

    if(os_plist_setting)
    {
        status = ehci_disable_periodic_list(usb_regs);
        if(SA_FAIL(status))
        {
            DRV_ERR("Pause EHCI failed: Cannot stop periodic list!\n");
            status = E_IO;
            goto pause_ehci_err;
        }

        DRV_INFO("EHCI periodic list paused\n");
    }

    // EHCI is stopped
    status = OS_SUCCESS;

pause_ehci_err:
    return status;
}

OSSTATUS do_CMD_IOSM_USB2_RESUME_EHCI(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    vaddr_t usb_regs = INVALID_VADDR;

    UNUSED(cmd_packet);
    DRV_INFO("Run EHCI [00:1d:00]\n");
    usb_regs = (vaddr_t)ehci_mmio_ptr + 0x20;

    // Run EHCI
    // [NOTE] We do not set Run/Stop bit in USBCMD, but instead run Async list and Periodic List.  
    // This is because of an issue: if we clear Run/Stop bit and then set the bit in IOSM-USB2, then the EHCI
    // cannot talk to its root hub. Similar issue when resuming EHCI here in the OS.
    if(os_alist_setting)
    {
        status = ehci_enable_async_list(usb_regs);
        if(SA_FAIL(status))
        {
            DRV_ERR("Run EHCI failed: Cannot run async list!\n");
            status = E_IO;
            goto resume_ehci_err;
        }

        DRV_INFO("EHCI async list resumed\n");
    }

    if(os_plist_setting)
    {
        status = ehci_enable_periodic_list(usb_regs);
        if(SA_FAIL(status))
        {
            DRV_ERR("Run EHCI failed: Cannot run periodic list!\n");
            status = E_IO;
            goto resume_ehci_err;
        }

        DRV_INFO("EHCI periodic list resumed\n");
    }

    // Clear the settings of <os_alist_setting> and <os_plist_setting>
    os_alist_setting = false;
    os_plist_setting = false;

    // EHCI is running
    status = OS_SUCCESS;

resume_ehci_err:
    return status;
}