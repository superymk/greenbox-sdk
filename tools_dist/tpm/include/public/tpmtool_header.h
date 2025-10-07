// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef TOOL_SECBASE_TPM_TOOLS_H
#define TOOL_SECBASE_TPM_TOOLS_H

#ifndef __ASSEMBLY__

#define TPM_QUOTE_NONCE_STRING_MAX_LENGTH   (20)
#define TPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH   (12)

/// @brief Max parameter length of the input BASE64 encoded string for TPM quote nonce.
#define TPM_QUOTE_NONCE_BASE64_MAX_LENGTH   (192)

#define SECBASE_TPM_PCR_DIGEST_LEN  (64)

struct tpm_quote_nonce
{
    char nonce_str[TPM_QUOTE_NONCE_STRING_MAX_LENGTH];
    unsigned char freshness[TPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH];
};

#endif // __ASSEMBLY__
#endif // TOOL_SECBASE_TPM_TOOLS_H