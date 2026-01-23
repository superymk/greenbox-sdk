// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_EVENT_NOTIFY_H
#define SECAGENT_WIN_EVENT_NOTIFY_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

/// @brief Initialize the app notification subsystem
extern void win_os_app_notif_subsystem_init(void);

/// @brief Finalize the app notification subsystem
extern void win_os_app_notif_subsystem_fini(void);

/// @brief Register an app event for notifications
/// @param info Registration info with event handle
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS win_os_app_notif_register(struct sa_app_notif_reginfo *info);

/// @brief Unregister app event for current process
extern void win_os_app_notif_unregister(void);

/// @brief Signal a registered app event by PID
/// @param pid Process ID to signal
extern void win_signal_app(process_id pid);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_EVENT_NOTIFY_H