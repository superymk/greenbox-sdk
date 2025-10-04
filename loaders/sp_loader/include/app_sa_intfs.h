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

#ifndef APP_SECAGENT_INTFS_H
#define APP_SECAGENT_INTFS_H

#ifndef __ASSEMBLY__
extern bool secagent_test_presence(void);
extern void secagent_test1(void);
extern void secagent_test2(ulong_t arg1);
extern void secagent_test3(void);
extern void secagent_print(uint32_t tag);

/// @brief Share data with any NS domain's process. The current will create map <selector, data> in SecAgent. Any
/// proceseses can get the data with the key <selector>.
/// @param selector 
/// @param data 
/// @return 
extern OSSTATUS secagent_nsdom_share_data_to_any_ns_process_set(uint64_t selector, uint64_t data);

/// @brief Get shared data with any NS domain's process. The current will create map <selector, data> in SecAgent. Any
/// proceseses can get the data with the key <selector>.
/// @param selector 
/// @param data 
/// @return 
extern OSSTATUS secagent_nsdom_share_data_to_any_ns_process_get(uint64_t selector, uint64_t* data);

extern OSSTATUS secagent_nsdom_global_spinlock_try_acquire(uint64_t selector, bool* lock_got);

extern OSSTATUS secagent_nsdom_global_spinlock_release(uint64_t selector);


// extern OSSTATUS secagent_app_cache_flush_range(vaddr_t vaddr, size_t sz, bool flush_all_data_cache_on_failure);
extern paddr_t secagent_vaddr_to_paddr(vaddr_t vaddr);
extern spid_t secagent_allocate_sp_id(void);
extern OSSTATUS secagent_deallocate_sp_id(spid_t sp_id);
//! @brief Send a request to the SecProcess <sp_id>
extern OSSTATUS secagent_notify_secprocess(spid_t sp_id);

extern OSSTATUS secagent_register_sp_mem(struct hv_reg_sections* sect);
extern OSSTATUS secagent_register_sp(SP_USTRUCT* sp_params);
extern OSSTATUS secagent_run_sp(spid_t sp_id);
extern OSSTATUS secagent_force_terminate_sp(spid_t sp_id);
extern OSSTATUS secagent_soft_terminate_sp(spid_t sp_id);

extern OSSTATUS secagent_register_app_notification(struct sa_app_notif_reginfo* reginfo);
extern OSSTATUS secagent_unregister_app_notification(void);
extern OSSTATUS secagent_poll_notification_info(struct sa_app_notif_info* out_info);

// extern vaddr_t secagent_req_dma_pool(uint32_t size);
// extern vaddr_t secagent_req_kern_pages(uint32_t size);
// extern int secagent_free_dma_pool(vaddr_t vaddr, uint32_t size);
// extern int secagent_free_kern_pages(vaddr_t vaddr, uint32_t size);

// SecOS resource provisioning/management
extern paddr_t secagent_try_alloc_sos_phy_mem(size_t size);
extern void secagent_try_free_sos_phy_mem(paddr_t paddr_in_sos);

// Trusted display
/*extern int secagent_init_trusted_display(uint32_t width, uint32_t height, vaddr_t* td_vmem_base, size_t* td_vmem_size);
extern int secagent_fini_trusted_display(void); */

#endif // __ASSEMBLY__
#endif // APP_SECAGENT_INTFS_H