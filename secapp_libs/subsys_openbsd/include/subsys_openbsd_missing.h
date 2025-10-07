// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SUBSYS_OPENBSD_HEADER_MISSING_H
#define SUBSYS_OPENBSD_HEADER_MISSING_H

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif 

#ifndef max
#define max(a, b) (a < b ? b : a)
#endif

#endif // SUBSYS_OPENBSD_HEADER_MISSING_H