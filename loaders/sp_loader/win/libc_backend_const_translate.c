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