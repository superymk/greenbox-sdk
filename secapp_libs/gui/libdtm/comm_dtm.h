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

#ifndef LIBDTM_COMM_H
#define LIBDTM_COMM_H

#include <sap_headers.h>
#include <palddk.h>
#include <stdio.h>
#include <string.h>

#ifndef __ASSEMBLY__

/// @brief Create Inter-SecProcess communication channels to DesktopMgr.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_dtm_init(spid_t in_dtm_spid);

extern void comm_dtm_fini(void);

extern SAP_STATUS dtm_register_window(uint32_t window_init_width, uint32_t window_init_height, shandle_t* out_h_fb, size_t* out_fb_size, uint32_t* out_fb_max_width, uint32_t* out_fb_max_height, shandle_t* out_h_rb_ctl, shandle_t* out_h_rb_buf);
extern SAP_STATUS dtm_unregister_window(void);


#endif // __ASSEMBLY__
#endif // LIBDTM_COMM_H