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



/// @brief Map <eFBDev ID, Other Info>
static struct efbdev_info_entry _efbdev_slot_map[EFBDEV_SLOTS];

static void _efbdev_slot_map_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, EFBDEV_SLOTS, EFBDEV_SLOTS, 0, 1)
    {
        _efbdev_slot_map[i].efbdev_id = EFBDEV_ID_INVALID;
        _efbdev_slot_map[i].efbdev_slot = 0;
        _efbdev_slot_map[i].h_fb = INVALID_HANDLE;
        _efbdev_slot_map[i].fb_vaddr = INVALID_VADDR;
        _efbdev_slot_map[i].pixel_format = PIXEL_FORMAT_INVALID;
        _efbdev_slot_map[i].fb_sz = 0;
        _efbdev_slot_map[i].max_rec_x = 0;
        _efbdev_slot_map[i].max_rec_y = 0;
        _efbdev_slot_map[i].cur_rec_x = 0;
        _efbdev_slot_map[i].cur_rec_y = 0;
        _efbdev_slot_map[i].cur_pos_x = 0;
        _efbdev_slot_map[i].cur_pos_y = 0;
        _efbdev_slot_map[i].screen_resolution_x = 0;
        _efbdev_slot_map[i].screen_resolution_y = 0;
    }
}

static int _find_slot(EFBDEV_ID efbdev_id)
{
    uint32_t i = 0;

    FOREACH_S(i, EFBDEV_SLOTS, EFBDEV_SLOTS, 0, 1)
    {
        struct efbdev_info_entry* cur = &_efbdev_slot_map[i];
        if(cur->efbdev_id == efbdev_id)
            return i;
    }
        
    return -1;
}




/********* Public functions *********/
void efbdev_mgr_init(void)
{
    _efbdev_slot_map_clean();
}

void efbdev_mgr_fini(void)
{
    _efbdev_slot_map_clean();
}

bool efbdev_register_check_params(
    EFBDEV_ID efbdev_id, 
    EFBDEV_PIXEL_FORMAT pixel_format, 
    size_t fb_sz, 
    uint32_t max_rec_x, 
    uint32_t max_rec_y,
    uint32_t screen_resolution_x,
	uint32_t screen_resolution_y
)
{
    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return false;
    if(pixel_format == PIXEL_FORMAT_INVALID)
        return false;
    if(!max_rec_x || !max_rec_y || !fb_sz)
        return false;
    
    // Check: <max_rec_x/y> must be smaller than <screen_resolution_x/y>
    if((max_rec_x > screen_resolution_x) || (max_rec_y > screen_resolution_y))
        return false;

    return true;
}

SAP_STATUS efbdev_register(
    EFBDEV_ID efbdev_id, 
    uint32_t efbdev_slot, 
    shandle_t h_fb, 
    vaddr_t fb_vaddr, 
    EFBDEV_PIXEL_FORMAT pixel_format, 
    size_t fb_sz, 
    uint32_t max_rec_x, 
    uint32_t max_rec_y,
    uint32_t screen_resolution_x,
	uint32_t screen_resolution_y
)
{
    int idx = 0;

    // Sanity checks
    if(!efbdev_register_check_params(efbdev_id, pixel_format, fb_sz, max_rec_x, max_rec_y, screen_resolution_x, screen_resolution_y))
        return EINVAL;

    if((h_fb == INVALID_HANDLE) || (fb_vaddr == INVALID_VADDR))
        return EINVAL;

    // Check: The eFBDev must not have been registered before
    idx = _find_slot(efbdev_id);
    if(idx != -1)
    {
        FB_ERR("Register eFBDev Error: The eFBDev (id:%u) has been registered!\n", efbdev_id);
        return EINTERNAL;
    }

    idx = _find_slot(EFBDEV_ID_INVALID);
    if(idx == -1)
    {
        FB_ERR("Register eFBDev Error: No space to register the eFBDev (id:%u)!\n", efbdev_id);
        return EINTERNAL;
    }

    _efbdev_slot_map[idx].efbdev_id = efbdev_id;
    _efbdev_slot_map[idx].efbdev_slot = efbdev_slot;
    _efbdev_slot_map[idx].h_fb = h_fb;
    _efbdev_slot_map[idx].fb_vaddr = fb_vaddr;
    _efbdev_slot_map[idx].pixel_format = pixel_format;
    _efbdev_slot_map[idx].fb_sz = fb_sz;
    _efbdev_slot_map[idx].max_rec_x = max_rec_x;
    _efbdev_slot_map[idx].max_rec_y = max_rec_y;
    _efbdev_slot_map[idx].screen_resolution_x = screen_resolution_x;
    _efbdev_slot_map[idx].screen_resolution_y = screen_resolution_y;
    _efbdev_slot_map[idx].cur_rec_x = 0;
    _efbdev_slot_map[idx].cur_rec_y = 0;
    _efbdev_slot_map[idx].cur_pos_x = 0;
    _efbdev_slot_map[idx].cur_pos_y = 0;

    return SUCCESS;
}

SAP_STATUS efbdev_unregister(EFBDEV_ID efbdev_id)
{
    int idx = 0;

    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    // Unregister
    idx = _find_slot(efbdev_id);
    if(idx == -1)
    {
        FB_ERR("Unregister eFBDev Error: The eFBDev (id:%u) has not been registered!\n", efbdev_id);
        return EINTERNAL;
    }

    _efbdev_slot_map[idx].efbdev_id = EFBDEV_ID_INVALID;
    _efbdev_slot_map[idx].efbdev_slot = 0;
    _efbdev_slot_map[idx].h_fb = INVALID_HANDLE;
    _efbdev_slot_map[idx].fb_vaddr = INVALID_VADDR;
    _efbdev_slot_map[idx].pixel_format = PIXEL_FORMAT_INVALID;
    _efbdev_slot_map[idx].fb_sz = 0;
    _efbdev_slot_map[idx].max_rec_x = 0;
    _efbdev_slot_map[idx].max_rec_y = 0;
    _efbdev_slot_map[idx].cur_rec_x = 0;
    _efbdev_slot_map[idx].cur_rec_y = 0;
    _efbdev_slot_map[idx].cur_pos_x = 0;
    _efbdev_slot_map[idx].cur_pos_y = 0;
    _efbdev_slot_map[idx].screen_resolution_x = 0;
    _efbdev_slot_map[idx].screen_resolution_y = 0;

    return SUCCESS;
}

/// @brief Return the info of eFBDev <efbdev_id>
SAP_STATUS efbdev_get_info(EFBDEV_ID efbdev_id, struct efbdev_info_entry** out_efbdev_info)
{
    int idx = 0;

    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    if(!out_efbdev_info)
        return EINVAL;

    // Get slot
    idx = _find_slot(efbdev_id);
    if(idx == -1)
    {
        FB_ERR("Get eFBDev Error: The eFBDev (id:%u) has not been registered!\n", efbdev_id);
        return EINTERNAL;
    }

    *out_efbdev_info = &_efbdev_slot_map[idx];
    return SUCCESS;
}

SAP_STATUS efbdev_update_disp_dimension(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y)
{
    SAP_STATUS status = SUCCESS;
    struct efbdev_info_entry* entry = NULL;

    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    // Get the info of the eFBDev
    status = efbdev_get_info(efbdev_id, &entry);
    if(FAIL(status))
    {
        FB_ERR("Update eFBDev Display Dimension Error: Failed to get info of the eFBDev (id:%u) status:%lu!\n", efbdev_id, status);
        return status;
    }

    // Check: The new eFBDev window must be in the screen area
    if((new_x <= entry->max_rec_x && (new_x + entry->cur_pos_x) <= entry->screen_resolution_x) &&
        (new_y <= entry->max_rec_y && (new_y + entry->cur_pos_y) <= entry->screen_resolution_y)
    )
    {
        entry->cur_rec_x = new_x;
        entry->cur_rec_y = new_y;

        return SUCCESS;
    }

    // Return EINVAL if the new eFBDev window is out of the screen area
    return EINVAL;
}

SAP_STATUS efbdev_update_disp_pos(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y)
{
    SAP_STATUS status = SUCCESS;
    struct efbdev_info_entry* entry = NULL;

    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    // Get the info of the eFBDev
    status = efbdev_get_info(efbdev_id, &entry);
    if(FAIL(status))
    {
        FB_ERR("Update eFBDev Display Position Error: Failed to get info of the eFBDev (id:%u) status:%lu!\n", efbdev_id, status);
        return status;
    }

    // Check: The new eFBDev window must be in the screen area
    if((new_x <= entry->max_rec_x && (new_x + entry->cur_rec_x) <= entry->screen_resolution_x) &&
        (new_y <= entry->max_rec_y && (new_y + entry->cur_rec_y) <= entry->screen_resolution_y)
    )
    {
        entry->cur_pos_x = new_x;
        entry->cur_pos_y = new_y;

        return SUCCESS;
    }

    // Return EINVAL if the new eFBDev window is out of the screen area
    return EINVAL;
}

/// @brief Calculate byte size for the given pixel format.
SAP_STATUS pixel_format_to_byte_sz(EFBDEV_PIXEL_FORMAT pixel_format, size_t* out_mem_sz)
{
    // Sanity checks
    if(!out_mem_sz)
        return EINVAL;

    switch(pixel_format)
    {
        case PIXEL_FORMAT_RGBX8888:
        {
            *out_mem_sz = 4;
            return SUCCESS;
        }
        default:
        {
            *out_mem_sz = 0;
            return EINVAL;
        }
    }

    return EINVAL;
}

/// @brief Return the byte size corresponding to the pixel format used by the eFBDev <efbdev_id>.
SAP_STATUS efbdev_get_pixel_byte_sz(EFBDEV_ID efbdev_id, size_t* out_mem_sz)
{
    SAP_STATUS status = SUCCESS;
    struct efbdev_info_entry* entry = NULL;

    // Sanity checks
    if((efbdev_id == EFBDEV_ID_INVALID) || (efbdev_id == EFBDEV_ID_NS))
        return EINVAL;

    if(!out_mem_sz)
        return EINVAL;

    // Get the info of the eFBDev
    status = efbdev_get_info(efbdev_id, &entry);
    if(FAIL(status))
    {
        FB_ERR("Get eFBDev's pixel size in bytes Error: Failed to get info of the eFBDev (id:%u) status:%lu!\n", efbdev_id, status);
        return status;
    }

    // Calculate byte size for a pixel, and return
    return pixel_format_to_byte_sz(entry->pixel_format, out_mem_sz);
}