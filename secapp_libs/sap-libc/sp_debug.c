// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include <stdio.h>

void sp_dbg_print_hex(const char *prefix, const void *prtptr, size_t size)
{
    size_t i;
    for ( i = 0; i < size; i++ ) {
        if ( i % 16 == 0 && prefix != NULL )
            printf("\n%s", prefix);
        printf("%02x ", *(const uint8_t *)prtptr++);
    }
    printf("\n");
}

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