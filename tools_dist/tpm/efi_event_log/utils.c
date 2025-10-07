// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* SPDX-License-Identifier: BSD-2 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wolftpm/tpm2.h>

#include "utils.h"

#define SHA1_LENGTH        20
#define SHA256_LENGTH      32
#define SM3_LENGTH         32
#define SHA384_LENGTH      48
#define SHA512_LENGTH      64

void PrintHex(const UINT8* buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}

size_t get_alg_size(UINT16 alg_id)
{
    switch (alg_id)
    {
    case TPM_ALG_SHA1:
        return SHA1_LENGTH;
    case TPM_ALG_SHA256:
        return SHA256_LENGTH;
    case TPM_ALG_SHA384:
        return SHA384_LENGTH;
    case TPM_ALG_SHA512:
        return SHA512_LENGTH;
    case TPM_ALG_SM3_256:
        return SM3_LENGTH;
    default:
        return 0;
    }
}

TCG_DIGEST2* get_next_digest(TCG_DIGEST2 *digest)
{
    return (TCG_DIGEST2 *)(digest->Digest + get_alg_size(digest->AlgorithmId));
}

size_t sizeof_digest2(TCG_DIGEST2 *digest)
{
    return sizeof(*digest) + get_alg_size(digest->AlgorithmId);
}

bool foreach_digest2(TCG_EVENT_HEADER2 *event_hdr,
                     DIGEST2_CALLBACK callback,
                     void *data)
{
    bool ret = true;

    TCG_DIGEST2 *digest = (TCG_DIGEST2 *)event_hdr->Digests;
    for (size_t i = 0; i < event_hdr->DigestCount && digest != NULL; ++i)
    {
        ret = callback(digest, data);
        if (!ret)
            break;
        digest = get_next_digest(digest);
    }
    return ret;
}

static bool digest2_accumulator_callback(TCG_DIGEST2 *digest,
                                         void *data)
{
    size_t *size = (size_t *)data;
    if (size == NULL)
        return false;

    *size += sizeof_digest2(digest);
    return true;
}

TCG_EVENT_HEADER2 *get_next_event(TCG_EVENT_HEADER2 *event_hdr)
{
    size_t digests_size = 0;
    bool ret;
    TCG_EVENT2 *event;

    ret = foreach_digest2(event_hdr,
                          digest2_accumulator_callback,
                          &digests_size);
    if (!ret)
        return NULL;
    event = (TCG_EVENT2 *)((uintptr_t)event_hdr->Digests + digests_size);
    return (TCG_EVENT_HEADER2 *)((uintptr_t)event->Event + event->EventSize);
}

bool foreach_event2(TCG_EVENT_HEADER2 *event_first,
                    TCG_EVENT_HEADER2 *event_last,
                    EVENT2_CALLBACK callback,
                    void *data)
{
    TCG_EVENT_HEADER2 *event;
    UINT32 event_idx = 0;

    if (event_first == NULL || event_last == NULL || callback == NULL)
        return false;

    for (event = event_first;
         event <= event_last && event != NULL;
         event = get_next_event(event))
    {
        if (!callback(event, event_idx, data))
            return false;
        event_idx++;
    }
    return true;
}