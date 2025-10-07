// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _COMMON_USB_DEFS_H
#define _COMMON_USB_DEFS_H

#define USB_DEFAULT_ADDR        (0)

#define USB_MAXADDR 127
#define IS_VALID_USBADDR_EXCLUDE_ZERO(usbaddr)      ((usbaddr > 0) && (usbaddr <= USB_MAXADDR))





#ifndef __ASSEMBLY__

/********* Definitions - I/O Separation *********/
#define IO_PID_INVALID 	        0
#define INVALID_SLOT 	        0xFFFFFFFF

#endif // __ASSEMBLY__
#endif //_COMMON_USB_DEFS_H