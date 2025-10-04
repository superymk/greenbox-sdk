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

#ifndef SA_DEVICE_PS2_H
#define SA_DEVICE_PS2_H

#include <os_types.h>



#ifndef __ASSEMBLY__
extern OSSTATUS do_CMD_IOSM_PS2_DRAIN_BUFFERS(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__

#endif // SA_DEVICE_PS2_H