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

/// @brief Initialize the cache of detailed info associated with notifications sent to the red applications.
/// @param 
extern void app_notif_mgr_init(void);

/// @brief Finalize the cache of detailed info associated with notifications sent to the red applications.
/// @param  
extern void app_notif_mgr_fini(void);

/// @brief Enqueue the detailed information of the notification sent to the untrusted application <app_pid>.
/// @param app_pid PID of the target untrusted application.
/// @param sp_id Sender of the notification.
/// @param info1 Detailed info.
/// @param info2 More detailed info, not always exist.
/// @return 
extern OSSTATUS app_notif_enqueue_info(process_id app_pid, spid_t sp_id, ulong_t info1, ulong_t info2);

/// @brief Dequeue the detailed information of the notification sent to the untrusted application <app_pid>.
/// @param app_pid PID of the target untrusted application.
/// @param sp_id Sender of the notification.
/// @param info1 Detailed info.
/// @param info2 More detailed info, not always exist.
/// @return 
extern OSSTATUS app_notif_dequeue_info(process_id app_pid, spid_t* sp_id, ulong_t* info1, ulong_t* info2);

extern OSSTATUS do_CMD_POLL_OS_NOTIFICATION_INFO(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_REGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_UNREGISTER_OS_NOTIFICATION(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_APP_EVENTS_MGR_H