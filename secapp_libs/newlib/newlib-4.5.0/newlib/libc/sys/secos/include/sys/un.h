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

/* based on IEEE Std 1003.1-2008
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_un.h.html
 */

#include <netinet/in.h>

#define UNIX_PATH_MAX	108

/* The <sys/un.h> header shall define the sockaddr_un structure, which
   shall include at least the following members: */
struct sockaddr_un {
  sa_family_t  sun_family;  /* Address family.  */
  char         sun_path[UNIX_PATH_MAX];  /* Socket pathname.  */
};
/* The sockaddr_un structure is used to store addresses for UNIX
   domain sockets. Values of this type shall be cast by applications
   to struct sockaddr for use with socket functions. */

