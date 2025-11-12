// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef __SECAGENT_CONFIG_H__
#define __SECAGENT_CONFIG_H__

// This macro must be the same as secbase/hypapps/whitevisor/config.mk .
// If secbase/hypapps/whitevisor/config.mk defines XMHF_LOG_BUF_SIZE variable,
// then XMHF_LOG_BUF_SIZE macro must have the same value here.
// If secbase/hypapps/whitevisor/config.mk does NOT define XMHF_LOG_BUF_SIZE
// variable, then XMHF_LOG_BUF_SIZE macro must NOT be defined here.
#ifndef XMHF_LOG_BUF_SIZE
#define XMHF_LOG_BUF_SIZE 256
#endif // XMHF_LOG_BUF_SIZE

/// @brief Set to 1 to enable SecAgent to allocate OS interrupts to receive IPI from SecOS.
#define SA_CONFIG_ENABLE_ALLOC_COMM_INTR (0)

#endif /* !__SECAGENT_CONFIG_H__ */
