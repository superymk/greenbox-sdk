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

#ifndef OS_ERRNO_H
#define OS_ERRNO_H

#define OS_SUCCESS                  0
#define E_NO_MEM                    1
#define E_INV_ARG                   2          //! invalid argument
#define E_CHKERR                    3        
#define E_IO                        4
#define E_COMM_ERR                  5        //! communication channel error
#define E_TIMEOUT                   6
#define E_INTERNAL_ERROR            7
#define E_SECOS_ERROR               8
#define E_SIG_NO_MORE               9       //! No more signal info stored in SecAgent
#define E_SA_NOT_RUNNING            10
#define E_COMM_FULL                 11        // The communication channel has no space to queue messages
#define E_DEV_UNKNOWN               12      // Unknown device

#ifndef __ASSEMBLY__

#define SA_FAIL(status)        (unlikely(status != OS_SUCCESS))
#define SA_PASS(status)        (likely(status == OS_SUCCESS))


typedef ulong_t OSSTATUS;

// Security check for kernel level requests/configs
#define OS_SECURITY_CHECK(x)        {        \
    do {                                                                  \
        if(!unlikely(x)) {                                                          \
            PRINT("[SecAgent][%s:%d]: SECURITY POLICY VIOLATION: %s\n", __FILE__, __LINE__, #x); \
            return E_CHKERR;                                                          \
        }                                                                   \
    } while(0); }

#endif // __ASSEMBLY__
#endif // OS_ERRNO_H