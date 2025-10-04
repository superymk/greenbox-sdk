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