// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h> //fprintf, printf
#include <string.h>
#include <pthread.h>
#include <drivers/gpu/gpu_config.h>

#include "elf.h"

static void apploader_deallocate_memory(struct sp_resc *resc, SP_USTRUCT *params);

// It is required to indicate the SecProcess entries.
struct sp_user_params;

static int alloc_pal_DMA(struct sp_resc *resc)
{
    vaddr_t vaddr = INVALID_VADDR;

    vaddr = app_mm_alloc_and_lock_4Kpages_for_sdom(resc->dma_size >> PAGE_SHIFT_4K, &resc->_sp_dma_pool_info);
    if (vaddr == INVALID_VADDR)
    {
        APP_ERR("Allocate DMA memory error!\n");
        return -1;
    }

    resc->sp_dma_pool = vaddr;
    return 0;
}

static vaddr_t app_mm_alloc_pal_sharemem(struct sp_resc *resc, size_t size)
{
    resc->sp_sharemem = app_mm_alloc_and_lock_4Kpages_for_sdom(size >> PAGE_SHIFT_4K, &resc->_sp_sharemem_info);

    return resc->sp_sharemem;
}

static vaddr_t app_mm_alloc_pal_heap(struct sp_resc *resc, size_t size)
{
    resc->sp_heap = app_mm_alloc_and_lock_4Kpages_for_sdom(size >> PAGE_SHIFT_4K, &resc->_sp_heap_info);

    return resc->sp_heap;
}

static vaddr_t app_mm_alloc_sp_app_comm(struct sp_resc *resc, size_t size)
{
    resc->sp_app_comm = app_mm_alloc_and_lock_4Kpages_for_sdom(size >> PAGE_SHIFT_4K, &resc->_sp_app_comm_info);

    return resc->sp_app_comm;
}

static vaddr_t app_mm_alloc_pal_params(struct sp_resc *resc, size_t size)
{
    resc->sp_params = app_mm_alloc_and_lock_4Kpages_for_sdom(size >> PAGE_SHIFT_4K, &resc->_sp_params_info);

    return resc->sp_params;
}

static vaddr_t app_mm_alloc_pal_stack(struct sp_resc *resc, size_t size)
{
    resc->sp_stack = app_mm_alloc_and_lock_4Kpages_for_sdom(size >> PAGE_SHIFT_4K, &resc->_sp_stack_info);

    return resc->sp_stack;
}

static void app_mm_destroy_pal_stack(struct sp_resc *resc)
{
    if (resc->sp_stack != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_stack_info);
        resc->sp_stack = INVALID_VADDR;
    }
}

static void app_mm_destroy_pal_params(struct sp_resc *resc)
{
    if (resc->sp_params != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_params_info);
        resc->sp_params = INVALID_VADDR;
    }
}

static void app_mm_destroy_pal_sharemem(struct sp_resc *resc)
{
    if (resc->sp_sharemem != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_sharemem_info);
        resc->sp_sharemem = INVALID_VADDR;
    }
}

static void app_mm_destroy_pal_heap(struct sp_resc *resc)
{
    if (resc->sp_heap != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_heap_info);
        resc->sp_heap = INVALID_VADDR;
    }
}

static void app_mm_destroy_sp_app_comm(struct sp_resc *resc)
{
    if (resc->sp_app_comm != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_app_comm_info);
        resc->sp_app_comm = INVALID_VADDR;
    }
}

static int destroy_pal_dma(struct sp_resc *resc)
{
    // vaddr_t sp_dma_pool = resc->sp_dma_pool;

    // // invoke kernel module to free phys mem
    // if (sp_dma_pool != INVALID_VADDR) {
    // 	if (secagent_free_dma_pool(sp_dma_pool, resc->dma_size)) {
    // 		APP_ERR("Free DMA Memory:0x%08X Error!\n", (uint32_t)sp_dma_pool);
    // 		return -1;
    // 	}

    // 	resc->sp_dma_pool = INVALID_VADDR;
    // }

    if (resc->sp_dma_pool != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_dma_pool_info);
        resc->sp_dma_pool = INVALID_VADDR;
    }

    return 0;
}

static void _copy_addrs(struct sp_scope_addrs *reg_addrs, struct sp_addrs_info *sp_addrs)
{
    reg_addrs->sp_scode_start = sp_addrs->sp_scode_start; // For SecProcess code
    reg_addrs->sp_scode_end = sp_addrs->sp_scode_end;
    reg_addrs->sp_scode_dest_gpa = sp_addrs->sp_scode_dest_gpa;

    reg_addrs->sp_sdata_start = sp_addrs->sp_sdata_start; // For SecProcess data
    reg_addrs->sp_sdata_end = sp_addrs->sp_sdata_end;
    reg_addrs->sp_sdata_dest_gpa = sp_addrs->sp_sdata_dest_gpa;

    reg_addrs->sp_rodata_start = sp_addrs->sp_rodata_start; // For SecProcess rodata
    reg_addrs->sp_rodata_end = sp_addrs->sp_rodata_end;
    reg_addrs->sp_rodata_dest_gpa = sp_addrs->sp_rodata_dest_gpa;

    // SecProcess params
    reg_addrs->sp_params_start = sp_addrs->sp_params_start; // For SecProcess params
    reg_addrs->sp_params_end = sp_addrs->sp_params_end;
    reg_addrs->sp_params_dest_gpa = sp_addrs->sp_params_dest_gpa;

    // SecProcess stack
    reg_addrs->sp_stack_start = sp_addrs->sp_stack_start; // For SecProcess stack
    reg_addrs->sp_stack_end = sp_addrs->sp_stack_end;
    reg_addrs->sp_stack_dest_gpa = sp_addrs->sp_stack_dest_gpa;

    // SecProcess memory
    reg_addrs->sp_mempool_start = sp_addrs->sp_mempool_start; // For SecProcess mem-pool
    reg_addrs->sp_mempool_end = sp_addrs->sp_mempool_end;
    reg_addrs->sp_mempool_dest_gpa = sp_addrs->sp_mempool_dest_gpa;

    // SecProcess-App communication channel
    reg_addrs->sp_comm_start = sp_addrs->sp_comm_start;
    reg_addrs->sp_comm_end = sp_addrs->sp_comm_end;
    reg_addrs->sp_comm_dest_gpa = sp_addrs->sp_comm_dest_gpa;

    // SecProcess shared memory
    reg_addrs->sp_sharemem_start = sp_addrs->sp_sharemem_start;
    reg_addrs->sp_sharemem_end = sp_addrs->sp_sharemem_end;
    reg_addrs->sp_sharemem_dest_gpa = sp_addrs->sp_sharemem_dest_gpa;

    // SecProcess DMA
    reg_addrs->sp_dmapool_start = sp_addrs->sp_dmapool_start;
    reg_addrs->sp_dmapool_end = sp_addrs->sp_dmapool_end;
    reg_addrs->sp_dmapool_dest_gpa = sp_addrs->sp_dmapool_dest_gpa;
}

//! \brief Register the SecProcess memory in the SecBase
//!
//! SecBase assigns SecProcess's memory from the non-secure IEE to the secure IEE
static int _apploader_register_sp_mem(struct sp_scope_addrs *pal_addrs)
{
    struct hv_reg_sections scode_info;
    int ret = 0;
    uint8_t os_already_muted = (uint8_t)false;

    // Fill the information of memory sections
    sp_reg_sections(&scode_info, pal_addrs);

    // Mute the untrusted OS because the sound glitches when registering/running the SecApp.
    ret = os_sound_mute(_apploader_register_sp_mem, &os_already_muted);
    if (ret)
    {
        APP_WARN("Register SecApp memory error: Untrusted OS mute error! status:%d\n", ret);
    }

    // Assign SecProcess memory to the secure IEE (for SecOS/SecProcesss)
    ret = secagent_register_sp_mem(&scode_info);
    if (ret)
    {
        APP_ERR("Register SecProcess memory error!\n");
        ret = 1;
        goto register_mem_err;
    }

    // Unmute the untrusted OS after the function finishes.
    ret = os_sound_unmute(_apploader_register_sp_mem, os_already_muted);
    if (ret)
    {
        APP_WARN("Register SecApp memory error: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    return 0;

register_mem_err:
    (void)os_sound_unmute(_apploader_register_sp_mem, os_already_muted);
    return ret;
}

static int _apploader_allocate_id(SP_USTRUCT *params)
{
    spid_t sp_id = INVALID_SP_ID;

    sp_id = secagent_allocate_sp_id();
    if (sp_id == INVALID_SP_ID)
        return 1;

    params->sp_id = sp_id;
    params->app_pid = app_get_current_procid();

    return 0;
}

static void _apploader_deallocate_id(SP_USTRUCT *params)
{
    secagent_deallocate_sp_id(params->sp_id);
}

// Load the SecOS image into memory, return its vaddr and the memory size.
static int apploader_load_sp_image(SP_USTRUCT *params, const char *sp_filepath, struct sp_resc *resc)
{
    bool ret = true;
    struct sp_addrs_info *sp_addrs = &params->sp_addrs;
    uint8_t *sp_buf = NULL;
    struct elfhdr *elf = {0};
    struct shdr *shdr = NULL;
    struct shdr *sh_strtab = NULL;
    uint8_t *sh_strtab_p = NULL;
    uint32_t shnum = 0;
    size_t sp_filesz = 0;
    uint32_t i = 0;

    ret = file_get_size(sp_filepath, &sp_filesz);
    if (!ret)
    {
        APP_ERR("Load SecProcess image error: Failed to read the image size!\n");
        return -1;
    }

    sp_buf = (uint8_t *)app_mm_alloc(sp_filesz);
    ret = file_read(sp_filepath, sp_buf, 0, sp_filesz);
    if (!ret)
    {
        APP_ERR("Load SecProcess image error: Read SecProcess image error!\n");
        goto err;
    }

    elf = (struct elfhdr *)sp_buf;
    // Is this an ELF executable?
    if (elf->magic != ELF_MAGIC)
    {
        printf("Load SecProcess image error: Invalid SecProcess image!\n");
        goto err;
    }

    // Traverse the sections
    shdr = (struct shdr *)((vaddr_t)sp_buf + elf->shoff);
    sh_strtab = &shdr[elf->shstrndx];
    sh_strtab_p = sp_buf + sh_strtab->sh_offset;
    shnum = elf->shnum;
    for (i = 0; i < shnum; i++)
    {
        vaddr_t section = INVALID_VADDR;
        size_t section_size = shdr[i].sh_size;
        char *section_name = (char *)(sh_strtab_p + shdr[i].sh_name);
        uint32_t num_pages = (SIZE_ROUNDUP_4K(section_size)) >> PAGE_SHIFT_4K;

        // Register SecProcess code, data and rodata
        if (!strcmp(section_name, ".text"))
        {
            // .text
            section = app_mm_alloc_and_lock_4Kpages_for_sdom(num_pages, &resc->_sp_scode_info);
            if (!section)
            {
                printf("Load SecProcess image error: Allocate memory for SecProcess code section error!\n");
                goto err;
            }

            app_mm_memcpy((void *)section, &sp_buf[shdr[i].sh_offset], section_size);

            sp_addrs->sp_scode_start = section;
            sp_addrs->sp_scode_end = section + section_size;
            sp_addrs->sp_scode_dest_gpa = secagent_try_alloc_sos_phy_mem(section_size);
            sp_addrs->sp_scode_dest_gva = shdr[i].sh_addr;
        }
        else if (!strcmp(section_name, ".data"))
        {
            // .data
            section = app_mm_alloc_and_lock_4Kpages_for_sdom(num_pages, &resc->_sp_sdata_info);
            if (!section)
            {
                printf("Load SecProcess image error: Allocate memory for SecProcess data section error!\n");
                goto err;
            }

            app_mm_memcpy((void *)section, &sp_buf[shdr[i].sh_offset], section_size);

            sp_addrs->sp_sdata_start = section;
            sp_addrs->sp_sdata_end = section + section_size;
            sp_addrs->sp_sdata_dest_gpa = secagent_try_alloc_sos_phy_mem(section_size);
            sp_addrs->sp_sdata_dest_gva = shdr[i].sh_addr;
        }
        else if (!strcmp(section_name, ".rodata"))
        {
            // .rodata
            section = app_mm_alloc_and_lock_4Kpages_for_sdom(num_pages, &resc->_sp_rodata_info);
            if (!section)
            {
                printf("Load SecProcess image error: Allocate memory for SecProcess rodata section error!\n");
                goto err;
            }

            app_mm_memcpy((void *)section, &sp_buf[shdr[i].sh_offset], section_size);

            sp_addrs->sp_rodata_start = section;
            sp_addrs->sp_rodata_end = section + section_size;
            sp_addrs->sp_rodata_dest_gpa = secagent_try_alloc_sos_phy_mem(section_size);
            sp_addrs->sp_rodata_dest_gva = shdr[i].sh_addr;
        }
    }

    app_mm_free((vaddr_t)sp_buf);
    APP_INFO("sp_scode_dest_gva:%08X, sp_sdata_dest_gva:%08X, sp_rodata_dest_gva:%08X\n",
             (u32)sp_addrs->sp_scode_dest_gva, (u32)sp_addrs->sp_sdata_dest_gva, (u32)sp_addrs->sp_rodata_dest_gva);
    return 0;

err:
    if (sp_buf)
        app_mm_free((vaddr_t)sp_buf);
    return -1;
}

extern void hypercall_test2(ulong_t arg1);
static int apploader_allocate_memory(struct sp_resc *resc, SP_USTRUCT *params)
{
    int ret = 0;
    struct sp_addrs_info *sp_addrs = &params->sp_addrs;
    vaddr_t mem_start_va = 0, mem_end_va = 0;
    vaddr_t pal_params_start_va = 0, pal_params_end_va = 0;
    vaddr_t pal_stack_start_va = 0, pal_stack_end_va = 0;
    vaddr_t shared_mem_start_va = 0, shared_mem_end_va = 0;
    vaddr_t comm_start_va = 0, comm_end_va = 0;
    vaddr_t last_sec_dest_gva = 0, last_sec_start = 0, last_sec_end = 0;

    // SecProcess code and data
    ret = apploader_load_sp_image(params, resc->filepath, resc);
    if (ret)
    {
        APP_ERR("Load SecProcess image error!\n");
        goto load_sp_image_err;
    }

    last_sec_dest_gva = sp_addrs->sp_rodata_dest_gva;
    last_sec_start = sp_addrs->sp_rodata_start;
    last_sec_end = sp_addrs->sp_rodata_end;

    // SecProcess params
    pal_params_start_va = app_mm_alloc_pal_params(resc, SAP_PARAMS_SIZE);
    if (pal_params_start_va == INVALID_VADDR)
    {
        APP_ERR("Allocate SecProcess's stack error!\n");
        goto stack_alloc_err;
    }
    pal_params_end_va = pal_params_start_va + SAP_PARAMS_SIZE;
    sp_addrs->sp_params_start = pal_params_start_va;
    sp_addrs->sp_params_end = pal_params_end_va;
    sp_addrs->sp_params_dest_gpa = secagent_try_alloc_sos_phy_mem(SAP_PARAMS_SIZE);
    sp_addrs->sp_params_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
    last_sec_dest_gva = sp_addrs->sp_params_dest_gva;
    last_sec_start = sp_addrs->sp_params_start;
    last_sec_end = sp_addrs->sp_params_end;

    // SecProcess stack
    last_sec_end += PAGE_SIZE_4K; // Add a 4KB page before and after the SecProcess's stack in its virtual address space
                                  // in S domain to detect stack overflow and underflow.
    pal_stack_start_va = app_mm_alloc_pal_stack(resc, SAP_STACK_SIZE);
    if (pal_stack_start_va == INVALID_VADDR)
    {
        APP_ERR("Allocate SecProcess's stack error!\n");
        goto stack_alloc_err;
    }
    pal_stack_end_va = pal_stack_start_va + SAP_STACK_SIZE;
    sp_addrs->sp_stack_start = pal_stack_start_va;
    sp_addrs->sp_stack_end = pal_stack_end_va;
    sp_addrs->sp_stack_dest_gpa = secagent_try_alloc_sos_phy_mem(SAP_STACK_SIZE);
    sp_addrs->sp_stack_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
    last_sec_dest_gva = sp_addrs->sp_stack_dest_gva;
    last_sec_start = sp_addrs->sp_stack_start;
    last_sec_end = sp_addrs->sp_stack_end;
    last_sec_end += PAGE_SIZE_4K; // Add a 4KB page before and after the SecProcess's stack in its virtual address space
                                  // in S domain to detect stack overflow and underflow.

    // SecProcess Mem Area
    mem_start_va = app_mm_alloc_pal_heap(resc, resc->heap_size);
    if (mem_start_va == INVALID_VADDR)
    {
        APP_ERR("Allocate SecProcess's heap error!\n");
        goto heap_alloc_err;
    }
    mem_end_va = mem_start_va + resc->heap_size;
    sp_addrs->sp_mempool_start = mem_start_va;
    sp_addrs->sp_mempool_end = mem_end_va;
    sp_addrs->sp_mempool_dest_gpa = secagent_try_alloc_sos_phy_mem(resc->heap_size);
    sp_addrs->sp_mempool_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
    last_sec_dest_gva = sp_addrs->sp_mempool_dest_gva;
    last_sec_start = sp_addrs->sp_mempool_start;
    last_sec_end = sp_addrs->sp_mempool_end;

    // SecProcess - App communication channel
    // [NOTE] Untrusted OS/Apps should access this memory region as uncached, because it is shared with the SecProcess.
    if (resc->alloc_sp_app_comm)
    {
        comm_start_va = app_mm_alloc_sp_app_comm(resc, SAP_APP_COMM_SIZE);
        if (comm_start_va == INVALID_VADDR)
        {
            APP_ERR("Allocate SecProcess's SecProcess-App communication channel error!\n");
            goto comm_alloc_err;
        }
        comm_end_va = comm_start_va + SAP_APP_COMM_SIZE;
        sp_addrs->sp_comm_start = comm_start_va;
        sp_addrs->sp_comm_end = comm_end_va;
        sp_addrs->sp_comm_dest_gpa = secagent_try_alloc_sos_phy_mem(SAP_APP_COMM_SIZE);
        sp_addrs->sp_comm_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
        last_sec_dest_gva = sp_addrs->sp_comm_dest_gva;
        last_sec_start = sp_addrs->sp_comm_start;
        last_sec_end = sp_addrs->sp_comm_end;
    }
    else
    {
        sp_addrs->sp_comm_start = INVALID_VADDR;
        sp_addrs->sp_comm_end = INVALID_VADDR;
        sp_addrs->sp_comm_dest_gpa = INVALID_PADDR;
        sp_addrs->sp_comm_dest_gva = INVALID_VADDR;
    }

    // SecProcess Shared Mem Area
    // [NOTE] Untrusted OS/Apps should access this memory region as uncached, because it is shared with the SecProcess.
    if (resc->shared_mem_size > 0)
    {
        shared_mem_start_va = app_mm_alloc_pal_sharemem(resc, resc->shared_mem_size);
        if (shared_mem_start_va == INVALID_VADDR)
        {
            APP_ERR("Allocate SecProcess's shared memory error!\n");
            goto shared_mem_alloc_err;
        }
        shared_mem_end_va = shared_mem_start_va + resc->shared_mem_size;
        sp_addrs->sp_sharemem_start = shared_mem_start_va;
        sp_addrs->sp_sharemem_end = shared_mem_end_va;
        sp_addrs->sp_sharemem_dest_gpa = secagent_try_alloc_sos_phy_mem(resc->shared_mem_size);
        sp_addrs->sp_sharemem_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
        last_sec_dest_gva = sp_addrs->sp_sharemem_dest_gva;
        last_sec_start = sp_addrs->sp_sharemem_start;
        last_sec_end = sp_addrs->sp_sharemem_end;
    }
    else
    {
        sp_addrs->sp_sharemem_start = INVALID_VADDR;
        sp_addrs->sp_sharemem_end = INVALID_VADDR;
        sp_addrs->sp_sharemem_dest_gpa = INVALID_PADDR;
        sp_addrs->sp_sharemem_dest_gva = INVALID_VADDR;
    }

    // Register SecProcess DMA Area
    if (resc->dma_size > 0)
    {
        ret = alloc_pal_DMA(resc);
        if (ret)
        {
            APP_ERR("Allocate SecProcess's DMA memory error!\n");
            goto dma_alloc_err;
        }

        sp_addrs->sp_dmapool_start = resc->sp_dma_pool;
        sp_addrs->sp_dmapool_end = resc->sp_dma_pool + resc->dma_size;
        sp_addrs->sp_dmapool_dest_gpa = secagent_try_alloc_sos_phy_mem(resc->dma_size);
        sp_addrs->sp_dmapool_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);
        last_sec_dest_gva = sp_addrs->sp_dmapool_dest_gva;
        last_sec_start = sp_addrs->sp_dmapool_start;
        last_sec_end = sp_addrs->sp_dmapool_end;
    }
    else
    {
        sp_addrs->sp_dmapool_start = INVALID_VADDR;
        sp_addrs->sp_dmapool_end = INVALID_VADDR;
        sp_addrs->sp_dmapool_dest_gpa = INVALID_PADDR;
        sp_addrs->sp_dmapool_dest_gva = INVALID_VADDR;
    }

    // Register SecProcess Remappable Virtual Memory Region
    sp_addrs->sp_remappable_dest_gva = last_sec_dest_gva + (last_sec_end - last_sec_start);

    APP_DEBUG("Stack address:%#X, Heap address: %#X, Shared mem address:%#X\n",
              (uint32_t)pal_stack_start_va, (uint32_t)mem_start_va, (uint32_t)shared_mem_start_va);
    return 0;

dma_alloc_err:
shared_mem_alloc_err:
comm_alloc_err:
heap_alloc_err:
stack_alloc_err:
load_sp_image_err:
    apploader_deallocate_memory(resc, params);
    return -1;
}

//! Perform additional initialization to the trusted devices assigned to the SecProcess
// static int apploader_init_devices(SP_USTRUCT* params)
// {
// 	/*struct sp_addrs_info * sp_addrs = &params->sp_addrs;
// 	int status = 0;

// 	// Register GPU Overlay region
// 	if(params->sp_type == SP_APPLICATION)
// 	{
// 		SAP_PROC_PARAMS* sp_proc_params = (SAP_PROC_PARAMS*)&((SP_PROC_PARAMS*)params->sp_exec_params_secure)->sp_params;
// 		if(sp_proc_params->need_tdisp)
// 		{
// 			struct td_conf* td_conf = &sp_proc_params->tp_devs[0].dev_conf.td_conf;
// 			vaddr_t td_mem_base = INVALID_VADDR;
// 			size_t td_mem_size = 0;

// 			status = secagent_init_trusted_display(
// 				td_conf->disp_width,
// 				td_conf->disp_height,
// 				&td_mem_base,
// 				&td_mem_size
// 			);
// 			if(status)
// 				return status;

// 			// [TODO][Ticket88] Need to think about what information should be
// 			// exposed to SecProcesss. (With such design, we can only display one
// 			// SecProcess at a time)
// 			sp_addrs->overlay_start = td_mem_base;
// 			sp_addrs->overlay_end = sp_addrs->overlay_start + td_mem_size;
// 			sp_addrs->overlay_dest_gpa = secagent_try_alloc_sos_phy_mem(td_mem_size);
// 			sp_addrs->overlay_dest_gva = 0xA0000000;

// 			td_conf->pipe_vaddr = sp_addrs->overlay_dest_gva;
// 			td_conf->pipe_size = td_mem_size;

// 			APP_DEBUG("Overlay vaddr:%#X\n", (uint32_t)sp_addrs->overlay_start);
// 		}
// 	} */

// 	return 0;
// }

static int apploader_fini_devices(SP_USTRUCT *params)
{
    /*int status = 0;

    if(params->sp_type == SP_APPLICATION)
    {
        SAP_PROC_PARAMS* sp_proc_params = (SAP_PROC_PARAMS*)&((SP_PROC_PARAMS*)params->sp_exec_params_secure)->sp_params;
        if(sp_proc_params->need_tdisp)
        {
            struct sp_addrs_info * sp_addrs = &params->sp_addrs;

            secagent_try_free_sos_phy_mem(sp_addrs->overlay_dest_gpa);
            status = secagent_fini_trusted_display();
            if(status)
                return status;
        }
    } */

    return 0;
}

static void apploader_deallocate_memory(struct sp_resc *resc, SP_USTRUCT *params)
{
    struct sp_addrs_info *sp_addrs = &params->sp_addrs;

    if (sp_addrs->sp_scode_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_scode_info);
        sp_addrs->sp_scode_start = INVALID_VADDR;
        secagent_try_free_sos_phy_mem(sp_addrs->sp_scode_dest_gpa);
    }

    if (sp_addrs->sp_sdata_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_sdata_info);
        sp_addrs->sp_sdata_start = INVALID_VADDR;
        secagent_try_free_sos_phy_mem(sp_addrs->sp_sdata_dest_gpa);
    }

    if (sp_addrs->sp_rodata_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&resc->_sp_rodata_info);
        sp_addrs->sp_rodata_start = INVALID_VADDR;
        secagent_try_free_sos_phy_mem(sp_addrs->sp_rodata_dest_gpa);
    }

    // Release the occupied SecOS physical memory region registered in SecOS's memory provisioning in the SecAgent
    if (sp_addrs->sp_stack_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_stack_dest_gpa);

    if (sp_addrs->sp_params_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_params_dest_gpa);

    if (sp_addrs->sp_mempool_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_mempool_dest_gpa);

    if (sp_addrs->sp_comm_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_comm_dest_gpa);

    if (sp_addrs->sp_sharemem_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_sharemem_dest_gpa);

    if (sp_addrs->sp_dmapool_start != INVALID_VADDR)
        secagent_try_free_sos_phy_mem(sp_addrs->sp_dmapool_dest_gpa);

    app_mm_destroy_pal_stack(resc);
    app_mm_destroy_pal_params(resc);
    destroy_pal_dma(resc);
    app_mm_destroy_pal_heap(resc);
    app_mm_destroy_sp_app_comm(resc);
    app_mm_destroy_pal_sharemem(resc);
}

static int _sl_load_sp_signature(SP_USTRUCT *params, const char *sp_signature_relative_filepath)
{
    bool ret = false;
    char sp_signature_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};

    // Check: Parameters must be valid
    if (!params || !sp_signature_relative_filepath)
        return -1;
    strncpy(sp_signature_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(sp_signature_file_path, sp_signature_relative_filepath);

    // Load SecProcess signature
    ret = file_read(sp_signature_file_path, (void *)params->sp_signature, 0, SECPROCESS_SIGNATURE_LENGTH);
    if (!ret)
    {
        return -1;
    }
    params->sp_signature_size = SECPROCESS_SIGNATURE_LENGTH;

    // On success
    return 0;
}

/*********** Public Functions **********/
int sp_loader_log_init(const char* in_log_filepath)
{
    return app_log_file_init(in_log_filepath);
}

void sp_loader_log_fini(void)
{
    app_log_file_fini();
}

int sp_loader_init(void)
{
    // Init SecProcess state manager
    sp_states_init();

    // Init SecProcess-App communication channel manager
    comm_app_sp_mgr_init();

    // On success
    return 0;
}

void sp_loader_fini(void)
{
    comm_app_sp_mgr_fini();
}

int apploader_register_secapp(SP_USTRUCT *params)
{
    int ret = 0;
    OSSTATUS status = OS_SUCCESS;
    uint8_t os_already_muted = (uint8_t)false;
    struct sp_scope_addrs reg_addrs = {0};

    // Check: Parameters must be valid
    if (!params)
        return 1;

    // Copy the information from params->sp_addrs to <reg_addrs>
    _copy_addrs(&reg_addrs, &params->sp_addrs);

    // Mute the untrusted OS because the sound glitches when registering/running the SecApp.
    ret = os_sound_mute(apploader_register_secapp, &os_already_muted);
    if (ret)
    {
        APP_WARN("Register SecApp error: Untrusted OS mute error! status:%d\n", ret);
    }

    // Register the SecProcess in SecBase
    ret = _apploader_register_sp_mem(&reg_addrs);
    if (ret)
    {
        APP_ERR("Call SecAgent to register SecProcess memory error! status:%d\n", ret);
        // Note: the caller will call apploader_unregister_secapp() to clean up
        ret = 1;
        goto register_sp_mem_err;
    }

    // Create SecProcess's shared memory
    ret = smem_mgr_init(params);
    if (ret)
    {
        APP_ERR("Register SecApp error: Create shared memory error! status:%d\n", ret);
        // Note: the caller will call apploader_unregister_secapp() to clean up
        ret = 1;
        goto create_shared_mem_err;
    }

    // Enable App-SecProcess communication
    ret = comm_app_sp_create(params);
    if (ret)
    {
        APP_ERR("Register SecApp error: init App-SecProcess communication error! status:%d\n", ret);
        // Note: the caller will call apploader_unregister_secapp() to clean up
        ret = 1;
        goto comm_init_err;
    }

    // Register the SecProcess in SecOS
    status = secagent_register_sp(params);
    if (SA_FAIL(status))
    {
        APP_ERR("Call SecAgent to register SecProcess error! status:%lu\n", status);
        // Note: the caller will call apploader_unregister_secapp() to clean up
        ret = 1;
        goto register_sp_err;
    }

    // Unmute the untrusted OS after the function finishes.
    ret = os_sound_unmute(apploader_register_secapp, os_already_muted);
    if (ret)
    {
        APP_WARN("Register SecApp error: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    return 0;

register_sp_err:
comm_init_err:
create_shared_mem_err:
register_sp_mem_err:
    // Unmute the untrusted OS on SecApp registration/run errors.
    (void)os_sound_unmute(apploader_register_secapp, os_already_muted);
    return ret;
}

int apploader_unregister_secapp(SP_USTRUCT *params)
{
    int ret = 0;
    struct sp_scope_addrs reg_addrs = {0};

    // Check: Parameters must be valid
    if (!params)
        return 1;

    // Finalize the SecProcess-App communication channel.
    ret = comm_app_sp_destroy(params);
    if (ret)
        return 1;

    // Finalize the shared memory manager.
    smem_mgr_fini(params);

    _copy_addrs(&reg_addrs, &params->sp_addrs);

    // On success
    return 0;
}

int apploader_run_sp(spid_t sp_id)
{
    int ret = 0;
    OSSTATUS status = OS_SUCCESS;
    uint8_t os_already_muted = (uint8_t)false;

    // Mute the untrusted OS because the sound glitches when registering/running the SecApp.
    ret = os_sound_mute(apploader_run_sp, &os_already_muted);
    if (ret)
    {
        APP_WARN("Run SecApp error: Untrusted OS mute error! status:%d\n", ret);
    }

    status = secagent_run_sp(sp_id);
    if (SA_FAIL(status))
    {
        APP_ERR("Call SecAgent to run SecProcess %u error! status:%lu\n", sp_id, status);
        ret = 1;
        goto run_sp_err;
    }

    // Unmute the untrusted OS after the function finishes.
    ret = os_sound_unmute(apploader_run_sp, os_already_muted);
    if (ret)
    {
        APP_WARN("Run SecApp error: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    return 0;

run_sp_err:
    // Unmute the untrusted OS on SecApp registration/run errors.
    (void)os_sound_unmute(apploader_run_sp, os_already_muted);
    return ret;
}

int apploader_force_terminate_sp(spid_t sp_id)
{
    int ret = 0;
    OSSTATUS status = OS_SUCCESS;
    uint8_t os_already_muted = (uint8_t)false;

    // Mute the untrusted OS because the sound glitches when registering/running the SecApp.
    ret = os_sound_mute(apploader_force_terminate_sp, &os_already_muted);
    if (ret)
    {
        APP_WARN("Force terminate SecApp error: Untrusted OS mute error! status:%d\n", ret);
    }

    status = secagent_force_terminate_sp(sp_id);
    if (SA_FAIL(status))
    {
        APP_ERR("Call SecAgent to force terminate SecProcess %u error! status:%lu\n", sp_id, status);
        ret = 1;
        goto terminate_sp_err;
    }

    // Unmute the untrusted OS after the function finishes.
    ret = os_sound_unmute(apploader_force_terminate_sp, os_already_muted);
    if (ret)
    {
        APP_WARN("Force terminate SecApp error: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    return 0;

terminate_sp_err:
    // Unmute the untrusted OS on SecApp registration/run errors.
    (void)os_sound_unmute(apploader_force_terminate_sp, os_already_muted);
    return ret;
}

int apploader_soft_terminate_sp(spid_t sp_id)
{
    int ret = 0;
    OSSTATUS status = OS_SUCCESS;
    uint8_t os_already_muted = (uint8_t)false;

    // Mute the untrusted OS because the sound glitches when registering/running the SecApp.
    ret = os_sound_mute(apploader_soft_terminate_sp, &os_already_muted);
    if (ret)
    {
        APP_WARN("Soft terminate SecApp error: Untrusted OS mute error! status:%d\n", ret);
    }

    status = secagent_soft_terminate_sp(sp_id);
    if (SA_FAIL(status))
    {
        APP_ERR("Call SecAgent to soft terminate SecProcess %u error! status:%lu\n", sp_id, status);
        ret = 1;
        goto terminate_sp_err;
    }

    // Unmute the untrusted OS after the function finishes.
    ret = os_sound_unmute(apploader_soft_terminate_sp, os_already_muted);
    if (ret)
    {
        APP_WARN("Soft terminate SecApp error: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    return 0;

terminate_sp_err:
    // Unmute the untrusted OS on SecApp registration/run errors.
    (void)os_sound_unmute(apploader_soft_terminate_sp, os_already_muted);
    return ret;
}

int prepare_secapp(struct sp_resc *resc, SP_USTRUCT *params, SP_SIGNATURE_PARAM *sp_signature_info)
{
    int ret = 0;

    // Check: Parameters must be valid
    if (!resc || !params || !sp_signature_info)
        return -1;

    // ID information
    ret = _apploader_allocate_id(params);
    if (ret)
    {
        APP_ERR("<_apploader_allocate_id> error!\n");
        return ret;
    }

    // Memory information
    ret = apploader_allocate_memory(resc, params);
    if (ret)
    {
        APP_ERR("<apploader_allocate_memory> error!\n");
        return ret;
    }

    // Load SecProcess's signature if there is one
    params->sp_has_1stparty_signature = sp_signature_info->has_1stparty_signature;
    ret = _sl_load_sp_signature(params, sp_signature_info->sp_signature_relative_filepath);
    if (ret)
    {
        // [NOTE] This is not an error because SecApps may not signed by us.
        APP_INFO("SecProcess does not have a signature\n");
    }

    // Information of trusted devices
    // ret = apploader_init_devices(params);
    // if(ret)
    // {
    // 	APP_ERR("<apploader_init_devices> error!\n");
    // 	return ret;
    // }

    // On success
    return 0;
}

void fini_secapp(struct sp_resc *resc, SP_USTRUCT *params)
{
    apploader_fini_devices(params);
    apploader_deallocate_memory(resc, params);
    _apploader_deallocate_id(params);
}

int apploader_register_sp_mem_one(vaddr_t vaddr, size_t size, enum tv_pal_section_type type, paddr_t *out_sp_paddr)
{
    int ret = 0;
    struct hv_reg_sections scode_info = {0};
    paddr_t paddr = INVALID_PADDR;

    // Check: the parameters must be valid
    if ((vaddr == INVALID_VADDR) || (!size) || (!out_sp_paddr))
        return 1;

    paddr = secagent_try_alloc_sos_phy_mem(size);
    if (paddr == INVALID_PADDR)
    {
        APP_ERR("Register SecProcess memory for one memory section error: Insufficient physical memory address space in Secure Domain!\n");
        return 1;
    }

    // Move memory to Secure Domain
    sp_reg_section_add(&scode_info, type, vaddr, size, paddr);

    ret = secagent_register_sp_mem(&scode_info);
    if (ret)
    {
        APP_ERR("Assign memory region (vaddr:0x%lX, size:0x%lX) to Secure Domain (paddr:0x%lX) error!\n", vaddr, size, paddr);
        return 1;
    }

    // On success
    *out_sp_paddr = paddr;
    return 0;
}

/*********** Util Functions **********/
int app_mem_force_commit_in_pagetable(void *ptr, size_t len)
{
    int i;

    for (i = 0; i < len; i += PAGE_SIZE_4K)
    {
        volatile unsigned int *addr = (unsigned int *)(ptr + i);

        // Write non-zero values to each page to prevent OS mapping empty pages to the same physical memory page.
        *addr = (i + 1);
        mfence();
    }

    return 0;
}

int app_touch_mem(void *ptr, size_t len)
{
    int i;

    for (i = 0; i < len; i += PAGE_SIZE_4K)
    {
        volatile unsigned int *addr = (unsigned int *)(ptr + i);
        volatile unsigned int data = *addr;

        // Write non-zero values to each page to prevent OS mapping empty pages to the same physical memory page.
        *addr = (i + 1);
        mfence();
        *addr = data;
        mfence();

        (void)data;
        (void)addr;
    }
    return 0;
}

/*********** [TODO] Get Executable Directory  **********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WINDOWS__)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

#define MAX_PATH_OUTPUT_LEN (MAX_PATH_LEN + 1) // Need one more character to append the last "directory delimiter"

static char exePath[MAX_PATH_OUTPUT_LEN] = {0};
char *apploader_get_exe_directory()
{
    char *lastSlash;

    memset(exePath, 0, MAX_PATH_OUTPUT_LEN);

    // if (exePath == NULL) {
    //     APP_ERR("Memory allocation error");
    //     return NULL;
    // }

#if defined(_WIN32) || defined(__WINDOWS__)
    // Windows specific
    if (GetModuleFileName(NULL, exePath, MAX_PATH_LEN) == 0)
    {
        APP_ERR("GetModuleFileName failed");
        return NULL;
    }

    // Extract the directory path
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash != NULL)
    {
        *lastSlash = '\\';
        *(lastSlash + 1) = '\0';
    }
#elif defined(__APPLE__)
    uint32_t bufsize = MAX_PATH_LEN;
    if (_NSGetExecutablePath(exePath, &bufsize) != 0)
    {
        APP_ERR("_NSGetExecutablePath failed");
        return NULL;
    }

    // Extract the directory path
    lastSlash = strrchr(exePath, '/');
    if (lastSlash != NULL)
    {
        *lastSlash = '/';
        *(lastSlash + 1) = '\0';
    }
#else
    // Linux specific
    ssize_t count = readlink("/proc/self/exe", exePath, MAX_PATH_LEN);
    if (count == -1 || count >= MAX_PATH_LEN)
    {
        APP_ERR("readlink failed");
        return NULL;
    }

    // Null-terminate the string
    exePath[count] = '\0';

    // Extract the directory path
    lastSlash = strrchr(exePath, '/');
    if (lastSlash != NULL)
    {
        *lastSlash = '/';
        *(lastSlash + 1) = '\0';
    }
#endif

    return exePath;
}