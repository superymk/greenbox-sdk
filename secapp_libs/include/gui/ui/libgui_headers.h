// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBGUI_OPS_H
#define LIBGUI_OPS_H

#include <stdio.h>
#include <string.h>
#include <sap_headers.h>
#include <palddk.h>

// LibDrv-HID
#include <drv/hid/input_events.h>
#include <gui/ui/lvgl/lvgl.h>
#include <gui/ui/lvgl/demos/lv_demos.h>

#define LIBGUI_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define GUI_DEBUG(format, ...) \
	do{	\
		if((LIBGUI_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess %u][Lib-GUI][Debug] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)

#define GUI_INFO(format, ...) \
	do{	\
		if((LIBGUI_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess %u][Lib-GUI] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)

#define GUI_ERR(format, ...) \
	do{	\
		if((LIBGUI_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess %u][Lib-GUI][ERROR] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)

struct libgui_config
{
    // Keyboard/Mouse
    void (*kbd)(void);
    void (*mouse)(void);
	uint32_t lines_per_wheel;

    // Display
    vaddr_t fb_vaddr;
    uint32_t fb_width;
    uint32_t fb_height;
};

extern SAP_STATUS libgui_init(struct libgui_config *cfg, bool create_mouse_cursor);
extern void libgui_fini(void);

/// @brief Return the absolute position of the mouse.
/// This function can only be called after LibGUI is initialized.
/// @param out_pos_x 
/// @param out_pos_y 
extern void libgui_mouse_get_pos(uint16_t* out_pos_x, uint16_t* out_pos_y);

extern void libgui_cycle(void);

//! \brief LibGUI's call back function to handle keyboard input events. 
//! This function should be registered in the keyboard driver to listen on keyboard events
extern void libgui_kbd_listener(struct kbd_key_event* event);

//! \brief LibGUI's call back function to handle mouse input events. 
//! This function should be registered in the mouse driver to listen on mouse events
extern void libgui_mouse_listener(struct mouse_event* event);

#endif // __ASSEMBLY__
#endif // LIBGUI_OPS_H