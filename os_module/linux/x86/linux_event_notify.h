// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_LINUX_EVENT_NOTIFY_H
#define SECAGENT_LINUX_EVENT_NOTIFY_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void linux_os_app_notif_subsystem_init(void);
extern void linux_os_app_notif_subsystem_fini(void);

extern OSSTATUS linux_os_app_notif_register(struct sa_app_notif_reginfo* info);
extern void linux_os_app_notif_unregister(void);

extern void linux_signal_app(process_id pid);

#endif // __ASSEMBLY__
#endif // SECAGENT_LINUX_EVENT_NOTIFY_H