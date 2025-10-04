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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wolftpm/tpm2.h>

#include "tpm2_event_log.h"
#include "tcg2_protocol.h"
#include "utils.h"

extern void tpm_event_log_mgr_clear(void);

extern int tpm_event_log_add(
    SECBASE_TPM_EVENT_TYPE ev_type, 
    SECBASE_TPM_DIGEST_ALG digest_algo, 
    uint8_t* digest
);


struct event_log_iterate_info
{
    uint32_t pcr;
    uint16_t digest_algo;

    // Private data for event log iteration
    SECBASE_TPM_EVENT_TYPE ev_type;
};

static SECBASE_TPM_EVENT_TYPE _ev_type_convert(TCG_EVENTTYPE event_type)
{
    switch (event_type)
    {
    case EV_PREBOOT_CERT:
        return SECBASE_TPM_EV_PREBOOT_CERT;
    case EV_POST_CODE:
        return SECBASE_TPM_EV_POST_CODE;
    case EV_NO_ACTION:
        return SECBASE_TPM_EV_NO_ACTION;
    case EV_SEPARATOR:
        return SECBASE_TPM_EV_SEPARATOR;
    case EV_ACTION:
        return SECBASE_TPM_EV_ACTION;
    case EV_EVENT_TAG:
        return SECBASE_TPM_EV_EVENT_TAG;
    case EV_S_CRTM_CONTENTS:
        return SECBASE_TPM_EV_S_CRTM_CONTENTS;
    case EV_S_CRTM_VERSION:
        return SECBASE_TPM_EV_S_CRTM_VERSION;
    case EV_CPU_MICROCODE:
        return SECBASE_TPM_EV_CPU_MICROCODE;
    case EV_PLATFORM_CONFIG_FLAGS:
        return SECBASE_TPM_EV_PLATFORM_CONFIG_FLAGS;
    case EV_TABLE_OF_DEVICES:
        return SECBASE_TPM_EV_TABLE_OF_DEVICES;
    case EV_COMPACT_HASH:
        return SECBASE_TPM_EV_COMPACT_HASH;
    case EV_NONHOST_CODE:
        return SECBASE_TPM_EV_NONHOST_CODE;
    case EV_NONHOST_CONFIG:
        return SECBASE_TPM_EV_NONHOST_CONFIG;
    case EV_NONHOST_INFO:
        return SECBASE_TPM_EV_NONHOST_INFO;
    case EV_OMIT_BOOT_DEVICE_EVENTS:
        return SECBASE_TPM_EV_OMIT_BOOT_DEVICE_EVENTS;
    case EV_EFI_VARIABLE_DRIVER_CONFIG:
        return SECBASE_TPM_EV_EFI_VARIABLE_DRIVER_CONFIG;
    case EV_EFI_VARIABLE_BOOT:
        return SECBASE_TPM_EV_EFI_VARIABLE_BOOT;
    case EV_EFI_BOOT_SERVICES_APPLICATION:
        return SECBASE_TPM_EV_EFI_BOOT_SERVICES_APPLICATION;
    case EV_EFI_BOOT_SERVICES_DRIVER:
        return SECBASE_TPM_EV_EFI_BOOT_SERVICES_DRIVER;
    case EV_EFI_RUNTIME_SERVICES_DRIVER:
        return SECBASE_TPM_EV_EFI_RUNTIME_SERVICES_DRIVER;
    case EV_EFI_GPT_EVENT:
        return SECBASE_TPM_EV_EFI_GPT_EVENT;
    case EV_EFI_ACTION:
        return SECBASE_TPM_EV_EFI_ACTION;
    case EV_EFI_PLATFORM_FIRMWARE_BLOB:
        return SECBASE_TPM_EV_EFI_PLATFORM_FIRMWARE_BLOB;
    case EV_EFI_HANDOFF_TABLES:
        return SECBASE_TPM_EV_EFI_HANDOFF_TABLES;
    case EV_EFI_VARIABLE_AUTHORITY:
        return SECBASE_TPM_EV_EFI_VARIABLE_AUTHORITY;
    default:
        return SECBASE_TPM_EV_INVALID;
    }
}

static SECBASE_TPM_DIGEST_ALG _digest_algo_convert(UINT16 alg_id)
{
    switch (alg_id)
    {
    case TPM_ALG_SHA1:
        return SECBASE_TPM_ALG_SHA1;
    case TPM_ALG_SHA256:
        return SECBASE_TPM_ALG_SHA256;
    case TPM_ALG_SHA384:
        return SECBASE_TPM_ALG_SHA384;
    case TPM_ALG_SHA512:
        return SECBASE_TPM_ALG_SHA512;
    case TPM_ALG_SM3_256:
        return SECBASE_TPM_ALG_SM3_256;
    default:
        return SECBASE_TPM_ALG_INVALID;
    }
}

static bool _event_log_tpm2_process_digest(TCG_DIGEST2 *digest,
                                      void *data)
{
    int ret = 0;
    struct event_log_iterate_info* info = (struct event_log_iterate_info*) data;
    SECBASE_TPM_DIGEST_ALG uxmhf_digest_algo = SECBASE_TPM_ALG_INVALID;

    if (!info)
        return false;

    uxmhf_digest_algo = _digest_algo_convert(digest->AlgorithmId);
    if(uxmhf_digest_algo == SECBASE_TPM_ALG_INVALID)
        return false;

    // Skip SHA1 digests
    if(uxmhf_digest_algo == SECBASE_TPM_ALG_SHA1)
        return true;

    // Add the event if it has valid non-SHA1 digests
    ret = tpm_event_log_add(info->ev_type, uxmhf_digest_algo, digest->Digest);
    if(ret)
    {
        printf("[UXMHF64-ERR] Add to TPM event log error! PCR:%u, Event type:%u, Digest algo:%u\n", info->pcr, info->ev_type, uxmhf_digest_algo);
        return false;
    }

    // On success
    return true;
}

static void _event_log_tpm2_process_event(
    TCG_EVENT_HEADER2 *event_hdr, 
    UINT32 event_idx, 
    struct event_log_iterate_info* info
)
{
    bool ret = true;

    if (!event_hdr)
        return;
    if (!info)
        return;

    // Check: If the event is not a measurement extend to TPM PCR <info->pcr>, then ignore it.  
    if(event_hdr->PCRIndex != info->pcr)
        return;

    info->ev_type = _ev_type_convert(event_hdr->EventType);

    ret = foreach_digest2(event_hdr,
                          _event_log_tpm2_process_digest,
                          info);
    if (!ret)
        return;
}

static bool _event_log_tpm2_iterate_callback(
    TCG_EVENT_HEADER2 *event_hdr,
    UINT32 event_idx,
    void *data)
{
    struct event_log_iterate_info* info = (struct event_log_iterate_info*)data;

    if(!info)
        return false;
    
    _event_log_tpm2_process_event(event_hdr, event_idx, info);
    return true;
}

static void _event_log_find_pcr_extend_trace(
    void* first,
    void* last,
    uint32_t pcr,
    uint16_t digest_algo)
{
    TCG_EVENT_HEADER2 *event = (TCG_EVENT_HEADER2 *)first;
    TCG_EVENT_HEADER2 *event_last = (TCG_EVENT_HEADER2 *)last;
    struct event_log_iterate_info info = {.pcr = pcr, .digest_algo = digest_algo};

    if (event == NULL)
    {
        return;
    }

    foreach_event2(event, event_last, _event_log_tpm2_iterate_callback, &info);
}




/********* Public Functions *********/
int efi_event_tpm2_get_pcr_extend_trace(uint8_t* evl_buf, size_t evl_size, uint32_t pcr_idx)
{
    tpm_event_log_mgr_clear();

    if (!evl_size)
    {
        // Empty log, no PCR measurement trace to collect.
        return 0;
    }

    // Add 1KB trailing zeros to <evl_buf>. Otherwise, <_event_log_find_pcr_extend_trace> may read outside the buffer
    // because its <last> parameter points to the  does not point to the last entry 

    _event_log_find_pcr_extend_trace(evl_buf, &evl_buf[evl_size - 1], pcr_idx, TPM_ALG_SHA256);

    // On success
    return 0;
}