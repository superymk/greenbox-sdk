// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SUBSYS_OPENBSD_API_RENAME_H
#define SUBSYS_OPENBSD_API_RENAME_H

#ifndef SKIP_OPENBSD_RENAME

#undef __aligned

#define malloc(size, type, flags)  openbsd_malloc(size, type, flags)

#endif // SKIP_OPENBSD_RENAME

#endif // SUBSYS_OPENBSD_API_RENAME_H