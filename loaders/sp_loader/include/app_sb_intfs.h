// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SECBASE_INTFS_H
#define APP_SECBASE_INTFS_H

#include <secbase_hypercalls.h>

#ifndef __ASSEMBLY__

extern int hypercall_secbase_get_boot_state(NS_HC_PARAMS_GET_SECBASE_BOOT_STATE *out_result);

extern int hypercall_secbase_get_tpm_pcrs(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT *out_result);

int hypercall_secbase_get_unique_machine_id(uint8_t *out_machine_id);

#endif // __ASSEMBLY__
#endif // APP_SECBASE_INTFS_H