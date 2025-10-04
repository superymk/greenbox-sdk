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

#include <palddk.h>
#include <libfb_headers.h>

#include "iosm_fb_calls.h"

static uint32_t io_pid = IOSM_FB_IO_PID_INVALID;
static bool enabled_sw_overlay = false;

/********* Public Functions *********/
SAP_STATUS libfb_init(spid_t iosm_fb_spid)
{
	SAP_STATUS status = SUCCESS;

	// Sanity checks
	if (iosm_fb_spid == INVALID_SP_ID)
		return EINVAL;

	// Create communication channel with the IOSM-FB service.
	status = comm_iosm_fb_init(iosm_fb_spid);
	if (FAIL(status))
	{
		FB_ERR("Init Lib-FB error: Failed to init Inter-SecProcess communication channel with IOSM-FB (id:%u)! status:%lu\n", iosm_fb_spid, status);
		status = EINTERNAL;
		goto init_comm_err;
	}

	// Init eFBDev manager
	efbdev_mgr_init();

	// Create FB I/O partition and register FB driver
	status = iosm_fb_register_fbdrv(&io_pid);
	if (FAIL(status))
	{
		FB_ERR("Init Lib-FB error: Failed to register FB driver with IOSM-FB (id:%u)! status:%lu\n", iosm_fb_spid, status);
		status = EINTERNAL;
		goto register_fbdrv_partition_err;
	}

	FB_INFO("Initial Lib-FB Success: Created FB I/O partition (id:%u)\n", io_pid);
	return SUCCESS;

register_fbdrv_partition_err:
	efbdev_mgr_fini();
	comm_iosm_fb_fini();
init_comm_err:
	io_pid = IOSM_FB_IO_PID_INVALID;
	return status;
}

void libfb_fini(void)
{
	SAP_STATUS status = SUCCESS;

	// Unregister FB driver and destroy partition
	status = iosm_fb_unregister_fbdrv();
	if (FAIL(status))
		FB_ERR("Finalize Lib-FB error: Failed to unregister FB driver with IOSM-FB! status:%lu\n", status);

	// Finalize eFBDev manager
	efbdev_mgr_fini();

	// Destroy communication channel with the IOSM-USB2 service.
	comm_iosm_fb_fini();

	io_pid = IOSM_FB_IO_PID_INVALID;
}

SAP_STATUS libfb_create_efbdev(
	EFBDEV_ID efbdev_id,
	uint32_t max_rec_x,
	uint32_t max_rec_y,
	uint32_t cur_rec_x,
	uint32_t cur_rec_y,
	uint32_t screen_resolution_x,
	uint32_t screen_resolution_y,
	EFBDEV_PIXEL_FORMAT pixel_format,
	vaddr_t *out_fb_vaddr,
	size_t *out_fb_sz
)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS err_status = SUCCESS;
	shandle_t h_fb = INVALID_HANDLE;
	vaddr_t fb_vaddr = INVALID_VADDR;
	size_t pixel_bytes = 0;
	size_t fb_sz = 0;
	uint32_t efbdev_slot = 0;
	 

	// Sanity checks
	if ((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
		return EINVAL;

	if (!out_fb_vaddr || !out_fb_sz)
		return EINVAL;

	// Calculate <fb_sz>
	status = pixel_format_to_byte_sz(pixel_format, &pixel_bytes);
	if (FAIL(status) || !pixel_bytes)
	{
		FB_ERR("Create Ephemeral FB Device Error: Failed to calculate pixel format's byte size! status:%lu\n", status);
		return EINVAL;
	}
	fb_sz = SIZE_ROUNDUP_4K(max_rec_x * max_rec_y * pixel_bytes);

	// Check most of the parameters of <efbdev_register> before calling <iosm_fb_create_fbdev>, so we can fail early 
	// caused by most of the invalid parameters.
	if(!efbdev_register_check_params(efbdev_id, pixel_format, fb_sz, max_rec_x, max_rec_y, screen_resolution_x, screen_resolution_y))
	{
		FB_ERR("Create Ephemeral FB Device Error: Invalid parameters for <efbdev_register> when creating device (id:%u)!\n", efbdev_id);
		return EINTERNAL;
	}

	// Create eFBDev
	status = iosm_fb_create_fbdev(efbdev_id, max_rec_x, max_rec_y, cur_rec_x, cur_rec_y, pixel_format, fb_sz, &efbdev_slot, &h_fb, &fb_vaddr, &enabled_sw_overlay);
	if (FAIL(status))
	{
		FB_ERR("Create Ephemeral FB Device Error: Failed to create device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	// Register eFBDev
	status = efbdev_register(efbdev_id, efbdev_slot, h_fb, fb_vaddr, pixel_format, fb_sz, max_rec_x, max_rec_y, screen_resolution_x, screen_resolution_y);
	if (FAIL(status))
	{
		FB_ERR("Create Ephemeral FB Device Error: Failed to register device (id:%u)! status:%lu\n", efbdev_id, status);
		status = EINTERNAL;
		goto register_err;
	}

	// Record the initial window dimension
	status = libfb_write_disp_dimension(efbdev_id, cur_rec_x, cur_rec_y);
	if (FAIL(status))
	{
		FB_ERR("Create Ephemeral FB Device Error: Failed to register device (id:%u)'s dimension! status:%lu\n", efbdev_id, status);
		status = EINTERNAL;
		goto register_err;
	}

	*out_fb_vaddr = fb_vaddr;
	*out_fb_sz = fb_sz;
	return SUCCESS;


register_err:
	err_status = iosm_fb_destroy_fbdev(efbdev_id, h_fb, fb_vaddr, fb_sz);
	if (FAIL(err_status))
		FB_ERR("Create Ephemeral FB Device Error: Failed to destroy the device (id:%u) on registration errors! status:%lu\n", efbdev_id, err_status);
	return status;
}

SAP_STATUS libfb_destroy_efbdev(EFBDEV_ID efbdev_id)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS status1 = SUCCESS, status2 = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;

	// Sanity checks
	if ((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
		return EINVAL;

	// Get info of eFBDev <efbdev_id>
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Destroy Ephemeral FB Device Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);

		// If we cannot get info of the eFBDev, we cannot destroy the eFBDev further.
		return EINTERNAL;
	}

	// Destroy eFBDev
	status1 = iosm_fb_destroy_fbdev(efbdev_info->efbdev_slot, efbdev_info->h_fb, efbdev_info->fb_vaddr, efbdev_info->fb_sz);
	if (FAIL(status1))
		FB_ERR("Destroy Ephemeral FB Device Error: Failed to destroy device (id:%u)(slot:%u)! status:%lu\n", efbdev_id, efbdev_info->efbdev_slot, status1);

	// Unregister eFBDev
	status2 = efbdev_unregister(efbdev_id);
	if (FAIL(status2))
		FB_ERR("Destroy Ephemeral FB Device Error: Failed to unregister device (id:%u)! status:%lu\n", efbdev_id, status2);

	// Return
	if (PASS(status1) && PASS(status2))
		return SUCCESS;
	else
		return EINTERNAL;
}

shandle_t libfb_efbdev_get_fb_handle(EFBDEV_ID efbdev_id)
{
	SAP_STATUS status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;

	// Get info of eFBDev <efbdev_id>
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Get frame buffer handle of eFBDev (id:%u) error! status:%lu\n", efbdev_id, status);

		return INVALID_HANDLE;
	}

	return efbdev_info->h_fb;
}

SAP_STATUS libfb_write_disp_dimension(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS err_status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;
	uint32_t orig_x = 0, orig_y = 0;

	// Sanity checks
	if ((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
		return EINVAL;

	// Get info of eFBDev <efbdev_id>
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Write Ephemeral FB Device Dimension Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	// Backup the old dimension
	orig_x = efbdev_info->cur_rec_x;
	orig_y = efbdev_info->cur_rec_y;

	// Write new dimension
	status = efbdev_update_disp_dimension(efbdev_id, new_x, new_y);
	if (FAIL(status))
	{
		FB_ERR("Write Ephemeral FB Device Dimension Error: Failed to update info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	// Call IOSM-FB
	status = iosm_fb_write_disp_dimension(efbdev_info->efbdev_slot, new_x, new_y);
	if (FAIL(status))
	{
		FB_ERR("Write Ephemeral FB Device Dimension Error: Failed to update device (id:%u)! status:%lu\n", efbdev_id, status);
		status = EINTERNAL;
		goto call_iosm_fb_err;
	}

	return SUCCESS;

call_iosm_fb_err:
	err_status = efbdev_update_disp_dimension(efbdev_id, orig_x, orig_y);
	if (FAIL(err_status))
		FB_ERR("Write Ephemeral FB Device Dimension Error: Failed to restore original dimension of device (id:%u)! status:%lu\n", efbdev_id, err_status);
	return status;
}

SAP_STATUS libfb_write_disp_position(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y)
{
	SAP_STATUS status = SUCCESS;
	SAP_STATUS err_status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;
	uint32_t orig_x = 0, orig_y = 0;

	// Sanity checks
	if ((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
		return EINVAL;

	// Get info of eFBDev <efbdev_id>
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Write Ephemeral FB Device Position Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	// Backup the old dimension
	orig_x = efbdev_info->cur_pos_x;
	orig_y = efbdev_info->cur_pos_y;

	// Write new dimension
	status = efbdev_update_disp_pos(efbdev_id, new_x, new_y);
	if (FAIL(status))
	{
		FB_ERR("Write Ephemeral FB Device Position Error: Failed to update info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	// Call IOSM-FB
	status = iosm_fb_write_disp_pos(efbdev_info->efbdev_slot, new_x, new_y);
	if (FAIL(status))
	{
		FB_ERR("Write Ephemeral FB Device Position Error: Failed to update device (id:%u)! status:%lu\n", efbdev_id, status);
		status = EINTERNAL;
		goto call_iosm_fb_err;
	}

	return SUCCESS;

call_iosm_fb_err:
	err_status = efbdev_update_disp_dimension(efbdev_id, orig_x, orig_y);
	if (FAIL(err_status))
		FB_ERR("Write Ephemeral FB Device Position Error: Failed to restore original position of device (id:%u)! status:%lu\n", efbdev_id, err_status);
	return status;
}

SAP_STATUS libfb_overlay_present(EFBDEV_ID efbdev_id)
{
	SAP_STATUS status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;

	// Flush Frame buffer to memory
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Present Overlay Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	status = iosm_fb_flush_frame_buffer(efbdev_info->fb_vaddr, efbdev_info->fb_sz);
	if (FAIL(status))
	{
		FB_ERR("Present Overlay Error: Failed to flush frame buffer of device (id:%u)! status:%lu\n", efbdev_id, status);
		return status;
	}

	// If using SW overlay, libfb needs to call IOSM-FB to perform SW overlay
	if(enabled_sw_overlay)
	{
		status = iosm_fb_sw_overlay_flush_screen(efbdev_info->efbdev_slot);
		if (FAIL(status))
		{
			FB_ERR("Flush screen with SW overlay device (id:%u) error! status:%lu\n", efbdev_id, status);
			return EINTERNAL;
		}
	}

	// On success
	return SUCCESS;
}

SAP_STATUS libfb_overlay_show(EFBDEV_ID efbdev_id)
{
	SAP_STATUS status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;

	// Flush Frame buffer to memory
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Show Overlay Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	status = iosm_fb_show(efbdev_info->efbdev_slot);
	if (FAIL(status))
	{
		FB_ERR("Show Overlay Error: Failed to flush frame buffer of device (id:%u)! status:%lu\n", efbdev_id, status);
		return status;
	}

	// On success
	return SUCCESS;
}

SAP_STATUS libfb_overlay_hide(EFBDEV_ID efbdev_id)
{
	SAP_STATUS status = SUCCESS;
	struct efbdev_info_entry *efbdev_info = NULL;

	// Flush Frame buffer to memory
	status = efbdev_get_info(efbdev_id, &efbdev_info);
	if (FAIL(status) || !efbdev_info)
	{
		FB_ERR("Hide Overlay Error: Failed to get info of device (id:%u)! status:%lu\n", efbdev_id, status);
		return EINTERNAL;
	}

	status = iosm_fb_hide(efbdev_info->efbdev_slot);
	if (FAIL(status))
	{
		FB_ERR("Hide Overlay Error: Failed to flush frame buffer of device (id:%u)! status:%lu\n", efbdev_id, status);
		return status;
	}

	// On success
	return SUCCESS;
}