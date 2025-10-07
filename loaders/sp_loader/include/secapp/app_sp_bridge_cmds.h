// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef HID_BRIDGE_CMDS_H
#define HID_BRIDGE_CMDS_H

#include <secbase.h>
#include <secbase_errno.h>
#include <secbase_hypercalls.h>

// Keyboard/mouse events
#define CMD_HID                             (1000)
#define CMD_HID_NEW_EVENT                   (CMD_HID + 1)
#define CMD_HID_RED_OS_RELEASE_INPUT        (CMD_HID + 2)
#define CMD_HID_DTM_REQ_HIGH_PRIORITY       (CMD_HID + 10)

#define CMD_GUI                             (2000)
#define CMD_GUI_NEW_WINDOW_POS              (CMD_GUI + 1)
#define CMD_GUI_WINDOW_HIDE                 (CMD_GUI + 10)
#define CMD_GUI_WINDOW_RESTORE              (CMD_GUI + 11)

// TPM Ops
#define CMD_TPM                             (3000)
#define CMD_TPM_QUOTE                       (CMD_TPM + 1)
#define CMD_TPM_QUOTE_FREE                  (CMD_TPM + 2)
/// @brief SecProcess requests SOSLoader to seal secbase secrets right after SecOS termination
#define CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD    (CMD_TPM + 3)

#ifndef __ASSEMBLY__

/********* TPM Ops *********/

#define TPM_QUOTE_NONCE_STRING_MAX_LENGTH   (20)   // Must be the same with TPM_QUOTE_NONCE_STRING_MAX_LENGTH of TPMTool
#define TPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH   (12) // Must be the same with TPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH of TPMTool

/// @brief Length of TPM RSA2048 Attestation Identity Key (AIK) in DER format.
#define TPM2_AIK_RSA2048_DER_LENGTH  (294)
#define SHA256_HASH_LENGTH    (32)
#define TPM2_QUOTE_BLOB_SIZE  (256)
#define TPM2_QUOTE_SIGNATURE_LENGTH  (256)

#define MHV_SENSITIVE_STATE_MAX_SIZE    (1024) // Must be the same with <SECBASE_BOOT_STATE_SIZE> defined in SecBase

// Values for <img_state_trace_source>
#define IMG_STATE_PCR_TRACE_FROM_INVALID (0)
#define IMG_STATE_PCR_TRACE_FROM_SECBASE (1)
#define IMG_STATE_PCR_TRACE_FROM_OS (2)

/// @brief Information of physical TPM devices. Untrusted OS/Apps read these info from TPM devices and SecBase.
/// [NOTE] PCR 4 is for SecBase's image. The image hash is always extended into PCR 4 so a faulty SecBase does not block 
/// BitLocker.
/// PCR 15 is for the SecBase (mhv)'s boot state (e.g., SecBase is running or not, and random uTPM Quote public key 
/// if any).
struct tpm_phy_info
{
    // Machine ID
    uint8_t machine_id[SECBASE_MACHINE_ID_MAX_LENGTH];

    // SecBase's boot state
    uint32_t secbase_boot_state_size;
    uint8_t secbase_boot_state[MHV_SENSITIVE_STATE_MAX_SIZE];

    // Data read from the physical TPM 
    uint32_t aik_der_size;
    uint8_t aik_der[TPM2_AIK_RSA2048_DER_LENGTH];
    uint8_t secbase_image_pcr[SHA256_HASH_LENGTH];
    uint8_t secbase_boot_state_pcr[SHA256_HASH_LENGTH];
    uint32_t quote_blob_size;
    uint8_t quote_blob[TPM2_QUOTE_BLOB_SIZE];
    uint8_t quote_sig[TPM2_QUOTE_SIGNATURE_LENGTH];

    //// Physical TPM - TPM PCR histories
    uint8_t boot_state_pcr_orig[SHA256_HASH_LENGTH];
    uint32_t img_state_trace_source; // Does <img_state_trace> come from SecBase or the untrusted OS?
    SECBASE_TPM_EVENT_LOG_TRACE img_state_trace;
} __attribute__((packed));


#endif // __ASSEMBLY__
#endif // HID_BRIDGE_CMDS_H