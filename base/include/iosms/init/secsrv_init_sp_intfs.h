// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
 */

#ifndef SECSRV_INIT_PUBLIC_INTF_H
#define SECSRV_INIT_PUBLIC_INTF_H

/// @brief Number of pages of any Inter-SecProcess channel to the SECSRV-INIT service.
#define SECSRV_INIT_INCOMMING_COMM_ISP_PAGES          (1)




/********* Sub-Commands of <SP_MESSAGE> *********/
#define SECSRV_INIT_SUBCMD_INVALID                                        (0)

#define SECSRV_INIT_SECPROCESS_PRIORITY_UPDATE                            (1)



/********* Status of <SECSRV_INIT_ISP_MSG_RESPONSE> *********/
#define SECSRV_INIT_STATUS1_INVALID                   (0)
#define SECSRV_INIT_STATUS1_SUCCESS                   (1)
#define SECSRV_INIT_STATUS1_FAIL                      (2)


#ifndef __ASSEMBLY__

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) SECSRV_INIT_ISP_MSG_HEADER;

#define SECSRV_INIT_ISP_MSG_REQUEST_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[SECSRV_INIT_ISP_MSG_REQUEST_MAX_DATA_SIZE];
} SECSRV_INIT_ISP_MSG_REQUEST;

#define SECSRV_INIT_ISP_MSG_RESPONSE_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[SECSRV_INIT_ISP_MSG_RESPONSE_MAX_DATA_SIZE];
} SECSRV_INIT_ISP_MSG_RESPONSE;

typedef struct
{
    SECSRV_INIT_ISP_MSG_HEADER    header;
    
    union{
        SECSRV_INIT_ISP_MSG_REQUEST   request;
        SECSRV_INIT_ISP_MSG_RESPONSE  response;
    };
} SECSRV_INIT_ISP_MSG;




/********* Input Parameters of Sub-Commands *********/
typedef enum 
{
    SP_PRIORITY_NORMAL = 0,
    SP_PRIORITY_LOW = 1,
} sp_priority_t;

typedef struct
{
    spid_t target_sp_id;
    sp_priority_t new_priority;
} SECSRV_INIT_SP_PRIORITY_UPDATE_PARAMS;


#endif // __ASSEMBLY__
#endif // SECSRV_INIT_PUBLIC_INTF_H