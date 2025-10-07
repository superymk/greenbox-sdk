// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECSRV_INIT_COMM_IOSM_H
#define SECSRV_INIT_COMM_IOSM_H



#ifndef __ASSEMBLY__



/// @brief Create Inter-SecProcess communication channels required by SECSRV-INIT.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_secsrv_init_init(spid_t in_secsrv_init_spid);
extern void comm_secsrv_init_fini(void);

#endif // __ASSEMBLY__
#endif // SECSRV_INIT_COMM_IOSM_H