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

#ifndef LIBUSB2_EEHCI_MGR_H
#define LIBUSB2_EEHCI_MGR_H

#define EFBDEV_SLOTS              (MAX_EFBDEV_NUM)




#ifndef __ASSEMBLY__



struct efbdev_info_entry
{
    EFBDEV_ID           efbdev_id;
    uint32_t            efbdev_slot;

    /* Framebuffer Info */
    shandle_t            h_fb;
    vaddr_t             fb_vaddr;
    EFBDEV_PIXEL_FORMAT pixel_format;
    size_t              fb_sz;

    /* Display Dimension */
    uint32_t            max_rec_x;
    uint32_t            max_rec_y;

    uint32_t            cur_rec_x;
    uint32_t            cur_rec_y;
    uint32_t            cur_pos_x;
    uint32_t            cur_pos_y;

    uint32_t 	screen_resolution_x;
	uint32_t 	screen_resolution_y;
};


extern void efbdev_mgr_init(void);
extern void efbdev_mgr_fini(void);

/// @brief Check most of the parameters of <efbdev_register> before calling <iosm_fb_create_fbdev>, so we can fail early 
/// caused by most of the invalid parameters.
extern bool efbdev_register_check_params(
    EFBDEV_ID efbdev_id, 
    EFBDEV_PIXEL_FORMAT pixel_format, 
    size_t fb_sz, 
    uint32_t max_rec_x, 
    uint32_t max_rec_y,
    uint32_t screen_resolution_x,
	uint32_t screen_resolution_y
);

extern SAP_STATUS efbdev_register(
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
);

extern SAP_STATUS efbdev_unregister(EFBDEV_ID efbdev_id);

/// @brief Return the info of eFBDev <efbdev_id>
extern SAP_STATUS efbdev_get_info(EFBDEV_ID efbdev_id, struct efbdev_info_entry** out_efbdev_info);

extern SAP_STATUS efbdev_update_disp_dimension(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y);
extern SAP_STATUS efbdev_update_disp_pos(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y);

/// @brief Calculate byte size for the given pixel format.
SAP_STATUS pixel_format_to_byte_sz(EFBDEV_PIXEL_FORMAT pixel_format, size_t* out_mem_sz);

/// @brief Return the byte size corresponding to the pixel format used by the eFBDev <efbdev_id>.
extern SAP_STATUS efbdev_get_pixel_byte_sz(EFBDEV_ID efbdev_id, size_t* out_mem_sz);

#endif // __ASSEMBLY__
#endif // LIBUSB2_EEHCI_MGR_H