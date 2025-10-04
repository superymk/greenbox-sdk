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

#ifndef RDESKTOP_SAP_HEADER_H
#define RDESKTOP_SAP_HEADER_H

#include <rdesktop_header.h>
#include <app_sp_intfs.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS rdesktop_init(SAP_CONFIG* conf, vaddr_t fb_vaddr, uint32_t win_width, uint32_t win_height, rfbClient **out_cl);
extern void rdesktop_cleanup(rfbClient *cl);

#endif // __ASSEMBLY__
#endif // RDESKTOP_SAP_HEADER_H