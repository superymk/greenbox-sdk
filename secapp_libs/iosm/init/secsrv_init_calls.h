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