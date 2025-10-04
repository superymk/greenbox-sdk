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