// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_LOADER_SECBASE_TPM_H
#define SECOS_LOADER_SECBASE_TPM_H

#ifndef __ASSEMBLY__

extern SL_STATUS _do_sl_secbase_tpm_create_keys(void);

extern SL_STATUS _do_sl_secbase_tpm_get_quote(struct tpm_quote_nonce* nonce, uint32_t sb_img_pcr_idx, uint32_t sb_boot_state_pcr_idx);

/// @brief 
/// @param sb_img_pcr_idx Physical TPM's PCR for SecBase's image measurement.
/// @param sb_boot_state_pcr_idx Physical TPM's PCR for SecBase's boot state measurement.
/// @return 
extern SL_STATUS _do_sl_secbase_tpm_get_pcrs(uint32_t sb_img_pcr_idx, uint32_t sb_boot_state_pcr_idx);

extern SL_STATUS _do_sl_secbase_tpm_seal(void);

extern SL_STATUS _do_sl_os_efi_tpm2_event_logs_get_trace(uint32_t sb_img_pcr_idx);

#endif // __ASSEMBLY__
#endif // SECOS_LOADER_SECBASE_TPM_H