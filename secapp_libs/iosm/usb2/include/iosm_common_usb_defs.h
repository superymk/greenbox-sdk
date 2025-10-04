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