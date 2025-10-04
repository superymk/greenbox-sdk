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

#ifndef SECOS_FUNCS_H
#define SECOS_FUNCS_H

#ifndef __ASSEMBLY__

/// @brief Return the status of SecOS
extern OSSTATUS sos_get_status(VKSTATUS* out_status);

extern OSSTATUS do_CMD_RUN_SECOS(struct tz_cmd_packet *cmd_packet);
// extern OSSTATUS do_CMD_RESUME_SECOS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_PROBE_SECOS_STATUS(struct tz_cmd_packet *cmd_packet);

extern OSSTATUS do_CMD_REGISTER_SECOS_PREPARE(struct tz_cmd_packet *cmd_packet);

extern OSSTATUS do_CMD_REGISTER_SECOS(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_UNREGISTER_SECOS(struct tz_cmd_packet *cmd_packet);

extern OSSTATUS do_CMD_TERMINATE_SECOS(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECOS_FUNCS_H