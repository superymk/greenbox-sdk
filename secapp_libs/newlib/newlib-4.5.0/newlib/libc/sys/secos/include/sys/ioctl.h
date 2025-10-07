// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

int ioctl_ret_1arg(int fd, int request, void* arg1, size_t arg1_value_size);

#endif // _SYS_IOCTL_H
