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

#ifndef SECAPP_DBG_PRINT_BCD_H
#define SECAPP_DBG_PRINT_BCD_H

/* number of bits per byte */
#define NBBY 8

#ifndef __ASSEMBLY__

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned short  ushort;         /* Sys V compatibility */
typedef unsigned int    uint;           /* Sys V compatibility */

// typedef int32_t         ssize_t;
typedef long long                 quad_t;
typedef unsigned long long        u_quad_t;       /* quads (deprecated) */
// typedef long long                 intmax_t;
// typedef unsigned long long        uintmax_t;



/* BCD conversions. */
extern u_char const	bcd2bin_data[];
extern u_char const	bin2bcd_data[];
extern char const	hex2ascii_data[];

#define	bcd2bin(bcd)	(bcd2bin_data[bcd])
#define	bin2bcd(bin)	(bin2bcd_data[bin])
#define	hex2ascii(hex)	(hex2ascii_data[hex])


#define isspace(c)      ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isascii(c)      (((c) & ~0x7f) == 0)
#define isupper(c)      ((c) >= 'A' && (c) <= 'Z')
#define islower(c)      ((c) >= 'a' && (c) <= 'z')
#define isalpha(c)      (isupper(c) || islower(c))
#define isdigit(c)      ((c) >= '0' && (c) <= '9')
#define isxdigit(c)     (isdigit(c) \
                          || ((c) >= 'A' && (c) <= 'F') \
                          || ((c) >= 'a' && (c) <= 'f'))
#define isprint(c)      ((c) >= ' ' && (c) <= '~')

#define toupper(c)      ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c)      ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))

#endif // __ASSEMBLY__
#endif // SECAPP_DBG_PRINT_BCD_H