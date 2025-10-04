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
// #include <appddk.h>
#include "../include/loader_os_intfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/file.h>

int os_file_lock_create(const char* filepath, ulong_t* out_fd)
{
	int fd;

    // Open or create the file with read and write permissions for the owner only
    fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return -1;

    // Attempt to acquire an exclusive lock on the file
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) 
	{
        close(fd);
        return -1;
    }

    // Write something to the file
    const char *text = "Hello, world!\n";
    if (write(fd, text, strlen(text)) == -1) 
	{
        // Attempt to release the lock
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
    }

	// On success
	*out_fd = (ulong_t)fd;
	return 0;
}

int os_file_lock_release(ulong_t fd)
{
    if (flock(fd, LOCK_UN) == -1) {
        close(fd);
        return -1;
    }

    close(fd);

	// On success
	return 0;
}