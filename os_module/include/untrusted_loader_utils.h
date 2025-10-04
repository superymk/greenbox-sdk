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

#ifndef SECAGENT_UNTRUSTED_LOADER_UTILS_H
#define SECAGENT_UNTRUSTED_LOADER_UTILS_H

extern OSSTATUS nsdom_utils_init(void);

extern void nsdom_utils_fini(void);

extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE(struct tz_cmd_packet *cmd_packet);

#endif // SECAGENT_UNTRUSTED_LOADER_UTILS_H
