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

// Network functions
static int hostname_to_ip(const char* hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        //herror("gethostbyname");
        return -1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}

#define NETWORK_TCP  1
#define NETWORK_UDP  2

//! <hostname> must use IP address
static int net_connect(char* hostname, int port, int sock_type)
{
	int sockfd = 0;
	struct sockaddr_in serv_addr; 
	char ip[100];

	if (sock_type == NETWORK_TCP)
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	else if (sock_type == NETWORK_UDP)
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	else
	{
		SAP_TEST_ERR("[NET] Wrong sock_type:0x%08X\n", sock_type);
		return -1;
	}
	
	if(sockfd < 0)
    {
        SAP_TEST_ERR("[NET] Could not create socket\n");
        return -1;
    } 

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

	hostname_to_ip(hostname, ip);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        SAP_TEST_ERR("[NET] inet_pton error occured\n");
        return -1;
    }

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
       	SAP_TEST_ERR("[NET] Connect Failed \n");
       	return -1;
    }

	return sockfd;
}

bool test_socket_send_recv(void)
{
	int fd = 0;
	size_t size = 0;
	char* req = "GET /index.html\n";
	char* read_buf = NULL;
	const int buf_size = 512;
	
	fd = net_connect("www.google.com", 80, NETWORK_TCP);
	SAP_TEST_DEBUG("fd:0x%08X\n", fd);
	if(fd != -1)
	{
		// Write
		size = send(fd, req, strnlen(req, 100), 0);
		SAP_TEST_DEBUG("write size:0x%lX\n", size);

		// Read
		read_buf = (char*)malloc(buf_size);
		size = recv(fd, read_buf, buf_size, 0);
		SAP_TEST_DEBUG("read result:%s, size:0x%lX\n", read_buf, size);
		free(read_buf);

		// Close
		close(fd);
	}

	if(size == buf_size)
		return true;

	SAP_TEST_DEBUG("Actual size: %lu\n", size);
	SAP_TEST_DEBUG("Expect size: %d\n", buf_size);
	return false;
}