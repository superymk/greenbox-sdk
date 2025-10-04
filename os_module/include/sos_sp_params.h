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

#ifndef IOCTL_LIB_SECBASE_H
#define IOCTL_LIB_SECBASE_H


#ifndef __ASSEMBLY__

struct sos_sect_addrs
{
	// For SecOS image
	vaddr_t sos_image_start; 
  	vaddr_t sos_image_end;
	paddr_t sos_image_dest_gpa;

	// For SecOS initdata
	vaddr_t sos_initdata_start; 
  	vaddr_t sos_initdata_end;
	paddr_t sos_initdata_dest_gpa;

	// SecOS Mem-Pool
	vaddr_t sos_mempool_start;
  	vaddr_t sos_mempool_end;
	paddr_t sos_mempool_dest_gpa;

	// SecOS shared memory
	vaddr_t sos_sharemem_start;
  	vaddr_t sos_sharemem_end;
	paddr_t sos_sharemem_dest_gpa;

	// Status Page (Share SecOS status between SecOS and commodity OS/Apps)
	vaddr_t param_start;
	vaddr_t param_end;
	paddr_t param_dest_gpa;

	// Signature Page
	vaddr_t sos_signature_start;
	vaddr_t sos_signature_end;
	paddr_t sos_signature_dest_gpa;

	// SecOS-OS communication channel
	vaddr_t comm_os_rx_chanbuf_va;
  	vaddr_t comm_os_rx_chanbuf_va_end;
	paddr_t comm_os_rx_chanbuf_dest_gpa;
	vaddr_t comm_os_rx_chanctl_va;
  	vaddr_t comm_os_rx_chanctl_va_end;
	paddr_t comm_os_rx_chanctl_dest_gpa;
	vaddr_t comm_os_tx_chanbuf_va;
  	vaddr_t comm_os_tx_chanbuf_va_end;
	paddr_t comm_os_tx_chanbuf_dest_gpa;
	vaddr_t comm_os_tx_chanctl_va;
  	vaddr_t comm_os_tx_chanctl_va_end;
	paddr_t comm_os_tx_chanctl_dest_gpa;
};

/// @brief Addresses of SecOS sections.
extern struct sos_sect_addrs sos_addrs;

/// @brief Is SecOS registered?
/// [NOTE] SecOS cannot be re-registered before its unregistration
extern bool sos_is_registered;

extern bool sos_is_running;

/// @brief Build SecOS's init data.
/// [NOTE] SecAgent construts init data. Otherwise, SecAgent has to verify the SecOS's initial page table built by 
/// SecOS loader, and this operation is expensive.
/// @param sos_addrs 
/// @return 
extern OSSTATUS sparams_build_sos_initdata(struct sos_sect_addrs* sos_addrs);

/// @brief Build SecOS's shared memory.
/// [NOTE] Only SecAgent can setup SecOS's shared memory, params, and communication channels. If these regions are 
/// setup by a red application, then other applications may access these shared memory (e.g., by attaching debuggers, 
/// injecting threads). In other words, the cost of protecting these memory in the non-secure partition would be higher.
/// @param sos_addrs 
/// @return 
extern OSSTATUS sparams_build_sos_sharedmem(struct sos_sect_addrs* sos_addrs);

/// @brief Build SecOS's parameters.
/// [NOTE] Only SecAgent can setup SecOS's shared memory, params, and communication channels. If these regions are 
/// setup by a red application, then other applications may access these shared memory (e.g., by attaching debuggers, 
/// injecting threads). In other words, the cost of protecting these memory in the non-secure partition would be higher.
/// @param sos_addrs 
/// @return 
extern OSSTATUS sparams_build_sos_params(struct sos_sect_addrs* sos_addrs);

/// @brief Load SecOS's signature.
/// @param in_sos_addrs 
/// @return 
extern OSSTATUS sparams_build_sos_signature(struct sos_sect_addrs* in_sos_addrs);

/// @brief Release SecOS's initdata, shared mem, and params space allocated in SecAgent.
/// @param  
extern void sparams_release_sos_initdata_sharedmem_params(void);

/// @brief Copy addresses in <sl_sos_sect_addrs_ptr> to <sos_sect_addrs>.
/// @param sl_sos_sect_addrs_ptr 
/// @param sos_sect_addrs 
/// @return 
extern int sparams_copy_sos_sect_addrs_from_sl(struct sl_sos_sect_addrs* sl_sos_sect_addrs_ptr, struct sos_sect_addrs* sos_sect_addrs);

/// @brief Generate <scode_info> used by the SecBase.
/// @param scode_info 
/// @param sos_addrs 
extern void hv_reg_secos_sections(struct hv_reg_sections *scode_info, struct sos_sect_addrs* sos_addrs);

/// @brief Debug function - Dump <scode_info>.
/// @param scode_info 
extern void hv_dump_sos_sections(struct hv_reg_sections *scode_info);

/// @brief Print SecOS's memory segments.
/// @param sos_sect_addrs 
void sparams_print_sos_segments(struct sos_sect_addrs* sos_sect_addrs);

#endif // __ASSEMBLY__
#endif // IOCTL_LIB_SECBASE_H