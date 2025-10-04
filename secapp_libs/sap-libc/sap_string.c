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

#include <palddk.h>

void* pal_mm_memcpy(void* dst, const void* src, size_t len)
{
	char *dst0 = (char *) dst;
	char *src0 = (char *) src;
	
	while (len--)
    {
      *dst0++ = *src0++;
    }

  	return dst;
}

void* pal_mm_memset(void* m, int c, size_t n)
{
	char *s = (char *) m;
	
	while (n--)
    	*s++ = (char) c;

  	return m;
}

int pal_mm_strncmp(const char* s1, const char* s2, size_t n)
{
	if (n == 0)
    return 0;

    while (n-- != 0 && *s1 == *s2)
    {
    	if (n == 0 || *s1 == '\0')
		break;
	    s1++;
	    s2++;
    }

    return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

char* pal_mm_strchr(const char* s, int c)
{
	for (; *s != '\0'; ++s)
	  if (*s == c)
	    return (char *) s;

	return NULL;
}

// copy from http://www.opensource.apple.com/source/bash/bash-80/bash/lib/sh/strnlen.c
size_t pal_mm_strnlen (register const char *s, size_t maxlen)
{
	register const char *e;
	size_t n;

	for (e = s, n = 0; *e && n < maxlen; e++, n++);
	return n;
}



