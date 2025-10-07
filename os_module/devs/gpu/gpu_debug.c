// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>
#include <device/pci.h>
#include <device/gpu/gpu.h>
#include <device/gpu/gpu_debug.h>

#define INGTT(gvaddr) (((volatile uint32_t *)ggtt_vaddr)[(gvaddr) >> PAGE_SHIFT_4K])
typedef unsigned long gpu_vaddr_t;

static void _ggtt_update_mapping(void* gpu_regs, gpu_vaddr_t gvaddr, paddr_t spaddr)
{
	vaddr_t ggtt_vaddr = (vaddr_t)gpu_regs + MB(2);
	uint32_t flags = 0;

	// Modify GGTT PTE
	gvaddr &= PAGE_MASK_4K;
	spaddr &= PAGE_MASK_4K;

	flags = INGTT(gvaddr) & PAGE_FLAGS_MASK_4K;

	INGTT(gvaddr) = (spaddr & PAGE_MASK_4K) | flags;
}

static vaddr_t fb_vaddr = INVALID_VADDR;
OSSTATUS do_CMD_IOSM_FB_TEST_ON(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    uint32_t fb_size = MB(2);
    uint32_t fb_pages = fb_size >> 12;
    void* gpu_regs = NULL;
    uint32_t gpu_regs_mem_size = MB(4);

    UNUSED(cmd_packet);
    DRV_INFO("do_CMD_IOSM_FB_TEST_ON\n");
    gpu_regs = os->mm_ioremap_nocache(0xd0000000, gpu_regs_mem_size);
    if(!gpu_regs)
    {
        DRV_ERR("do_CMD_IOSM_FB_TEST_ON: Cannot access GPU!\n");
        status = E_INTERNAL_ERROR;
        goto out;
    }

    // Allocate frame buffer
    fb_vaddr = os->mm_alloc_physical_contiguous_pages_nocache(fb_pages, ALLOC_DMA_MEMORY);
    if(fb_vaddr == INVALID_VADDR)
    {
        DRV_ERR("do_CMD_IOSM_FB_TEST_ON: Cannot allocate framebuffer!\n");
        status = E_INTERNAL_ERROR;
        goto out;
    }

    os->mm_memset((void*)fb_vaddr, 0, fb_size);

    // Mapping in GGTT
    {
        uint32_t i = 0;
        for(i = 0; i < fb_pages; i++)
        {
            gpu_vaddr_t gvaddr = 0x02000000 + i*0x1000;
            paddr_t paddr = os->mm_vaddr_to_paddr(fb_vaddr + i * 0x1000);
            _ggtt_update_mapping(gpu_regs, gvaddr, paddr);
        }
    }

    // Write registers
    gpu_write_reg((vaddr_t)gpu_regs, _DVSACNTR, 0x84000000);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSALINOFF, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASTRIDE, 0xC80);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSAPOS, 0x14000A);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASIZE, 0x2580320);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASURF, 0x02001000);

    // Return Success
    status = OS_SUCCESS;
    
    // flush cache
    os->wbinvd();


    os->timer_msleep(3000);
    os->mm_memset((void*)fb_vaddr, 0xFF, fb_size);
    os->wbinvd();

out:
    if(gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_mem_size);

    return status;
}

OSSTATUS do_CMD_IOSM_FB_TEST_OFF(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    uint32_t fb_size = MB(2);
    uint32_t fb_pages = fb_size >> 12;
    void* gpu_regs = NULL;
    uint32_t gpu_regs_mem_size = MB(4);

    UNUSED(cmd_packet);
    DRV_INFO("do_CMD_IOSM_FB_TEST_OFF\n");
    gpu_regs = os->mm_ioremap_nocache(0xd0000000, gpu_regs_mem_size);
    if(!gpu_regs)
    {
        DRV_ERR("do_CMD_IOSM_FB_TEST_ON: Cannot access GPU!\n");
        status = E_INTERNAL_ERROR;
        goto out;
    }

    if(fb_vaddr != INVALID_VADDR)
        os->mm_free_physical_contiguous_pages(fb_vaddr, fb_pages, ALLOC_DMA_MEMORY);

    // Write registers
    gpu_write_reg((vaddr_t)gpu_regs, _DVSACNTR, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSALINOFF, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASTRIDE, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSAPOS, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASIZE, 0);
    gpu_write_reg((vaddr_t)gpu_regs, _DVSASURF, 0);

    // Return Success
    status = OS_SUCCESS;


out:
    if(gpu_regs)
        os->mm_iounmap(gpu_regs, gpu_regs_mem_size);

    return status;
}