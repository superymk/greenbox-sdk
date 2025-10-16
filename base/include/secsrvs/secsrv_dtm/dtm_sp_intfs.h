// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header - Limited License
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

#ifndef SECSRV_DTM_SP_INTFS_H
#define SECSRV_DTM_SP_INTFS_H

#include <drv/hid/input_events.h>

#define MAX_DTM_WINDOWS                  1

#define DTM_EVENT_RB_CONTROL_PAGES      (1)
#define DTM_EVENT_RB_BUFFER_PAGES       (4)


/// @brief Number of pages of any Inter-SecProcess channel to the DesktopMgr service.
#define SECSRV_DTM_INCOMMING_COMM_ISP_PAGES                             (1)

/********* Sub-Commands of <SP_MESSAGE> *********/
#define SECSRV_DTM_SUBCMD_INVALID                                       (0)

#define SECSRV_DTM_SUBCMD_CREATE_WINDOW                                 (1)
#define SECSRV_DTM_SUBCMD_DESTROY_WINDOW                                (2)

#define SECSRV_DTM_SUBCMD_NOTIFY_EVENT                                  (1000)
#define     EVENT_TYPE_KBD          (1)
#define     EVENT_TYPE_MOUSE        (2)




/********* Status of <IOSM_PCIE_ISP_MSG_RESPONSE> *********/
#define DTM_STATUS1_INVALID                   (0)
#define DTM_STATUS1_SUCCESS                   (1)
#define DTM_STATUS1_FAIL                      (2)



#ifndef __ASSEMBLY__
typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) DTM_ISP_MSG_HEADER;

#define DTM_ISP_MSG_REQUEST_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char       data[DTM_ISP_MSG_REQUEST_MAX_DATA_SIZE];
} DTM_ISP_MSG_REQUEST;

#define DTM_ISP_MSG_RESPONSE_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char       data[DTM_ISP_MSG_RESPONSE_MAX_DATA_SIZE];
} DTM_ISP_MSG_RESPONSE;

typedef struct
{
    DTM_ISP_MSG_HEADER    header;
    
    union{
        DTM_ISP_MSG_REQUEST   request;
        DTM_ISP_MSG_RESPONSE  response;
    };
} DTM_ISP_MSG;



/********* Requests and Responses for Sub-Commands of <SP_MESSAGE>*********/
typedef struct
{
    uint32_t            window_init_width;
    uint32_t            window_init_height;
} DTM_REQUEST_CREATE_WINDOW;

typedef struct
{
    shandle_t           h_fb;
    size_t              fb_size;
    uint32_t            fb_max_width;
    uint32_t            fb_max_height;

    // DesktopMgr event ring buffer
    shandle_t           h_rb_ctl;
    shandle_t           h_rb_buf;
} DTM_RESPONSE_CREATE_WINDOW;

typedef struct
{
    uint32_t event_type;

    union
    {
        struct kbd_key_event kbd_event;
        struct mouse_event mouse_event;
    };
} DTM_EVENT;

/// Max. number of struct <DTM_EVENT> that can be put in the DTM event ring buffer.
#define MAX_DTM_EVENTS_IN_RINGBUFFER        (((DTM_EVENT_RB_BUFFER_PAGES * PAGE_SIZE_4K) / sizeof(DTM_EVENT)) - 1)

#endif // __ASSEMBLY__
#endif // SECSRV_DWM_SP_INTFS_H