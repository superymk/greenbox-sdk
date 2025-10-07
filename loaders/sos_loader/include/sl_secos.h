// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

