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

#ifndef SECAGENT_PROCESS_H
#define SECAGENT_PROCESS_H

#ifndef __ASSEMBLY__

extern OSSTATUS do_CMD_REGISTER_SECPROCESS_MEMORY(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_REGISTER_SECPROCESS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_RUN_SECPROCESS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_SOFT_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_FORCE_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_NOTIFY_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/********* SecProcess Information Query *********/
extern OSSTATUS do_CMD_SECPROCESS_GET_IDS_AND_NAMES(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_PROCESS_H