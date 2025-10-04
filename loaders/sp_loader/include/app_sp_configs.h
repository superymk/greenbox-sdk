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

#ifndef APP_SECAPP_CONFIG_H
#define APP_SECAPP_CONFIG_H

/* Memory pool for ordinary use */
#define SAP_APP_COMM_SIZE			KB(64)
#define SAP_PARAMS_SIZE             SIZE_ROUNDUP_4K(sizeof(SP_UTASK))
#define SAP_STACK_SIZE 				KB(24)

#endif // APP_SECAPP_CONFIG_H

