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

#ifndef IOSM_PS2_PUBLIC_INTF_H
#define IOSM_PS2_PUBLIC_INTF_H

/// @brief Number of pages of any Inter-SecProcess channel to the IOSM-USB2 service.
#define IOSM_PS2_INCOMMING_COMM_ISP_PAGES          (1)


/********* Sub-Commands of <SP_MESSAGE> *********/
#define IOSM_PS2_SUBCMD_INVALID                                        (0)

/// @brief Register the owner SecProcess of the given device.
/// Caller SecProcess must send a message to invoke this function. 
/// Format: <SP_MESSAGE->data> = bdf
/// Caller SecProcess must read response from the Inter-SecProcess shared memory channels. This operation does not 
/// request SecOS to send any response messages.
/// Format: <IOSM_PS2_ISP_MSG->header.is_response> = true
///			<IOSM_PS2_ISP_MSG->response.status1> = IOSM_PS2_STATUS1_SUCCESS on success. Or IOSM_PS2_STATUS1_FAIL on failure
///			<IOSM_PS2_ISP_MSG->response.status2> = internal status
#define IOSM_PS2_SUBCMD_REGISTER_USER                             (1)

/// @brief Unregister the owner SecProcess of the given device.
/// Caller SecProcess must send a message to invoke this function. 
/// Format: <SP_MESSAGE->data> = bdf
/// Caller SecProcess must read response from the Inter-SecProcess shared memory channels. This operation does not 
/// request SecOS to send any response messages.
/// Format: <IOSM_PS2_ISP_MSG->header.is_response> = true
///			<IOSM_PS2_ISP_MSG->response.status1> = IOSM_PS2_STATUS1_SUCCESS on success. Or IOSM_PS2_STATUS1_FAIL on failure
///			<IOSM_PS2_ISP_MSG->response.status2> = internal status
#define IOSM_PS2_SUBCMD_UNREGISTER_USER                           (2)




/********* Status of <IOSM_PS2_ISP_MSG_RESPONSE> *********/
#define IOSM_PS2_STATUS1_INVALID                   (0)
#define IOSM_PS2_STATUS1_SUCCESS                   (1)
#define IOSM_PS2_STATUS1_FAIL                      (2)


#ifndef __ASSEMBLY__

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) IOSM_PS2_ISP_MSG_HEADER;

typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;
} IOSM_PS2_ISP_MSG_REQUEST;


typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;
} IOSM_PS2_ISP_MSG_RESPONSE;

typedef struct
{
    IOSM_PS2_ISP_MSG_HEADER    header;
    
    union{
        IOSM_PS2_ISP_MSG_REQUEST   request;
        IOSM_PS2_ISP_MSG_RESPONSE  response;
    };
} IOSM_PS2_ISP_MSG;

#endif // __ASSEMBLY__
#endif // IOSM_PS2_PUBLIC_INTF_H