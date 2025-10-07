// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SECAPP_CONFIG_H
#define APP_SECAPP_CONFIG_H

/* Memory pool for ordinary use */
#define SAP_APP_COMM_SIZE			KB(64)
#define SAP_PARAMS_SIZE             SIZE_ROUNDUP_4K(sizeof(SP_UTASK))
#define SAP_STACK_SIZE 				KB(24)

#endif // APP_SECAPP_CONFIG_H

