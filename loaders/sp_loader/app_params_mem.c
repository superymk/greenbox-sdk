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
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
static size_t tv_ptr_diff(vaddr_t end, vaddr_t start)
{
  return end - start;
}

void sp_reg_section_add(struct hv_reg_sections *scode_info,
                         int type,
                         vaddr_t start_addr, size_t len, paddr_t dest_gpaddr)
{
  int sections = scode_info->num_sections;

  if(sections >= TV_MAX_SECTIONS)
  {
	  APP_ERR("SecProcess: Too many sections to be registered!\n");
	  return;
  }

  scode_info->sections[sections].type = type;
  scode_info->sections[sections].start_addr = start_addr;
  scode_info->sections[sections].dest_gpaddr = dest_gpaddr;
  //assert((len % PAGE_SIZE) == 0);
  scode_info->sections[sections].page_num = (len % PAGE_SIZE) ? (1 + len / PAGE_SIZE): (len / PAGE_SIZE) ;
  //scode_info->sections[sections].page_num = (len / PAGE_SIZE) ;
  scode_info->num_sections++;
}

//! Build <scode_info> according to section memory addresses recorded in <sp_addrs>
void sp_reg_sections(struct hv_reg_sections *scode_info, struct sp_scope_addrs* sp_addrs)
{
	vaddr_t sp_scode_start, sp_scode_end;
  	vaddr_t sp_sdata_start, sp_sdata_end;
	vaddr_t sp_rodata_start, sp_rodata_end;
	vaddr_t sp_mempool_start, sp_mempool_end; // For SecProcess mem-pool ;
	vaddr_t sp_comm_start, sp_comm_end; 
	vaddr_t sp_sharemem_start, sp_sharemem_end; 
	vaddr_t sp_dmapool_start, sp_dmapool_end;
	vaddr_t sp_params_start, sp_params_end;
	vaddr_t sp_stack_start, sp_stack_end;


	scode_info->num_sections = 0;

  	// add SecProcess scode section 
  	sp_scode_end = sp_addrs->sp_scode_end;
	sp_scode_start = sp_addrs->sp_scode_start;
	
	if (sp_scode_start != INVALID_VADDR && sp_scode_end != sp_scode_start) {
  		sp_reg_section_add(scode_info, TV_PAL_SECTION_CODE, sp_scode_start,
                      tv_ptr_diff(sp_scode_end, sp_scode_start), sp_addrs->sp_scode_dest_gpa);
	}

  	// add SecProcess sdata section 
  	sp_sdata_end = sp_addrs->sp_sdata_end;
	sp_sdata_start = sp_addrs->sp_sdata_start;
  
  	if (sp_sdata_start != INVALID_VADDR && sp_sdata_end != sp_sdata_start) 
	{
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_IMAGE_DATA, sp_sdata_start,
                        tv_ptr_diff(sp_sdata_end, sp_sdata_start), sp_addrs->sp_sdata_dest_gpa);
  	}

	// add SecProcess RODATA section
  	sp_rodata_end = sp_addrs->sp_rodata_end;
	sp_rodata_start = sp_addrs->sp_rodata_start;
  
  	if (sp_rodata_start != INVALID_VADDR && sp_rodata_end != sp_rodata_start) 
	{
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_RODATA, sp_rodata_start,
                        tv_ptr_diff(sp_rodata_end, sp_rodata_start), sp_addrs->sp_rodata_dest_gpa);
  	}

	// add SecProcess params section
	sp_params_end = sp_addrs->sp_params_end;
	sp_params_start = sp_addrs->sp_params_start;
  
  	if (sp_params_start != INVALID_VADDR && sp_params_end != sp_params_start) 
	{
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_DATA, sp_params_start,
                        tv_ptr_diff(sp_params_end, sp_params_start), sp_addrs->sp_params_dest_gpa);
  	}

	// add SecProcess stack section
  	sp_stack_end = sp_addrs->sp_stack_end;
	sp_stack_start = sp_addrs->sp_stack_start;
  
  	if (sp_stack_start != INVALID_VADDR && sp_stack_end != sp_stack_start) 
	{
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_DATA, sp_stack_start,
                        tv_ptr_diff(sp_stack_end, sp_stack_start), sp_addrs->sp_stack_dest_gpa);
  	}

	// add SecProcess memory-pool section 
  	sp_mempool_end = sp_addrs->sp_mempool_end;
	sp_mempool_start = sp_addrs->sp_mempool_start;
  
  	if (sp_mempool_start != INVALID_VADDR && sp_mempool_end != sp_mempool_start) {
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_DATA, sp_mempool_start,
                        tv_ptr_diff(sp_mempool_end, sp_mempool_start), sp_addrs->sp_mempool_dest_gpa);
  	}

	// add SecProcess-App communication channel section 
  	sp_comm_end = sp_addrs->sp_comm_end;
	sp_comm_start = sp_addrs->sp_comm_start;
  
  	if (sp_comm_start != INVALID_VADDR && sp_comm_end != sp_comm_start) {
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_SHARED, sp_comm_start,
                        tv_ptr_diff(sp_comm_end, sp_comm_start), sp_addrs->sp_comm_dest_gpa);
  	}

	// add SecProcess shared memory section 
  	sp_sharemem_end = sp_addrs->sp_sharemem_end;
	sp_sharemem_start = sp_addrs->sp_sharemem_start;
  
  	if (sp_sharemem_start != INVALID_VADDR && sp_sharemem_end != sp_sharemem_start) {
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_SHARED, sp_sharemem_start,
                        tv_ptr_diff(sp_sharemem_end, sp_sharemem_start), sp_addrs->sp_sharemem_dest_gpa);
  	}

	// add SecProcess DMA-pool section 
  	sp_dmapool_end = sp_addrs->sp_dmapool_end;
	sp_dmapool_start = sp_addrs->sp_dmapool_start;
  
  	if (sp_dmapool_start != INVALID_VADDR && sp_dmapool_end != sp_dmapool_start) {
    	sp_reg_section_add(scode_info, TV_PAL_SECTION_DATA, sp_dmapool_start,
                        tv_ptr_diff(sp_dmapool_end, sp_dmapool_start), sp_addrs->sp_dmapool_dest_gpa);
  	}
}