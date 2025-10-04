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

#ifndef SECAGENT_ARCH_X86_ARCH_H
#define SECAGENT_ARCH_X86_ARCH_H

#ifndef __ASSEMBLY__

extern void __x86_os_cpu_lapic_mapping_init(void);

extern void __x86_os_cpu_lapic_mapping_fini(void);

extern bool __x86_os_cpu_lapic_mapping_find(uint32_t cpu_id, lapic_id_t* out_lapic_id);

extern OSSTATUS do_CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_ARCH_X86_ARCH_H