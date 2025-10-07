// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_LOADER_H
#define APP_LOADER_H

#include <secbase.h>

#ifndef __ASSEMBLY__

#if defined(_WIN32) || defined(__WINDOWS__)
// We cannot include <windows.h> because it defines conflict macros with GreenBox. So we copy the MAX_PATH definition
// from Windows instead.
// #include <windows.h>
#ifndef MAX_PATH
#define MAX_PATH        (260)
#endif // MAX_PATH

#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

#if defined(_WIN32) || defined(__WINDOWS__)
#define MAX_PATH_LEN MAX_PATH
#elif defined(__APPLE__)
#define MAX_PATH_LEN PATH_MAX
#else
#define MAX_PATH_LEN PATH_MAX
#endif

#define FS_MAX_PATH_BUFFER_LEN  (MAX_PATH_LEN + 1)  // +1 for the string terminator '\0'


/// @brief This struct holds the information about the provisioned resources
/// 	         of one SecApp
struct sp_resc
{
	char filepath[FS_MAX_PATH_BUFFER_LEN];
	
	vaddr_t sp_sharemem;
	vaddr_t sp_heap;
	vaddr_t sp_params;
	vaddr_t sp_stack;
	vaddr_t sp_app_comm;

	struct mem_alignment_info _sp_scode_info;
	struct mem_alignment_info _sp_sdata_info;
	struct mem_alignment_info _sp_rodata_info;

	struct mem_alignment_info _sp_sharemem_info;
	struct mem_alignment_info _sp_heap_info;
	struct mem_alignment_info _sp_params_info;
	struct mem_alignment_info _sp_stack_info;
	struct mem_alignment_info _sp_app_comm_info;

	// Memory Region
	vaddr_t sp_dma_pool;
	size_t	dma_size;

	struct mem_alignment_info _sp_dma_pool_info;

	size_t 	heap_size;
	size_t	shared_mem_size;

	/// @brief If true, then allocate SecProcess-App communication channels
	bool 	alloc_sp_app_comm;	
};

typedef struct
{
	/// @brief True iff the loader needs SecOS to verify the SecProcess's signature. SecOS can only verify 1st party  
	/// signatures (i.e., signatures signed by us (e.g., CMU)).
	/// [NOTE] SecProcess loader can load a signature and setting <has_1stparty_signature> to be false. SecOS does not
	/// verify the signature in this case.
	bool has_1stparty_signature; 
	char* sp_signature_relative_filepath;
} SP_SIGNATURE_PARAM;

/// @brief Init the log subsystem to use log functions (e.g., APP_INFO, APP_ERR).
/// @param log_relative_filepath 
/// @return 
extern int sp_loader_log_init(const char* log_relative_filepath);

/// @brief Finalize the log subsystem.
/// @param  
extern void sp_loader_log_fini(void);

/// @brief Initialize SecProcess loader library.
/// @param  
/// @return 
extern int sp_loader_init(void);

/// @brief Finalize SecProcess loader library.
/// @param  
extern void sp_loader_fini(void);


extern int apploader_register_secapp(SP_USTRUCT* params);
extern int apploader_unregister_secapp(SP_USTRUCT* params);
extern int apploader_run_sp(spid_t sp_id);
extern int apploader_force_terminate_sp(spid_t sp_id);
extern int apploader_soft_terminate_sp(spid_t sp_id);



extern int prepare_secapp(struct sp_resc* resc, SP_USTRUCT* params, SP_SIGNATURE_PARAM* sp_signature_info);
extern void fini_secapp(struct sp_resc* resc, SP_USTRUCT* params);

/// @brief Assign a memory region to the Secure Domain. The function also locks the memory region 
extern int apploader_register_sp_mem_one(vaddr_t vaddr, size_t size, enum tv_pal_section_type type, paddr_t* out_sp_paddr);




/*********** Util Functions **********/

/// @brief Force memory <ptr> commited in the current process's page table. Untrusted OS does not guarantee newly allocated 
/// memory mapped to their own pages in page tables; e.g., to save physical memory resource by pointing all empty memory to a
/// single page. This function forces allocating of physical memory by writing garbage data to the memory <ptr>
/// 
/// @param ptr 
/// @param len 
/// @return 
extern int app_mem_force_commit_in_pagetable(void *ptr, size_t len);
extern int app_touch_mem(void *ptr, size_t len);





/*********** [TODO] Get Executable Directory  **********/
extern char* apploader_get_exe_directory();


#endif // __ASSEMBLY__
#endif // APP_LOADER_H