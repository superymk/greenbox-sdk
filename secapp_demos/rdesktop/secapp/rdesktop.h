// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RDESKTOP_SAP_HEADER_H
#define RDESKTOP_SAP_HEADER_H

#include <rdesktop_header.h>
#include <app_sp_intfs.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS rdesktop_init(SAP_CONFIG* conf, vaddr_t fb_vaddr, uint32_t win_width, uint32_t win_height, rfbClient **out_cl);
extern void rdesktop_cleanup(rfbClient *cl);

#endif // __ASSEMBLY__
#endif // RDESKTOP_SAP_HEADER_H