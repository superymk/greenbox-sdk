// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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