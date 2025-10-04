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

/// @file [NOTE] We do not name the file as "stdio.c" because newlib-4.5.0 seems to have bugs to link all C files with the
/// same file name though they are in different sub-folders in newlib/libc/sys/secos. 

#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <sap_headers_libc.h>

#define STRING_MAX_SIZE		(16384) // The maximum size of the string to be printed is 16KB.

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    if (!strp || !fmt) {
        errno = EINVAL;
        return -1;
    }
    *strp = NULL;

    // --- Fast path: try to learn required length with a 0-sized probe ---
    va_list ap_probe;
    va_copy(ap_probe, ap);
    int n = vasnprintf(NULL, 0, fmt, ap_probe);
    va_end(ap_probe);

    if (n >= 0) {
        // Needed length is n (excluding NUL). Allocate n+1.
        size_t need = (size_t)n + 1;

        // Overflow guard
        if (need == 0 || need > STRING_MAX_SIZE) {
            errno = EOVERFLOW;
            return -1;
        }

        char *buf = (char *)malloc(need);
        if (!buf) {
            errno = ENOMEM;
            return -1;
        }

        va_list ap_write;
        va_copy(ap_write, ap);
        int n2 = vasnprintf(buf, need, fmt, ap_write);
        va_end(ap_write);

        if (n2 < 0 || (size_t)n2 >= need) {
            // Shouldn't happen if vasnprintf behaved like vsnprintf.
            free(buf);
            errno = EINVAL;
            return -1;
        }

        *strp = buf;
        return n2; // number of bytes written (no NUL)
    }

    // --- Fallback: some vasnprintf impls don't support NULL,0 probes ---
    size_t size = 128;
    char *buf = NULL;

    for (;;) {
        // Grow (or first alloc)
        char *newbuf = (char *)realloc(buf, size);
        if (!newbuf) {
            free(buf);
            errno = ENOMEM;
            return -1;
        }
        buf = newbuf;

        va_list ap_try;
        va_copy(ap_try, ap);
        n = vasnprintf(buf, size, fmt, ap_try);
        va_end(ap_try);

        if (n >= 0 && (size_t)n < size) {
            // Success (fully written)
            *strp = buf;
            return n;
        }

        if (n < 0) {
            // Unknown length; double the buffer and retry
            if (size > STRING_MAX_SIZE / 2) {
                free(buf);
                errno = EOVERFLOW;
                return -1;
            }
            size *= 2;
            continue;
        }

        // n >= size: vasnprintf reports required length (like C99 vsnprintf)
        size_t need = (size_t)n + 1;
        if (need <= size) {
            // Defensive (shouldn't occur): treat as success path
            *strp = buf;
            return n;
        }
        if (need > STRING_MAX_SIZE) {
            free(buf);
            errno = EOVERFLOW;
            return -1;
        }
        size = need;
        // loop will realloc to exact size and write again
    }
}