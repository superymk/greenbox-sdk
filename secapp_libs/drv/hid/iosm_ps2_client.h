// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_IOSM_PS2_CLIENT_H
#define LIBDRV_HID_IOSM_PS2_CLIENT_H

#include <palddk.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS init_iosm_ps2_client(void);
extern void fini_libps2_client(void);

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_IOSM_PS2_CLIENT_H