// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

