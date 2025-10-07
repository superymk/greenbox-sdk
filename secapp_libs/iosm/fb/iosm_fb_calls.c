// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include <iosm_fb_sp_intfs.h>
#include <libfb_headers.h>
#include "iosm_fb_calls.h"

/********* Data *********/
static COMM_ISP_SHAREDMEM* comm_iosm_fb = NULL;
static spid_t iosm_fb_spid = INVALID_SP_ID;




/********* Public Functions *********/
/// @brief Create Inter-SecProcess communication channels required by IOSM-FB.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
SAP_STATUS comm_iosm_fb_init(spid_t in_iosm_fb_spid)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE* iosm_fb_msg = NULL;

    // Create communication channel with the IOSM-FB service.
	comm_iosm_fb = comm_isp_sharedmem_create(in_iosm_fb_spid, IOSM_FB_INCOMMING_COMM_ISP_PAGES);
    if(!comm_iosm_fb)
    {
        FB_ERR("Failed to create Inter-SecProcess communication channel with IOSM-FB (id:%u)!\n", in_iosm_fb_spid);
        return EINTERNAL;
    }

    status = comm_isp_sharedmem_register(comm_iosm_fb);
    if(FAIL(status))
    {
        FB_ERR("Failed to register Inter-SecProcess communication channel with IOSM-FB (id:%u)! status:%lu\n", in_iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Switch to the IOSM-FB service to setup the channel.
    sos_sys_switch_to_secprocess_event_handling(in_iosm_fb_spid);
    
    // Check the reponse 
    iosm_fb_msg = (COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE*)comm_iosm_fb->comm_buf;
    if((iosm_fb_msg->is_response) && (iosm_fb_msg->status == COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED))
    {
        iosm_fb_spid = in_iosm_fb_spid;
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

void comm_iosm_fb_fini(void)
{
    if(comm_iosm_fb)
    {
        comm_isp_sharedmem_unregister_and_close(comm_iosm_fb);
        comm_iosm_fb = NULL;
    }
}

static bool _is_response_success(IOSM_FB_ISP_MSG* msg)
{
    if((msg->header.is_response) && (msg->response.status1 == IOSM_FB_STATUS1_SUCCESS))
        return true;
    
    return false;
}




/********* Operations of IOSM-FB *********/
static bool _check_area(uint32_t max_rec_x, uint32_t max_rec_y, uint32_t cur_rec_x, uint32_t cur_rec_y)
{
    uint64_t max_area = ((uint64_t)max_rec_x) * max_rec_y;
    uint64_t cur_area = ((uint64_t)cur_rec_x) * cur_rec_y;

    if(max_area >= cur_area)
        return true;
    else
        return false;
}

/// @brief Register the caller SecProcess's FB driver in IOSM-FB.
SAP_STATUS iosm_fb_register_fbdrv(uint32_t* out_io_pid)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_io_pid)
        return EINVAL;

    // Call IOSM-FB
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_REGISTER_FBDRV, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_REGISTER_FBDRV! status:%lu\n", iosm_fb_spid, status);
        status = EINTERNAL;
        goto call_iosm_fb_err;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
    {
        memcpy(out_io_pid, iosm_fb_msg->response.data, sizeof(uint32_t));
        return SUCCESS;
    }
    else
        return EINTERNAL;


call_iosm_fb_err:
    return status;
}

/// @brief Unregister the caller SecProcess's FB driver in IOSM-FB.
SAP_STATUS iosm_fb_unregister_fbdrv(void)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-FB
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_UNREGISTER_FBDRV, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_UNREGISTER_FBDRV! status:%lu\n", iosm_fb_spid, status);
        status = EINTERNAL;
        goto call_iosm_fb_err;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;


call_iosm_fb_err:
    return status;
}

/// @brief Create an eFBDev by calling IOSM-FB
///
/// @param max_rec_x is the width of SecProcess's display area in pixel.
/// @param max_rec_y is the height of SecProcess's display area in pixel.
SAP_STATUS iosm_fb_create_fbdev(
    EFBDEV_ID efbdev_id, 
    uint32_t max_rec_x, 
    uint32_t max_rec_y, 
    uint32_t cur_rec_x, 
    uint32_t cur_rec_y, 
    EFBDEV_PIXEL_FORMAT pixel_format, 
    size_t fb_sz, 
    uint32_t* out_efbdev_slot,
    shandle_t* out_h_fb, 
    vaddr_t* out_fb_addr,
    bool* out_enabled_sw_overlay
)
{
    SAP_STATUS status = SUCCESS;
    SAP_STATUS err_status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    SUBCMD_CREATE_EFBDEV_PARAMS params;
    SUBCMD_CREATE_EFBDEV_RESPONSE result;
    vaddr_t fb_vaddr = INVALID_VADDR;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    if(!out_efbdev_slot || !out_h_fb || !out_fb_addr || !out_enabled_sw_overlay)
        return EINVAL;

    if(!max_rec_x || !max_rec_y)
        return EINVAL;
    if(!cur_rec_x || !cur_rec_y)
        return EINVAL;
    if(!_check_area(max_rec_x, max_rec_y, cur_rec_x, cur_rec_y))
    {
        FB_ERR("[IOSM-FB (id:%u)] Invalid init window size (x:%u, y:%u) with the max window size (x:%u, y:%u)!\n", iosm_fb_spid, 
            cur_rec_x, cur_rec_y, max_rec_x, max_rec_y);
        return EINVAL;
    }

    // Call IOSM-FB
    params.s_efbdev_id = efbdev_id;
    params.max_rec_x = max_rec_x;
    params.max_rec_y = max_rec_y;
    params.cur_rec_x = cur_rec_x;
    params.cur_rec_y = cur_rec_y;
    params.pixel_format = pixel_format;
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_CREATE_EFBDEV, &params, sizeof(SUBCMD_CREATE_EFBDEV_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_CREATE_EFBDEV! status:%lu\n", iosm_fb_spid, status);
        status = EINTERNAL;
        goto call_iosm_fb_err;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(!_is_response_success(iosm_fb_msg))
    {
        status = EINTERNAL;
        goto iosm_fb_resp_err;
    }

    // Reference the memory defined by <buf_4k_pages>
    memcpy(&result, iosm_fb_msg->response.data, sizeof(SUBCMD_CREATE_EFBDEV_RESPONSE));
    {
        shandle_t h_fb = result.h_fb;
        
        fb_vaddr = mm_pick_vaddr(fb_sz);
        if(fb_vaddr == INVALID_VADDR)
        {
            FB_ERR("Create eFBDev Error: Failed to allocate virtual memory address for the USB buffer (handle:%u)!\n", h_fb);
            status = EINTERNAL;
            goto create_buf_addr_err;
        }

        // Map the frame buffer with cache enabled to improve the memory access performance to it.
        status = sos_sys_obj_reference_object(h_fb, fb_vaddr, OA_RW_CACHE);
        if(FAIL(status))
        {
            FB_ERR("Create eFBDev Error: Failed to reference the USB buffer (handle:%u)! status:%lu\n", h_fb, status);
            status = EINTERNAL;
            goto ref_buf_err;
        }
    }

    *out_h_fb = result.h_fb;
    *out_efbdev_slot = result.efbdev_slot;
    *out_fb_addr = fb_vaddr;
    *out_enabled_sw_overlay = result.enable_sw_overlay;
    
    return SUCCESS;
    
    
ref_buf_err:
    mm_return_vaddr(fb_vaddr, fb_sz);
create_buf_addr_err:
    err_status = iosm_fb_destroy_fbdev(efbdev_id, result.h_fb, fb_vaddr, fb_sz);
    if(FAIL(err_status))
        FB_ERR("Create eFBDev Error: Failed to destroy the eFBPDev (id:%u) on registration errors! status:%lu\n", efbdev_id, err_status);
iosm_fb_resp_err:
call_iosm_fb_err:
    return status;
}

SAP_STATUS iosm_fb_destroy_fbdev(uint32_t efbdev_slot, shandle_t h_fb, vaddr_t fb_addr, size_t fb_sz)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    if((h_fb == INVALID_HANDLE) || (fb_addr == INVALID_VADDR))
        return EINVAL;

    if(!fb_sz)
        return EINVAL;

    // Unreference the memory defined by <h_fb> and <fb_addr>
    {
        status = sos_sys_obj_unreference_object(h_fb);
        if(FAIL(status))
            FB_ERR("Unregister eFBDev error: Failed to unref memory object (handle:%u)! status:%lu\n", 
			    h_fb, status);

        mm_return_vaddr(fb_addr, fb_sz);
    }

    // Call IOSM-FB
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_DESTROY_EFBDEV, &efbdev_slot, sizeof(uint32_t), NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_DESTROY_EFBDEV! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(!_is_response_success(iosm_fb_msg))
        return EINTERNAL;

    return SUCCESS;
}

SAP_STATUS iosm_fb_write_disp_dimension(uint32_t efbdev_slot, uint32_t new_x, uint32_t new_y)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    SUBCMD_WRITE_DISP_DIMENSION_PARAMS params;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    if(!new_x || !new_y)
        return EINVAL;

    // Call IOSM-FB
    params.efbdev_slot = efbdev_slot;
    params.new_x = new_x;
    params.new_y = new_y;
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_WRITE_DISP_DIMENSION, &params, sizeof(SUBCMD_WRITE_DISP_DIMENSION_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_WRITE_DISP_DIMENSION! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

SAP_STATUS iosm_fb_write_disp_pos(uint32_t efbdev_slot, uint32_t new_x, uint32_t new_y)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    SUBCMD_WRITE_DISP_POS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-FB
    params.efbdev_slot = efbdev_slot;
    params.new_x = new_x;
    params.new_y = new_y;
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_WRITE_DISP_POS, &params, sizeof(SUBCMD_WRITE_DISP_POS_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_WRITE_DISP_POS! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

SAP_STATUS iosm_fb_show(uint32_t efbdev_slot)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    // SUBCMD_WRITE_DISP_POS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-FB
    // params.efbdev_slot = efbdev_slot;
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_EFBDEV_SHOW, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_EFBDEV_SHOW! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

SAP_STATUS iosm_fb_hide(uint32_t efbdev_slot)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    // SUBCMD_WRITE_DISP_POS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-FB
    // params.efbdev_slot = efbdev_slot;
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_EFBDEV_HIDE, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_EFBDEV_HIDE! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}

SAP_STATUS iosm_fb_flush_frame_buffer(vaddr_t fb_vaddr, size_t fb_sz)
{
    VKSTATUS sos_status = SUCCESS;

    sos_status = sos_sys_mm_flush_cache_range(fb_vaddr, fb_sz);
    if(FAIL(sos_status))
    {
        FB_ERR("Failed to flush frame buffer! status:%lu\n", sos_status);
        return sos_status;
    }
        
    return SUCCESS;
}

SAP_STATUS iosm_fb_sw_overlay_flush_screen(uint32_t efbdev_slot)
{
    SAP_STATUS status = SUCCESS;
    IOSM_FB_ISP_MSG* iosm_fb_msg = NULL;
    // SUBCMD_WRITE_DISP_POS_PARAMS params;

    // Sanity checks
    if(!comm_iosm_fb)
        return EINVAL;

    if(iosm_fb_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-FB
    // params.efbdev_slot = efbdev_slot;
    // PERF_ONESHOT_END();
    status = comm_isp_msg_call(iosm_fb_spid, false, IOSM_FB_SUBCMD_SW_OVERLAY_FLUSH_SCREEN, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        FB_ERR("[IOSM-FB (id:%u)] Failed to send the request IOSM_FB_SUBCMD_WRITE_DISP_POS! status:%lu\n", iosm_fb_spid, status);
        return EINTERNAL;
    }

    // Check the response
    iosm_fb_msg = (IOSM_FB_ISP_MSG*)comm_iosm_fb->comm_buf;
    if(_is_response_success(iosm_fb_msg))
        return SUCCESS;
    else
        return EINTERNAL;
}