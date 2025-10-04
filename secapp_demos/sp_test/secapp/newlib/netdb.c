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

//netdb
bool test_netdb_gethostbyname(void)
{
	struct hostent *remoteHost;

	remoteHost = gethostbyname("www.google.com");

	if(!strcmp(remoteHost->h_name, "www.google.com"))
		return true; 
	
	SAP_TEST_DEBUG("Actual h_name: %s\n", remoteHost->h_name);
	SAP_TEST_DEBUG("Expect h_name: %s\n", "www.google.com");
	return false;
}

static void dump_hostent(struct hostent* remoteHost)
{
	SAP_TEST_DEBUG("h_name: %s\n", remoteHost->h_name);
	SAP_TEST_DEBUG("h_aliases: 0x%lX\n", (vaddr_t)remoteHost->h_aliases);
	SAP_TEST_DEBUG("h_aliases[0]: %s\n", remoteHost->h_aliases[0]);
	SAP_TEST_DEBUG("h_addrtype: %d\n", remoteHost->h_addrtype);
	SAP_TEST_DEBUG("h_length: %d\n", remoteHost->h_length);
	SAP_TEST_DEBUG("h_addr_list: 0x%lX\n", (vaddr_t)remoteHost->h_addr_list);
	SAP_TEST_DEBUG("h_addr_list[0]: 0x%08X\n", *(uint32_t*)remoteHost->h_addr_list[0]);
	SAP_TEST_DEBUG("h_addr: %s\n", remoteHost->h_addr);
}

bool test_netdb_gethostbyname_localhost(void)
{
	struct hostent *remoteHost;

	remoteHost = gethostbyname("127.0.0.1");
	
	dump_hostent(remoteHost);
	return true;
}

bool test_netdb_hostname_to_ip(void)
{
	char* hostname = "www.google.com";
    struct hostent *he;
    struct in_addr **addr_list;
	char ip[100] = {0};
    int i;
	
    if ( (he = gethostbyname( hostname ) ) == NULL) 
		return false;   

    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
		SAP_TEST_DEBUG("<test_hostname_to_ip> Resolved to IP:%s\n", ip);
        return true;
    }

    return false;
}