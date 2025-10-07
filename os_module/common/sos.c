// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

static void _trigger_intrs(uint32_t* intr_bitmap)
{
	int idx;
	int shift;
	
	for (idx = 1; idx < 8; idx++)
	{
		u32 intr_map_value = intr_bitmap[idx];
		for(shift = 0; shift < (sizeof(u32)*8); shift++)
		{
			if(intr_map_value & (1<<shift))
			{
				uint32_t vector = idx * 32 + shift;
				os->intr_trigger_interrupt(vector);
				DRV_DEBUG("trigger interrupt: 0x%x!\n", vector);
			}
		}
	}
}

/********* Public Functions *********/
// OSSTATUS do_CMD_RESUME_SECOS(struct tz_cmd_packet *cmd_packet)
// {
//     OSSTATUS status = OS_SUCCESS;
//     ulong_t ret = 0;
//     struct sos_boot_params* boot_params = (struct sos_boot_params*)sos_addrs.param_start;

//     if(!sos_is_running)
//     {
//         status = E_INV_ARG;
//         cmd_packet->arg1 = SECOS_INFO_EXITED;
//         goto sos_resume_ret;
//     }

//     ret = os->hypercall_0arg(NS_HC_RESUME_SECOS);
//     if (ret)
//         DRV_ERR("[IOCTL-CMD_RESUME_SECOS] Resume SecOS error!\n");

//     // Get returned SecOS status 
//     cmd_packet->arg1 = boot_params->status.status;

//     if(cmd_packet->arg1 == SECOS_INFO_EXITED)
//         sos_is_running = false;
//     else if (cmd_packet->arg1 == SECOS_INFO_PREEMPTED)
//         sos_is_running = true;

//     status = OS_SUCCESS;

// sos_resume_ret:
//     return status;
// }

OSSTATUS sos_get_status(VKSTATUS* out_status)
{
    volatile struct sos_boot_params* boot_params = (volatile struct sos_boot_params*)sos_addrs.param_start;

    if(!out_status)
        return E_INV_ARG;

    if(!boot_params)
        return E_INV_ARG;

    // Flush cache to read fresh data
    os->cache_flush_range((vaddr_t)boot_params, sizeof(struct sos_boot_params));

    *out_status = boot_params->status.status;

    // DRV_ERR("[Superymk] <sos_get_status> boot_params->vk_comm_intr:%u, boot_params->vk_comm_intr:%u, boot_params->vk_comm_intr:%llu\n", 
    //     (uint32_t)boot_params->vk_comm_intr, (uint32_t)boot_params->sos_compat_conf, (uint64_t)boot_params->status.status);
    return OS_SUCCESS;
}

static OSSTATUS sos_get_ret_intr(uint32_t** out_intr)
{
    volatile struct sos_boot_params* boot_params = (volatile struct sos_boot_params*)sos_addrs.param_start;

    if(!out_intr)
    {
        return E_INV_ARG;
    }

    if(!boot_params)
    {
        return E_INV_ARG;
    }


    // Flush cache to read fresh data
    os->cache_flush_range((vaddr_t)boot_params, sizeof(struct sos_boot_params));

    *out_intr = (uint32_t*)boot_params->status.intr;
    return OS_SUCCESS;
}

OSSTATUS do_CMD_PROBE_SECOS_STATUS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    VKSTATUS sos_status;

    // Check: SecOS must have been registered 
    if(!sos_is_registered)
        return E_INV_ARG;

    // Get returned SecOS status 
    status = sos_get_status(&sos_status);
    if(SA_FAIL(status))
        return E_INV_ARG;

    cmd_packet->arg1 = sos_status;

    if(cmd_packet->arg1 == SECOS_INFO_PREEMPTED)
        sos_is_running = true;
    else if (cmd_packet->arg1 == SECOS_INFO_RETURN_FROM_PREEMPTION) 
        sos_is_running = true;
    else
        sos_is_running = false;

    return OS_SUCCESS;
}

OSSTATUS do_CMD_RUN_SECOS(struct tz_cmd_packet *cmd_packet)
{
    ulong_t hyp_ret = 0;
    struct sos_boot_params* boot_params = (struct sos_boot_params*)sos_addrs.param_start;

    DRV_INFO("Starting SecOS\n");

    // Call SecBase to boot SecOS
    hyp_ret = os->hypercall_0arg(NS_HC_BOOT_SECOS);
    if (hyp_ret)
        DRV_ERR("[IOCTL-CMD_RUN_SECOS] Run SecOS error!\n");

    // Get returned SecOS status
    //// Flush cache to read fresh data
    os->cache_flush_range((vaddr_t)boot_params, sizeof(struct sos_boot_params));
    
    cmd_packet->arg1 = boot_params->status.status;
    sos_is_running = true;

    return OS_SUCCESS;
}

OSSTATUS do_CMD_REGISTER_SECOS_PREPARE(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;

    (void)cmd_packet;
    status = sos_comm_resc_provision();
    if(SA_FAIL(status))
    {
        DRV_ERR("Provision Memory of SecOS-OS Communication Channel Error!\n");
        goto mm_resc_provision_err;
    }

    // Setup SecOS-OS communication of the SecAgent's side
    status = comm_sos_init(vk_manifest.vk_os_rx_chanctl_start_vaddr, 
        vk_manifest.vk_os_rx_chanbuf_start_vaddr, 
        vk_manifest.vk_os_tx_chanctl_start_vaddr, 
        vk_manifest.vk_os_tx_chanbuf_start_vaddr
    );
    if(SA_FAIL(status))
    {
        DRV_ERR("Setup SecOS-OS Communication Channel Error!\n"); 
        goto sos_init_err;
    }

    // On success
    return OS_SUCCESS;


sos_init_err:
	sos_comm_resc_return();
mm_resc_provision_err:
    return E_INTERNAL_ERROR;
}

OSSTATUS do_CMD_REGISTER_SECOS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    int fn_ret = 0;
    ulong_t hyp_ret = 0;
    struct sl_sos_sect_addrs* sl_sos_sect_addrs_ptr = (struct sl_sos_sect_addrs*)(vaddr_t)cmd_packet->arg1;
    struct hv_reg_sections scode_info = {0};

    // Copy parameters
    fn_ret = sparams_copy_sos_sect_addrs_from_sl(sl_sos_sect_addrs_ptr, &sos_addrs);
    if(fn_ret)
    {
        status = E_INTERNAL_ERROR;
        DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Copy user params error!\n");
        goto register_secos_ret;
    }

    // Check parameters
    if(sos_is_registered)
    {
        status = E_INTERNAL_ERROR;
        DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Another SecOS has been registered!\n");
        goto register_secos_ret;
    }

    // Check: The current process must have RW permission to memory recorded in <sl_sos_sect_addrs_ptr>
    {
        bool allowed_read_write = false;

        //// Check the permissions to SecOS image
        status = os->mm_check_current_process_page_permission(sos_addrs.sos_image_start, sos_addrs.sos_image_end - sos_addrs.sos_image_start, &allowed_read_write);
        if(SA_FAIL(status))
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Failed to check SecOS loader's memory permissions of SecOS image, start vaddr:0x%lX!\n", 
                sos_addrs.sos_image_start);
            status = E_INTERNAL_ERROR;
            goto register_secos_ret;
        }
        if(!allowed_read_write)
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] SecOS loader does not have RW permission to SecOS image, start vaddr:0x%lX!\n",
                sos_addrs.sos_image_start);
            status = E_CHKERR;
            goto register_secos_ret;
        }

        //// Check the permissions to SecOS heap memory
        status = os->mm_check_current_process_page_permission(sos_addrs.sos_mempool_start, sos_addrs.sos_mempool_end - sos_addrs.sos_mempool_start, &allowed_read_write);
        if(SA_FAIL(status))
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Failed to check SecOS loader's memory permissions of SecOS heap, start vaddr:0x%lX!\n", 
                sos_addrs.sos_mempool_start);
            status = E_INTERNAL_ERROR;
            goto register_secos_ret;
        }
        if(!allowed_read_write)
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] SecOS loader does not have RW permission to SecOS heap, start vaddr:0x%lX!\n",
                sos_addrs.sos_mempool_start);
            status = E_CHKERR;
            goto register_secos_ret;
        }
    }

    // Allocate memory of SecOS's parameters, SecOS-OS communication channels, and shared memory between SecOS 
    // and OS
    //
    // [NOTE] Why not create these shared memory in SecOS loader?
    // Reason: The SecAgent must protect these shared memory from user-level applications in the red. Otherwise,
    // a rogue SecOS loader can arbitarily access these shared memory and may cause TOCTTOU attacks.
    //
    // [NOTE] Why not allocate SecOS's code and data memory in SecAgent?
    // Reason: 1. SecOS code and data memory could be large, but red OS's kernel space is limited
    //		   2. Unlike SecOs's shared memory with OS, SecBase separates SecOS's code and data memory from 
    // non-secure domain. Thus commodity OS/Apps cannot access those memory once registering SecOS
    {
        sos_addrs.comm_os_rx_chanbuf_va = vk_manifest.vk_os_rx_chanbuf_start_vaddr;
        sos_addrs.comm_os_rx_chanctl_va = vk_manifest.vk_os_rx_chanctl_start_vaddr;
        sos_addrs.comm_os_tx_chanbuf_va = vk_manifest.vk_os_tx_chanbuf_start_vaddr;
        sos_addrs.comm_os_tx_chanctl_va = vk_manifest.vk_os_tx_chanctl_start_vaddr;
        sos_addrs.comm_os_rx_chanbuf_va_end = sos_addrs.comm_os_rx_chanbuf_va + COMM_OS_CHANBUF_SIZE;
        sos_addrs.comm_os_rx_chanctl_va_end = sos_addrs.comm_os_rx_chanctl_va + COMM_OS_CHANCTL_SIZE;
        sos_addrs.comm_os_tx_chanbuf_va_end = sos_addrs.comm_os_tx_chanbuf_va + COMM_OS_CHANBUF_SIZE;
        sos_addrs.comm_os_tx_chanctl_va_end = sos_addrs.comm_os_tx_chanctl_va + COMM_OS_CHANCTL_SIZE;

        sos_addrs.comm_os_rx_chanctl_dest_gpa = K_VIRT_TO_PHYS(SOS_COMM_OS_BASE);
        sos_addrs.comm_os_rx_chanbuf_dest_gpa = sos_addrs.comm_os_rx_chanctl_dest_gpa + COMM_OS_CHANCTL_SIZE;
        sos_addrs.comm_os_tx_chanctl_dest_gpa = sos_addrs.comm_os_rx_chanbuf_dest_gpa + COMM_OS_CHANBUF_SIZE;
        sos_addrs.comm_os_tx_chanbuf_dest_gpa = sos_addrs.comm_os_tx_chanctl_dest_gpa + COMM_OS_CHANCTL_SIZE;

        status = sparams_build_sos_sharedmem(&sos_addrs);
        if(SA_FAIL(status))
        {
            status = E_INTERNAL_ERROR;
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Initialize SecOS's shared memory error!\n");
            goto register_secos_ret;
        }

        status = sparams_build_sos_initdata(&sos_addrs);
        if(SA_FAIL(status))
        {
            status = E_INTERNAL_ERROR;
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Initialize SecOS's init data error!\n");
            goto register_secos_ret;
        }

        status = sparams_build_sos_params(&sos_addrs);
        if(SA_FAIL(status))
        {
            status = E_INTERNAL_ERROR;
            DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Initialize SecOS's parameters error!\n");
            goto register_secos_ret;
        }

        // Print SecOS's memory segments
        sparams_print_sos_segments(&sos_addrs);
    }
    
    hv_reg_secos_sections(&scode_info, &sos_addrs);

    // Init SecOS-OS communication channel's interrupt configurations
    status = sa_init_secos_os_comm_intr();
    if(SA_FAIL(status))
    {
        status = E_INTERNAL_ERROR;
        DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Init SecOS-OS communication channel's interrupt configurations error!\n");
        goto register_secos_ret;
    }
    
    // Short pause to print all pending info.
    os->timer_msleep(100);

    // Call SecBase
    hyp_ret = os->hypercall_1arg(NS_HC_REGISTER_SECOS, (vaddr_t)&scode_info);
    if (hyp_ret)
    {
        status = E_INTERNAL_ERROR;
        DRV_ERR("[IOCTL-CMD_REGISTER_SECOS] Register SecOS via hypercall error! ret:0x%lX\n", hyp_ret);
        goto register_secos_ret;
    }
    else
    {
        DRV_DEBUG("[IOCTL-CMD_REGISTER_SECOS] Success\n");
        sos_is_registered = true;
    }

    // On success
    return OS_SUCCESS;


register_secos_ret:
    return status;
}

OSSTATUS do_CMD_UNREGISTER_SECOS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    ulong_t hyp_ret = 0;
    uint32_t* ret_intr = NULL;

    (void)cmd_packet;

    // Call SecBase to unregister SecOS
    hyp_ret = os->hypercall_0arg(NS_HC_UNREGISTER_SECOS);
    if (hyp_ret)
    {
        DRV_ERR("[IOCTL-CMD_UNREGISTER_SECOS] Unregister SecOS via hypercall error! hyp_ret:%lu\n", hyp_ret);
    }

    // Finalize SecOS-OS communication channel's interrupt configurations.
    sa_fini_secos_os_comm_intr();

    // SecOS tells SecAgent the set of interrupts that need to be triggered in the red OS. Trigger them here.
    // [NOTE] If not triggering them, the red OS may not work properly after SecOS exits.
    status = sos_get_ret_intr(&ret_intr);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_UNREGISTER_SECOS] Unregister SecOS error: Failed to get interrupts to be triggered! status:%lu\n", status);
    }

    if(ret_intr)
    {
        _trigger_intrs(ret_intr);
    }
    else
    {
        DRV_ERR("[IOCTL-CMD_UNREGISTER_SECOS] Unregister SecOS error: Interrupts to be triggered is a NULL array!\n");
    }

    // Trigger timer interrupt. 
    // [NOTE] If not triggering it, the red OS may not work properly after SecOS exits.
    os->intr_trigger_interrupt(os->intr_timer());

    // Finalize SecOS's memory allocated in SecAgent
    sparams_release_sos_initdata_sharedmem_params();

    // Finalize SecOS-OS communication
    comm_sos_fini();
    
    sos_comm_resc_return();

    sos_is_registered = false;
    sos_is_running = false;

    // os->intr_local_irq_enable();

    // On success
    return OS_SUCCESS;
}

OSSTATUS do_CMD_TERMINATE_SECOS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct comm_os_msg* msg = NULL;

    (void)cmd_packet;

    msg = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));
    if(!msg)
    {
        DRV_ERR("[IOCTL-CMD_TERMINATE_SECOS] Failed to alloc request to SecOS!\n");
        return E_INTERNAL_ERROR;
    }
    
    // Fill the msg header 
    msg->header.data_size = 0;
    msg->header.cmd = CMD_SECOS_EXIT;
    status = comm_sos_prepare_request(msg, false, SECOS_ID);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_TERMINATE_SECOS] Failed to generate request to SecOS! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto err;
    }

    // Send it out
    status = comm_sos_send_msg_nowait(msg);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_TERMINATE_SECOS] Failed to submit command to SecOS!\n");
        status = E_COMM_ERR;
        goto err;
    }

    os->mm_free(msg);
    sos_is_running = false;

    // On success
    return OS_SUCCESS;

err:
    if(msg)
        os->mm_free(msg);
    return status;
}