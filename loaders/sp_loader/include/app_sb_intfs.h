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

#ifndef APP_SECBASE_INTFS_H
#define APP_SECBASE_INTFS_H

#include <secbase_hypercalls.h>

#ifndef __ASSEMBLY__

extern int hypercall_secbase_get_boot_state(NS_HC_PARAMS_GET_SECBASE_BOOT_STATE *out_result);

extern int hypercall_secbase_get_tpm_pcrs(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT *out_result);

int hypercall_secbase_get_unique_machine_id(uint8_t *out_machine_id);

#endif // __ASSEMBLY__
#endif // APP_SECBASE_INTFS_H