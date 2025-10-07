// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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