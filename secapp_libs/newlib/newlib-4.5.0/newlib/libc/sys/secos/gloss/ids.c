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

/// @file There is no notion of users, groups, or sessions in SecApps. So their functions either return error or do
/// nothing.

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <sap_headers_libc.h>



uid_t getuid()
{
	return 0;
}

uid_t geteuid()
{
	return 0;
}

int setuid(uid_t uid)
{
	return -1;
}

int seteuid(uid_t euid)
{
	return -1;
}

int setreuid(uid_t ruid, uid_t euid)
{
	return -1;
}

gid_t getgid()
{
	return 0;
}

gid_t getegid()
{
	return 0;
}

int setgid(gid_t gid)
{
	return -1;
}

int setegid(gid_t egid)
{
	return -1;
}

int setregid(gid_t rgid, gid_t egid)
{
	return -1;
}

int setpgid(pid_t pid, pid_t pgid)
{
	return -1;
}

pid_t getsid(pid_t pid)
{
	return -1;
}

pid_t setsid()
{
	return -1;
}
