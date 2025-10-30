// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

static struct sa_siginfo infos[NUM_SIGINFO_SLOTS];
static lock_t infos_lock; // Lock to update <infos>

static bool find_free_siginfo_slot(uint32_t *out_idx)
{
    uint32_t i = 0;

    if(!out_idx)
        return false;

    FOREACH_S(i, NUM_SIGINFO_SLOTS, NUM_SIGINFO_SLOTS, 0, 1)
    {
        struct sa_siginfo* info = &infos[i];
        if(info->polled_by_app)
        {
            *out_idx = i;
            return true;
        }
    }

    return false;
}

static bool find_siginfo_slot_by_app_pid(process_id app_pid, uint32_t *out_idx)
{
    uint32_t i = 0;

    if(!out_idx)
        return false;

    FOREACH_S(i, NUM_SIGINFO_SLOTS, NUM_SIGINFO_SLOTS, 0, 1)
    {
        struct sa_siginfo* info = &infos[i];
        if(info->app_pid == app_pid)
        {
            *out_idx = i;
            return true;
        }
    }

    return false;
}

/// @brief Init the cache of detailed info associated with events sent to the red applications.
/// @param  
void app_notif_mgr_init(void)
{
    uint32_t i = 0;

    // Init <infos_lock>
    os->spinlock_init(&infos_lock);

    FOREACH_S(i, NUM_SIGINFO_SLOTS, NUM_SIGINFO_SLOTS, 0, 1)
    {
        struct sa_siginfo* info = &infos[i];

        info->polled_by_app = true;
        info->app_pid = INVALID_UNTRUSTED_PROC_ID;
        info->sp_id = INVALID_SP_ID;
        info->info1 = 0;
        info->info2 = 0;
    }
}

/// @brief Finalize the cache of detailed info associated with events sent to the red applications.
/// @param  
void app_notif_mgr_fini(void)
{
    uint32_t i = 0;

    FOREACH_S(i, NUM_SIGINFO_SLOTS, NUM_SIGINFO_SLOTS, 0, 1)
    {
        struct sa_siginfo* info = &infos[i];

        info->polled_by_app = true;
        info->app_pid = INVALID_UNTRUSTED_PROC_ID;
        info->sp_id = INVALID_SP_ID;
        info->info1 = 0;
        info->info2 = 0;
    }
}

/// @brief Enqueue the detailed information of the notification sent to the untrusted application <app_pid>.
/// @param app_pid PID of the target untrusted application.
/// @param sp_id Sender of the notification.
/// @param info1 Detailed info.
/// @param info2 More detailed info, not always exist.
/// @return 
OSSTATUS app_notif_enqueue_info(process_id app_pid, spid_t sp_id, ulong_t info1, ulong_t info2)
{
    OSSTATUS status = OS_SUCCESS;
    bool has_free_slot = false;
    uint32_t slot_idx = 0;
    struct sa_siginfo* info = NULL;

    // Check params
    if(app_pid == INVALID_UNTRUSTED_PROC_ID)
        return E_INV_ARG;

    // Acquire lock
    os->spinlock_lock(&infos_lock);

    // Drop new siginfo if there is no space
    has_free_slot = find_free_siginfo_slot(&slot_idx);
    if(!has_free_slot)
    {
        SACOMM_WARN("Failed to enqueue incoming signal sending to untrusted process (%lu)! No space to store the info\n", app_pid);
        status = E_INTERNAL_ERROR;
        goto out;
    }

    // Store the siginfo
    info = &infos[slot_idx];
    info->polled_by_app = false;
    info->app_pid = app_pid;
    info->sp_id = sp_id;
    info->info1 = info1;
    info->info2 = info2;

    status = OS_SUCCESS;

out:
    // Release lock
    os->spinlock_unlock(&infos_lock);
    return status;
}

/// @brief Dequeue the detailed information of the notification sent to the untrusted application <app_pid>.
/// @param app_pid PID of the target untrusted application.
/// @param sp_id Sender of the notification.
/// @param info1 Detailed info.
/// @param info2 More detailed info, not always exist.
/// @return 
OSSTATUS app_notif_dequeue_info(process_id app_pid, spid_t* sp_id, ulong_t* info1, ulong_t* info2)
{
    OSSTATUS status = OS_SUCCESS;
    bool found = false;
    uint32_t slot_idx = 0;
    struct sa_siginfo* info = NULL;

    // Check params
    if(app_pid == INVALID_UNTRUSTED_PROC_ID)
        return E_INV_ARG;
    if(!sp_id || !info1 || !info2)
        return E_INV_ARG;

    // Acquire lock
    os->spinlock_lock(&infos_lock);

    // Find the siginfo for the untrusted process <app_pid>
    found = find_siginfo_slot_by_app_pid(app_pid, &slot_idx);
    if(!found)
    {
        status = E_INV_ARG;
        goto out;
    }

    // Dequeue the siginfo
    info = &infos[slot_idx];
    info->polled_by_app = true;
    *sp_id = info->sp_id;
    *info1 = info->info1;
    *info2 = info->info2;

    // Clear the slot
    info->polled_by_app = true;
    info->app_pid = INVALID_UNTRUSTED_PROC_ID;
    info->sp_id = INVALID_SP_ID;
    info->info1 = 0;
    info->info2 = 0;

    status = OS_SUCCESS;

out:
    // Release lock
    os->spinlock_unlock(&infos_lock);
    return status;
}

OSSTATUS do_CMD_POLL_OS_NOTIFICATION_INFO(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    process_id cur_pid = INVALID_UNTRUSTED_PROC_ID;
    struct sa_app_notif_info result;
    vaddr_t result_uvaddr = INVALID_VADDR;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet || !cmd_packet->arg1)
        return E_INV_ARG;
    result_uvaddr = (vaddr_t)cmd_packet->arg1;

    // Read out the siginfo stored in SecAgent
    cur_pid = os->get_current_pid();
    status = app_notif_dequeue_info(cur_pid, &result.sp_id, &result.info1, &result.info2);
    if (SA_FAIL(status))
        return E_SIG_NO_MORE;

    // Copy siginfo into the result buffer provided by the calling untrusted process.
    status = os->copy_to_user((void*) result_uvaddr, &result, sizeof(struct sa_app_notif_info)); 
    if (SA_FAIL(status))
    {
        SACOMM_ERR("Poll notification info from SecAgent error! Copy error\n");
        return E_INTERNAL_ERROR;
    }

    return OS_SUCCESS;
}

OSSTATUS do_CMD_REGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct sa_app_notif_reginfo notif_reginfo = {0};
    struct sa_app_notif_reginfo* in_notif_reginfo_ptr = NULL;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet || !cmd_packet->arg1)
        return E_INV_ARG;
    in_notif_reginfo_ptr = (struct sa_app_notif_reginfo*)(vaddr_t)cmd_packet->arg1;

    // Copy parameters
    status = os->copy_from_user(&notif_reginfo, in_notif_reginfo_ptr, sizeof(struct sa_app_notif_reginfo));
    if (SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_OS_NOTIFICATION] Copy user params error!\n");
        status = E_INTERNAL_ERROR;
        goto copy_params_err;
    }
    
    status = os->os_app_notif_register(&notif_reginfo);
    if (SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_OS_NOTIFICATION] Copy user params error!\n");
        status = E_INTERNAL_ERROR;
        goto notif_register_err;
    }

    // On success
    DRV_DEBUG("[IOCTL-CMD_REGISTER_OS_NOTIFICATION] Success\n");
    return OS_SUCCESS;
    

notif_register_err:
copy_params_err:
    return status;
}

OSSTATUS do_CMD_UNREGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet)
{
    (void)cmd_packet;
    os->os_app_notif_unregister();

    return OS_SUCCESS;
}