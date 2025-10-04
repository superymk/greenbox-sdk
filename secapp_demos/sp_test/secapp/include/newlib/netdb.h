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

#ifndef TEST_NEWLIB_NETDB_H
#define TEST_NEWLIB_NETDB_H

extern bool test_netdb_gethostbyname(void);
extern bool test_netdb_gethostbyname_localhost(void);
extern bool test_netdb_hostname_to_ip(void);

#endif // TEST_NEWLIB_NETDB_H