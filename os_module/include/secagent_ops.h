// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_OPS_HEADER_H
#define SECAGENT_OPS_HEADER_H

//! This struct abstract the required common OS functions  
extern struct os_funcs *os;

extern void secagent_mark_running(void);
extern void secagent_mark_not_running(void);
extern bool secagent_is_running(void);

extern OSSTATUS secagent_init(void);

extern void secagent_fini(void);

extern void os_init(void);

extern void os_fini(void);

#ifndef GREENBOX_USE_UXMHF64_V1
OSSTATUS do_CMD_SECBASE_BOOT_TINYOS(struct tz_cmd_packet* cmd_packet);
#endif // !GREENBOX_USE_UXMHF64_V1

extern OSSTATUS do_CMD_SECBASE_INIT_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_SECBASE_FINI_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_SECAGENT_PRESENCE_REQ(struct tz_cmd_packet *cmd_packet);

#endif // SECAGENT_OPS_HEADER_H
