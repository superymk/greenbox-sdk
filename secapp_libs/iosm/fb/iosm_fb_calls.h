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

#ifndef IOSM_FB_COMM_IOSM_H
#define IOSM_FB_COMM_IOSM_H



#ifndef __ASSEMBLY__



/// @brief Create Inter-SecProcess communication channels required by IOSM-FB.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_iosm_fb_init(spid_t in_iosm_fb_spid);
extern void comm_iosm_fb_fini(void);

/// @brief Register the caller SecProcess's FB driver in IOSM-FB.
extern SAP_STATUS iosm_fb_register_fbdrv(uint32_t* out_io_pid);

/// @brief Unregister the caller SecProcess's FB driver in IOSM-FB.
extern SAP_STATUS iosm_fb_unregister_fbdrv(void);

/// @brief Create an eFBDev by calling IOSM-FB
///
/// @param max_rec_x is the width of SecProcess's display area in pixel.
/// @param max_rec_y is the height of SecProcess's display area in pixel.
extern SAP_STATUS iosm_fb_create_fbdev(
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
);
extern SAP_STATUS iosm_fb_destroy_fbdev(uint32_t efbdev_slot, shandle_t h_fb, vaddr_t fb_addr, size_t fb_sz);


extern SAP_STATUS iosm_fb_write_disp_dimension(uint32_t efbdev_slot, uint32_t new_x, uint32_t new_y);
extern SAP_STATUS iosm_fb_write_disp_pos(uint32_t efbdev_slot, uint32_t new_x, uint32_t new_y);


extern SAP_STATUS iosm_fb_show(uint32_t efbdev_slot);
extern SAP_STATUS iosm_fb_hide(uint32_t efbdev_slot);

/// @brief The lib-iosm-fb maps the current SecProcess's frame buffer with cache enabled. Thus, the SecProcess always
/// has to flush the cache after its frame buffer is ready.
/// @param fb_vaddr 
/// @param fb_sz 
/// @return 
extern SAP_STATUS iosm_fb_flush_frame_buffer(vaddr_t fb_vaddr, size_t fb_sz);
extern SAP_STATUS iosm_fb_sw_overlay_flush_screen(uint32_t efbdev_slot);

#endif // __ASSEMBLY__
#endif // IOSM_FB_COMM_IOSM_H