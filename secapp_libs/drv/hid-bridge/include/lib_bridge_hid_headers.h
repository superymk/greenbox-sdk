// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file This lib is the HID/GUI-bridge functions of the SecProcess side

#ifndef LIBBRIDGE_HEADERS_H
#define LIBBRIDGE_HEADERS_H

#include <stdio.h>
#include <string.h>
#include <sap_headers.h>
#include <palddk.h>

#include <libdrv_hid_headers.h>

#define LIBBRIDGE_HID_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define BRIDGE_HID_DEBUG(format, ...) \
	do{	\
		if((LIBBRIDGE_HID_LOG >= LOG_DEBUG) && (LIBBRIDGE_HID_LOG != LOG_NONE)) {  \
		printf("[LibBridge-HID][Debug] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_HID_INFO(format, ...) \
	do{	\
		if((LIBBRIDGE_HID_LOG >= LOG_INFO) && (LIBBRIDGE_HID_LOG != LOG_NONE)) {  \
		printf("[LibBridge-HID] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_HID_ERR(format, ...) \
	do{	\
		if((LIBBRIDGE_HID_LOG >= LOG_ERR) && (LIBBRIDGE_HID_LOG != LOG_NONE)) {  \
		printf("[LibBridge-HID][ERROR] "format, ## __VA_ARGS__); } }while(0)


typedef enum
{
	HID_DEV_INVALID = 0,
	HID_KEYBOARD = 1,
	HID_MOUSE = 2
} HID_DEV;

struct hid_event
{
	HID_DEV hid_dev;
	union
	{
		struct kbd_key_event kbd_event;
		struct mouse_event mouse_event;
	};
};

extern SAP_STATUS hid_bridge_sent_kbd_event(struct kbd_key_event* event);

extern SAP_STATUS hid_bridge_sent_mouse_event(struct mouse_event* event);

#endif // __ASSEMBLY__
#endif // LIBBRIDGE_HEADERS_H