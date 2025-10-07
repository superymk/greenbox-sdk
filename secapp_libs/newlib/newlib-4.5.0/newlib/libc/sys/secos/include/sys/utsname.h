// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* libc/sys/linux/sys/utsname.h - System identification */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#define __UTSNAMELEN 65	/* synchronize with kernel */

struct utsname {
    char sysname[__UTSNAMELEN];
    char nodename[__UTSNAMELEN];
    char release[__UTSNAMELEN];
    char version[__UTSNAMELEN];
    char machine[__UTSNAMELEN];
    char domainname[__UTSNAMELEN];
};


int uname(struct utsname *name);

#endif
