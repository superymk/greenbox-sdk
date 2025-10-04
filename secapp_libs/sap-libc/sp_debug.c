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

#include <palddk.h>
#include <stdio.h>

void debug_sp_print_memory(void* ptr, size_t sz)
{
	uint32_t i = 0;

	SAP_INFO("");
	FOREACH_S(i, sz, sz, 0, 1)
	{
		SAP_INFOs("0x%02X ", ((uint8_t*)ptr)[i]);
	}
	SAP_INFOs("\n");
}

void debug_sp_utask_dump(SP_UTASK* params)
{
	SAP_ERR("sp_id:%u, is_init_sp:%u, init_sp_id:%u\n", params->sp_id, params->is_init_sp, params->init_sp_id);
	SAP_ERR("dma_pool:0x%lX, mem_pool:0x%lX, sp_app_comm:0x%lX, sharedmem_start:0x%lX\n", params->dma_pool, params->mem_pool, params->sp_app_comm, params->sharedmem_start);
	SAP_ERR("remappable_base:0x%lX, exec_params_secure:0x%lX, exec_params_secure[0]:0x%lX\n", params->remappable_base, (vaddr_t)params->exec_params_secure, *(ulong_t*)params->exec_params_secure);
}

uint32_t sos_sys_test(uint32_t arg1)
{
	return sp_sysenter(SYS_SP_TEST, (ulong_t)arg1, 0, 0);
}