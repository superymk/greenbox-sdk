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

#ifndef IOSM_USB3_INTERFACES_H
#define IOSM_USB3_INTERFACES_H

#include "usb.h"

/// @brief Number of pages of any Inter-SecProcess channel to the IOSM-USB3 service.
#define IOSM_USB3_INCOMMING_COMM_ISP_PAGES          (1)

/********* Sub-Commands of <SP_MESSAGE> *********/
#define IOSM_USB3_SUBCMD_INVALID                                        (0)

#define IOSM_USB3_SUBCMD_REGISTER_USBDRV                                (1)
#define IOSM_USB3_SUBCMD_UNREGISTER_USBDRV                              (2)
#define IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV                               (3)
#define IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV                             (4)
#define IOSM_USB3_SUBCMD_ACTIVATE_EXHCI                                 (5)
#define IOSM_USB3_SUBCMD_DEACTIVATE_EXHCI                               (6)

#define IOSM_USB3_SUBCMD_SUBMIT_TRBS                                    (21)
// #define IOSM_USB3_SUBCMD_XHCI_WAITTICK                                  (22)
#define IOSM_USB3_SUBCMD_QUERY_TRB_STATUS                               (23)
#define IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED                              (24)
#define IOSM_USB3_SUBCMD_GET_USBPDEV_DESC                               (25)



// #define IOSM_USB3_SUBCMD_REACTIVATE_QTD		                            (21)
// #define IOSM_USB3_SUBCMD_EHCI_WAITTICK		                            (22)
// #define IOSM_USB3_SUBCMD_QUERY_TRB_STATUS                             (23)
// #define IOSM_USB3_SUBCMD_ATTACH_QH_INTR                                 (24)
// #define IOSM_USB3_SUBCMD_DEATTACH_QH                                    (25)
// #define IOSM_USB3_SUBCMD_ALLOCATE_USBTD		                            (26)
// #define IOSM_USB3_SUBCMD_DEALLOCATE_USBTD		                        (27)
// #define IOSM_USB3_SUBCMD_TD_SUBMIT_AND_VERIFY	                        (28)
// #define IOSM_USB3_SUBCMD_EEHCI_WRITE_USBTDREG	                        (29)
// #define IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED	                            (30)

#define IOSM_USB3_SUBCMD_DEBUG_PRINT_XHCI                               (100)



/********* Status of <IOSM_PCIE_ISP_MSG_RESPONSE> *********/
#define IOSM_USB3_STATUS1_INVALID                   (0)
#define IOSM_USB3_STATUS1_SUCCESS                   (1)
#define IOSM_USB3_STATUS1_FAIL                      (2)


#ifndef __ASSEMBLY__

//////// Definitions from the verified code
// Invalid USB TD slot ID
#define USBTD_SlotID_INVALID    0xFFFFFFFF

// #define QH_MAX_QTDS     4

// #define eEHCI_USBTD_SlotID_NUMS         128
// #define WimpUSBPDev_Info_ENTRIES        16
// #define eEHCI_INSTANCE_NUM              128
// #define WimpDrv_Info_ENTRIES            128

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) IOSM_USB3_ISP_MSG_HEADER;

#define IOSM_USB3_ISP_MSG_REQUEST_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_USB3_ISP_MSG_REQUEST_MAX_DATA_SIZE];
} IOSM_USB3_ISP_MSG_REQUEST;

#define IOSM_USB3_ISP_MSG_RESPONSE_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_USB3_ISP_MSG_RESPONSE_MAX_DATA_SIZE];
} IOSM_USB3_ISP_MSG_RESPONSE;

typedef struct
{
    IOSM_USB3_ISP_MSG_HEADER    header;
    
    union{
        IOSM_USB3_ISP_MSG_REQUEST   request;
        IOSM_USB3_ISP_MSG_RESPONSE  response;
    };
} IOSM_USB3_ISP_MSG;




/********* Input Parameters of Sub-Commands *********/
/// Common flags for all IOSM_USB3 operations
#define IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG (1 << 0) // If set, then enable the debug mode of the operation

// #define INVALID_USBPIPE_HANDLE      (0)
// typedef vaddr_t usbpipe_handle_t;
// typedef int usbtd_reg_slot_t;

// // In <usb_new_pipe>'s qtds, which qtd will link which qtd in circle
// struct qtd_circular_cfg{
//     uint32_t src_qtd_slot;   // The qtd_slot that will link the dst_qtd_slot
//     uint32_t dst_qtd_slot;
// };

typedef struct
{
    /// @brief Number of 4K pages for the memory to hold USB DMA data
    size_t do_4k_pages;
} IOSM_USB3_SUBCMD_REGISTER_USBDRV_PARAMS;

typedef struct
{
    uint32_t io_pid;

    /// @brief The memory object to hold USB DMA data
    shandle_t h_do_obj;
} IOSM_USB3_SUBCMD_REGISTER_USBDRV_RESPONSE;

typedef struct
{
    USBPDEV_ADDR usbpdev_addr;
} IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV_PARAMS;

typedef struct
{
    uint32_t usbpdev_slot;
} IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV_PARAMS;

typedef struct
{
    USBPDEV_ADDR devaddr;
    uint32_t ep;
    uint32_t trb_id;

    /// @brief Flags of the IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY operation
    uint32_t op_flags;
} IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS;



#define TRBS_MAX_NUM    (8)

struct sp_trb
{
    uint32_t id;
    struct ring_trb trb;
};

typedef struct
{
    /// @brief Address of the USB device
    USBPDEV_ADDR devaddr;

    /// @brief Endpoint address
    uint32_t ep;

    /// @brief TRB content
    struct sp_trb trbs[TRBS_MAX_NUM];

    /// @brief Number of submitted TRBs
    uint32_t num_trbs;

    /// @brief Flags of the submitted TRBs
    uint32_t trbs_flags; 
    #define SP_TRBS_CHAINED     (1 << 0) // If TRBs are chained, then only the first TRB needs to toggle TRB_C

    /// @brief Flags of the IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY operation
    uint32_t op_flags;
} IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY_PARAMS;

typedef struct
{
    USBPDEV_ADDR usbpdev_addr;
} IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED_PARAMS;



typedef struct
{
    /// @brief Address of the USB device
    USBPDEV_ADDR devaddr;

    /// @brief Flags of the IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY operation
    uint32_t op_flags;
} IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_PARAMS;

#define USBPDEV_CONFIG_DESCRIPTOR_MAX_LEN   (2560)
typedef struct
{
    /// @brief Address of the USB device
    USBPDEV_ADDR devaddr;

    struct usb_device_descriptor usbpdev_dev_desc;

    uint8_t usbpdev_config_desc[USBPDEV_CONFIG_DESCRIPTOR_MAX_LEN];
    uint32_t usbpdev_config_desc_size;
} IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_RESPONSE;


#endif // __ASSEMBLY__
#endif // IOSM_USB3_INTERFACES_H