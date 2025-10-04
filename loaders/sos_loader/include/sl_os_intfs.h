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

#ifndef SOS_LD_OS_INTERFACE_H
#define SOS_LD_OS_INTERFACE_H

#include <vk_errno.h>

#ifndef __ASSEMBLY__

/*extern paddr_t secagent_get_lapic_base(void);
extern paddr_t secagent_get_ioapic_base(void);*/

// extern uint32_t secagent_trigger_all_intrs(u32* intr_map);

/********* SecOS Functions in SecAgent*********/
//! Register SecOS
extern uint32_t secagent_register_secos(struct sl_sos_sect_addrs* sos_addrs);
//! Unregister SecOS
extern uint32_t secagent_unregister_secos(void);
//! Run SecOS
extern uint32_t secagent_run_secos(void);
//! Get SecOS's most recent return status
VKSTATUS secagent_get_secos_status(bool* secagent_success);
//! Terminate SecOS
extern uint32_t secagent_terminate_secos(void);

// [TODO] Ugly hack. We split this one from CMD_REGISTER_SECOS 
// to make sure these initialization are finished before the hypercall in <CMD_REGISTER_SECOS>.
// Later we should merge them with the <CMD_REGISTER_SECOS>.
extern uint32_t secagent_register_secos_prepare(void);




/********* SecProcess Information Query *********/
extern uint32_t secagent_sps_get_ids_and_names(struct sp_info_id_name_resp* resp_buf);




/********* Other Functions in SecAgent*********/
extern uint32_t secagent_init_secbase_ondemand_percpu(void);
extern uint32_t secagent_fini_secbase_ondemand_percpu(void);
extern uint32_t secagent_x86_init_collect_lapic_id_percpu(void);

extern uint32_t secagent_x86_os_irq_get_intr_vector(uint32_t irq, uint32_t* out_intr);
extern uint32_t secagent_os_trigger_intr(uint32_t intr);

// extern uint32_t secagent_disable_local_intr(void);

// extern uint32_t secagent_enable_local_intr(void);

// extern uint32_t secagent_create_secapp(offset_t sp_param_offset_in_sharedmem);

// extern uint32_t secagent_run_secapp(spid_t sp_id);

// extern vaddr_t secagent_req_dma_pool(uint32_t size);
// extern int secagent_free_dma_pool(vaddr_t vaddr, uint32_t size);

#endif // __ASSEMBLY__
#endif // SOS_LD_OS_INTERFACE_H