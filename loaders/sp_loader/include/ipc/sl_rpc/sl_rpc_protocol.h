// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file Protocol for SOSLoader client and server.
/// SOSLoader server runs with the admin/root privilege. SOSLoader clients run with the user privilege.

#ifndef SL_PROTOCOL_H
#define SL_PROTOCOL_H

#include <app_types.h>
// #include <sl_status.h>
#include <app_var.h>
#include <vk_common_intfs.h>
#include <secapp_defs.h>

#include <tpmtool/include/tpmtool_header.h>

#define SL_SERVER_PORT      (2204)
#define SL_SERVER_PORT_STR      "2204"

#define MAX_JSON_MSG_SIZE          (8192)

/// @brief Client uses this command test SL_SERVER is alive or not. Server returns SL_SUCCESS if the client sent 
/// <SL_PING_REQUEST>
#define SL_SERVER_CMD_sl_ping                                       (1)
    #define SL_PING_REQUEST                                             (0xBABECAFE)
#define SL_SERVER_CMD_sl_init                                       (2)
#define SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs             (3)
#define SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs     (4)
#define SL_SERVER_CMD_sl_fini                                       (5)
#define SL_SERVER_CMD_sl_secprocesses_soft_terminate                (6)
#define SL_SERVER_CMD_sl_secprocesses_get_ids_and_names             (7)

#define SL_SERVER_CMD_sl_get_dtm_status                             (8)
#define SL_SERVER_CMD_sl_dump_dmesg                                 (9)

/// @brief STASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL successfully unloaded. 
#define SL_SERVER_CMD_sl_mark_tasks_after_secos_unload           (10)
    #define TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL             (1)  

// #define SL_SERVER_CMD_sl_dev_usb_hc_reset                           (10)

#define SL_SERVER_CMD_secbase_tpm_create_keys                       (20)
#define SL_SERVER_CMD_secbase_tpm_get_quote                         (21)
#define SL_SERVER_CMD_secbase_tpm_get_pcrs                          (22)
#define SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace              (23)

#ifndef __ASSEMBLY__

#endif // __ASSEMBLY__
#endif // SL_PROTOCOL_H