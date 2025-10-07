// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifdef __linux__
#include <os/linux_headers.h>
#elif defined (_WIN32) || defined (_WIN64) || defined(__WINDOWS__)
#include <os/win_headers.h>
#endif