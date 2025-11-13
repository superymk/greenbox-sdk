// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils_fs.h"

/* Returns a malloc'd string containing the directory part of `path`.
   Caller must free(). Behaves like POSIX dirname():
   - "file"      -> "."
   - "/file"     -> "/"
   - "/a/b/c"    -> "/a/b"
   - "/a/b/"     -> "/a"
   - "///"       -> "/"
*/
char *path_dirname(const char *path)
{
    if (!path || !*path)
        return strdup(".");

    size_t len = strlen(path);

    /* 1) Strip trailing slashes (except keep one if path is all slashes). */
    size_t end = len;
    while (end > 1 && path[end - 1] == '/')
        end--;

    /* 2) Find last slash before (or at) end-1. */
    size_t i = end;
    while (i > 0 && path[i - 1] != '/')
        i--;

    if (i == 0)
    {
        /* No slash found: dirname is "." */
        return strdup(".");
    }

    /* 3) Dir is [0, i), but strip trailing slashes from it (leave one if root). */
    size_t dirlen = i;
    while (dirlen > 1 && path[dirlen - 1] == '/')
        dirlen--;

    /* 4) Copy out */
    char *out = (char *)malloc(dirlen + 1);
    if (!out)
        return NULL;
    memcpy(out, path, dirlen);
    out[dirlen] = '\0';
    return out;
}