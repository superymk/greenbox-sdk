// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file [NOTE] We do not name the file as "mm.c" because newlib-4.5.0 seems to have bugs to link all C files with the
/// same file name though they are in different sub-folders in newlib/libc/sys/secos. 

#include <stdlib.h>   // aligned_alloc, free
#include <stdint.h>   // SIZE_MAX
#include <errno.h>    // EINVAL, ENOMEM
#include <sap_headers_libc.h>

int posix_memalign(void **memptr, size_t alignment, size_t bytes)
{
    // Validate pointer argument
    if (memptr == NULL)
        return EINVAL;

    // POSIX: alignment must be a power of two AND a multiple of sizeof(void*)
    if (alignment < sizeof(void*) || (alignment & (alignment - 1)) != 0)
        return EINVAL;

    // POSIX: size==0 behaves like malloc(0): return either NULL or a pointer
    // that can be passed to free(). We'll return NULL explicitly.
    if (bytes == 0) {
        *memptr = NULL;
        return 0;
    }

    // C11 aligned_alloc requires size to be a multiple of alignment.
    // Round up safely and detect overflow.
    if (bytes > SIZE_MAX - (alignment - 1))
        return ENOMEM;  // rounding would overflow

    size_t size_rounded = (bytes + alignment - 1) & ~(alignment - 1);

    void *p = aligned_alloc(alignment, size_rounded);
    if (!p)
        return ENOMEM;

    *memptr = p;
    return 0;
}