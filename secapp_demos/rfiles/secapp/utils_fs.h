// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RFILES_UTILS_FS_H
#define RFILES_UTILS_FS_H

#include <sap_headers.h>
#include <palddk.h>



#ifndef __ASSEMBLY__

/// @brief Returns a malloc'd string containing the directory part of `path`. Caller must free().
///    Behaves like POSIX dirname():
///    - "file"      -> "."
///    - "/file"     -> "/"
///    - "/a/b/c"    -> "/a/b"
///    - "/a/b/"     -> "/a"
///    - "///"       -> "/"
extern char *path_dirname(const char *path);

#endif // __ASSEMBLY__
#endif // RFILES_UTILS_FS_H