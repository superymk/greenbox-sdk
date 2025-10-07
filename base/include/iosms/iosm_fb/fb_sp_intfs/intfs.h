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

#ifndef IOSM_FB_INTERFACES_H
#define IOSM_FB_INTERFACES_H

#define MAX_EFBDEV_NUM                  3

/// @brief Number of pages of any Inter-SecProcess channel to the IOSM-USB2 service.
#define IOSM_FB_INCOMMING_COMM_ISP_PAGES          (1)

/********* Sub-Commands of <SP_MESSAGE> *********/
#define IOSM_FB_SUBCMD_INVALID                                          (0)

#define IOSM_FB_SUBCMD_REGISTER_FBDRV                                   (1)
#define IOSM_FB_SUBCMD_UNREGISTER_FBDRV                                 (2)

#define IOSM_FB_SUBCMD_CREATE_EFBDEV		                            (3)
#define IOSM_FB_SUBCMD_DESTROY_EFBDEV		                            (4)
#define IOSM_FB_SUBCMD_WRITE_DISP_DIMENSION                             (5)
#define IOSM_FB_SUBCMD_WRITE_DISP_POS                                   (6)

#define IOSM_FB_SUBCMD_EFBDEV_SHOW                                      (7)
#define IOSM_FB_SUBCMD_EFBDEV_HIDE                                      (8)

#define IOSM_FB_SUBCMD_SW_OVERLAY_FLUSH_SCREEN                          (10)



/********* Status of <IOSM_PCIE_ISP_MSG_RESPONSE> *********/
#define IOSM_FB_STATUS1_INVALID                   (0)
#define IOSM_FB_STATUS1_SUCCESS                   (1)
#define IOSM_FB_STATUS1_FAIL                      (2)


#ifndef __ASSEMBLY__

typedef enum
{
    EFBDEV_ID_INVALID = 0,
    EFBDEV_ID_NS = 1,
    EFBDEV_ID_SURFACEA = 2,
    EFBDEV_ID_SURFACEB = 3,
    EFBDEV_ID_SURFACEC = 4,
} EFBDEV_ID;

/// @brief The pixel format of ephemeral framebuffer device (eFBDev).
typedef enum
{
    PIXEL_FORMAT_INVALID = 0,
    PIXEL_FORMAT_RGBX8888 = 1,
} EFBDEV_PIXEL_FORMAT;

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) IOSM_FB_ISP_MSG_HEADER;

#define IOSM_FB_ISP_MSG_REQUEST_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_FB_ISP_MSG_REQUEST_MAX_DATA_SIZE];
} IOSM_FB_ISP_MSG_REQUEST;

#define IOSM_FB_ISP_MSG_RESPONSE_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_FB_ISP_MSG_RESPONSE_MAX_DATA_SIZE];
} IOSM_FB_ISP_MSG_RESPONSE;

typedef struct
{
    IOSM_FB_ISP_MSG_HEADER    header;
    
    union{
        IOSM_FB_ISP_MSG_REQUEST   request;
        IOSM_FB_ISP_MSG_RESPONSE  response;
    };
} IOSM_FB_ISP_MSG;


typedef struct
{
    EFBDEV_ID s_efbdev_id;
    uint32_t max_rec_x;
    uint32_t max_rec_y;
    uint32_t cur_rec_x;
    uint32_t cur_rec_y;
    EFBDEV_PIXEL_FORMAT pixel_format;
} SUBCMD_CREATE_EFBDEV_PARAMS;

typedef struct
{
    shandle_t   h_fb;
    uint32_t    efbdev_slot;
    bool        enable_sw_overlay; // Is the eFBDev using software overlay? If true, the client needs to call IOSM-FB to 
                        // flush screen periodically.
} SUBCMD_CREATE_EFBDEV_RESPONSE;

typedef struct
{
    uint32_t efbdev_slot;
    uint32_t new_x;
    uint32_t new_y;
} SUBCMD_WRITE_DISP_DIMENSION_PARAMS;

typedef struct
{
    uint32_t efbdev_slot;
    uint32_t new_x;
    uint32_t new_y;
} SUBCMD_WRITE_DISP_POS_PARAMS;


#endif // __ASSEMBLY__
#endif // IOSM_FB_INTERFACES_H