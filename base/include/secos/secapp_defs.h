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

#ifndef PUBLIC_HEADERS_SECAPP_DEFS_H
#define PUBLIC_HEADERS_SECAPP_DEFS_H

#define SOS_MAX_SPs 				256  	//!< The maximum allowed number of PALs at the same time.
#define SP_USER_ID_START			(SOS_RESERVED_IDS + 1)
#define SP_MAX_ID					(SOS_MAX_SPs - 1) //!< The maximum ID allowed for SecApps


#ifndef __ASSEMBLY__

/// @brief Red application's process ID
typedef uint32_t 	app_pid_t;

/// @brief The ID of the SecProcess.
/// [NOTE] <spid_t> must be 32-bit long. This is the assumption of driver IDs in IOSM-USB2 and IOSM-FB.
typedef uint32_t 	spid_t;

#endif // __ASSEMBLY__
#endif // PUBLIC_HEADERS_SECAPP_DEFS_H