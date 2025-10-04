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
#include <device/pci.h>
#include <device/gpu/intel/i965_regs.h>
#include <device/gpu/gpu_utils.h>

#include "gen9.h"

static void _gpu_regs_dump(vaddr_t gpu_regs, char *prefix);

static uint32_t os_gpu_render_rb_ctl;
static uint32_t os_gpu_vcs0_rb_ctl;
static uint32_t os_gpu_ecs_rb_ctl;
static uint32_t os_gpu_vcs1_rb_ctl;
static uint32_t os_gpu_blt_rb_ctl;
static uint32_t os_spa_cntr;
static uint32_t os_spb_cntr;
static uint32_t os_spc_cntr;
static uint32_t os_cura_cntr;
static uint32_t os_curb_cntr;
static uint32_t os_curc_cntr;

static uint32_t os_pipea_cntr;
static uint32_t os_pipeb_cntr;
static uint32_t os_pipec_cntr;

static uint32_t os_oa_cntr;
static uint32_t os_fbc_cntr;
static uint32_t os_vga_cntr;

static OSSTATUS _gpu_pause(struct intel_gpu_intl *i965_gpu, vaddr_t gpu_regs)
{
    // Sanity checks
    if (!gpu_regs)
        return E_INV_ARG;

    // Backup OS's GPU register contents
    os_gpu_render_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(RENDER_RING_BASE));
    os_gpu_vcs0_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(VIDEO_CS0_RING_BASE));
    os_gpu_ecs_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(VIDEO_ECS_RING_BASE));
    os_gpu_vcs1_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(VIDEO_CS1_RING_BASE));
    os_gpu_blt_rb_ctl = gpu_read_reg(gpu_regs, RING_CTL(BLT_RING_BASE));

    os_spa_cntr = gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET));
    os_spb_cntr = gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET));
    os_spc_cntr = gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_C_OFFSET));

    os_cura_cntr = gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_A_OFFSET));
    os_curb_cntr = gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_B_OFFSET));
    os_curc_cntr = gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_C_OFFSET));

    os_pipea_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET));
    os_pipeb_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET));
    os_pipec_cntr = gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_C_OFFSET));

    os_oa_cntr = gpu_read_reg(gpu_regs, _OACONTROL);
    os_fbc_cntr = gpu_read_reg(gpu_regs, _FBC_CONTROL);
    os_vga_cntr = gpu_read_reg(gpu_regs, _VGACNTRL);

    // Pause GPU
    //// Pause ring buffers
    gpu_write_reg(gpu_regs, RING_CTL(RENDER_RING_BASE), os_gpu_render_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_CS0_RING_BASE), os_gpu_vcs0_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_ECS_RING_BASE), os_gpu_ecs_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_CS1_RING_BASE), os_gpu_vcs1_rb_ctl & (~(1 << 31)));
    gpu_write_reg(gpu_regs, RING_CTL(BLT_RING_BASE), os_gpu_blt_rb_ctl & (~(1 << 31)));

    // Pause surfaces
    {
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_A_OFFSET), os_cura_cntr & (CURCNTR_DISABLE_CURSOR_MASK));
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET), os_spa_cntr & (~DISPLAY_PLANE_ENABLE));
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET), os_pipea_cntr & (~PIPE_ENABLE));
    }

    {
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_B_OFFSET), os_curb_cntr & (CURCNTR_DISABLE_CURSOR_MASK));
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET), os_spb_cntr & (~DISPLAY_PLANE_ENABLE));
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET), os_pipeb_cntr & (~PIPE_ENABLE));
    }

    {
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_C_OFFSET), os_curc_cntr & (CURCNTR_DISABLE_CURSOR_MASK));
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_C_OFFSET), os_spc_cntr & (~DISPLAY_PLANE_ENABLE));
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_C_OFFSET), os_pipec_cntr & (~PIPE_ENABLE));
    }
    os->timer_msleep(16);

    //// Pause other registers
    gpu_write_reg(gpu_regs, _OACONTROL, os_oa_cntr & (~OA_PERF_ENABLE));
    gpu_write_reg(gpu_regs, _VGACNTRL, os_vga_cntr | VGA_DISABLE);

    if(i965_gpu->has_fb_compression)
        gpu_write_reg(gpu_regs, _FBC_CONTROL, os_fbc_cntr & (~FBC_ENABLE));
    

    os->timer_msleep(16);

    return OS_SUCCESS;
}

static OSSTATUS _gpu_resume(struct intel_gpu_intl *i965_gpu, vaddr_t gpu_regs)
{
    // Sanity checks
    if (!gpu_regs)
        return E_INV_ARG;

    // Resume GPU
    //// Resume ring buffers
    gpu_write_reg(gpu_regs, RING_CTL(RENDER_RING_BASE), os_gpu_render_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_CS0_RING_BASE), os_gpu_vcs0_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_ECS_RING_BASE), os_gpu_ecs_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(VIDEO_CS1_RING_BASE), os_gpu_vcs1_rb_ctl);
    gpu_write_reg(gpu_regs, RING_CTL(BLT_RING_BASE), os_gpu_blt_rb_ctl);

    //// Resume surfaces
    {
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET), os_pipea_cntr);
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET), os_spa_cntr);
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_A_OFFSET), os_cura_cntr);
    }

    {
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET), os_pipeb_cntr);
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET), os_spb_cntr);
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_B_OFFSET), os_curb_cntr);
    }

    {
        gpu_write_reg(gpu_regs, PIPECONF(I965_PLANE_C_OFFSET), os_pipec_cntr);
        gpu_write_reg(gpu_regs, DSPCNTR(I965_PLANE_C_OFFSET), os_spc_cntr);
        gpu_write_reg(gpu_regs, CURCNTR(I965_PLANE_C_OFFSET), os_curc_cntr);
    }

    os->timer_msleep(16);

    //// Resume other registers
    gpu_write_reg(gpu_regs, _OACONTROL, os_oa_cntr);
    gpu_write_reg(gpu_regs, _VGACNTRL, os_vga_cntr);

    if(i965_gpu->has_fb_compression)
        gpu_write_reg(gpu_regs, _FBC_CONTROL, os_fbc_cntr);

    os->timer_msleep(16);

    return OS_SUCCESS;
}

OSSTATUS i965_gen9_gpu_pause(struct gpu_pdev *gpu)
{
    OSSTATUS status = OS_SUCCESS;
    void *gpu_regs = NULL;
    paddr_t gpu_regs_paddr = INVALID_PADDR;
    size_t gpu_regs_size = 0;
    struct pci_device *gpu_pci = &gpu->pci;
    struct intel_gpu_intl *i965_gpu = (struct intel_gpu_intl *)gpu->private;

    gpu_regs_paddr = pci_device_bar_to_spaddr(gpu_pci->pci_bar[0], gpu_pci->pci_bar[1], NULL);
    gpu_regs_size = SIZE_ROUNDUP_4K(gpu_pci->pci_bar_size[0]);

    // Map the GPU's MMIO
    gpu_regs = os->mm_ioremap_nocache(gpu_regs_paddr, gpu_regs_size);
    if (!gpu_regs)
    {
        DRV_ERR("[Intel-Gen9] Pause GPU failed: Failed to map GPU's MMIO!\n");
        return E_NO_MEM;
    }

    // Pause the GPU
    _gpu_regs_dump((vaddr_t)gpu_regs, "Before <_gpu_pause>");

    status = _gpu_pause(i965_gpu, (vaddr_t)gpu_regs);

    // Unmap the GPU's MMIO
    if (gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_size);

    return status;
}

extern OSSTATUS i965_gen9_gpu_resume(struct gpu_pdev *gpu)
{
    OSSTATUS status = OS_SUCCESS;
    void *gpu_regs = NULL;
    paddr_t gpu_regs_paddr = INVALID_PADDR;
    size_t gpu_regs_size = 0;
    struct pci_device *gpu_pci = &gpu->pci;
    struct intel_gpu_intl *i965_gpu = (struct intel_gpu_intl *)gpu->private;

    gpu_regs_paddr = pci_device_bar_to_spaddr(gpu_pci->pci_bar[0], gpu_pci->pci_bar[1], NULL);
    gpu_regs_size = SIZE_ROUNDUP_4K(gpu_pci->pci_bar_size[0]);

    // Map the GPU's MMIO
    gpu_regs = os->mm_ioremap_nocache(gpu_regs_paddr, gpu_regs_size);
    if (!gpu_regs)
    {
        DRV_ERR("[Intel-Gen9] Resume GPU failed: Failed to map GPU's MMIO!\n");
        return E_NO_MEM;
    }

    // Resume the GPU
    status = _gpu_resume(i965_gpu, (vaddr_t)gpu_regs);

    _gpu_regs_dump((vaddr_t)gpu_regs, "After <_gpu_resume>");

    // Unmap the GPU's MMIO
    if (gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_size);

    return status;
}

/********* Debug Functions *********/
static void _gpu_regs_dump(vaddr_t gpu_regs, char *prefix)
{
    DRV_DEBUG("%s\n", prefix);
    DRV_DEBUG("\t RING_CTL(RENDER_RING_BASE):0x%08X\n", gpu_read_reg(gpu_regs, RING_CTL(RENDER_RING_BASE)));
    DRV_DEBUG("\t RING_CTL(VIDEO_CS0_RING_BASE):0x%08X\n", gpu_read_reg(gpu_regs, RING_CTL(VIDEO_CS0_RING_BASE)));
    DRV_DEBUG("\t RING_CTL(VIDEO_ECS_RING_BASE):0x%08X\n", gpu_read_reg(gpu_regs, RING_CTL(VIDEO_ECS_RING_BASE)));
    DRV_DEBUG("\t RING_CTL(VIDEO_CS1_RING_BASE):0x%08X\n", gpu_read_reg(gpu_regs, RING_CTL(VIDEO_CS1_RING_BASE)));
    DRV_DEBUG("\t RING_CTL(BLT_RING_BASE):0x%08X\n", gpu_read_reg(gpu_regs, RING_CTL(BLT_RING_BASE)));

    // Plane A
    DRV_DEBUG("\n");
    DRV_DEBUG("\t PIPECONF(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_A_OFFSET)));
    DRV_DEBUG("\t PIPESRC(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, PIPESRC(I965_PLANE_A_OFFSET)));

    DRV_DEBUG("\t DSPCNTR(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_A_OFFSET)));
    DRV_DEBUG("\t DSPSURF(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURF(I965_PLANE_A_OFFSET)));
    DRV_DEBUG("\t DSPSURFLIVE(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURFLIVE(I965_PLANE_A_OFFSET)));

    DRV_DEBUG("\t CURCNTR(I965_PLANE_A):0x%08X\n", gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_A_OFFSET)));

    // Plane B
    DRV_DEBUG("\n");
    DRV_DEBUG("\t PIPECONF(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_B_OFFSET)));
    DRV_DEBUG("\t PIPESRC(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, PIPESRC(I965_PLANE_B_OFFSET)));

    DRV_DEBUG("\t DSPCNTR(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_B_OFFSET)));
    DRV_DEBUG("\t DSPSURF(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURF(I965_PLANE_B_OFFSET)));
    DRV_DEBUG("\t DSPSURFLIVE(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURFLIVE(I965_PLANE_B_OFFSET)));

    DRV_DEBUG("\t CURCNTR(I965_PLANE_B):0x%08X\n", gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_B_OFFSET)));

    // Plane C
    DRV_DEBUG("\n");
    DRV_DEBUG("\t PIPECONF(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, PIPECONF(I965_PLANE_C_OFFSET)));
    DRV_DEBUG("\t PIPESRC(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, PIPESRC(I965_PLANE_C_OFFSET)));

    DRV_DEBUG("\t DSPCNTR(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, DSPCNTR(I965_PLANE_C_OFFSET)));
    DRV_DEBUG("\t DSPSURF(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURF(I965_PLANE_C_OFFSET)));
    DRV_DEBUG("\t DSPSURFLIVE(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, DSPSURFLIVE(I965_PLANE_C_OFFSET)));

    DRV_DEBUG("\t CURCNTR(I965_PLANE_C):0x%08X\n", gpu_read_reg(gpu_regs, CURCNTR(I965_PLANE_C_OFFSET)));

    // Other registers
    DRV_DEBUG("\n");
    DRV_DEBUG("\t _OACONTROL:0x%08X\n", gpu_read_reg(gpu_regs, _OACONTROL));
    DRV_DEBUG("\t _FBC_CONTROL:0x%08X\n", gpu_read_reg(gpu_regs, _FBC_CONTROL));
    DRV_DEBUG("\t _VGACNTRL:0x%08X\n", gpu_read_reg(gpu_regs, _VGACNTRL));
}