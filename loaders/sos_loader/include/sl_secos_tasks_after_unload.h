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

