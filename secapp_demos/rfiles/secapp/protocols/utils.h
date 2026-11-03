// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RFILES_PROTOS_UTILS_H
#define RFILES_PROTOS_UTILS_H

#include <rfiles.h>

/// @brief Maximum size of URL strings
#define URL_MAX_SIZE 4096

#ifndef __ASSEMBLY__

extern char *url_alloc(void);

extern void url_free(char* url);

#endif // __ASSEMBLY__
#endif // RFILES_PROTOS_UTILS_H