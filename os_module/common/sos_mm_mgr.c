// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

// The SOS-OS communication channel timeout for SecOS memory request operations is 3000ms.
#define SOS_MM_MGR_COMM_SOS_TIMEOUT (3000)

extern struct sos_sect_addrs sos_addrs;

// Information of SecOS's memory allocated from SecAgent
struct vk_resource_manifest vk_manifest = 
	{
		.ipc_vector = INVALID_INTR,
		.vk_os_rx_chanbuf_start_vaddr = INVALID_VADDR,
	 	.vk_os_rx_chanctl_start_vaddr = INVALID_VADDR,
	 	.vk_os_tx_chanbuf_start_vaddr = INVALID_VADDR,
	 	.vk_os_tx_chanctl_start_vaddr = INVALID_VADDR
	}; 

static paddr_t next_pick_paddr = SPS_MEM_PADDR_BASE;

// #define PAGE4K_SHIFT			12
// #define PAGE_MASK_4K				0xfffff000
// #define PAGE_SIZE_4K				(1<<PAGE4K_SHIFT)
// #define SIZE_ROUNDUP_4K(size)	(((size) + PAGE_SIZE_4K - 1) & PAGE_MASK_4K)

paddr_t sos_mm_try_alloc_phys(size_t size)
{
	paddr_t result;

	// [TODO] ugly hack, and it will cause a lot of internal fragment
	// [TODO][Issue 17] Reimplement <sos_mm_try_alloc_phys> and related functions
	if(next_pick_paddr > SPS_MEM_PADDR_END)
		next_pick_paddr = SPS_MEM_PADDR_BASE;

	result = next_pick_paddr;
	
	next_pick_paddr += SIZE_ROUNDUP_4K(size);
	return result;
}

void sos_mm_try_free_phys(paddr_t paddr_in_sos)
{
	UNUSED(paddr_in_sos);
	PRINT("<sos_mm_try_free_phys> is un-implemented\n");
}

//! Allocate memory and put the information in <vk_manifest>
OSSTATUS sos_comm_resc_provision(void)
{
	OSSTATUS status = OS_SUCCESS;

	vaddr_t* p_rx_chanctl_start = &vk_manifest.vk_os_rx_chanctl_start_vaddr;
	vaddr_t* p_rx_chanbuf_start = &vk_manifest.vk_os_rx_chanbuf_start_vaddr;
	vaddr_t* p_tx_chanctl_start = &vk_manifest.vk_os_tx_chanctl_start_vaddr;
	vaddr_t* p_tx_chanbuf_start = &vk_manifest.vk_os_tx_chanbuf_start_vaddr;
	
	// Step 1. Provision the interrupt vector for SecOS-OS and SecOS-SecBase channels
	status = os->sos_os_intr_probe_and_register();
	if(SA_FAIL(status))
	{
		DRV_ERR("Probe and Register SecOS-OS Interrupt Error! status:%lu\n", status);
		return E_INTERNAL_ERROR;
	}

	vk_manifest.ipc_vector = os->sos_os_intr_get_vector();
	if(vk_manifest.ipc_vector == INVALID_INTR)
	{
		DRV_ERR("Probe and Register SecOS-OS Interrupt Vector Error!\n"); 
		return E_INTERNAL_ERROR;
	}

	// Step 2. Provision the ring buffer for SecOS-OS communication channel
	if(*p_rx_chanctl_start == INVALID_VADDR)
	{
		*p_rx_chanctl_start = os->mm_alloc_pages(COMM_OS_CHANCTL_PAGES, ALLOC_DMA_MEMORY);
		*p_rx_chanbuf_start = os->mm_alloc_pages(COMM_OS_CHANBUF_PAGES, ALLOC_DMA_MEMORY);
		*p_tx_chanctl_start = os->mm_alloc_pages(COMM_OS_CHANCTL_PAGES, ALLOC_DMA_MEMORY);
		*p_tx_chanbuf_start = os->mm_alloc_pages(COMM_OS_CHANBUF_PAGES, ALLOC_DMA_MEMORY);
		
		// Return on any memory allocation errors
		if(	*p_rx_chanctl_start == INVALID_VADDR || *p_rx_chanbuf_start == INVALID_VADDR || 
			*p_tx_chanctl_start == INVALID_VADDR || *p_tx_chanbuf_start == INVALID_VADDR)
			return E_NO_MEM;

		os->mm_memset((void*)(*p_rx_chanctl_start), 0, COMM_OS_CHANCTL_SIZE);
		os->mm_memset((void*)(*p_rx_chanbuf_start), 0, COMM_OS_CHANBUF_SIZE);
		os->mm_memset((void*)(*p_tx_chanctl_start), 0, COMM_OS_CHANCTL_SIZE);
		os->mm_memset((void*)(*p_tx_chanbuf_start), 0, COMM_OS_CHANBUF_SIZE);
	}

	return OS_SUCCESS;
}

//! Free memory recorded in <vk_manifest>
void sos_comm_resc_return(void)
{
	vaddr_t* p_rx_chanctl_start = &vk_manifest.vk_os_rx_chanctl_start_vaddr;
	vaddr_t* p_rx_chanbuf_start = &vk_manifest.vk_os_rx_chanbuf_start_vaddr;
	vaddr_t* p_tx_chanctl_start = &vk_manifest.vk_os_tx_chanctl_start_vaddr;
	vaddr_t* p_tx_chanbuf_start = &vk_manifest.vk_os_tx_chanbuf_start_vaddr;

	if(*p_rx_chanctl_start != INVALID_VADDR)
	{
		os->mm_free_pages(*p_rx_chanctl_start, COMM_OS_CHANCTL_PAGES, ALLOC_DMA_MEMORY);
		os->mm_free_pages(*p_rx_chanbuf_start, COMM_OS_CHANBUF_PAGES, ALLOC_DMA_MEMORY);
		os->mm_free_pages(*p_tx_chanctl_start, COMM_OS_CHANCTL_PAGES, ALLOC_DMA_MEMORY);
		os->mm_free_pages(*p_tx_chanbuf_start, COMM_OS_CHANBUF_PAGES, ALLOC_DMA_MEMORY);
		
		*p_rx_chanctl_start = INVALID_VADDR;
		*p_rx_chanbuf_start = INVALID_VADDR;
		*p_tx_chanctl_start = INVALID_VADDR;
		*p_tx_chanbuf_start = INVALID_VADDR;
	}

	vk_manifest.ipc_vector = INVALID_INTR;

#ifndef WINNT
	// Release SOS-OS communication interrupt resource.
	// [TODO][Issue 104] Due to Issue 104, SecAgent only invokes IoDisconnectInterrupt during DriverUnload
	// os->sos_os_intr_unregister();
#endif

}


//! Request SecOS to allocate shared memory 
OSSTATUS sos_sharedmem_alloc(offset_t* out_offset, uint32_t alignment, uint32_t size)
{
    OSSTATUS status = OS_SUCCESS;
	struct comm_os_msg request = {{0}};
	struct comm_os_msg response = {{0}};
	
	// Fill the msg header 
	request.header.data_size = 8;
	request.header.cmd = CMD_SECOS_ALLOC_SHAREDMEM;
	((uint32_t*)request.req_data)[0] = (uint32_t)alignment;
	((uint32_t*)request.req_data)[1] = (uint32_t)size;
	status = comm_sos_prepare_request(&request, true, SECOS_ID);
	if(SA_FAIL(status))
    {
        SOS_MM_ERR("SecOS shared memory allocation - Failed to generate request to SecOS! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

	// Send it out
	status = comm_sos_send_msg_and_wait(&request, &response, SOS_MM_MGR_COMM_SOS_TIMEOUT);
	if(SA_FAIL(status))
	{
		SOS_MM_ERR("SecOS shared memory allocation - Send msg to SecOS error! status:%lu\n", status);
		return E_COMM_ERR;
	}

	// Response
	if(response.header.response != SOS_COMM_SUCCESS)
	{
		SOS_MM_ERR("SecOS cannot allocate shared memory!\n");
		return E_NO_MEM;
	}
	
	*out_offset = *(offset_t*)response.req_data;
	return OS_SUCCESS;
}

//! Request SecOS to free shared memory 
OSSTATUS sos_sharedmem_free(offset_t offset, size_t sz)
{
	OSSTATUS status = OS_SUCCESS;
	struct comm_os_msg request = {{0}};
	struct comm_os_msg_free_sharedmem* req = (struct comm_os_msg_free_sharedmem*)request.req_data;
	
	// Fill the msg header 
	request.header.data_size = sizeof(struct comm_os_msg_free_sharedmem);
	request.header.cmd = CMD_SECOS_DEALLOC_SHAREDMEM;
	req->sharedmem_offset = offset;
	req->sharedmem_sz = sz;
	status = comm_sos_prepare_request(&request, false, SECOS_ID);
	if(SA_FAIL(status))
    {
        SOS_MM_ERR("SecOS shared memory deallocation - Failed to generate request to SecOS! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

	// Send it out
	status = comm_sos_send_msg_nowait(&request);
	if(SA_FAIL(status))
	{
		SOS_MM_ERR("SecOS shared memory deallocation - Send msg to SecOS error!\n");
		return E_COMM_ERR;
	}

	// [TODO][Issue 20] In <sos_sharedmem_free>, it is unknown what will happen in case of double free
	return OS_SUCCESS;
}

//! Convert <offset> to SecAgent addressable <vaddr>
vaddr_t sos_sharedmem_ptr(offset_t offset)
{
	if(sos_addrs.sos_sharemem_start == INVALID_VADDR)
	{
		SOS_MM_ERR("Cannot get the memory pointer of SecOS's shared memory!\n");
		return INVALID_VADDR;
	}

	return (vaddr_t)(offset + sos_addrs.sos_sharemem_start);
}