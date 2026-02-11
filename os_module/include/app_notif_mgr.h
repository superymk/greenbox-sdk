// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_APP_EVENTS_MGR_H
#define SECAGENT_APP_EVENTS_MGR_H

#define NUM_SIGINFO_SLOTS       (16)

#ifndef __ASSEMBLY__

struct sa_siginfo
{
    bool polled_by_app;
    process_id app_pid;
    spid_t sp_id;

    ulong_t info1;
    ulong_t info2;
};

/// @brief Initialize the cache of detailed info associated with notifications sent to the red applications
extern void app_notif_mgr_init(void);

/// @brief Finalize the cache of detailed info associated with notifications sent to the red applications
extern void app_notif_mgr_fini(void);

/// @brief Enqueue detailed notification information for an untrusted application
/// @param app_pid PID of the target untrusted application
/// @param sp_id Sender of the notification
/// @param info1 Detailed info
/// @param info2 More detailed info (optional)
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS app_notif_enqueue_info(process_id app_pid, spid_t sp_id, ulong_t info1, ulong_t info2);

/// @brief Dequeue detailed notification information for an untrusted application
/// @param app_pid PID of the target untrusted application
/// @param sp_id Sender of the notification
/// @param info1 Detailed info
/// @param info2 More detailed info (optional)
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS app_notif_dequeue_info(process_id app_pid, spid_t* sp_id, ulong_t* info1, ulong_t* info2);

/// @brief Poll OS notification info from SecAgent
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_POLL_OS_NOTIFICATION_INFO(struct tz_cmd_packet *cmd_packet);

/// @brief Register OS notification handler
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_REGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet);

/// @brief Unregister OS notification handler
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_UNREGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_APP_EVENTS_MGR_H