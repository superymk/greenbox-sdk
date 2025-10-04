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

// @file This file includes the headers for general SecApps. Most SecApps should include this header only to function.
// This file does not contain any headers of IOSM clients, because those clients require additional headers exported by
// IOSMs. General SecApps do not need to include IOSM headers. Thus, SecApps using IOSM clients should include IOSM 
// headers by themselves.

#ifndef SECAPP_HEADERS_H
#define SECAPP_HEADERS_H

#include <sap_headers_libc.h>

// Additional libc headers often missed but needed in the main SecProcess file
#include <sys/ipc/comm.h>


#include <drv/hid/input_events.h>

#include <gui/dtm/libdtm.h>
#include <gui/ui/libgui_headers.h>


#ifndef __ASSEMBLY__

extern void sp_exit(SP_UTASK* sp_params);
#endif // __ASSEMBLY__
#endif // SECAPP_HEADERS_H