// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_NOTIFY_TO_APP_H
#define SECAGENT_NOTIFY_TO_APP_H

#include <app_types.h>

#ifndef __ASSEMBLY__
/// @brief This function must be implemented by the untrusted application
extern void on_secagent_notify(void);

extern bool app_sa_notify_init(void);
extern void app_sa_notify_fini(void);

#endif // __ASSEMBLY__
#endif // SECAGENT_NOTIFY_TO_APP_H