/* libc/sys/linux/sys/ioctl.h - ioctl prototype */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

// #include <bits/ioctls.h>

int ioctl_ret_1arg(int fd, int request, void* arg1, size_t arg1_value_size);

#endif
