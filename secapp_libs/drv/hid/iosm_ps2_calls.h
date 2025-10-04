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

#ifndef IOSM_PS2_COMM_IOSM_H
#define IOSM_PS2_COMM_IOSM_H

#include <palddk.h>
#include <ps2_def.h>
#include <iosm_ps2_sp_intfs.h>

#ifndef __ASSEMBLY__

/// @brief Create Inter-SecProcess communication channels required by IOSM-PS2.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_iosm_ps2_init(spid_t in_iosm_ps2_spid);
extern void comm_iosm_ps2_fini(void);

/// @brief Register the caller SecProcess's PS2 driver in IOSM-PS2.
extern SAP_STATUS iosm_ps2_register_ps2drv(void);

/// @brief Unregister the caller SecProcess's PS2 driver in IOSM-PS2.
extern SAP_STATUS iosm_ps2_unregister_ps2drv(void);

#endif // __ASSEMBLY__
#endif // IOSM_PS2_COMM_IOSM_H