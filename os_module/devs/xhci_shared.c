// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// Pause/resume XHCIs that must be shared between the untrusted OS and the IOSM-USB3. IOSM-USB3 does not need to call 
// these functions for non-shared XHCIs. The untrusted OS should just release those XHCIs; i.e., not access them any
// longer.

#include <sa_headers.h>
#include <device/usb/xhci.h>

// Disabling XHCI waits for a maximum of 100ms
#define XHCI_DISABLE_TIMEOUT_MICROSEC      (100 * 1000)
// Each yield of disabling XHCI is 100 us
#define XHCI_DISABLE_MICROSEC_PER_YIELD    (100)
#define XHCI_DISABLE_YIELDS                (XHCI_DISABLE_TIMEOUT_MICROSEC/XHCI_DISABLE_MICROSEC_PER_YIELD)

static union pci_bdf xhci_bdf = {.bus = 0, .dev = 0x14, .fn = 0};
static void* xhci_mmio_ptr = NULL;
static size_t xhci_mmio_size = 0;
static bool os_xhci_run_setting = false;
static bool os_xhci_intr_setting = false;

/// @brief Map XHCI MMIO space
/// @param hc_bdf XHCI PCI BDF
/// @param out_mmio_size Output MMIO size
/// @return Mapped MMIO pointer or NULL on failure
static void* xhci_map_mmio(union pci_bdf hc_bdf, size_t* out_mmio_size)
{
    pci_bar_t bar0 = 0;
    pci_bar_t bar1 = 0;
    paddr_t pci_bar0_paddr = INVALID_PADDR;
    void* bar0_ptr = NULL;

    // MMIO region is stored in BAR0
    pci_read_barinfo(hc_bdf, PCI_BASE_ADDRESS_0, &bar0, out_mmio_size);
    pci_read_barinfo(hc_bdf, PCI_BASE_ADDRESS_1, &bar1, NULL);
    // pci_bar0_paddr = bar0 & PCI_BASE_ADDRESS_MEM_MASK;
    pci_bar0_paddr = pci_device_bar_to_spaddr(bar0, bar1, NULL);

    bar0_ptr = os->mm_ioremap_nocache(pci_bar0_paddr, *out_mmio_size);
    DRV_DEBUG("XHCI BAR0 paddr:0x%lx, vaddr:0x%lX\n", (paddr_t)pci_bar0_paddr, (vaddr_t)bar0_ptr);

    return bar0_ptr;
}

/// @brief Unmap XHCI MMIO space
/// @param mmio_ptr MMIO pointer
/// @param mmio_size MMIO size
static void xhci_unmap_mmio(void* mmio_ptr, size_t mmio_size)
{
    os->mm_iounmap(mmio_ptr, mmio_size);
}

/// @brief Read XHCI USBCMD
/// @param xhci_ob_base XHCI operational base
/// @return USBCMD value
static uint32_t xhci_read_usbcmd(vaddr_t xhci_ob_base)
{
    vaddr_t usbcmd = xhci_ob_base + XHCI_OB_OFFSET_USBCMD;
    return sa_read32(usbcmd);
}

/// @brief Write XHCI USBCMD
/// @param xhci_ob_base XHCI operational base
/// @param value USBCMD value
static void xhci_write_usbcmd(vaddr_t xhci_ob_base, uint32_t value)
{
    vaddr_t usbcmd = xhci_ob_base + XHCI_OB_OFFSET_USBCMD;
    sa_write32(usbcmd, value);
}

/// @brief Read XHCI USBSTS
/// @param xhci_ob_base XHCI operational base
/// @return USBSTS value
static uint32_t xhci_read_usbsts(vaddr_t xhci_ob_base)
{
    vaddr_t usbsts = xhci_ob_base + XHCI_OB_OFFSET_USBSTS;
    return sa_read32(usbsts);
}

/// @brief Enable or disable XHCI interrupts
/// @param xhci_ob_base XHCI operational base
/// @param enable True to enable, false to disable
static void _xhci_intr_enable_disable(vaddr_t xhci_ob_base, bool enable)
{
    uint32_t usbcmd = xhci_read_usbcmd(xhci_ob_base);

	// [NOTE] Use write() instead of directly writing to xhci_regs.* registers. Otherwise, the function should put a
	// mfence at the end.
	if(enable)
        xhci_write_usbcmd(xhci_ob_base, usbcmd | XHCI_REG_CMD_INTE);
	else
        xhci_write_usbcmd(xhci_ob_base, usbcmd & ~XHCI_REG_CMD_INTE);
}


/// @brief Stop XHCI and wait for halt
/// @param xhci_ob_base XHCI operational base
/// @return OS_SUCCESS on success, error code on failure
static OSSTATUS xhci_stop(vaddr_t xhci_ob_base)
{
    vaddr_t usbcmd = xhci_ob_base;
	uint32_t cmd = 0;
	uint32_t sts = 0;
	uint32_t i = 0;
    uint32_t cmd_mask = XHCI_REG_CMD_RS;
    uint32_t sts_mask = XHCI_REG_STS_RS;
	
    // clear bits of USBCMD
    cmd = xhci_read_usbcmd(xhci_ob_base);
    sa_write32(usbcmd, uint32_clear_bits(cmd, cmd_mask));
	
    for (i = 0; i < XHCI_DISABLE_YIELDS; i++)
    {
        cmd = xhci_read_usbcmd(xhci_ob_base);
        if (!uint32_get_bits(cmd, cmd_mask))
            break;
        if (i == XHCI_DISABLE_YIELDS - 1) 
        {
            DRV_ERR("Disable XHCI error: Timeout to check usbcmd: cmd:%#X\n", cmd);
            return E_TIMEOUT;
        }
        os->timer_usleep(XHCI_DISABLE_MICROSEC_PER_YIELD);
    }

	// check bit of USBSTS util the relevant status bits are cleared or timeout
    for (i = 0; i < XHCI_DISABLE_YIELDS; i++)
    {
        sts = xhci_read_usbsts(xhci_ob_base);
        if (uint32_get_bits(sts,sts_mask))
            break;
        if (i == XHCI_DISABLE_YIELDS - 1) 
        {
            DRV_ERR("Disable XHCI error: Timeout to check usbsts: sts:%#X\n", sts);
            return E_TIMEOUT;
        }
        os->timer_usleep(XHCI_DISABLE_MICROSEC_PER_YIELD);
    }

	return OS_SUCCESS;
}

/// @brief Run XHCI and wait for ready
/// @param xhci_ob_base XHCI operational base
/// @return OS_SUCCESS on success, error code on failure
DECL_UNUSED static OSSTATUS xhci_run(vaddr_t xhci_ob_base)
{
	vaddr_t usbcmd = xhci_ob_base;
    uint32_t cmd = 0;
    uint32_t sts = 0;
    uint32_t i = 0;
    uint32_t cmd_mask = XHCI_REG_CMD_RS;
    uint32_t sts_mask = XHCI_REG_STS_RS;

    // set bits of USBCMD
    cmd = xhci_read_usbcmd(xhci_ob_base);
    sa_write32(usbcmd, uint32_set_bits(cmd,cmd_mask));

    for (i = 0; i < XHCI_DISABLE_YIELDS; i++)
    {
        cmd = xhci_read_usbcmd(xhci_ob_base);
        if (uint32_get_bits(cmd,cmd_mask))
            break;
        if (i == XHCI_DISABLE_YIELDS - 1) 
        {
            DRV_ERR("Enable XHCI error: Timeout to check usbcmd: cmd:%#X\n", cmd);
            return E_TIMEOUT;
        }
        os->timer_usleep(XHCI_DISABLE_MICROSEC_PER_YIELD);
    }
	
    for (i = 0; i < XHCI_DISABLE_YIELDS; i++)
    {
        sts = xhci_read_usbsts(xhci_ob_base);
        if (!uint32_get_bits(sts,sts_mask))
            break;
        if (i == XHCI_DISABLE_YIELDS - 1) 
        {
            DRV_ERR("Enable XHCI error: Timeout to check usbsts: sts:%#X\n", sts);
            return E_TIMEOUT;
        }
        os->timer_usleep(XHCI_DISABLE_MICROSEC_PER_YIELD);
    }

	return OS_SUCCESS;
}

/// @brief Read byte from XHCI MMIO
/// @param _xhci_mmio_ptr MMIO base
/// @param offset Register offset
/// @return Register value
static uint8_t _xhci_read8(void* _xhci_mmio_ptr, uint32_t offset)
{
    return sa_read8((vaddr_t)_xhci_mmio_ptr + offset);
}




/********* Public Functions *********/
OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PROBE(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    uint32_t dev_vendor_id = 0;
    // uint32_t dev_id = 0;
    uint32_t vendor_id = 0;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;
    xhci_bdf.raw = (unsigned short)cmd_packet->arg1;

    DRV_INFO("Probe XHCI [%02X:%02X:%02X]\n", xhci_bdf.bus, xhci_bdf.dev, xhci_bdf.fn);
    dev_vendor_id = pci_read_vendor_device(xhci_bdf);
    // dev_id = pci_dev_vendor_id_to_dev_id(dev_vendor_id);
    vendor_id = pci_dev_vendor_id_to_vendor_id(dev_vendor_id);

    // Check: PCI device ID and vendor ID are good
    if(vendor_id != XHCI_PCI_VENDOR_ID)
    {
        DRV_ERR("Probe XHCI failed: Unrecongized XHCI!\n");
        status = E_CHKERR;
        goto probe_err;
    }

    // Map the MMIO of the XHCI
    if(!xhci_mmio_ptr)
    {
        xhci_mmio_ptr = xhci_map_mmio(xhci_bdf, &xhci_mmio_size);
        if(!xhci_mmio_ptr)
        {
            DRV_ERR("Probe XHCI failed: Cannot access XHCI!\n");
            status = E_IO;
            goto map_mmio_err;
        }
    }
    
    // Map XHCI's MMIO is succeed
    return OS_SUCCESS;
    

map_mmio_err:
probe_err:
    return status;
}

OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_UNPROBE(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;

    UNUSED(cmd_packet);

    DRV_INFO("Release Probe Info of XHCI [%02X:%02X:%02X]\n", xhci_bdf.bus, xhci_bdf.dev, xhci_bdf.fn);
    if(xhci_mmio_ptr)
    {
        xhci_unmap_mmio(xhci_mmio_ptr, xhci_mmio_size);
        xhci_mmio_ptr = NULL;
        xhci_mmio_size = 0;
    }

    return status;
}

/// @brief Pause XHCI controller
/// @return OS_SUCCESS on success, error code on failure
static OSSTATUS _do_pause_xhci(void)
{
    OSSTATUS status = OS_SUCCESS;
    vaddr_t xhci_ob_base = INVALID_VADDR;
    uint8_t xhci_ob_offset = _xhci_read8(xhci_mmio_ptr, XHCI_REG_OFFSET_CAPLENGTH);

    DRV_INFO("Pause XHCI [%02X:%02X:%02X]\n", xhci_bdf.bus, xhci_bdf.dev, xhci_bdf.fn);
    xhci_ob_base = (vaddr_t)xhci_mmio_ptr + xhci_ob_offset;
    

    // Pause the XHCI.
    status = xhci_stop(xhci_ob_base);
    if(SA_FAIL(status))
    {
        DRV_ERR("Pause XHCI failed: Cannot stop XHCI!\n");
        status = E_IO;
        goto pause_xhci_err;
    }

    // Disable XHCI interrupts
    _xhci_intr_enable_disable(xhci_ob_base, false);

    DRV_INFO("XHCI paused\n");

    // XHCI is stopped
    return OS_SUCCESS;

pause_xhci_err:
    return status;
}

OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PAUSE(struct tz_cmd_packet *cmd_packet)
{
    vaddr_t xhci_ob_base = INVALID_VADDR;
    uint8_t xhci_ob_offset = _xhci_read8(xhci_mmio_ptr, XHCI_REG_OFFSET_CAPLENGTH);
    uint32_t cmd = 0;

    UNUSED(cmd_packet);
    if (!xhci_mmio_ptr)
        return E_INV_ARG;

    xhci_ob_base = (vaddr_t)xhci_mmio_ptr + xhci_ob_offset;

    //// Check the USBCMD of the RUN bit.
    cmd = xhci_read_usbcmd(xhci_ob_base);
    if(cmd & XHCI_REG_CMD_RS)
        os_xhci_run_setting = true;
    if(cmd & XHCI_REG_CMD_INTE)
        os_xhci_intr_setting = true;

    return _do_pause_xhci();
}

OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_RESUME(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    vaddr_t xhci_ob_base = INVALID_VADDR;
    uint8_t xhci_ob_offset = _xhci_read8(xhci_mmio_ptr, XHCI_REG_OFFSET_CAPLENGTH);

    UNUSED(cmd_packet);
    if (!xhci_mmio_ptr)
        return E_INV_ARG;

    DRV_INFO("Run XHCI [%02X:%02X:%02X]\n", xhci_bdf.bus, xhci_bdf.dev, xhci_bdf.fn);
    xhci_ob_base = (vaddr_t)xhci_mmio_ptr + xhci_ob_offset;

    // Run XHCI
    if(os_xhci_intr_setting)
    {
        // Enable XHCI interrupts
        _xhci_intr_enable_disable(xhci_ob_base, true);
        DRV_INFO("XHCI interrupts enabled\n");
    }

    if(os_xhci_run_setting)
    {
        // [TODO][Issue 120] We cannot correctly restore CRCR at the moment and hence needs to reset XHCI. So we had to 
        // disable <xhci_run> temporarily. Otherwise, XHCI will use incorrect command ring address and DMA to memory.
        // status = xhci_run(xhci_ob_base);
        // if(SA_FAIL(status))
        // {
        //     DRV_ERR("Run XHCI failed: Cannot run XHCI!\n");
        //     status = E_IO;
        //     goto resume_xhci_err;
        // }

        DRV_INFO("XHCI resumed\n");
    }

    // Clear the settings of <os_xhci_run_setting> and <os_xhci_intr_setting>
    os_xhci_run_setting = false;
    os_xhci_intr_setting = false;

    // XHCI is running
    status = OS_SUCCESS;

// resume_xhci_err:
    return status;
}