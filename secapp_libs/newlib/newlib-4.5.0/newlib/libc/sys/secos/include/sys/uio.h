/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

/* minimal compliance with IEEE Std 1003.1
   http://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/uio.h.html#tag_13_68 */
#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#include <sys/types.h> /* FIXME non-compliant. makes symbols from
                          sys/types.h visible */

/* The <sys/uio.h> header shall define the iovec structure that
   includes at least the following members: */
struct iovec {
  void   *iov_base;  /* Base address of a memory region for input or output.  */
  size_t  iov_len;   /* The size of the memory pointed to by iov_base.  */
};

/* The <sys/uio.h> header uses the iovec structure for scatter/gather
   I/O. */

/* The ssize_t and size_t types shall be defined as described in
   <sys/types.h>. */

/* The following shall be declared as functions and may also be
   defined as macros. Function prototypes shall be provided. */
ssize_t readv(int, const struct iovec *, int);
ssize_t writev(int, const struct iovec *, int);

#endif
