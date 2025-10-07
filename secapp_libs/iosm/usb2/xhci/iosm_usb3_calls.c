// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include <iosm_usb3_sp_intfs.h>
#include "../lib-iosm-usb-internal.h"
#include "../util.h"
#include "iosm_usb3_calls.h"

/********* Data *********/
static COMM_ISP_SHAREDMEM* comm_iosm_usb3 = NULL;
static spid_t iosm_usb3_spid = INVALID_SP_ID;

/// @brief The default USB address after a device is reset
#define USB_DEFAULT_ADDR  		(0)




/********* Public Functions *********/
/// @brief Create Inter-SecProcess communication channels required by IOSM-USB3.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
SAP_STATUS comm_iosm_usb3_init(spid_t in_iosm_usb3_spid)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE* iosm_usb3_msg = NULL;

    // Create communication channel with the IOSM-USB3 service.
	comm_iosm_usb3 = comm_isp_sharedmem_create(in_iosm_usb3_spid, IOSM_USB3_INCOMMING_COMM_ISP_PAGES);
    if(!comm_iosm_usb3)
    {
        USB_ERR("Failed to create Inter-SecProcess communication channel with IOSM-USB3 (id:%u)!\n", in_iosm_usb3_spid);
        return EINTERNAL;
    }

    status = comm_isp_sharedmem_register(comm_iosm_usb3);
    if(FAIL(status))
    {
        USB_ERR("Failed to register Inter-SecProcess communication channel with IOSM-USB3 (id:%u)! status:%lu\n", in_iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Switch to the IOSM-USB3 service to setup the channel.
    sos_sys_switch_to_secprocess_event_handling(in_iosm_usb3_spid);
    
    // Check the reponse 
    iosm_usb3_msg = (COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE*)comm_iosm_usb3->comm_buf;
    if((iosm_usb3_msg->is_response) && (iosm_usb3_msg->status == COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED))
    {
        iosm_usb3_spid = in_iosm_usb3_spid;
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

void comm_iosm_usb3_fini(void)
{
    if(comm_iosm_usb3)
    {
        comm_isp_sharedmem_unregister_and_close(comm_iosm_usb3);
        comm_iosm_usb3 = NULL;
    }
}

static bool _is_response_success(IOSM_USB3_ISP_MSG* msg)
{
    if((msg->header.is_response) && (msg->response.status1 == IOSM_USB3_STATUS1_SUCCESS))
        return true;
    
    return false;
}




/********* Operations of IOSM-USB3 *********/
/// @brief Register the caller SecProcess's USB peripheral driver in IOSM-USB3.
///
/// @param do_4k_pages is the number of 4K pages to be used by the driver's USB commands and buffers.
SAP_STATUS iosm_usb3_register_usbdrv(size_t buf_4k_pages, uint32_t* out_io_pid, shandle_t* out_h_buf_obj, vaddr_t* out_buf_addr)
{
    SAP_STATUS status = SUCCESS;
    SAP_STATUS err_status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_REGISTER_USBDRV_PARAMS params;
    IOSM_USB3_SUBCMD_REGISTER_USBDRV_RESPONSE result;
    vaddr_t buf_vaddr = INVALID_VADDR;
    size_t buf_size = PAGE_4K_TO_BYTES(buf_4k_pages);

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_io_pid || !out_h_buf_obj || !out_buf_addr)
        return EINVAL;

    if(!buf_4k_pages)
        return EINVAL;

    // Call IOSM-USB3
    params.do_4k_pages = buf_4k_pages;
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_REGISTER_USBDRV, &params, sizeof(IOSM_USB3_SUBCMD_REGISTER_USBDRV_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_REGISTER_USBDRV! status:%lu\n", iosm_usb3_spid, status);
        status = EINTERNAL;
        goto call_iosm_usb3_err;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(!_is_response_success(iosm_usb3_msg))
    {
        status = EINTERNAL;
        goto iosm_usb3_resp_err;
    }

    // Reference the memory defined by <buf_4k_pages>
    memcpy(&result, iosm_usb3_msg->response.data, sizeof(IOSM_USB3_SUBCMD_REGISTER_USBDRV_RESPONSE));
    {
        shandle_t h_buf_obj = result.h_do_obj;
        
        buf_vaddr = mm_pick_vaddr(buf_size);
        if(buf_vaddr == INVALID_VADDR)
        {
            USB_ERR("[IOSM-USB3 (id:%u)] Failed to allocate virtual memory address for the USB buffer (handle:%u)!\n", iosm_usb3_spid, h_buf_obj);
            status = EINTERNAL;
            goto create_buf_addr_err;
        }

        status = sos_sys_obj_reference_object(h_buf_obj, buf_vaddr, OA_RW_NO_CACHE);
        if(FAIL(status))
        {
            USB_ERR("[IOSM-USB3 (id:%u)] Failed to reference the USB buffer (handle:%u)! status:%lu\n", iosm_usb3_spid, h_buf_obj, status);
            status = EINTERNAL;
            goto ref_buf_err;
        }
    }

    *out_io_pid = result.io_pid;
    *out_h_buf_obj = result.h_do_obj;
    *out_buf_addr = buf_vaddr;
    
    return SUCCESS;
    
    
ref_buf_err:
    mm_return_vaddr(buf_vaddr, buf_size);
create_buf_addr_err:
    err_status = iosm_usb3_unregister_usbdrv(result.h_do_obj, buf_vaddr, buf_size);
    if(FAIL(err_status))
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to unregister the USB driver on registration errors! status:%lu\n", iosm_usb3_spid, err_status);
iosm_usb3_resp_err:
call_iosm_usb3_err:
    return status;
}

SAP_STATUS iosm_usb3_unregister_usbdrv(shandle_t h_buf_obj, vaddr_t buf_vaddr, size_t buf_size)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if((h_buf_obj == INVALID_HANDLE) || (buf_vaddr == INVALID_VADDR))
        return EINVAL;

    if(!buf_size)
        return EINVAL;

    // Unreference the memory defined by <h_buf_obj> and <buf_vaddr>
    {
        status = sos_sys_obj_unreference_object(h_buf_obj);
        if(FAIL(status))
            USB_ERR("Unregister USB driver error: Failed to unref memory object (handle:%u)! status:%lu\n", 
			    h_buf_obj, status);

        mm_return_vaddr(buf_vaddr, buf_size);
    }

    // Call IOSM-USB3
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_UNREGISTER_USBDRV, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_UNREGISTER_USBDRV! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(!_is_response_success(iosm_usb3_msg))
        return EINTERNAL;

    return SUCCESS;
}

SAP_STATUS iosm_usb3_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV_PARAMS params;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_usbpdev_slot)
        return EINVAL;

    // Check: IOSM-USB3 clients must not access the USB addr USB_DEFAULT_ADDR (0)
	if(usbpdev_addr.dev_usbaddr == USB_DEFAULT_ADDR)
        return EPERM;
    
    // [NOTE] We do not need to check "usbpdev_addr.hub_usbaddr == USB_DEFAULT_ADDR", because (1) Windows 10 sets 
    // roothub's USB addresses to be 0, and (2) the check is meaningless without both os_hc_roothub_usb_addr and 
    // is_root_hub of the hub corresponding to <hub_usbaddr>. So we rely on IOSM-USB3 to check <usbpdev_addr>.

    // Call IOSM-USB3
    params.usbpdev_addr = usbpdev_addr;
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV, &params, sizeof(IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_ACTIVATE_USBPDEV! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
    {
        memcpy(out_usbpdev_slot, iosm_usb3_msg->response.data, sizeof(uint32_t));
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

SAP_STATUS iosm_usb3_deactivate_usbpdev(uint32_t usbpdev_slot)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV_PARAMS params;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-USB3
    params.usbpdev_slot = usbpdev_slot;
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV, &params, sizeof(IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_DEACTIVATE_USBPDEV! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

// SAP_STATUS __iosm_usb3_activate_eehci(union eEHCI_ID* out_eehci_id, uint32_t* out_eehci_handle, uint32_t* out_eehci_slot)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     if(!out_eehci_id || !out_eehci_handle || !out_eehci_slot)
//         return EINVAL;

//     // Call IOSM-USB3
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_ACTIVATE_EEHCI, NULL, 0, NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_ACTIVATE_EEHCI! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//     {
//         IOSM_USB3_SUBCMD_ACTIVATE_EEHCI_RESPONSE result;
//         memcpy(&result, iosm_usb3_msg->response.data, sizeof(IOSM_USB3_SUBCMD_ACTIVATE_EEHCI_RESPONSE));

//         out_eehci_id->v = result.eehci_id;
//         *out_eehci_handle = result.eehci_handle;
//         *out_eehci_slot = result.eehci_slot;
//         return SUCCESS;
//     }
//     else
//         return EINTERNAL;
// }

// SAP_STATUS __iosm_usb3_deactivate_eehci(uint32_t eehci_slot)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_DEACTIVATE_EEHCI_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.eehci_slot = eehci_slot;
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_DEACTIVATE_EEHCI, &params, sizeof(IOSM_USB3_SUBCMD_DEACTIVATE_EEHCI_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_DEACTIVATE_EEHCI! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_reactivate_qtd(uint32_t td_slot)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_REACTIVATE_QTD_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.td_slot = td_slot;
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_REACTIVATE_QTD, &params, sizeof(IOSM_USB3_SUBCMD_REACTIVATE_QTD_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_REACTIVATE_QTD! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_ehci_waittick(void)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_EHCI_WAITTICK, NULL, 0, NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_EHCI_WAITTICK! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_query_usbtd_status(uint32_t td_slot, uint32_t* out_td_status)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     if(!out_td_status)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.td_slot = td_slot;
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS, &params, sizeof(IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//     {
//         memcpy(out_td_status, iosm_usb3_msg->response.data, sizeof(uint32_t));
//         return SUCCESS;
//     }
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_attach_qh_intr(
//     uint8_t pipe_eptype, 
//     uint32_t pipe_qh_slot, 
//     int pipe_frameexp, 
//     bool need_circular_qtd, 
//     uint32_t cir_from_qtd_slot, 
//     uint32_t cir_to_qtd_slot, 
//     usbpipe_handle_t* out_h_usbpipe
// )
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_ATTACH_QH_INTR_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     if(need_circular_qtd)
//     {
//         params.pipe_qtd_cir_cfg.src_qtd_slot = cir_from_qtd_slot;
//         params.pipe_qtd_cir_cfg.dst_qtd_slot = cir_to_qtd_slot;
//     }
//     else
//     {
//         params.pipe_qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
//         params.pipe_qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;
//     }
//     params.pipe_eptype = pipe_eptype;
//     params.pipe_qh_slot = pipe_qh_slot;
//     params.pipe_frameexp = pipe_frameexp;

//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_ATTACH_QH_INTR, &params, sizeof(IOSM_USB3_SUBCMD_ATTACH_QH_INTR_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_ATTACH_QH_INTR! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//     {
//         memcpy(out_h_usbpipe, iosm_usb3_msg->response.data, sizeof(usbpipe_handle_t));
//         return SUCCESS;
//     }
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_deattach_qh(usbpipe_handle_t h_usbpipe, uint8_t pipe_eptype)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_DEATTACH_QH_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.h_usbpipe = h_usbpipe;
//     params.pipe_eptype = pipe_eptype;

//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_DEATTACH_QH, &params, sizeof(IOSM_USB3_SUBCMD_DEATTACH_QH_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_DEATTACH_QH! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_allocate_usbtd(uint32_t td_type, uint32_t* out_td_slot)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_ALLOCATE_USBTD_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     if(!out_td_slot)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.td_type = td_type;
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_ALLOCATE_USBTD, &params, sizeof(IOSM_USB3_SUBCMD_ALLOCATE_USBTD_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_ALLOCATE_USBTD! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//     {
//         memcpy(out_td_slot, iosm_usb3_msg->response.data, sizeof(uint32_t));
//         return SUCCESS;
//     }
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_deallocate_usbtd(uint32_t td_slot)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_DEALLOCATE_USBTD_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.td_slot = td_slot;

//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_DEALLOCATE_USBTD, &params, sizeof(IOSM_USB3_SUBCMD_DEALLOCATE_USBTD_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_DEALLOCATE_USBTD! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_usbtd_usbmit_and_verify(
//     void* td,
//     size_t td_size, 
//     uint32_t td_slot, 
//     union eEHCI_ID eehci_id, 
//     uint32_t usbpdev_slot, 
//     uint32_t other_params1,
//     uint32_t other_params2,
//     uint32_t other_params3
// )
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_USBTD_SUBMIT_AND_VERIFY_PARAMS* params = NULL;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     if(!td)
//         return EINVAL;

//     if(td_size > USBTD_MAX_SIZE)
//         return EINVAL;

//     // Submit parameters
//     params = (IOSM_USB3_SUBCMD_USBTD_SUBMIT_AND_VERIFY_PARAMS*)comm_iosm_usb3->comm_buf;

//     memcpy(params->usb_td_content, td, td_size);
//     params->td_slot = td_slot;
//     params->eehci_id = eehci_id.v;
//     params->usbpdev_slot = usbpdev_slot;
//     params->other_params1 = other_params1;
//     params->other_params2 = other_params2;
//     params->other_params3 = other_params3;

//     // Call IOSM-USB3
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_TD_SUBMIT_AND_VERIFY, NULL, 0, NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_TD_SUBMIT_AND_VERIFY! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

// SAP_STATUS iosm_usb3_eehci_write_usbtd_reg(uint32_t usbtd_reg_id, uint32_t new_v)
// {
//     SAP_STATUS status = SUCCESS;
//     IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
//     IOSM_USB3_SUBCMD_EEHCI_WRITE_USBTDREG_PARAMS params;

//     // Sanity checks
//     if(!comm_iosm_usb3)
//         return EINVAL;

//     if(iosm_usb3_spid == INVALID_SP_ID)
//         return EINVAL;

//     // Call IOSM-USB3
//     params.usbtd_reg_id = usbtd_reg_id;
//     params.new_v = new_v;
//     status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_EEHCI_WRITE_USBTDREG, &params, sizeof(IOSM_USB3_SUBCMD_EEHCI_WRITE_USBTDREG_PARAMS), NULL, NULL);
//     if(FAIL(status))
//     {
//         USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_EEHCI_WRITE_USBTDREG! status:%lu\n", iosm_usb3_spid, status);
//         return EINTERNAL;
//     }

//     // Check the response
//     iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
//     if(_is_response_success(iosm_usb3_msg))
//         return SUCCESS;
//     else
//         return EINTERNAL;
// }

SAP_STATUS iosm_usb3_submit_trbs(
    struct usb3_pipe* p,
    struct sp_trb* trbs,
    uint32_t num_trbs,
    uint32_t trbs_flags,
    bool iosm_usb3_op_enable_debug
)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY_PARAMS* params = NULL;
    uint32_t i = 0;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!p || !trbs)
        return EINVAL;

    if(num_trbs > TRBS_MAX_NUM)
        return EINVAL;

    // Submit parameters
    params = (IOSM_USB3_SUBCMD_TRBS_SUBMIT_AND_VERIFY_PARAMS*)comm_iosm_usb3->comm_buf;

    memcpy(&params->devaddr, &p->devaddr, sizeof(USBPDEV_ADDR));
    params->ep = p->ep;
    params->num_trbs = num_trbs;
    params->trbs_flags = trbs_flags;
    
    if(iosm_usb3_op_enable_debug)
        params->op_flags |= IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;
    else
        params->op_flags &= ~IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;

    // Copy <trbs> to <params>
    //// [NOTE] Use a for-loop instead of memcpy only to reduce regression errors of not checking <num_trbs>
    FOREACH_S(i, num_trbs, TRBS_MAX_NUM, 0, 1)
    {
        memcpy(&params->trbs[i], &trbs[i], sizeof(struct sp_trb));
    }

    // Call IOSM-USB3
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_SUBMIT_TRBS, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_SUBMIT_TRBS! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

SAP_STATUS iosm_usb3_query_trb_status(USBPDEV_ADDR devaddr, uint32_t ep, uint32_t trb_id, uint32_t* out_trb_status, bool iosm_usb3_op_enable_debug)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_trb_status)
        return EINVAL;

    // Call IOSM-USB3
    params.trb_id = trb_id;
    memcpy(&params.devaddr, &devaddr, sizeof(USBPDEV_ADDR));
    params.ep = ep;

    params.op_flags = 0;
    if(iosm_usb3_op_enable_debug)
        params.op_flags |= IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;
    else
        params.op_flags &= ~IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;

    // Call IOSM-USB3
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_QUERY_TRB_STATUS, &params, sizeof(IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_QUERY_TRB_STATUS! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
    {
        memcpy(out_trb_status, iosm_usb3_msg->response.data, sizeof(uint32_t));
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

SAP_STATUS iosm_usb3_get_usbpdev_speed(USBPDEV_ADDR usbpdev_addr, uint8_t* out_speed)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED_PARAMS params;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_speed)
        return EINVAL;

    if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

    // Check: IOSM-USB3 clients must not access the USB addr USB_DEFAULT_ADDR (0)
    if(usbpdev_addr.dev_usbaddr == USB_DEFAULT_ADDR)
        return EPERM;

    // [NOTE] We do not need to check "usbpdev_addr.hub_usbaddr == USB_DEFAULT_ADDR", because (1) Windows 10 sets 
    // roothub's USB addresses to be 0, and (2) the check is meaningless without both os_hc_roothub_usb_addr and 
    // is_root_hub of the hub corresponding to <hub_usbaddr>. So we rely on IOSM-USB3 to check <usbpdev_addr>.
        
    // Call IOSM-USB3
    params.usbpdev_addr = usbpdev_addr;
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED, &params, sizeof(IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_GET_USBPDEV_SPEED! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
    {
        memcpy(out_speed, iosm_usb3_msg->response.data, sizeof(uint8_t));
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

SAP_STATUS iosm_usb3_usbpdev_get_desc(
    USBPDEV_ADDR usbpdev_addr, 
    struct usb_device_descriptor* out_usbpdev_dev_desc, 
    struct usb_config_descriptor** out_p_usbpdev_config_desc,
    bool iosm_usb3_op_enable_debug
)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_PARAMS params;
    IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_RESPONSE result;
    size_t config_desc_size = 0;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_usbpdev_dev_desc || !out_p_usbpdev_config_desc)
        return EINVAL;

    if(usbpdev_addr_equal(&usbpdev_addr, &USBADDR_INVALID))
        return EINVAL;

    // Check: IOSM-USB3 clients must not access the USB addr USB_DEFAULT_ADDR (0)
    if(usbpdev_addr.dev_usbaddr == USB_DEFAULT_ADDR)
        return EPERM;

    // Call IOSM-USB3
    memcpy(&params.devaddr, &usbpdev_addr, sizeof(USBPDEV_ADDR));
    params.op_flags = 0;
    if(iosm_usb3_op_enable_debug)
        params.op_flags |= IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;
    else
        params.op_flags &= ~IOSM_USB3_SUBCMD_FLAG_ENABLE_DEBUG;

    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_GET_USBPDEV_DESC, &params, sizeof(IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_GET_USBPDEV_DESC! status:%lu\n", iosm_usb3_spid, status);
        status = EINTERNAL;
        goto call_iosm_usb3_err;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(!_is_response_success(iosm_usb3_msg))
    {
        status = EINTERNAL;
        goto iosm_usb3_resp_err;
    }

    // Read the returned result
    memcpy(&result, iosm_usb3_msg->response.data, sizeof(IOSM_USB3_SUBCMD_GET_USBPDEV_DESC_RESPONSE));
    config_desc_size = result.usbpdev_config_desc_size;

    //// Check: the result must be valid.
    {
        if(!usbpdev_addr_equal(&usbpdev_addr, &result.devaddr))
        {
            status = EINTERNAL;
            goto parse_result_err;
        }

        if(config_desc_size > USBPDEV_CONFIG_DESCRIPTOR_MAX_LEN)
        {
            status = EINTERNAL;
            goto parse_result_err;
        }
    }

    *out_p_usbpdev_config_desc = (struct usb_config_descriptor *)usbbuf_alloc(USBBUF_MEM_DMA, config_desc_size);
    if (!*out_p_usbpdev_config_desc)
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Allocate device configuration descriptor error!\n", iosm_usb3_spid);
        status = ENOMEM;
        goto alloc_config_desc_err;
    }

    //// Return the result
    memcpy(out_usbpdev_dev_desc, &result.usbpdev_dev_desc, sizeof(struct usb_device_descriptor));
    memcpy(*out_p_usbpdev_config_desc, &result.usbpdev_config_desc, config_desc_size);
    
    // On success
    return SUCCESS;
    
    
alloc_config_desc_err:
parse_result_err:
iosm_usb3_resp_err:
call_iosm_usb3_err:
    return status;
}




/********* Debug functions *********/
SAP_STATUS iosm_usb3_debug_print_xhci(void)
{
    SAP_STATUS status = SUCCESS;
    IOSM_USB3_ISP_MSG* iosm_usb3_msg = NULL;
    // IOSM_USB3_SUBCMD_QUERY_USBTD_STATUS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_usb3)
        return EINVAL;

    if(iosm_usb3_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-USB3
    status = comm_isp_msg_call(iosm_usb3_spid, false, IOSM_USB3_SUBCMD_DEBUG_PRINT_XHCI, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        USB_ERR("[IOSM-USB3 (id:%u)] Failed to send the request IOSM_USB3_SUBCMD_DEBUG_PRINT_XHCI! status:%lu\n", iosm_usb3_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_usb3_msg = (IOSM_USB3_ISP_MSG*)comm_iosm_usb3->comm_buf;
    if(_is_response_success(iosm_usb3_msg))
    {
        return SUCCESS;
    }
    else
        return EINTERNAL;
}