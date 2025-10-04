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

#ifndef SECAGENT_OPS_HEADER_H
#define SECAGENT_OPS_HEADER_H

//! This struct abstract the required common OS functions  
extern struct os_funcs *os;

extern void secagent_mark_running(void);
extern void secagent_mark_not_running(void);

extern OSSTATUS secagent_init(void);

extern void secagent_fini(void);

extern void os_init(void);

extern void os_fini(void);

extern OSSTATUS do_CMD_SECBASE_INIT_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_SECBASE_FINI_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_SECAGENT_PRESENCE_REQ(struct tz_cmd_packet *cmd_packet);

#endif // SECAGENT_OPS_HEADER_H
