// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>
#include <device/pci.h>
#include <device/gpu/intel/i965_regs.h>
#include <device/gpu/gpu_utils.h>

#include "ilk.h"

static uint32_t os_gpu_render_rb_ctl;
static uint32_t os_gpu_bsd_rb_ctl;
static uint32_t os_gpu_blt_rb_ctl;
static uint32_t os_spa_cntr;
static uint32_t os_spb_cntr;
static uint32_t os_spra_cntr;
static uint32_t os_sprb_cntr;
static uint32_t os_cura_cntr;
static uint32_t os_curb_cntr;

static uint32_t os_pipea_cntr;
static uint32_t os_pipeb_cntr;

static uint32_t os_oa_cntr;
// static uint32_t os_fbc_cntr; // ILK does not have FBC
static uint32_t os_vga_cntr;

static OSSTATUS _gpu_pause(struct intel_gpu_intl *i965_gpu, vaddr_t gpu_regs)
{
    (void)i965_gpu;

    // Sanity checks
    if (!gpu_regs)
        return E_INV_ARG;

    // Backup OS's GPU register contents
    os_gpu_render_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(RENDER_RING_BASE));
    os_gpu_bsd_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(BSD_RING_BASE));
    os_gpu_blt_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(BLT_RING_BASE));

    os_spa_cntr = gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET));
    os_spb_cntr = gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET));

    os_spra_cntr = gpu_read_reg(gpu_regs, _DVSACNTR);
    os_sprb_cntr = gpu_read_reg(gpu_regs, _DVSBCNTR);

    os_cura_cntr = gpu_read_reg(gpu_regs, ILK_CURACNTR);
    os_curb_cntr = gpu_read_reg(gpu_regs, ILK_CURBCNTR);

    os_pipea_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET));
    os_pipeb_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET));

    os_oa_cntr = gpu_read_reg(gpu_regs, _OACONTROL);
    // os_fbc_cntr = gpu_read_reg(gpu_regs, _FBC_CONTROL); // ILK does not have FBC
    os_vga_cntr = gpu_read_reg(gpu_regs, _VGACNTRL);

    // Pause GPU
    //// Pause ring buffers
    gpu_write_reg(gpu_regs, RING_CTL(RENDER_RING_BASE), os_gpu_render_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(BSD_RING_BASE), os_gpu_bsd_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(BLT_RING_BASE), os_gpu_blt_rb_ctl & (~(1 << 31)));

    //// Pause surfaces
    gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET), os_spa_cntr & (~DISPLAY_PLANE_ENABLE));
    gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET), os_spb_cntr & (~DISPLAY_PLANE_ENABLE));

    gpu_write_reg(gpu_regs, _DVSACNTR, os_spra_cntr & (~DVS_ENABLE));
    gpu_write_reg(gpu_regs, _DVSBCNTR, os_sprb_cntr & (~DVS_ENABLE));

    gpu_write_reg(gpu_regs, ILK_CURACNTR, os_cura_cntr & (CURCNTR_DISABLE_CURSOR_MASK));
    gpu_write_reg(gpu_regs, ILK_CURBCNTR, os_curb_cntr & (CURCNTR_DISABLE_CURSOR_MASK));
    os->timer_msleep(16);

    {
        uint32_t pipea_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET));
        uint32_t pipeb_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET));

        // DRV_ERR("[Superymk] <gpu_pause> Before writing pipea_cntr:%#X\n", pipea_cntr);
        // DRV_ERR("[Superymk] <gpu_pause> Before writing pipeb_cntr:%#X\n", pipeb_cntr);
        (void)pipea_cntr;
        (void)pipeb_cntr;
    }

    gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET), os_pipea_cntr & (~PIPE_ENABLE));
    gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET), os_pipeb_cntr & (~PIPE_ENABLE));

    {
        uint32_t pipea_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET));
        uint32_t pipeb_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET));

        // DRV_ERR("[Superymk] <gpu_pause> After writing pipea_cntr:%#X\n", pipea_cntr);
        // DRV_ERR("[Superymk] <gpu_pause> After writing pipeb_cntr:%#X\n", pipeb_cntr);
        (void)pipea_cntr;
        (void)pipeb_cntr;
    }

    //// Pause other registers
    gpu_write_reg(gpu_regs, _OACONTROL, os_oa_cntr & (~OA_PERF_ENABLE));
    // gpu_write_reg(gpu_regs, _FBC_CONTROL, os_fbc_cntr & (~FBC_ENABLE)); // ILK does not have FBC
    gpu_write_reg(gpu_regs, _VGACNTRL, os_vga_cntr & (~VGA_ENABLE));

    os->timer_msleep(16);

    return OS_SUCCESS;
}

static OSSTATUS _gpu_resume(struct intel_gpu_intl *i965_gpu, vaddr_t gpu_regs)
{
    (void)i965_gpu;

    // Sanity checks
    if (!gpu_regs)
        return E_INV_ARG;

    // Resume GPU
    //// Resume ring buffers
    gpu_write_reg(gpu_regs, RING_CTL(RENDER_RING_BASE), os_gpu_render_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(BSD_RING_BASE), os_gpu_bsd_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(BLT_RING_BASE), os_gpu_blt_rb_ctl);

    //// Resume surfaces
    gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET), os_pipea_cntr);
    gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET), os_pipeb_cntr);
    os->timer_msleep(16);

    gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET), os_spa_cntr);
    gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET), os_spb_cntr);

    gpu_write_reg(gpu_regs, _DVSACNTR, os_spra_cntr);
    gpu_write_reg(gpu_regs, _DVSBCNTR, os_sprb_cntr);

    gpu_write_reg(gpu_regs, ILK_CURACNTR, os_cura_cntr);
    gpu_write_reg(gpu_regs, ILK_CURBCNTR, os_curb_cntr);

    //// Resume other registers
    gpu_write_reg(gpu_regs, _OACONTROL, os_oa_cntr);
    // gpu_write_reg(gpu_regs, _FBC_CONTROL, os_fbc_cntr); // ILK does not have FBC
    gpu_write_reg(gpu_regs, _VGACNTRL, os_vga_cntr);
    os->timer_msleep(16);

    return OS_SUCCESS;
}

/********* Public Functions *********/
OSSTATUS i965_ilk_gpu_pause(struct gpu_pdev *gpu)
{
    OSSTATUS status = OS_SUCCESS;
    void *gpu_regs = NULL;
    paddr_t gpu_regs_paddr = INVALID_PADDR;
    size_t gpu_regs_size = 0;
    struct pci_device *gpu_pci = &gpu->pci;
    struct intel_gpu_intl *i965_gpu = (struct intel_gpu_intl *)gpu->private;

    // gpu_regs_paddr = gpu_pci->pci_bar[0] & PCI_BASE_ADDRESS_MEM_MASK;
    gpu_regs_paddr = pci_device_bar_to_spaddr(gpu_pci->pci_bar[0], gpu_pci->pci_bar[1], NULL);
    gpu_regs_size = SIZE_ROUNDUP_4K(gpu_pci->pci_bar_size[0]);

    // Map the GPU's MMIO
    gpu_regs = os->mm_ioremap_nocache(gpu_regs_paddr, gpu_regs_size);
    if (!gpu_regs)
    {
        DRV_ERR("[Intel-ILK] Pause GPU failed: Failed to map GPU's MMIO!\n");
        return E_NO_MEM;
    }

    // Pause the GPU
    status = _gpu_pause(i965_gpu, (vaddr_t)gpu_regs);

    // Unmap the GPU's MMIO
    if (gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_size);

    return status;
}

OSSTATUS i965_ilk_gpu_resume(struct gpu_pdev *gpu)
{
    OSSTATUS status = OS_SUCCESS;
    void *gpu_regs = NULL;
    paddr_t gpu_regs_paddr = INVALID_PADDR;
    size_t gpu_regs_size = 0;
    struct pci_device *gpu_pci = &gpu->pci;
    struct intel_gpu_intl *i965_gpu = (struct intel_gpu_intl *)gpu->private;

    // gpu_regs_paddr = gpu_pci->pci_bar[0] & PCI_BASE_ADDRESS_MEM_MASK;
    gpu_regs_paddr = pci_device_bar_to_spaddr(gpu_pci->pci_bar[0], gpu_pci->pci_bar[1], NULL);
    gpu_regs_size = SIZE_ROUNDUP_4K(gpu_pci->pci_bar_size[0]);

    // Map the GPU's MMIO
    gpu_regs = os->mm_ioremap_nocache(gpu_regs_paddr, gpu_regs_size);
    if (!gpu_regs)
    {
        DRV_ERR("[Intel-ILK] Resume GPU failed: Failed to map GPU's MMIO!\n");
        return E_NO_MEM;
    }

    // Resume the GPU
    status = _gpu_resume(i965_gpu, (vaddr_t)gpu_regs);

    // Unmap the GPU's MMIO
    if (gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_size);

    return status;
}