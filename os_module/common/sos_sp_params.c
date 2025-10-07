// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

/// @brief Init the boot params of SecOS
static OSSTATUS _sparams_init_sos_boot_params(struct sos_boot_params *boot_params);

/// @brief Add memory sections to be registered in SecBase. These memory regions will be moved to the Secure Domain soon.
/// @param scode_info
/// @param type
/// @param start_addr
/// @param len
/// @param dest_gpaddr
static void sb_mem_sections_add(
    struct hv_reg_sections *scode_info,
    int type,
    vaddr_t start_addr, size_t len, paddr_t dest_gpaddr);
static size_t tv_ptr_diff(vaddr_t end, vaddr_t start);

//! Addresses of SecOS sections
struct sos_sect_addrs sos_addrs = {0};
//! Is SecOS registered?
//! [NOTE] SecOS cannot be re-registered before its unregistration
bool sos_is_registered = false;
bool sos_is_running = false;

static vaddr_t sos_initdata_vaddr = INVALID_VADDR;
static vaddr_t sos_sharemem_start_va = INVALID_VADDR;
static vaddr_t sos_params_start_va = INVALID_VADDR;

#ifdef __i386__

OSSTATUS sparams_build_sos_initdata(struct sos_sect_addrs *in_sos_addrs)
{
    vaddr_t init_pt_base, vaddr;

    uint32_t sos_initdata_pages = SOS_INITDATA_SIZE / PAGE_SIZE_4K;
    sos_initdata_vaddr = os->mm_alloc_pages(sos_initdata_pages, ALLOC_ORDINARY_MEMORY);
    if (sos_initdata_vaddr == INVALID_VADDR)
    {
        DRV_ERR("Allocate SecOS's init data error!\n");
        return E_NO_MEM;
    }

    // Step 1. Build the init page table used by the SecOS
    // [NOTE] The SecOS will build its own page table later, which is separated
    // from this page table. In other words, this page table is used only during
    // SecOS initialization.
    init_pt_base = sos_initdata_vaddr + SOS_LOADER_PT_BASE;
    for (vaddr = SOS_INITDATA_BASE; vaddr < SOS_MAPPABLE_SPACE_BASE; vaddr += X86_LARGE_PAGE_SIZE)
    {
        paddr_t paddr = K_VIRT_TO_PHYS(vaddr);

        ((uint32_t *)init_pt_base)[vaddr >> PD_SHIFT] = (paddr & PAGE_MASK_4K) | PTE_P | PTE_W | PTE_PS;
    }

    // [NOTE] We do not allocate the stack used by SecOS explicity, because it
    // has already been there when allocating <sos_initdata_vaddr>

    // Step 2. Register the memory region of SecOS initdata
    in_sos_addrs->sos_initdata_start = sos_initdata_vaddr;
    in_sos_addrs->sos_initdata_end = sos_initdata_vaddr + SOS_INITDATA_SIZE;
    in_sos_addrs->sos_initdata_dest_gpa = K_VIRT_TO_PHYS(SOS_INITDATA_BASE);

    return OS_SUCCESS;
}

#elif defined(__amd64__)

OSSTATUS sparams_build_sos_initdata(struct sos_sect_addrs *in_sos_addrs)
{
    vaddr_t init_pt_base;
    vaddr_t pdp_base;
    vaddr_t pd_base;
    uint64_t offset_last_1g_mem;

    uint32_t sos_initdata_pages;

    // Check: SOS_KERN_BASE must be the last 1GB of the vaddr space. Otherwise, the following setup of init page table
    // is incorrect.
    if (SOS_KERN_BASE != 0xFFFFFFFFC0000000)
    {
        DRV_ERR("Build SecOS's init page table error! SecOS must be loaded in the highest 1GB virtual address space!\n");
        return E_INTERNAL_ERROR;
    }

    sos_initdata_pages = SOS_INITDATA_SIZE / PAGE_SIZE_4K;
    sos_initdata_vaddr = os->mm_alloc_pages(sos_initdata_pages, ALLOC_ORDINARY_MEMORY);
    if (sos_initdata_vaddr == INVALID_VADDR)
    {
        DRV_ERR("Allocate SecOS's init data error!\n");
        return E_NO_MEM;
    }

    // Step 1. Build the init page table used by the SecOS
    // [NOTE] The SecOS will build its own page table later, which is separated
    // from this page table. In other words, this page table is used only during
    // SecOS initialization.
    init_pt_base = sos_initdata_vaddr + SOS_LOADER_PT_BASE;
    pdp_base = init_pt_base + PAGE_SIZE_4K;
    pd_base = pdp_base + PAGE_SIZE_4K;

    //// Construct PML4 of the init PT
    os->mm_memset((void *)init_pt_base, 0, PAGE_SIZE_4K);
    {
        paddr_t pdp_base_paddr = (paddr_t)(pdp_base - init_pt_base);
        ((ulong_t *)init_pt_base)[511] = (pdp_base_paddr & PAGE_MASK_4K) | PTE_P | PTE_W;
    }

    //// Construct PDP of the init PT
    os->mm_memset((void *)pdp_base, 0, PAGE_SIZE_4K);
    {
        paddr_t pd_base_paddr = (paddr_t)(pd_base - init_pt_base);
        ((ulong_t *)pdp_base)[511] = (pd_base_paddr & PAGE_MASK_4K) | PTE_P | PTE_W;
    }

    //// Construct PD of the init PT
    for (offset_last_1g_mem = SOS_INITDATA_BASE - SOS_KERN_BASE; offset_last_1g_mem < SOS_MAPPABLE_SPACE_BASE - SOS_KERN_BASE; offset_last_1g_mem += X86_LARGE_PAGE_SIZE)
    {
        vaddr_t vaddr = offset_last_1g_mem + SOS_KERN_BASE;
        paddr_t paddr = K_VIRT_TO_PHYS(vaddr);

        // Here we use <offset_last_1g_mem> because <pd_base> points to the page table structure for the last 1GB memory
        // space.
        ((ulong_t *)pd_base)[offset_last_1g_mem >> PD_SHIFT] = (paddr & PAGE_MASK_4K) | PTE_P | PTE_W | PTE_PS;
    }

    // [NOTE] We do not allocate the stack used by SecOS explicity, because it
    // has already been there when allocating <sos_initdata_vaddr>

    // Step 2. Register the memory region of SecOS initdata
    in_sos_addrs->sos_initdata_start = sos_initdata_vaddr;
    in_sos_addrs->sos_initdata_end = sos_initdata_vaddr + SOS_INITDATA_SIZE;
    in_sos_addrs->sos_initdata_dest_gpa = K_VIRT_TO_PHYS(SOS_INITDATA_BASE);

    return OS_SUCCESS;
}

#else /* !defined(__i386__) && !defined(__amd64__) */
#error "Unsupported Arch"
#endif /* !defined(__i386__) && !defined(__amd64__) */

OSSTATUS sparams_build_sos_sharedmem(struct sos_sect_addrs *in_sos_addrs)
{
    uint32_t sos_sharedmem_pages = VK_SHAREMEM_SIZE / PAGE_SIZE_4K;

    sos_sharemem_start_va = os->mm_alloc_pages(sos_sharedmem_pages, ALLOC_ORDINARY_MEMORY);
    if (sos_sharemem_start_va == INVALID_VADDR)
    {
        DRV_ERR("Allocate SecOS's shared memory error!\n");
        return E_NO_MEM;
    }

    in_sos_addrs->sos_sharemem_start = sos_sharemem_start_va;
    in_sos_addrs->sos_sharemem_end = sos_sharemem_start_va + VK_SHAREMEM_SIZE;
    in_sos_addrs->sos_sharemem_dest_gpa = K_VIRT_TO_PHYS(SOS_SHARED_MEM_BASE);

    return OS_SUCCESS;
}

OSSTATUS sparams_build_sos_params(struct sos_sect_addrs *in_sos_addrs)
{
    OSSTATUS status = OS_SUCCESS;
    struct sos_boot_params boot_params;
    uint32_t sos_param_pages = SOS_PARAM_PAGE_SIZE / PAGE_SIZE_4K;

    //// Fill other SecOS parameters during its initialization.
    os->mm_memset(&boot_params, 0, sizeof(struct sos_boot_params));
    status = _sparams_init_sos_boot_params(&boot_params);
    if (SA_FAIL(status))
    {
        DRV_ERR("Initialize SecOS's boot params error! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

    //// Register the memory region of SecOS parameters
    sos_params_start_va = os->mm_alloc_pages(sos_param_pages, ALLOC_ORDINARY_MEMORY);
    in_sos_addrs->param_start = sos_params_start_va;
    in_sos_addrs->param_end = in_sos_addrs->param_start + SOS_PARAM_PAGE_SIZE;
    in_sos_addrs->param_dest_gpa = K_VIRT_TO_PHYS(SOS_PARAM_PAGE_BASE);
    os->mm_memcpy((void *)in_sos_addrs->param_start, &boot_params, sizeof(struct sos_boot_params));

    return OS_SUCCESS;
}

void sparams_release_sos_initdata_sharedmem_params(void)
{
    if (sos_initdata_vaddr != INVALID_VADDR)
    {
        uint32_t sos_initdata_pages = SOS_INITDATA_SIZE / PAGE_SIZE_4K;
        os->mm_free_pages(sos_initdata_vaddr, sos_initdata_pages, ALLOC_ORDINARY_MEMORY);
        sos_initdata_vaddr = INVALID_VADDR;
        sos_addrs.sos_initdata_start = INVALID_VADDR;
        sos_addrs.sos_initdata_end = INVALID_VADDR;
        sos_addrs.sos_initdata_dest_gpa = INVALID_PADDR;
    }

    if (sos_sharemem_start_va != INVALID_VADDR)
    {
        uint32_t sos_sharedmem_pages = VK_SHAREMEM_SIZE / PAGE_SIZE_4K;
        os->mm_free_pages(sos_sharemem_start_va, sos_sharedmem_pages, ALLOC_ORDINARY_MEMORY);
        sos_sharemem_start_va = INVALID_VADDR;
        sos_addrs.sos_sharemem_start = INVALID_VADDR;
        sos_addrs.sos_sharemem_end = INVALID_VADDR;
        sos_addrs.sos_sharemem_dest_gpa = INVALID_PADDR;
    }

    if (sos_params_start_va != INVALID_VADDR)
    {
        uint32_t sos_param_pages = SOS_PARAM_PAGE_SIZE / PAGE_SIZE_4K;
        os->mm_free_pages(sos_params_start_va, sos_param_pages, ALLOC_ORDINARY_MEMORY);
        sos_params_start_va = INVALID_VADDR;
        sos_addrs.param_start = INVALID_VADDR;
        sos_addrs.param_end = INVALID_VADDR;
        sos_addrs.param_dest_gpa = INVALID_PADDR;
    }
}

int sparams_copy_sos_sect_addrs_from_sl(struct sl_sos_sect_addrs *sl_sos_sect_addrs_ptr, struct sos_sect_addrs *sos_sect_addrs)
{
    OSSTATUS status = OS_SUCCESS;

    status = os->copy_from_user(&sos_sect_addrs->sos_image_start, &sl_sos_sect_addrs_ptr->sos_image_start, sizeof(sos_sect_addrs->sos_image_start));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_image_end, &sl_sos_sect_addrs_ptr->sos_image_end, sizeof(sos_sect_addrs->sos_image_end));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_image_dest_gpa, &sl_sos_sect_addrs_ptr->sos_image_dest_gpa, sizeof(sos_sect_addrs->sos_image_dest_gpa));
    if (SA_FAIL(status))
        return -1;

    status = os->copy_from_user(&sos_sect_addrs->sos_mempool_start, &sl_sos_sect_addrs_ptr->sos_mempool_start, sizeof(sos_sect_addrs->sos_mempool_start));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_mempool_end, &sl_sos_sect_addrs_ptr->sos_mempool_end, sizeof(sos_sect_addrs->sos_mempool_end));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_mempool_dest_gpa, &sl_sos_sect_addrs_ptr->sos_mempool_dest_gpa, sizeof(sos_sect_addrs->sos_mempool_dest_gpa));
    if (SA_FAIL(status))
        return -1;

    status = os->copy_from_user(&sos_sect_addrs->sos_signature_start, &sl_sos_sect_addrs_ptr->sos_signature_start, sizeof(sos_sect_addrs->sos_signature_start));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_signature_end, &sl_sos_sect_addrs_ptr->sos_signature_end, sizeof(sos_sect_addrs->sos_signature_end));
    if (SA_FAIL(status))
        return -1;
    status = os->copy_from_user(&sos_sect_addrs->sos_signature_dest_gpa, &sl_sos_sect_addrs_ptr->sos_signature_dest_gpa, sizeof(sos_sect_addrs->sos_signature_dest_gpa));
    if (SA_FAIL(status))
        return -1;

    return 0;
}

void sparams_print_sos_segments(struct sos_sect_addrs *sos_sect_addrs)
{
    DRV_INFO("************\n");

    DRV_INFO("SecOS memory allocation from Non-Secure Domain:\n");

    DRV_INFO("Init Data \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->sos_initdata_start, sos_sect_addrs->sos_initdata_dest_gpa, sos_sect_addrs->sos_initdata_end);
    DRV_INFO("Params \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->param_start, sos_sect_addrs->param_dest_gpa, sos_sect_addrs->param_end);
    DRV_INFO("Signature \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->sos_signature_start, sos_sect_addrs->sos_signature_dest_gpa, sos_sect_addrs->sos_signature_end);

    DRV_INFO("Code \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->sos_image_start, sos_sect_addrs->sos_image_dest_gpa, sos_sect_addrs->sos_image_end);
    DRV_INFO("Heap \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->sos_mempool_start, sos_sect_addrs->sos_mempool_dest_gpa, sos_sect_addrs->sos_mempool_end);
    DRV_INFO("Shared Mem \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->sos_sharemem_start, sos_sect_addrs->sos_sharemem_dest_gpa, sos_sect_addrs->sos_sharemem_end);

    DRV_INFO("RX Comm Ctl \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->comm_os_rx_chanctl_va, sos_sect_addrs->comm_os_rx_chanctl_dest_gpa, sos_sect_addrs->comm_os_rx_chanctl_va_end);
    DRV_INFO("RX Comm Buf \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->comm_os_rx_chanbuf_va, sos_sect_addrs->comm_os_rx_chanbuf_dest_gpa, sos_sect_addrs->comm_os_rx_chanbuf_va_end);
    DRV_INFO("TX Comm Ctl \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->comm_os_tx_chanctl_va, sos_sect_addrs->comm_os_tx_chanctl_dest_gpa, sos_sect_addrs->comm_os_tx_chanctl_va_end);
    DRV_INFO("TX Comm Buf \t (start:0x%lX (gpaddr:0x%lX) - end:0x%lX)\n",
             sos_sect_addrs->comm_os_tx_chanbuf_va, sos_sect_addrs->comm_os_tx_chanbuf_dest_gpa, sos_sect_addrs->comm_os_tx_chanbuf_va_end);

    DRV_INFO("************\n");
}

void hv_reg_secos_sections(struct hv_reg_sections *scode_info, struct sos_sect_addrs *in_sos_addrs)
{
    vaddr_t sos_image_start, sos_image_end;
    vaddr_t sos_initdata_start, sos_initdata_end;
    vaddr_t sos_mempool_end, sos_mempool_start;
    vaddr_t sos_sharemem_end, sos_sharemem_start;
    vaddr_t param_start, param_end;
    vaddr_t sos_signature_start, sos_signature_end;
    vaddr_t comm_os_rx_chanbuf_va, comm_os_rx_chanbuf_va_end;
    vaddr_t comm_os_rx_chanctl_va, comm_os_rx_chanctl_va_end;
    vaddr_t comm_os_tx_chanbuf_va, comm_os_tx_chanbuf_va_end;
    vaddr_t comm_os_tx_chanctl_va, comm_os_tx_chanctl_va_end;

    scode_info->num_sections = 0;

    // add SecOS kernel image section
    sos_image_start = in_sos_addrs->sos_image_start;
    sos_image_end = in_sos_addrs->sos_image_end;

    if (sos_image_end != sos_image_start)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_IMAGE, sos_image_start,
                            tv_ptr_diff(sos_image_end, sos_image_start), in_sos_addrs->sos_image_dest_gpa);
    }

    // add SecOS initdata section
    sos_initdata_start = in_sos_addrs->sos_initdata_start;
    sos_initdata_end = in_sos_addrs->sos_initdata_end;

    if (sos_initdata_end != sos_initdata_start)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_DATA, sos_initdata_start,
                            tv_ptr_diff(sos_initdata_end, sos_initdata_start), in_sos_addrs->sos_initdata_dest_gpa);
    }

    // add SecOS memory-pool section
    sos_mempool_end = in_sos_addrs->sos_mempool_end;
    sos_mempool_start = in_sos_addrs->sos_mempool_start;

    if (sos_mempool_end != sos_mempool_start)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_DATA, sos_mempool_start,
                            tv_ptr_diff(sos_mempool_end, sos_mempool_start), in_sos_addrs->sos_mempool_dest_gpa);
    }

    // add SecOS shared memory section
    sos_sharemem_end = in_sos_addrs->sos_sharemem_end;
    sos_sharemem_start = in_sos_addrs->sos_sharemem_start;

    if (sos_sharemem_end != sos_sharemem_start)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, sos_sharemem_start,
                            tv_ptr_diff(sos_sharemem_end, sos_sharemem_start), in_sos_addrs->sos_sharemem_dest_gpa);
    }

    // add SecOS status page
    param_start = in_sos_addrs->param_start;
    param_end = in_sos_addrs->param_end;

    if (param_end != param_start)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, param_start,
                            tv_ptr_diff(param_end, param_start), in_sos_addrs->param_dest_gpa);
    }

    // add SecOS signature page
    sos_signature_start = in_sos_addrs->sos_signature_start;
    sos_signature_end = in_sos_addrs->sos_signature_end;

    if (sos_signature_end != sos_signature_start)
    {
        sb_mem_sections_add(scode_info, SOS_SECTION_SIGNATURE, sos_signature_start,
                            tv_ptr_diff(sos_signature_end, sos_signature_start), in_sos_addrs->sos_signature_dest_gpa);
    }

    // add SecOS-OS communication memory section
    comm_os_rx_chanbuf_va_end = in_sos_addrs->comm_os_rx_chanbuf_va_end;
    comm_os_rx_chanbuf_va = in_sos_addrs->comm_os_rx_chanbuf_va;

    if (comm_os_rx_chanbuf_va_end != comm_os_rx_chanbuf_va)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, comm_os_rx_chanbuf_va,
                            tv_ptr_diff(comm_os_rx_chanbuf_va_end, comm_os_rx_chanbuf_va), in_sos_addrs->comm_os_rx_chanbuf_dest_gpa);
    }

    comm_os_rx_chanctl_va_end = in_sos_addrs->comm_os_rx_chanctl_va_end;
    comm_os_rx_chanctl_va = in_sos_addrs->comm_os_rx_chanctl_va;

    if (comm_os_rx_chanctl_va_end != comm_os_rx_chanctl_va)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, comm_os_rx_chanctl_va,
                            tv_ptr_diff(comm_os_rx_chanctl_va_end, comm_os_rx_chanctl_va), in_sos_addrs->comm_os_rx_chanctl_dest_gpa);
    }

    comm_os_tx_chanbuf_va_end = in_sos_addrs->comm_os_tx_chanbuf_va_end;
    comm_os_tx_chanbuf_va = in_sos_addrs->comm_os_tx_chanbuf_va;

    if (comm_os_tx_chanbuf_va_end != comm_os_tx_chanbuf_va)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, comm_os_tx_chanbuf_va,
                            tv_ptr_diff(comm_os_tx_chanbuf_va_end, comm_os_tx_chanbuf_va), in_sos_addrs->comm_os_tx_chanbuf_dest_gpa);
    }

    comm_os_tx_chanctl_va_end = in_sos_addrs->comm_os_tx_chanctl_va_end;
    comm_os_tx_chanctl_va = in_sos_addrs->comm_os_tx_chanctl_va;

    if (comm_os_tx_chanctl_va_end != comm_os_tx_chanctl_va)
    {
        sb_mem_sections_add(scode_info, TV_VK_SECTION_SHARED, comm_os_tx_chanctl_va,
                            tv_ptr_diff(comm_os_tx_chanctl_va_end, comm_os_tx_chanctl_va), in_sos_addrs->comm_os_tx_chanctl_dest_gpa);
    }
}

//! Debug function - Dump <scode_info>
void hv_dump_sos_sections(struct hv_reg_sections *scode_info)
{
    uint32_t i = 0;

    DRV_INFO("scode_info addr:0x%lX\n", (vaddr_t)scode_info);
    for (i = 0; i < scode_info->num_sections; i++)
    {
        DRV_INFO("Mapping from gvaddr 0x%lX, num_pages: %lu, type: %u\n",
                 scode_info->sections[i].start_addr, scode_info->sections[i].page_num, (unsigned int)scode_info->sections[i].type);
    }
}

/********* Private Functions *********/
//! @brief Init the boot params of SecOS
static OSSTATUS _sparams_init_sos_boot_params(struct sos_boot_params *boot_params)
{
    boot_params->vk_comm_intr = vk_manifest.ipc_vector;

    os->sos_boot_params_init(boot_params);

    boot_params->os_comm_cpu_lapic_id = (ulong_t)os->sos_os_intr_get_os_lapic_id();
    if (boot_params->os_comm_cpu_lapic_id == INVALID_LAPIC_ID)
    {
        DRV_ERR("Init SecOS Boot parameters error! Invalid LAPIC ID of OS's dedicated CPU for SecOS-OS communications\n");
        return E_INTERNAL_ERROR;
    }

    return OS_SUCCESS;
}

static void sb_mem_sections_add(struct hv_reg_sections *scode_info,
                                int type,
                                vaddr_t start_addr, size_t len, paddr_t dest_gpaddr)
{
    int sections = scode_info->num_sections;

    if (sections >= TV_MAX_SECTIONS)
    {
        DRV_ERR("SecOS/SecApp: Too many sections to be registered\n");
        return;
    }

    scode_info->sections[sections].type = type;
    scode_info->sections[sections].start_addr = start_addr;
    scode_info->sections[sections].dest_gpaddr = dest_gpaddr;
    scode_info->sections[sections].page_num = (len % PAGE_SIZE) ? (1 + len / PAGE_SIZE) : (len / PAGE_SIZE);
    scode_info->num_sections++;
}

static size_t tv_ptr_diff(vaddr_t end, vaddr_t start)
{
    return end - start;
}