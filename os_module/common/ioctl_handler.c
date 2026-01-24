// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>
#include <device/usb/ehci.h>
#include <device/gpu/gpu.h>
#include <device/gpu/gpu_debug.h>
#include <device/ps2.h>

extern struct vk_resource_manifest vk_manifest;
static sp_id_t sp_next_id = SP_USER_ID_START;

extern void os_send_ipi_to_cpu(uint8_t vector, lapic_id_t apic_id);

#define HANDLE(hc)                 \
    case hc:                       \
        ret = do_##hc(cmd_packet); \
        break
OSSTATUS ioctl_request_handler(struct tz_cmd_packet *cmd_packet, int *copy_retval)
{
    OSSTATUS ret = OS_SUCCESS;

    if (cmd_packet->id != CMD_PROBE_SECOS_STATUS)
        DRV_DEBUG("Handling IOCTL:%d...\n", cmd_packet->id);

    switch (cmd_packet->id)
    {
        HANDLE(CMD_SECBASE_INIT_ONDEMAND_PER_CPU);
        HANDLE(CMD_SECBASE_FINI_ONDEMAND_PER_CPU);
#ifndef GREENBOX_USE_UXMHF64_V1
        HANDLE(CMD_SECBASE_BOOT_TINYOS);
#endif // !GREENBOX_USE_UXMHF64_V1
    case CMD_SECAGENT_TEST1:
    {
        DRV_INFO("[IOCTL-CMD_SECAGENT_TEST1] Success\n");
        break;
    }
    case CMD_SECAGENT_TEST2:
    {
        OSSTATUS status = OS_SUCCESS;
        int fn_ret = 0;
        ulong_t hyp_ret = 0;
        struct sl_sos_sect_addrs *sl_sos_sect_addrs_ptr = (struct sl_sos_sect_addrs *)(vaddr_t)cmd_packet->arg1;
        struct hv_reg_sections scode_info = {0};

        {
            (void)scode_info;
            (void)sl_sos_sect_addrs_ptr;
            (void)hyp_ret;
            (void)fn_ret;
            (void)status;
        }

        DRV_INFO("[IOCTL-CMD_SECAGENT_TEST2] Success\n");
        break;
    }
    case CMD_SECAGENT_TEST3:
    {
        DRV_INFO("[IOCTL-CMD_SECAGENT_TEST3] Success\n");
        break;
    }
        HANDLE(CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU);

        // Untrusted loaders share information
        HANDLE(CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET);
        HANDLE(CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET);

        // Untrusted loaders utility functions
        HANDLE(CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE);
        HANDLE(CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE);

    case CMD_TEST_HYPERCALL:
    {
        ulong_t hyp_ret = 0;

        hyp_ret = os->hypercall_0arg(NS_HC_TEST);
        if (hyp_ret)
            DRV_ERR("[IOCTL-CMD_TEST_HYPERCALL] Issue hypercall error!\n");

        if (copy_retval)
            *copy_retval = 0;
        ret = OS_SUCCESS;
        break;
    }
    case CMD_KERNEL_PRINT:
    {
        DRV_INFO("[CMD_KERNEL_PRINT] 0x%llX\n", cmd_packet->arg1);

        if (copy_retval)
            *copy_retval = 0;
        ret = OS_SUCCESS;
        break;
    }
        HANDLE(CMD_SECAGENT_PRESENCE_REQ);

        // case CMD_ALLOC_OS_CONTIGUOUS_MEM:
        // {
        // 	ulong_t i = 0;
        // 	int mem_type = (int)cmd_packet->arg1;
        // 	ulong_t num_pages = (ulong_t)cmd_packet->arg2;
        // 	ulong_t size = num_pages << 12;
        // 	vaddr_t vaddr = INVALID_VADDR;

        // 	if(mem_type == OS_KERN_DMA)
        // 		vaddr = os->mm_alloc_physical_contiguous_pages(num_pages, ALLOC_DMA_MEMORY);

        // 	// [NOTE] No one uses it currently
        // 	//else if (mem_type == OS_KERN_MEMORY)
        // 	//	vaddr = os->mm_alloc_physical_contiguous_pages(num_pages, ALLOC_ORDINARY_MEMORY);

        // 	if (!vaddr)
        // 	{
        // 		DRV_ERR("Allocate kernel memory error! Type:%d\n", mem_type);
        // 		ret = ENOMEM;
        // 		break;
        // 	}

        // 	cmd_packet->arg1 = (ulong_t)vaddr;
        // 	DRV_DEBUG("CMD_ALLOC_OS_CONTIGUOUS_MEM vaddr:0x%08lX, paddr:0x%08lX with size 0x%lx, type:%d!\n",
        // 		vaddr, (ulong_t)cmd_packet->arg1, size, mem_type);

        // 	// for (i = (ulong_t)vaddr; i<(ulong_t)vaddr + size; i+= PAGE_SIZE) {
        // 	// 	*(int *)i = i;
        // 	// }

        // 	if(copy_retval)
        // 		*copy_retval = 1;
        // 	ret = OS_SUCCESS;
        // 	break;
        // }

        // case CMD_FREE_OS_CONTIGUOUS_MEM:
        // {
        // 	ulong_t num_pages = (ulong_t)cmd_packet->arg1;
        // 	vaddr_t vaddr = (vaddr_t)cmd_packet->arg2;
        // 	int memory_type = (int)cmd_packet->arg3;

        // 	if (vaddr != INVALID_VADDR)
        // 	{
        // 		DRV_DEBUG("kfree dma pool at vaddr:0x%lX with size 0x%lx!\n",
        // 			vaddr, num_pages << 12);

        // 		if(memory_type == OS_KERN_DMA)
        // 			os->mm_free_pages(vaddr, num_pages, ALLOC_DMA_MEMORY);
        // 		// [NOTE] No one uses it currently
        // 		//else if (memory_type == OS_KERN_MEMORY)
        // 			//os->mm_free_pages(vaddr, num_pages, ALLOC_ORDINARY_MEMORY);
        // 	}

        // 	if(copy_retval)
        // 		*copy_retval = 0;
        // 	ret = OS_SUCCESS;
        // 	break;
        // }

        HANDLE(CMD_ALLOC_AND_LOCK_PAGES_FOR_SDOM);
        HANDLE(CMD_FREE_AND_UNLOCK_PAGES_FOR_SDOM);

    case CMD_VADDR_TO_PADDR:
    {
        vaddr_t vaddr = (vaddr_t)cmd_packet->arg1;
        paddr_t paddr;

        paddr = os->mm_vaddr_to_paddr(vaddr);
        cmd_packet->arg1 = paddr;

        if (copy_retval)
            *copy_retval = 1;
        ret = OS_SUCCESS;
        break;
    }

    case CMD_DISABLE_LOCAL_IRQ:
    {
        os->intr_local_irq_disable();
        break;
    }
    case CMD_ENABLE_LOCAL_IRQ:
    {
        os->intr_local_irq_enable();
        break;
    }

        HANDLE(CMD_POLL_OS_NOTIFICATION_INFO);
        HANDLE(CMD_REGISTER_OS_NOTIFICATION);
        HANDLE(CMD_UNREGISTER_OS_NOTIFICATION);

        // IRQ and interrupts
        HANDLE(CMD_SECAGENT_X86_OS_IRQ_GET_INTR_VECTOR);
        HANDLE(CMD_OS_TRIGGER_INTR);

        // SecOS related handlers
        HANDLE(CMD_REGISTER_SECOS_PREPARE);
        HANDLE(CMD_REGISTER_SECOS);
        HANDLE(CMD_UNREGISTER_SECOS);
        HANDLE(CMD_RUN_SECOS);
        HANDLE(CMD_PROBE_SECOS_STATUS);
        HANDLE(CMD_TERMINATE_SECOS);

    // SecProcess related handlers
    case CMD_SAP_ID_ALLOCATE:
    {
        cmd_packet->arg1 = (uint64_t)sp_next_id++;

        if (copy_retval)
            *copy_retval = 1;
        ret = 0;
        break;
    }
    case CMD_SAP_ID_DEALLOCATE:
    {
        // [TODO][Issue 18] Re-implement CMD_SAP_ID_ALLOCATE and CMD_SAP_ID_DEALLOCATE
        break;
    }
        HANDLE(CMD_REGISTER_SECPROCESS_MEMORY);
        HANDLE(CMD_REGISTER_SECPROCESS);
        HANDLE(CMD_RUN_SECPROCESS);
        HANDLE(CMD_SOFT_TERMINATE_SECPROCESS);
        HANDLE(CMD_FORCE_TERMINATE_SECPROCESS);
        HANDLE(CMD_NOTIFY_SECPROCESS);

        HANDLE(CMD_SECPROCESS_GET_IDS_AND_NAMES);

    case CMD_TRY_ALLOC_SOS_PHYSICAL_MEM:
    {
        size_t mem_size = (size_t)cmd_packet->arg1;
        paddr_t alloc_sos_mem = INVALID_PADDR;

        alloc_sos_mem = sos_mm_try_alloc_phys(mem_size);
        cmd_packet->arg1 = (uint64_t)alloc_sos_mem;

        if (copy_retval)
            *copy_retval = 1;
        ret = 0;

        break;
    }

    case CMD_TRY_FREE_SOS_PHYSICAL_MEM:
    {
        paddr_t paddr_in_sos = (paddr_t)cmd_packet->arg1;

        sos_mm_try_free_phys(paddr_in_sos);

        // No response is needed
        if (copy_retval)
            *copy_retval = 0;
        ret = 0;
        break;
    }

        HANDLE(CMD_IOSM_USB2_EHCI_PROBE);
        HANDLE(CMD_IOSM_USB2_EHCI_UNPROBE);
        HANDLE(CMD_IOSM_USB2_PAUSE_EHCI);
        HANDLE(CMD_IOSM_USB2_RESUME_EHCI);

        HANDLE(CMD_IOSM_USB3_SHARED_XHCI_PROBE);
        HANDLE(CMD_IOSM_USB3_SHARED_XHCI_UNPROBE);
        HANDLE(CMD_IOSM_USB3_SHARED_XHCI_PAUSE);
        HANDLE(CMD_IOSM_USB3_SHARED_XHCI_RESUME);

        HANDLE(CMD_IOSM_FB_PROBE_GPU);
        HANDLE(CMD_IOSM_FB_PAUSE_GPU);
        HANDLE(CMD_IOSM_FB_RESUME_GPU);
        HANDLE(CMD_IOSM_FB_TEST_ON);
        HANDLE(CMD_IOSM_FB_TEST_OFF);

        HANDLE(CMD_IOSM_PS2_DRAIN_BUFFERS);

    default:
    {
        DRV_ERR("Unrecognized cmd %d!\n", cmd_packet->id);

        if (copy_retval)
            *copy_retval = 0;
        ret = E_INV_ARG;
        break;
    }
    }

    if (cmd_packet->id != CMD_PROBE_SECOS_STATUS)
        DRV_DEBUG("Handle IOCTL:%d finished\n", cmd_packet->id);

    return ret;
}