// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOS_LD_TASKS_AFTER_UNLOAD_H
#define SOS_LD_TASKS_AFTER_UNLOAD_H

#include <sl_status.h>
#include <sl_cfg_reader.h>

#ifndef __ASSEMBLY__

/// @brief Set a task required to run or not after SecOS is successfully unloaded. 
/// @param task_cmd 
/// @param task_need_run 
/// @return 
extern SL_STATUS sl_task_run_after_unload_secos_set(uint32_t task_cmd, bool task_need_run);

/// @brief Run all tasks required to run after SecOS is successfully unloaded.
/// @param  
/// @return 
extern SL_STATUS sl_tasks_run_after_unload_secos(void);

#endif // __ASSEMBLY__
#endif // SOS_LD_TASKS_AFTER_UNLOAD_H

