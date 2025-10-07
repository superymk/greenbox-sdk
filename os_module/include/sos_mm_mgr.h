// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_MM_MANAGER_H
#define SECOS_MM_MANAGER_H

#ifndef __ASSEMBLY__

#define SOS_MM_DEBUG(format, ...) \
	do{	\
		if(SOS_MM_LOG >= LOG_DEBUG) {  \
		PRINT("[CPU:%u][SOS_MM_MGR-Debug] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define SOS_MM_INFO(format, ...) \
	do{	\
		if((SOS_MM_LOG >= LOG_INFO) && (SOS_MM_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SOS_MM_MGR] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define SOS_MM_ERR(format, ...) \
	do{	\
		if((SOS_MM_LOG >= LOG_ERR) && (SOS_MM_LOG != LOG_NONE)) {  \
		PRINT_ERR("[CPU:%u][SOS_MM_MGR-ERR] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

// <vk_resource_manifest> indicates what resources are allocated to SecOS. The
// untrusted applications can request it, but not allowed to update it. 
// Otherwise, it will cause errors in the SecAgent (but not affect the security
// of SecOS and SecApps)
struct vk_resource_manifest
{
	intr_t 		ipc_vector; // SecAgent reserves an interrupt vector on each core 
					// with the same vector number. It is used for Inter-Processor
					// Communcation (IPC) purpose between SecOS <--> SecOS, 
					// SecOS <--> OS, and SecOS <--> SecBase

	vaddr_t 	vk_os_rx_chanbuf_start_vaddr;
	vaddr_t 	vk_os_rx_chanctl_start_vaddr;
	vaddr_t 	vk_os_tx_chanbuf_start_vaddr;
	vaddr_t 	vk_os_tx_chanctl_start_vaddr;
};
extern struct vk_resource_manifest vk_manifest; // apps can only get a read only copy

/// @brief 
/// @param size 
/// @return Return INVALID_PADDR on error 
extern paddr_t sos_mm_try_alloc_phys(size_t size);

extern void sos_mm_try_free_phys(paddr_t paddr_in_sos);

//! Allocate memory and put the information in <vk_manifest>
extern OSSTATUS sos_comm_resc_provision(void);

//! Free memory recorded in <vk_manifest>
extern void sos_comm_resc_return(void);


//! Request SecOS to allocate shared memory 
extern OSSTATUS sos_sharedmem_alloc(offset_t* out_offset, uint32_t alignment, uint32_t size);

//! Request SecOS to free shared memory 
extern OSSTATUS sos_sharedmem_free(offset_t offset, size_t sz);

//! Convert <offset> to SecAgent addressable <vaddr>
extern vaddr_t sos_sharedmem_ptr(offset_t offset);

#endif // __ASSEMBLY__
#endif // SECOS_MM_MANAGER_H