// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBC_BACKEND_INFO_H
#define LIBC_BACKEND_INFO_H

#ifndef __ASSEMBLY__

#define ERR(format, ...) \
	do{	\
		printf("[LIBC-BACKEND][ERR] "format, ## __VA_ARGS__); } while(0)

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_INFO_H