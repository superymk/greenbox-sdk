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

#ifndef IOSM_USB_TYPES_H
#define IOSM_USB_TYPES_H

#ifndef __ASSEMBLY__

typedef	unsigned char	u_char;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

typedef paddr_t bus_addr_t;
typedef size_t bus_size_t;

typedef char* caddr_t;


#ifndef __packed
#define __packed __attribute__ ((packed))
#endif

#endif // __ASSEMBLY__
#endif // IOSM_USB_TYPES_H