// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <fcntl.h>	// for open
// #include <unistd.h> // for close
// #include <time.h> // nanosleep

#include <sl_headers.h>

// uint32_t secagent_trigger_all_intrs(u32* intr_map) 
// {
// 	OSSTATUS sa_status = OS_SUCCESS;
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	cmd.id = CMD_TRIGGER_OS_PENDING_INTRS;
// 	cmd.arg1 = (vaddr_t)intr_map;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		SL_ERR("<secagent_trigger_all_intrs> error with code %d!\n", ret);
// 		return 1;
// 	}

// 	sa_status = cmd.status;
// 	if(SA_FAIL(sa_status))
// 	{
// 		SL_ERR("<secagent_trigger_all_intrs> SecAgent error: %d!\n", sa_status);
// 		return 1;
// 	}

// 	return 0;
// }




/********* SecOS Functions in SecAgent*********/
//! Register SecOS
uint32_t secagent_register_secos(struct sl_sos_sect_addrs* sos_addrs)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_REGISTER_SECOS;
	cmd.arg1 = (vaddr_t)sos_addrs;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_register_secos> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_register_secos> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

//! Unregister SecOS
uint32_t secagent_unregister_secos(void) 
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_UNREGISTER_SECOS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_unregister_secos> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_unregister_secos> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

//! Run SecOS
uint32_t secagent_run_secos(void) 
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	cmd.id = CMD_RUN_SECOS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_run_secos> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_run_secos> SecAgent error: %d!\n", sa_status);
		return 1;
	}
	
	return 0;
}

//! Get SecOS's most recent return status
VKSTATUS secagent_get_secos_status(bool* secagent_success)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	// Sanity check
	if(!secagent_success)
	{
		SL_ERR("<secagent_get_secos_status> Invalid parameters!\n");
		return FAILED;
	}

	cmd.id = CMD_PROBE_SECOS_STATUS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_get_secos_status> error with code %d!\n", ret);
		*secagent_success = false;
		return FAILED;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		*secagent_success = false;
		return FAILED;
	}
	
	*secagent_success = true;

	return (VKSTATUS)cmd.arg1;
}

//! Terminate SecOS
uint32_t secagent_terminate_secos(void) 
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_TERMINATE_SECOS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_terminate_secos> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_terminate_secos> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_register_secos_prepare(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_REGISTER_SECOS_PREPARE;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_register_secos_prepare> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_register_secos_prepare> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}




/********* SecProcess Information Query *********/
uint32_t secagent_sps_get_ids_and_names(struct sp_info_id_name_resp* resp_buf)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	if(!resp_buf)
	{
		SL_ERR("<secagent_sps_get_ids_and_names> invalid parameters!\n");
		return 1;
	}
	
	cmd.id = CMD_SECPROCESS_GET_IDS_AND_NAMES;
	cmd.arg1 = (vaddr_t)resp_buf;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_sps_get_ids_and_names> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_sps_get_ids_and_names> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}




/********* Other Functions in SecAgent*********/

#ifndef GREENBOX_USE_UXMHF64_V1
uint32_t secagent_secbase_boot_tinyos(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SECBASE_BOOT_TINYOS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_secbase_boot_tinyos> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_secbase_boot_tinyos> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}
#endif // !GREENBOX_USE_UXMHF64_V1

uint32_t secagent_init_secbase_ondemand_percpu(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SECBASE_INIT_ONDEMAND_PER_CPU;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_init_secbase_ondemand_percpu> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_init_secbase_ondemand_percpu> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_fini_secbase_ondemand_percpu(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SECBASE_FINI_ONDEMAND_PER_CPU;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_fini_secbase_ondemand_percpu> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_fini_secbase_ondemand_percpu> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_x86_init_collect_lapic_id_percpu(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_x86_init_collect_lapic_id_percpu> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_x86_init_collect_lapic_id_percpu> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_x86_os_irq_get_intr_vector(uint32_t irq, uint32_t* out_intr)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	if(!out_intr)
		return 1;
	
	cmd.id = CMD_SECAGENT_X86_OS_IRQ_GET_INTR_VECTOR;
	cmd.arg1 = irq;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_x86_os_irq_get_intr_vector> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_x86_os_irq_get_intr_vector> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	*out_intr = cmd.arg1;

	return 0;
}

uint32_t secagent_os_trigger_intr(uint32_t intr)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_OS_TRIGGER_INTR;
	cmd.arg1 = intr;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_os_trigger_intr> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_os_trigger_intr> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}



/*uint32_t secagent_disable_local_intr(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_DISABLE_LOCAL_IRQ;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_disable_local_intr> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_disable_local_intr> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_enable_local_intr(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_ENABLE_LOCAL_IRQ;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_enable_local_intr> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_enable_local_intr> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

uint32_t secagent_create_secapp(offset_t sp_param_offset_in_sharedmem)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_REGISTER_SECPROCESS;
	cmd.arg1 = sp_param_offset_in_sharedmem;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_create_secapp> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_create_secapp> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return (uint32_t)cmd.arg1;
}

uint32_t secagent_run_secapp(spid_t sp_id)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_RUN_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_run_secapp> error with code %d!\n", ret);
		return -1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_ERR("<secagent_run_secapp> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return (uint32_t)cmd.arg1;
}

uint32_t secagent_force_exit_secapp(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_FORCE_TERMINATE_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_force_exit_secapp> error with code %d!\n", ret);
		return -1;
	}

	return 0;
}

uint32_t secagent_soft_exit_secapp(spid_t sp_id)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_SOFT_TERMINATE_SECPROCESS;
	cmd.arg1 = sp_id;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_soft_exit_secapp> error with code %d!\n", ret);
		return -1;
	}

	return 0;
}

offset_t secagent_alloc_smem(uint32_t alignment, size_t size)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_ALLOC_SHAREDMEM;
	cmd.arg1 = alignment;
	cmd.arg2 = size;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_alloc_smem> error with code %d!\n", ret);
		return -1;
	}

	// Check status
	if(cmd.arg1)
		return -1;
	
	return cmd.arg2;
}

uint32_t secagent_dealloc_smem(offset_t offset)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_DEALLOC_SHAREDMEM;
	cmd.arg1 = offset;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<secagent_dealloc_smem> error with code %d!\n", ret);
		return -1;
	}

	return 0;
}*/

/*static vaddr_t _sa_req_kern_mem(
	uint32_t size, 
	int memory_type
)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_ALLOC_OS_CONTIGUOUS_MEM;
	cmd.arg1 = memory_type;
	cmd.arg2 = (size >> 12);
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<_sa_req_kern_mem> error with code %d!\n", ret);
		return INVALID_VADDR;
	}

	return (vaddr_t)cmd.arg1;
}

static int _sa_free_kern_mem(
	vaddr_t vaddr, 
	uint32_t size, 
	int memory_type
)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_FREE_OS_CONTIGUOUS_MEM;
	cmd.arg1 = (size >> 12);
	cmd.arg2 = vaddr;
	cmd.arg3 = memory_type;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_ERR("<_sa_free_kern_mem> error with code %d!\n", ret);
		return -1;
	}

	return 0;
}

vaddr_t secagent_req_dma_pool(uint32_t size) 
{
	return _sa_req_kern_mem(size, OS_KERN_DMA);
}

int secagent_free_dma_pool(vaddr_t vaddr, uint32_t size) 
{
	return _sa_free_kern_mem(vaddr, size, OS_KERN_DMA);
}*/


