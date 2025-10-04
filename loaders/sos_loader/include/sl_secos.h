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

#ifndef SOS_LD_SECOS_H
#define SOS_LD_SECOS_H

#include <sl_status.h>
#include <sl_cfg_reader.h>

#define RUN 1
#define STOP 2

#ifndef __ASSEMBLY__

extern void ld_sos_run_control_set(uint32_t new_cmd);

/// @brief 
/// @param sos_cfg 
/// @param cpu_affinity Must be a different CPU from the CPU for SecOS.
/// @return Return SL_SUCCESS on success. Otherwise, return errors and unload the SecOS.
extern SL_STATUS sl_secos_load(SL_CFG* sos_cfg, uint32_t cpu_affinity);



extern SL_STATUS sl_secos_register_and_run(void);

extern SL_STATUS ld_secos_terminate(void);
extern SL_STATUS ld_secos_unregister(void);
extern void sl_secos_unload(void);

#endif // __ASSEMBLY__
#endif // SOS_LD_SECOS_H

