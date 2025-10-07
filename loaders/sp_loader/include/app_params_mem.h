// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _APP_PARAMS_MEM_H
#define _APP_PARAMS_MEM_H

#include "app_os_if.h"

// Used to record the start and the end address of each section in the PAL
struct sp_scope_addrs {
	vaddr_t sp_scode_start; // For PAL code
  	vaddr_t sp_scode_end;
	paddr_t sp_scode_dest_gpa;
	
	vaddr_t sp_sdata_start; // For PAL data
  	vaddr_t sp_sdata_end;
	paddr_t sp_sdata_dest_gpa;

	vaddr_t sp_rodata_start; // For PAL rodata
  	vaddr_t sp_rodata_end;
	paddr_t sp_rodata_dest_gpa;

	// SecProcess params
	vaddr_t sp_params_start; // For PAL stack
  	vaddr_t sp_params_end;
	paddr_t sp_params_dest_gpa;

	// SecProcess stack
	vaddr_t sp_stack_start; // For PAL stack
  	vaddr_t sp_stack_end;
	paddr_t sp_stack_dest_gpa;

	// SecProcess memory
	vaddr_t sp_mempool_start; // For PAL mem-pool
  	vaddr_t sp_mempool_end;
	paddr_t sp_mempool_dest_gpa;
	
	// SecProcess-App communication channel
	vaddr_t sp_comm_start;
	vaddr_t sp_comm_end;
	paddr_t sp_comm_dest_gpa;

	// SecProcess shared memory
	vaddr_t sp_sharemem_start;
  	vaddr_t sp_sharemem_end;
	paddr_t sp_sharemem_dest_gpa;

	// SecProcess DMA
	vaddr_t sp_dmapool_start;
  	vaddr_t sp_dmapool_end;
	paddr_t sp_dmapool_dest_gpa;
};

//! Build <scode_info> according to section memory addresses recorded in <sp_addrs>
extern void sp_reg_sections(struct hv_reg_sections *scode_info, struct sp_scope_addrs* sp_addrs);

extern void sp_reg_section_add(struct hv_reg_sections *scode_info,
                         int type,
                         vaddr_t start_addr, size_t len, paddr_t dest_gpaddr);

#endif // _APP_PARAMS_MEM_H