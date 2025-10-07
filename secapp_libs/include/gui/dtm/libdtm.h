// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file This lib allows the current SecProcess to function as a DesktopMgr client. The SecProcess will get frame buffer
/// memory and window events. and only needs to provide 

#ifndef LIBDTM_HEADERS_H
#define LIBDTM_HEADERS_H

#include <sap_headers.h>
#include <palddk.h>
#include <stdio.h>
#include <string.h>

// #include <dtm_sp_intfs.h>
// #include <input_events.h>

#define LIBBRIDGE_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define BRIDGE_DEBUG(format, ...) \
	do{	\
		if((LIBBRIDGE_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-BRIDGE][Debug] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_INFO(format, ...) \
	do{	\
		if((LIBBRIDGE_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-BRIDGE] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_ERR(format, ...) \
	do{	\
		if((LIBBRIDGE_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-BRIDGE][ERROR] "format, ## __VA_ARGS__); } }while(0)

// Define RGB pixel format used by the libdtm
#define LIBDTM_RED_BIT_SHIFT	(16)
#define LIBDTM_GREEN_BIT_SHIFT	(8)
#define LIBDTM_BLUE_BIT_SHIFT	(0)


typedef struct 
{
	vaddr_t fb_vaddr;
	uint32_t fb_width;
	uint32_t fb_height;
} WINDOW_INFO;

typedef void (*KBD_LISTENER)(struct kbd_key_event* event);
typedef void (*MOUSE_LISTENER)(struct mouse_event* event);

extern SAP_STATUS libdtm_init(uint32_t win_init_width, uint32_t win_init_height);
extern void libdtm_fini(void);
extern SAP_STATUS libdtm_get_window_info(WINDOW_INFO* out_info);
extern SAP_STATUS libdtm_on_dtm_event(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm);

// /// @brief 
// /// @return Return FAILED if failed to pull a DTM event due to empty ring buffer.
// ///			Return EINTERNAL on errors
// extern SAP_STATUS libdtm_pull_dtm_event(DTM_EVENT* out_event);

/// @brief Pull and handle DTM events until the ring buffer is empty
/// @param  
/// @return Return SUCCESS if all events are handled.
///			Return EINTERNAL on errors.
extern SAP_STATUS libdtm_handle_dtm_events(void);

extern SAP_STATUS libdtm_hid_register_kbd_listener(KBD_LISTENER listener);
extern SAP_STATUS libdtm_hid_unregister_kbd_listener(KBD_LISTENER listener);
extern SAP_STATUS libdtm_hid_register_mouse_listener(MOUSE_LISTENER listener);
extern SAP_STATUS libdtm_hid_unregister_mouse_listener(MOUSE_LISTENER listener);

#endif // __ASSEMBLY__
#endif // LIBDTM_HEADERS_H