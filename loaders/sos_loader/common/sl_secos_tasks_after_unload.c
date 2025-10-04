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

#include <appddk.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sl_headers.h>

#include "../hash/hash.h"
#include "sl_sb_tpm.h"

/************ Forward Declarations **********/
static SL_STATUS _do_TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL(void);

/************ Type Definitions **********/
/// @brief Max number of task table entries.
#define TASKS_TABLE_MAX_ENTRIES   (32)

typedef SL_STATUS (*TASK_FUNC)(void);

struct task_info
{
    uint32_t task_cmd;
    bool task_need_run;
    TASK_FUNC task_func;
};

/************ Local Data **********/
/// @brief Each entry represents a task's information. Different tasks have different <task_cmd>.
static struct task_info tasks_table[TASKS_TABLE_MAX_ENTRIES] =
{
    {TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL, false, _do_TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL}
};

/************ Private Functions **********/
static SL_STATUS _do_TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL(void)
{
    return _do_sl_secbase_tpm_seal();
}

/************ Public Functions **********/
SL_STATUS sl_task_run_after_unload_secos_set(uint32_t task_cmd, bool task_need_run)
{
    uint32_t i = 0;
    bool task_found = false;

    FOREACH_S(i, TASKS_TABLE_MAX_ENTRIES, TASKS_TABLE_MAX_ENTRIES, 0, 1)
    {
        struct task_info* task = &tasks_table[i];

        if(!task)
        {
            // Should not hit
            return SL_EINTERNAL;
        }
            
        if(task->task_cmd == task_cmd)
        {
            // Update the info of the first found task. This is sufficient because different tasks have different 
            // <task_cmd>.
            task->task_need_run = task_need_run;
            task_found = true;
            break;
        }
    }

    // If not found the task <task_cmd>, then tell the caller about the invalid parameters.
    if(!task_found)
        return SL_EINVAL;

    // On success
    return SL_SUCCESS;
}

SL_STATUS sl_tasks_run_after_unload_secos(void)
{
    SL_STATUS status = SL_SUCCESS;
    uint32_t i = 0;

    FOREACH_S(i, TASKS_TABLE_MAX_ENTRIES, TASKS_TABLE_MAX_ENTRIES, 0, 1)
    {
        struct task_info* task = &tasks_table[i];

        if(!task)
        {
            // Should not hit
            return SL_EINTERNAL;
        }
            
        if(task->task_need_run && task->task_func)
        {
            status = task->task_func();
            if(SL_FAIL(status))
            {
                // Continue to run other tasks even if error happens.
                SL_ERR("Tasks After SecOS Unload: Failed to run Task (id:%u)! status:%u\n", task->task_cmd, status);
            }
        }
    }

    // On success
    return SL_SUCCESS;
}

