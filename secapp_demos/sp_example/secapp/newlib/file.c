// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_test.h>

#include <netdb.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define TEST_FILENAME "/tmp/test.svg"

bool test_file_create_write(void)
{
	int fd = 0;
	int result = 0;
	
	fd = open(TEST_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

	if(fd >= 0)
	{
		result = write(fd, "abcd", 5);
		close(fd);
	}

	if(result == 5)
		return true;

	SAP_TEST_DEBUG("Actual <result>:%d <fd>:%d\n", result, fd);
	return false;
}

bool test_file_create_write_longdata(void)
{
	int fd = 0;
	int result = 0;
	
	fd = open(TEST_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

	if(fd >= 0)
	{
		result = write(fd, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", 81);
		close(fd);
	}

	if(result == 81)
		return true;

	SAP_TEST_DEBUG("Actual <result>:%d <fd>:%d\n", result, fd);
	return false;
}

bool test_file_open_read(void)
{
	int fd = 0;
	int result = 0;
	const int buf_size = 8;
	
	fd = open(TEST_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	if(fd >= 0)
	{
		// write some content
		result = write(fd, "abcdefg", buf_size);
		close(fd);
	}
	else
		return false;

	fd = open(TEST_FILENAME, O_RDONLY);
	if(fd >= 0)
	{
		char buf[buf_size];

		// Test read
		result = read(fd, buf, buf_size);
		close(fd);
	}

	if(result == buf_size)
		return true;

	SAP_TEST_DEBUG("Actual <result>:%d <fd>:%d \n", result, fd);
	SAP_TEST_DEBUG("Expect result: %d \n", buf_size);
	return false;
}

bool test_file_open_read_longdata(void)
{
	int fd = 0;
	int result = 0;
	const int buf_size = 81;
	
	fd = open(TEST_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	if(fd >= 0)
	{
		// write some content
		result = write(fd, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", buf_size);
		close(fd);
	}
	else
		return false;
	

	fd = open(TEST_FILENAME, O_RDONLY);
	if(fd >= 0)
	{
		char buf[buf_size];
		
		// Test read
		result = read(fd, &buf, buf_size);
		close(fd);
	}

	if(result == buf_size)
		return true;

	SAP_TEST_DEBUG("Actual <result>:%d <fd>:%d \n", result, fd);
	SAP_TEST_DEBUG("Expect result: %d \n", buf_size);
	return false;
}

bool test_file_open_read_more_longdata(void)
{
	int fd = 0;
	int result = 0;
	const int buf_size = 81;
	
	fd = open(TEST_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	if(fd >= 0)
	{
		// write some content
		result = write(fd, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", buf_size);
		close(fd);
	}
	else
		return false;
	

	fd = open(TEST_FILENAME, O_RDONLY);
	if(fd >= 0)
	{
		char buf[buf_size];
		
		// Test read
		result = read(fd, &buf, buf_size + 10);
		close(fd);

		if(!strcmp(buf, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"))
			return true;
	}

	SAP_TEST_DEBUG("Actual <result>:%d <fd>:%d \n", result, fd);
	SAP_TEST_DEBUG("Expect result: %d \n", buf_size);
	return false;
}

