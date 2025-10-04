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

#include "tpm2_event_log.h"
#include "tcg2_protocol.h"

#define TPM_EVENT_ENTRIES_MAX       (16)

#define IS_VALID_PCR(pcr) (((uint32_t)(pcr)) < 24)

#define SHA1_LENGTH        20
#define SHA256_LENGTH      32
#define SM3_LENGTH         32
#define SHA384_LENGTH      48
#define SHA512_LENGTH      64

// Secure foreach
// we should use it as much as possible for every for statement
#define FOREACH_S(var_name, m1, m2, initial, step) for(var_name = (initial); (var_name < (m1)) && (var_name < (m2)); var_name += (step))

typedef union {
    uint8_t    sha1[SHA1_LENGTH];
    uint8_t    sha256[SHA256_LENGTH];
    uint8_t    sm3[SM3_LENGTH];
    uint8_t    sha384[SHA384_LENGTH];
    uint8_t    sha512[SHA512_LENGTH];
} TPM_DIGEST;

// static SECBASE_TPM_EVENT_LOG_ENTRY _trace.entries[TPM_EVENT_ENTRIES_MAX] = {0};
static SECBASE_TPM_EVENT_LOG_TRACE _trace = {0};

static void _records_clear()
{
    uint32_t i = 0;

    FOREACH_S(i, TPM_EVENT_ENTRIES_MAX, TPM_EVENT_ENTRIES_MAX, 0, 1)
    {
        _trace.entries[i].ev_type = SECBASE_TPM_EV_INVALID;

        _trace.entries[i].digest_algo = SECBASE_TPM_ALG_INVALID;

        memset(&_trace.entries[i].digest, 0, sizeof(TPM_DIGEST));
        _trace.entries[i].digest_size = 0;
    }

    _trace.count = 0;
}

static size_t _get_digest_size(SECBASE_TPM_DIGEST_ALG alg_id)
{
    switch (alg_id)
    {
    case SECBASE_TPM_ALG_SHA1:
        return SHA1_LENGTH;
    case SECBASE_TPM_ALG_SHA256:
        return SHA256_LENGTH;
    case SECBASE_TPM_ALG_SHA384:
        return SHA384_LENGTH;
    case SECBASE_TPM_ALG_SHA512:
        return SHA512_LENGTH;
    case SECBASE_TPM_ALG_SM3_256:
        return SM3_LENGTH;
    default:
        return 0;
    }
}




/********* Public Functions *********/
void tpm_event_log_mgr_clear(void)
{
    _records_clear();
}

int tpm_event_log_add(
    SECBASE_TPM_EVENT_TYPE ev_type, 
    SECBASE_TPM_DIGEST_ALG digest_algo, 
    uint8_t* digest
)
{
    uint32_t insert_idx = _trace.count;
    size_t digest_size = 0;

    // Check: params must be valid
    if((ev_type == SECBASE_TPM_EV_INVALID) || (digest_algo == SECBASE_TPM_ALG_INVALID))
        return 1;
    if(!digest)
        return 1;

    // Check: the records must have space
    if(_trace.count >= TPM_EVENT_ENTRIES_MAX)
        return 1;

    // Add the record
    _trace.entries[insert_idx].ev_type = ev_type;
    _trace.entries[insert_idx].digest_algo = digest_algo;

    digest_size = _get_digest_size(digest_algo);
    if(!digest_size)
        return 1;

    memcpy(&_trace.entries[insert_idx].digest, digest, digest_size);
    _trace.entries[insert_idx].digest_size = digest_size;

    _trace.count++;

    // On success
    return 0;
}

SECBASE_TPM_EVENT_LOG_TRACE* tpm_event_log_get_trace(void)
{
    return &_trace;
}

// uint32_t tpm_event_log_get_count(void)
// {
//     return _trace.count;
// }

// MHV_STATUS tpm_event_log_get(
//     uint32_t log_idx, 
//     uint32_t* out_pcr_idx, 
//     SECBASE_TPM_EVENT_TYPE* out_ev_type, 
//     SECBASE_TPM_DIGEST_ALG* out_digest_algo, 
//     uint8_t* out_digest,
//     uint32_t* out_digest_size
// )
// {
//     // Check: params must be valid
//     if(log_idx >= _trace.count)
//         return MHV_EINVAL;
//     if(!out_pcr_idx || !out_ev_type || !out_digest_algo || !out_digest || !out_digest_size)
//         return MHV_EINVAL;

//     *out_pcr_idx = _trace.entries[log_idx].pcr_idx;
//     *out_ev_type = _trace.entries[log_idx].ev_type;
//     *out_digest_algo = _trace.entries[log_idx].digest_algo;

//     *out_digest_size = _trace.entries[log_idx].digest_size;
//     memcpy(out_digest, &_trace.entries[log_idx].digest, *out_digest_size);

//     // On success
//     return MHV_SUCCESS;
// }