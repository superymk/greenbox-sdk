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

/* Copy from XMHF */
#ifndef _PAL_STRING_H_
#define _PAL_STRING_H_

//#include <sap_types.h>


#ifndef __ASSEMBLY__
extern void* pal_mm_memcpy(void* dst, const void* src, size_t len);
extern void* pal_mm_memset(void* m, int c, size_t n);
extern int pal_mm_strncmp(const char* s1, const char* s2, size_t n);
extern char* pal_mm_strchr(const char* s, int c);
extern size_t pal_mm_strnlen (register const char *s, size_t maxlen);
#endif // __ASSEMBLY__
#endif //_PAL_STRING_H_