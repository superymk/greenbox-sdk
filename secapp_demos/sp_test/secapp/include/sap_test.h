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

#ifndef SECAPP_TEST_H
#define SECAPP_TEST_H

#include <sap_headers.h>
#include <palddk.h>

#include <app_sp_intfs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define SAP_TEST_LOG 				LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#include "utils.h"

#include "newlib/headers.h"
#include "other_libs/openssl.h"

#define SAP_TEST_DEBUG(format, ...) \
	do{	\
		if((SAP_TEST_LOG >= LOG_DEBUG) && (SAP_TEST_LOG != LOG_NONE)) {  \
		printf("[Debug] <%s> "format, __func__, ## __VA_ARGS__); } }while(0)
#define SAP_TEST_INFO(format, ...) \
	do{ \
        if((SAP_TEST_LOG >= LOG_INFO) && (SAP_TEST_LOG != LOG_NONE)) {  \
		printf("[TestSecApp] "format, ## __VA_ARGS__); } }while(0)
#define SAP_TEST_ERR(format, ...) \
	do{ \
        if((SAP_TEST_LOG >= LOG_ERR) && (SAP_TEST_LOG != LOG_NONE)) {  \
		printf("[ERR] <%s> "format, __func__, ## __VA_ARGS__); } }while(0)

#define OUTPUT(format, ...) printf("[TEST] " format, ## __VA_ARGS__)
#define TEST_PASS(format, ...) printf("[TEST-Pass] " format, ## __VA_ARGS__)
#define TEST_FAIL(format, ...) printf("[TEST-FAILED] " format, ## __VA_ARGS__)

#define AssertEqual(x1, x2) {   												\
  do {                                                                  \
	if((x1) != (x2)) {                                                          \
	  printf("ASSERT NOT EQUAL!! Real Value: (%u, %u) for %s == %s\n", x1, x2, #x1, #x2); \
	}                                                                   \
  } while(0); }
	  
#endif // __ASSEMBLY__
#endif // SECAPP_TEST_H