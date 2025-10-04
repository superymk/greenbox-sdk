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

#ifndef _TPM2_EVENT_LOG_H
#define _TPM2_EVENT_LOG_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <os/os_headers.h>
#include <app_types.h>
#include <secbase_hypercalls.h>
#include <public/tpmtool_header.h>

#ifndef __ASSEMBLY__

/// @brief Parse UEFI event log of TPM2.X.
/// @param pcr 
/// @return 
extern int efi_event_tpm2_get_pcr_extend_trace(uint8_t* evl_buf, size_t evl_size, uint32_t pcr_idx);

extern SECBASE_TPM_EVENT_LOG_TRACE* tpm_event_log_get_trace(void);

#endif // __ASSEMBLY__
#endif // _TPM2_EVENT_LOG_H