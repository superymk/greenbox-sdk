// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h>

#include <stdint.h>
#include <errno.h>

/********* Errno translate *********/
int os_to_newlib_errno(int os_errno)
{
    // Cygwin has same errno definitions with Newlib
	return os_errno;
}




/********* Socket constants translate *********/
int newlib_to_os_socket_types(int newlib_socket_constant)
{
    return newlib_socket_constant;
}

int newlib_to_os_socket_option_flags(int newlib_socket_constant)
{
    return newlib_socket_constant;
}

int newlib_to_os_socket_option_level(int newlib_socket_constant)
{
    return newlib_socket_constant;
}