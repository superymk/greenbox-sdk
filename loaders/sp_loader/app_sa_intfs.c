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

#include <appddk.h>
#include <string.h>

bool secagent_test_presence(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_SECAGENT_PRESENCE_REQ;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_test_presence> error with code %d!\n", ret);
		return false;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		APP_ERR("<secagent_test_presence> SecAgent error: %d!\n", sa_status);
		return false;
	}
	
	return true;
}

void secagent_test1(void)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_SECAGENT_TEST1;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_test1> error with code %d!\n", ret);
}

void secagent_test2(ulong_t arg1)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_SECAGENT_TEST2;
	cmd.arg1 = arg1;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_test2> error with code %d!\n", ret);
}

void secagent_test3(void)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_SECAGENT_TEST3;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_test3> error with code %d!\n", ret);
}

void secagent_print(uint32_t tag) 
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_KERNEL_PRINT;
	cmd.arg1 = tag;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_print> error with code %d!\n", ret);
}

OSSTATUS secagent_nsdom_share_data_to_any_ns_process_set(uint64_t selector, uint64_t data)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET;
	cmd.arg1 = selector;
	cmd.arg2 = data;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_nsdom_share_data_to_any_ns_process_set> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_nsdom_share_data_to_any_ns_process_get(uint64_t selector, uint64_t* data)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	if(!data)
		return E_INV_ARG;
	
	cmd.id = CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET;
	cmd.arg1 = selector;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_nsdom_share_data_to_any_ns_process_set> error with code %d!\n", ret);

	*data = cmd.arg2;
	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_nsdom_global_spinlock_try_acquire(uint64_t selector, bool* lock_got)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

    if(!lock_got)
		return E_INV_ARG;
	
	cmd.id = CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE;
	cmd.arg1 = selector;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_nsdom_global_spinlock_try_acquire> error with code %d!\n", ret);

    *lock_got = (cmd.arg2 == 0) ? false : true;
	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_nsdom_global_spinlock_release(uint64_t selector)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE;
	cmd.arg1 = selector;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_nsdom_global_spinlock_release> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

// OSSTATUS secagent_app_cache_flush_range(vaddr_t vaddr, size_t sz, bool flush_all_data_cache_on_failure)
// {
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	APP_ERR("[Superymk] <secagent_app_cache_flush_range> vaddr:0x%lX, sz:0x%lX\n", vaddr, sz);

// 	cmd.id = CMD_APP_CACHE_FLUSH_RANGE;
// 	cmd.arg1 = vaddr;
// 	cmd.arg2 = sz;
// 	cmd.arg3 = flush_all_data_cache_on_failure;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 		APP_ERR("<secagent_app_cache_flush_range> error with code %d!\n", ret);

// 	return (OSSTATUS)cmd.status;
// }

paddr_t secagent_vaddr_to_paddr(vaddr_t vaddr) 
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_VADDR_TO_PADDR;
	cmd.arg1 = vaddr;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_vaddr_to_paddr> error with code %d!\n", ret);
		return INVALID_PADDR;
	}

	return (paddr_t)cmd.arg1;
}

//! @brief Send a request to the SecProcess <sp_id>
OSSTATUS secagent_notify_secprocess(spid_t sp_id) 
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_NOTIFY_SECPROCESS;
	cmd.arg1 = (uint32_t)sp_id;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_notify_secprocess> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

spid_t secagent_allocate_sp_id(void) 
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SAP_ID_ALLOCATE;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_allocate_sp_id> error with code %d!\n", ret);
		return INVALID_SP_ID;
	}

	return (spid_t)cmd.arg1;
}

OSSTATUS secagent_deallocate_sp_id(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SAP_ID_DEALLOCATE;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_deallocate_sp_id> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_register_sp_mem(struct hv_reg_sections* sect)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_REGISTER_SECPROCESS_MEMORY;
	cmd.arg1 = (vaddr_t)sect;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_register_sp_mem> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_register_sp(SP_USTRUCT* sp_params) 
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_REGISTER_SECPROCESS;
	cmd.arg1 = (vaddr_t)sp_params;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_register_sp> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_run_sp(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_RUN_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_run_sp> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_force_terminate_sp(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_FORCE_TERMINATE_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_force_terminate_sp> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_soft_terminate_sp(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SOFT_TERMINATE_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_soft_terminate_sp> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_register_app_notification(struct sa_app_notif_reginfo* reginfo)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

    // Check: Valid parameters
    if(!reginfo)
    {
		APP_ERR("<secagent_register_app_notification> Invalid parameters!\n");
		return E_INV_ARG;
	}
	
	cmd.id = CMD_REGISTER_OS_NOTIFICATION;
	cmd.arg1 = (vaddr_t)reginfo;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_register_app_notification> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_unregister_app_notification(void)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_UNREGISTER_OS_NOTIFICATION;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_unregister_app_notification> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_poll_notification_info(struct sa_app_notif_info* out_info)
{
	bool status = false;
	struct tz_cmd_packet cmd;
    struct sa_app_notif_info siginfo;
	int ret = 0;

    // Check: Valid parameters
    if(!out_info)
    {
		APP_ERR("<secagent_poll_notification_info> Invalid parameters!\n");
		return E_INV_ARG;
	}
	
	cmd.id = CMD_POLL_OS_NOTIFICATION_INFO;
	cmd.arg1 = (vaddr_t)&siginfo;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_poll_notification_info> error with code %d!\n", ret);

    memcpy(out_info, &siginfo, sizeof(struct sa_app_notif_info));

	return (OSSTATUS)cmd.status;
}




// static vaddr_t _sa_req_kern_mem(
// 	uint32_t size, 
// 	int memory_type
// )
// {
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;
	
// 	cmd.id = CMD_ALLOC_OS_CONTIGUOUS_MEM;
// 	cmd.arg1 = memory_type;
// 	cmd.arg2 = (size >> 12);
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		APP_ERR("<_sa_req_kern_mem> error with code %d!\n", ret);
// 		return INVALID_VADDR;
// 	}

// 	return (vaddr_t)cmd.arg1;
// }

// static int _sa_free_kern_mem(
// 	vaddr_t vaddr, 
// 	uint32_t size, 
// 	int memory_type
// )
// {
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;
	
// 	cmd.id = CMD_FREE_OS_CONTIGUOUS_MEM;
// 	cmd.arg1 = (size >> 12);
// 	cmd.arg2 = vaddr;
// 	cmd.arg3 = memory_type;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		APP_ERR("<_sa_free_kern_mem> error with code %d!\n", ret);
// 		return -1;
// 	}

// 	return 0;
// }

// vaddr_t secagent_req_dma_pool(uint32_t size) 
// {
// 	return _sa_req_kern_mem(size, OS_KERN_DMA);
// }

// vaddr_t secagent_req_kern_pages(uint32_t size) 
// {
// 	return _sa_req_kern_mem(size, OS_KERN_MEMORY);
// }

// int secagent_free_dma_pool(vaddr_t vaddr, uint32_t size) 
// {
// 	return _sa_free_kern_mem(vaddr, size, OS_KERN_DMA);
// }

// int secagent_free_kern_pages(vaddr_t vaddr, uint32_t size) 
// {
// 	return _sa_free_kern_mem(vaddr, size, OS_KERN_MEMORY);
// }

paddr_t secagent_try_alloc_sos_phy_mem(size_t size)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_TRY_ALLOC_SOS_PHYSICAL_MEM;
	cmd.arg1 = (uint64_t)size;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_try_alloc_sos_phy_mem> error with code %d!\n", ret);
		return INVALID_PADDR;
	}

	return (paddr_t)cmd.arg1;
}

void secagent_try_free_sos_phy_mem(paddr_t paddr_in_sos)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_TRY_FREE_SOS_PHYSICAL_MEM;
	cmd.arg1 = (uint64_t)paddr_in_sos;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
		APP_ERR("<secagent_try_free_sos_phy_mem> error with code %d!\n", ret);
}

/*int secagent_init_trusted_display(uint32_t width, uint32_t height, vaddr_t* td_vmem_base, size_t* td_vmem_size)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	vaddr_t vmem_base = INVALID_VADDR;
	size_t vmem_size = 0;
	
	cmd.id = CMD_TD_INIT;
	cmd.arg1 = width;
	cmd.arg2 = height;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_init_trusted_display> error with code %d!\n", ret);
		return -1;
	}

	ret = (int)cmd.arg1;
	vmem_base = (vaddr_t)cmd.arg2;
	vmem_size = (size_t)cmd.arg3;

	if(td_vmem_base)
		*td_vmem_base = vmem_base;
	if(td_vmem_size)
		*td_vmem_size = vmem_size;
	return ret;
}

int secagent_fini_trusted_display(void)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_TD_FINI;
	
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		APP_ERR("<secagent_fini_trusted_display> error with code %d!\n", ret);
		return -1;
	}

	return 0;
}*/

