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

#ifndef LIBDRV_HID_IOSM_PS2_CLIENT_H
#define LIBDRV_HID_IOSM_PS2_CLIENT_H

#include <palddk.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS init_iosm_ps2_client(void);
extern void fini_libps2_client(void);

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_IOSM_PS2_CLIENT_H