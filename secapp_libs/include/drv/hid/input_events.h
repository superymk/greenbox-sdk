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

#ifndef LIBDRV_HID_INPUT_EVENTS_H
#define LIBDRV_HID_INPUT_EVENTS_H

#include <stdint.h>

/********* Keyboard Definitions *********/
// Note: LIBDRV_HID_MOD_* (e.g. LIBDRV_HID_MOD_LEFT_CTRL) macros are for the
// "modifier" field in "struct kbd_key_event". They are different from
// LIBDRV_HID_KEY_* macros (e.g. LIBDRV_HID_KEY_LEFT_CTRL), which are for the
// "key" field in "struct kbd_key_event". This design may change after
// reviewing design of keyboard events of modern operating systems.
#define LIBDRV_HID_MOD_LEFT_CTRL		0x001
#define LIBDRV_HID_MOD_LEFT_SHIFT		0x002
#define LIBDRV_HID_MOD_LEFT_ALT			0x004
#define LIBDRV_HID_MOD_LEFT_GUI			0x008
#define LIBDRV_HID_MOD_RIGHT_CTRL		0x010
#define LIBDRV_HID_MOD_RIGHT_SHIFT		0x020
#define LIBDRV_HID_MOD_RIGHT_ALT		0x040
#define LIBDRV_HID_MOD_RIGHT_GUI		0x080
#define LIBDRV_HID_MOD_CAPS_LOCK		0x100
#define LIBDRV_HID_MOD_NUM_LOCK			0x200
#define LIBDRV_HID_MOD_SCROLL_LOCK		0x400

#define LIBDRV_HID_KEY_A				4
#define LIBDRV_HID_KEY_B				5
#define LIBDRV_HID_KEY_C				6
#define LIBDRV_HID_KEY_D				7
#define LIBDRV_HID_KEY_E				8
#define LIBDRV_HID_KEY_F				9
#define LIBDRV_HID_KEY_G				10
#define LIBDRV_HID_KEY_H				11
#define LIBDRV_HID_KEY_I				12
#define LIBDRV_HID_KEY_J				13
#define LIBDRV_HID_KEY_K				14
#define LIBDRV_HID_KEY_L				15
#define LIBDRV_HID_KEY_M				16
#define LIBDRV_HID_KEY_N				17
#define LIBDRV_HID_KEY_O				18
#define LIBDRV_HID_KEY_P				19
#define LIBDRV_HID_KEY_Q				20
#define LIBDRV_HID_KEY_R				21
#define LIBDRV_HID_KEY_S				22
#define LIBDRV_HID_KEY_T				23
#define LIBDRV_HID_KEY_U				24
#define LIBDRV_HID_KEY_V				25
#define LIBDRV_HID_KEY_W				26
#define LIBDRV_HID_KEY_X				27
#define LIBDRV_HID_KEY_Y				28
#define LIBDRV_HID_KEY_Z				29
#define LIBDRV_HID_KEY_1				30
#define LIBDRV_HID_KEY_2				31
#define LIBDRV_HID_KEY_3				32
#define LIBDRV_HID_KEY_4				33
#define LIBDRV_HID_KEY_5				34
#define LIBDRV_HID_KEY_6				35
#define LIBDRV_HID_KEY_7				36
#define LIBDRV_HID_KEY_8				37
#define LIBDRV_HID_KEY_9				38
#define LIBDRV_HID_KEY_0				39
#define LIBDRV_HID_KEY_ENTER			40	// '\n'
#define LIBDRV_HID_KEY_ESC				41	// '\x1b'
#define LIBDRV_HID_KEY_BACKSPACE		42	// '\x7f'
#define LIBDRV_HID_KEY_TAB				43	// '\t'
#define LIBDRV_HID_KEY_SPACE			44	// ' '
#define LIBDRV_HID_KEY_MINUS			45	// '-'
#define LIBDRV_HID_KEY_EQUAL			46	// '='
#define LIBDRV_HID_KEY_LEFT_BRACE		47	// '('
#define LIBDRV_HID_KEY_RIGHT_BRACE		48	// ')'
#define LIBDRV_HID_KEY_BACKSLASH		49	// '\\'
#define LIBDRV_HID_KEY_NUMBER			50
#define LIBDRV_HID_KEY_SEMICOLON		51	// ';'
#define LIBDRV_HID_KEY_QUOTE			52	// '\''
#define LIBDRV_HID_KEY_TILDE			53
#define LIBDRV_HID_KEY_COMMA			54	// ','
#define LIBDRV_HID_KEY_PERIOD			55	// '.'
#define LIBDRV_HID_KEY_SLASH			56	// '/'
#define LIBDRV_HID_KEY_CAPS_LOCK		57
#define LIBDRV_HID_KEY_F1				58
#define LIBDRV_HID_KEY_F2				59
#define LIBDRV_HID_KEY_F3				60
#define LIBDRV_HID_KEY_F4				61
#define LIBDRV_HID_KEY_F5				62
#define LIBDRV_HID_KEY_F6				63
#define LIBDRV_HID_KEY_F7				64
#define LIBDRV_HID_KEY_F8				65
#define LIBDRV_HID_KEY_F9				66
#define LIBDRV_HID_KEY_F10				67
#define LIBDRV_HID_KEY_F11				68
#define LIBDRV_HID_KEY_F12				69
#define LIBDRV_HID_KEY_PRINTSCREEN		70
#define LIBDRV_HID_KEY_SCROLL_LOCK		71
#define LIBDRV_HID_KEY_PAUSE			72
#define LIBDRV_HID_KEY_INSERT			73
#define LIBDRV_HID_KEY_HOME				74
#define LIBDRV_HID_KEY_PAGE_UP			75
#define LIBDRV_HID_KEY_DELETE			76
#define LIBDRV_HID_KEY_END				77
#define LIBDRV_HID_KEY_PAGE_DOWN		78
#define LIBDRV_HID_KEY_RIGHT			79
#define LIBDRV_HID_KEY_LEFT				80
#define LIBDRV_HID_KEY_DOWN				81
#define LIBDRV_HID_KEY_UP				82
#define LIBDRV_HID_KEY_NUM_LOCK			83
#define LIBDRV_HID_KEYPAD_SLASH			84
#define LIBDRV_HID_KEYPAD_ASTERIX		85
#define LIBDRV_HID_KEYPAD_MINUS			86
#define LIBDRV_HID_KEYPAD_PLUS			87
#define LIBDRV_HID_KEYPAD_ENTER			88
#define LIBDRV_HID_KEYPAD_1				89
#define LIBDRV_HID_KEYPAD_2				90
#define LIBDRV_HID_KEYPAD_3				91
#define LIBDRV_HID_KEYPAD_4				92
#define LIBDRV_HID_KEYPAD_5				93
#define LIBDRV_HID_KEYPAD_6				94
#define LIBDRV_HID_KEYPAD_7				95
#define LIBDRV_HID_KEYPAD_8				96
#define LIBDRV_HID_KEYPAD_9				97
#define LIBDRV_HID_KEYPAD_0				98
#define LIBDRV_HID_KEYPAD_PERIOD		99
#define LIBDRV_HID_KEY_MENU				100
#define LIBDRV_HID_KEY_BACKTICK			101	// '`'
#define LIBDRV_HID_KEY_TICK				102
#define LIBDRV_HID_KEY_LEFT_SQUARE		103
#define LIBDRV_HID_KEY_RIGHT_SQUARE		104
#define LIBDRV_HID_KEY_LEFT_CTRL		105
#define LIBDRV_HID_KEY_LEFT_SHIFT		106
#define LIBDRV_HID_KEY_LEFT_ALT			107
#define LIBDRV_HID_KEY_LEFT_GUI			108
#define LIBDRV_HID_KEY_RIGHT_CTRL		109
#define LIBDRV_HID_KEY_RIGHT_SHIFT		110
#define LIBDRV_HID_KEY_RIGHT_ALT		111
#define LIBDRV_HID_KEY_RIGHT_GUI		112

// Key Event
#define LIBDRV_HID_KBD_NO_EVENT			0
#define LIBDRV_HID_KBD_KEY_PRESSED		1
#define LIBDRV_HID_KBD_KEY_RELEASED		2




/********* Mouse Definitions *********/
// Mouse Button
#define LIBDRV_HID_BTN_LEFT				(1 << 0)
#define LIBDRV_HID_BTN_RIGHT			(1 << 1)
#define LIBDRV_HID_BTN_MIDDLE			(1 << 2)
#define LIBDRV_HID_BTN_WHEEL			(1 << 3)

// Mouse Wheel
#define LIBDRV_HID_WHEEL_UP				0
#define LIBDRV_HID_WHEEL_DOWN			1
#define LIBDRV_HID_WHEEL_LEFT			2
#define LIBDRV_HID_WHEEL_RIGHT			3

// Mouse Action
#define LIBDRV_HID_MOUSE_NO_EVENT		0
#define LIBDRV_HID_MOUSE_BTN_PRESSED	(1 << 1)
#define LIBDRV_HID_MOUSE_BTN_RELEASED	(1 << 2)
#define LIBDRV_HID_MOUSE_MOVED			(1 << 3)


#ifndef __ASSEMBLY__

//! Specifies the key the user pressed and whether any modifier
//! keys (CTRL, ALT, and SHIFT) were pressed at the same time.
struct kbd_key_event
{
	uint32_t	modifier : 30, //! e.g. 'ctrl', 'win', ...
				action : 2; //! e.g. press down, press up, ...
	uint8_t		key; //! e.g. 'A', 'B', ...
	
}__attribute__ ((packed));

//! Specifies the mouse event the user triggered
struct mouse_event
{
	uint32_t	button: 4, //! the button caused the event
				wheel: 2,
				action : 10, //! e.g. press down, press up, move
				wheel_diff : 4,
				__reserved : 12;
	int			delta_x;
	int			delta_y;
}__attribute__ ((packed));

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_INPUT_EVENTS_H
