// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBFB_OPS_H
#define LIBFB_OPS_H

#define LIBFB_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define FB_DEBUG(format, ...) \
	do{	\
		if((LIBFB_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-FB][Debug] "format, ## __VA_ARGS__); } }while(0)

#define FB_INFO(format, ...) \
	do{	\
		if((LIBFB_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-FB] "format, ## __VA_ARGS__); } }while(0)

#define FB_ERR(format, ...) \
	do{	\
		if((LIBFB_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-FB][ERROR] "format, ## __VA_ARGS__); } }while(0)

extern SAP_STATUS libfb_init(spid_t iosm_fb_spid);
extern void libfb_fini(void);

/// @brief After created the eFBDev, the overlay is shown on screen by default.
/// @param efbdev_id 
/// @param max_rec_x 
/// @param max_rec_y 
/// @param cur_rec_x 
/// @param cur_rec_y 
/// @param screen_resolution_x 
/// @param screen_resolution_y 
/// @param pixel_format 
/// @param out_fb_vaddr 
/// @param out_fb_sz 
/// @return 
extern SAP_STATUS libfb_create_efbdev(
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
);
extern SAP_STATUS libfb_destroy_efbdev(EFBDEV_ID efbdev_id);

/// @brief Return the frame buffer handle of eFBDev <efbdev_id>
/// @param efbdev_id 
/// @return 
extern shandle_t libfb_efbdev_get_fb_handle(EFBDEV_ID efbdev_id);

extern SAP_STATUS libfb_write_disp_dimension(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y);
extern SAP_STATUS libfb_write_disp_position(EFBDEV_ID efbdev_id, uint32_t new_x, uint32_t new_y);

/// @brief Present the overlay on screen
/// @param efbdev_id 
extern SAP_STATUS libfb_overlay_present(EFBDEV_ID efbdev_id);

/// @brief Show the overlay on screen
/// @param efbdev_id 
/// @return 
extern SAP_STATUS libfb_overlay_show(EFBDEV_ID efbdev_id);

/// @brief Hide the overlay
/// @param efbdev_id 
/// @return 
extern SAP_STATUS libfb_overlay_hide(EFBDEV_ID efbdev_id);

#endif // __ASSEMBLY__
#endif // LIBFB_OPS_H