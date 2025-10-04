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

#ifndef SECAGENT_WIN_EVENT_NOTIFY_H
#define SECAGENT_WIN_EVENT_NOTIFY_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void win_os_app_notif_subsystem_init(void);
extern void win_os_app_notif_subsystem_fini(void);

extern OSSTATUS win_os_app_notif_register(struct sa_app_notif_reginfo* info);
extern void win_os_app_notif_unregister(void);

extern void win_signal_app(process_id pid);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_EVENT_NOTIFY_H