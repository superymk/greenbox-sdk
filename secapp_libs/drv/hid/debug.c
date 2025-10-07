// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include "debug.h"

static bool enable_event_print = false;

void hid_event_print_set(bool new_status)
{
	enable_event_print = new_status;
}

void print_kbd_key_event(const char *driver, const struct kbd_key_event *event)
{
	const char *action_str = NULL;
	const char *key_str = NULL;
	// static uint32_t modifier;

	// Check the overall control of HID event printing
	if(!enable_event_print)
		return;

	// Check: Parameters must be valid
    if(!event)
	{
        return;
	}

	// Calculate 
	switch (event->action) {
	case LIBDRV_HID_KBD_KEY_PRESSED:
		action_str = "Pressed ";
		break;
	case LIBDRV_HID_KBD_KEY_RELEASED:
		action_str = "Released";
		break;
	default:
		action_str = "Unknown ";
		break;
	}

	// // Calculate modifiers
	// modifier = event->modifier & ~(LIBDRV_HID_MOD_CAPS_LOCK | LIBDRV_HID_MOD_NUM_LOCK | LIBDRV_HID_MOD_SCROLL_LOCK);

	// //// Calculate CAPSLOCK, SCREENLOCK and NUMLOCK
	// if(event->action == LIBDRV_HID_KBD_KEY_PRESSED)
	// {
	// 	if(event->modifier & LIBDRV_HID_MOD_CAPS_LOCK)
	// 		modifier ^= LIBDRV_HID_MOD_CAPS_LOCK;
	// 	if(event->modifier & LIBDRV_HID_MOD_NUM_LOCK)
	// 		modifier ^= LIBDRV_HID_MOD_NUM_LOCK;
	// 	if(event->modifier & LIBDRV_HID_MOD_SCROLL_LOCK)
	// 		modifier ^= LIBDRV_HID_MOD_SCROLL_LOCK;
	// }

	// Print keys
#define REGISTER_KEY(x, y) \
	case LIBDRV_HID_##x: \
		key_str = #x y; \
		break;

	switch (event->key) {
		REGISTER_KEY(KEY_A, "           ")
		REGISTER_KEY(KEY_B, "           ")
		REGISTER_KEY(KEY_C, "           ")
		REGISTER_KEY(KEY_D, "           ")
		REGISTER_KEY(KEY_E, "           ")
		REGISTER_KEY(KEY_F, "           ")
		REGISTER_KEY(KEY_G, "           ")
		REGISTER_KEY(KEY_H, "           ")
		REGISTER_KEY(KEY_I, "           ")
		REGISTER_KEY(KEY_J, "           ")
		REGISTER_KEY(KEY_K, "           ")
		REGISTER_KEY(KEY_L, "           ")
		REGISTER_KEY(KEY_M, "           ")
		REGISTER_KEY(KEY_N, "           ")
		REGISTER_KEY(KEY_O, "           ")
		REGISTER_KEY(KEY_P, "           ")
		REGISTER_KEY(KEY_Q, "           ")
		REGISTER_KEY(KEY_R, "           ")
		REGISTER_KEY(KEY_S, "           ")
		REGISTER_KEY(KEY_T, "           ")
		REGISTER_KEY(KEY_U, "           ")
		REGISTER_KEY(KEY_V, "           ")
		REGISTER_KEY(KEY_W, "           ")
		REGISTER_KEY(KEY_X, "           ")
		REGISTER_KEY(KEY_Y, "           ")
		REGISTER_KEY(KEY_Z, "           ")
		REGISTER_KEY(KEY_1, "           ")
		REGISTER_KEY(KEY_2, "           ")
		REGISTER_KEY(KEY_3, "           ")
		REGISTER_KEY(KEY_4, "           ")
		REGISTER_KEY(KEY_5, "           ")
		REGISTER_KEY(KEY_6, "           ")
		REGISTER_KEY(KEY_7, "           ")
		REGISTER_KEY(KEY_8, "           ")
		REGISTER_KEY(KEY_9, "           ")
		REGISTER_KEY(KEY_0, "           ")
		REGISTER_KEY(KEY_ENTER, "       ")
		REGISTER_KEY(KEY_ESC, "         ")
		REGISTER_KEY(KEY_BACKSPACE, "   ")
		REGISTER_KEY(KEY_TAB, "         ")
		REGISTER_KEY(KEY_SPACE, "       ")
		REGISTER_KEY(KEY_MINUS, "       ")
		REGISTER_KEY(KEY_EQUAL, "       ")
		REGISTER_KEY(KEY_LEFT_BRACE, "  ")
		REGISTER_KEY(KEY_RIGHT_BRACE, " ")
		REGISTER_KEY(KEY_BACKSLASH, "   ")
		REGISTER_KEY(KEY_NUMBER, "      ")
		REGISTER_KEY(KEY_SEMICOLON, "   ")
		REGISTER_KEY(KEY_QUOTE, "       ")
		REGISTER_KEY(KEY_TILDE, "       ")
		REGISTER_KEY(KEY_COMMA, "       ")
		REGISTER_KEY(KEY_PERIOD, "      ")
		REGISTER_KEY(KEY_SLASH, "       ")
		REGISTER_KEY(KEY_CAPS_LOCK, "   ")
		REGISTER_KEY(KEY_F1, "          ")
		REGISTER_KEY(KEY_F2, "          ")
		REGISTER_KEY(KEY_F3, "          ")
		REGISTER_KEY(KEY_F4, "          ")
		REGISTER_KEY(KEY_F5, "          ")
		REGISTER_KEY(KEY_F6, "          ")
		REGISTER_KEY(KEY_F7, "          ")
		REGISTER_KEY(KEY_F8, "          ")
		REGISTER_KEY(KEY_F9, "          ")
		REGISTER_KEY(KEY_F10, "         ")
		REGISTER_KEY(KEY_F11, "         ")
		REGISTER_KEY(KEY_F12, "         ")
		REGISTER_KEY(KEY_PRINTSCREEN, " ")
		REGISTER_KEY(KEY_SCROLL_LOCK, " ")
		REGISTER_KEY(KEY_PAUSE, "       ")
		REGISTER_KEY(KEY_INSERT, "      ")
		REGISTER_KEY(KEY_HOME, "        ")
		REGISTER_KEY(KEY_PAGE_UP, "     ")
		REGISTER_KEY(KEY_DELETE, "      ")
		REGISTER_KEY(KEY_END, "         ")
		REGISTER_KEY(KEY_PAGE_DOWN, "   ")
		REGISTER_KEY(KEY_RIGHT, "       ")
		REGISTER_KEY(KEY_LEFT, "        ")
		REGISTER_KEY(KEY_DOWN, "        ")
		REGISTER_KEY(KEY_UP, "          ")
		REGISTER_KEY(KEY_NUM_LOCK, "    ")
		REGISTER_KEY(KEYPAD_SLASH, "    ")
		REGISTER_KEY(KEYPAD_ASTERIX, "  ")
		REGISTER_KEY(KEYPAD_MINUS, "    ")
		REGISTER_KEY(KEYPAD_PLUS, "     ")
		REGISTER_KEY(KEYPAD_ENTER, "    ")
		REGISTER_KEY(KEYPAD_1, "        ")
		REGISTER_KEY(KEYPAD_2, "        ")
		REGISTER_KEY(KEYPAD_3, "        ")
		REGISTER_KEY(KEYPAD_4, "        ")
		REGISTER_KEY(KEYPAD_5, "        ")
		REGISTER_KEY(KEYPAD_6, "        ")
		REGISTER_KEY(KEYPAD_7, "        ")
		REGISTER_KEY(KEYPAD_8, "        ")
		REGISTER_KEY(KEYPAD_9, "        ")
		REGISTER_KEY(KEYPAD_0, "        ")
		REGISTER_KEY(KEYPAD_PERIOD, "   ")
		REGISTER_KEY(KEY_MENU, "        ")
		REGISTER_KEY(KEY_BACKTICK, "    ")
		REGISTER_KEY(KEY_TICK, "        ")
		REGISTER_KEY(KEY_LEFT_SQUARE, " ")
		REGISTER_KEY(KEY_RIGHT_SQUARE, "")
		REGISTER_KEY(KEY_LEFT_CTRL, "   ")
		REGISTER_KEY(KEY_LEFT_SHIFT, "  ")
		REGISTER_KEY(KEY_LEFT_ALT, "    ")
		REGISTER_KEY(KEY_LEFT_GUI, "    ")
		REGISTER_KEY(KEY_RIGHT_CTRL, "  ")
		REGISTER_KEY(KEY_RIGHT_SHIFT, " ")
		REGISTER_KEY(KEY_RIGHT_ALT, "   ")
		REGISTER_KEY(KEY_RIGHT_GUI, "   ")
		default: key_str = "(Unknown)       "; break;
	}

#undef REGISTER_KEY

#define REGISTER_MASK(x) \
	(event->modifier & LIBDRV_HID_MOD_##x ? " " #x : "")

	printf("%s %s %s (%s%s%s%s%s%s%s%s%s%s%s )\n", action_str, key_str,
			driver,
			REGISTER_MASK(LEFT_CTRL),
			REGISTER_MASK(LEFT_SHIFT),
			REGISTER_MASK(LEFT_ALT),
			REGISTER_MASK(LEFT_GUI),
			REGISTER_MASK(RIGHT_CTRL),
			REGISTER_MASK(RIGHT_SHIFT),
			REGISTER_MASK(RIGHT_ALT),
			REGISTER_MASK(RIGHT_GUI),
			REGISTER_MASK(CAPS_LOCK),
			REGISTER_MASK(NUM_LOCK),
			REGISTER_MASK(SCROLL_LOCK)
	);

#undef REGISTER_MASK

}

void print_mouse_event(const char *driver, struct mouse_event* event)
{
	// Check the overall control of HID event printing
	if(!enable_event_print)
		return;

	// Check: Parameters must be valid
    if(!event)
        return;
        
    if(event->action == LIBDRV_HID_MOUSE_BTN_PRESSED)
        printf("[%s] Mouse Button Pressed ", driver);
    else if (event->action == LIBDRV_HID_MOUSE_BTN_RELEASED) 
        printf("[%s] Mouse Button Released ", driver);
    else if (event->action == LIBDRV_HID_MOUSE_MOVED)
    {
        printf("[%s] Mouse Moved delta x:%d, delta y:%d", driver, event->delta_x, event->delta_y);
    }
	else
	{
		// No event, so return
		return;
	}

    // Buttons
    if(event->button & LIBDRV_HID_BTN_LEFT)
        printf("[%s] Mouse Left Button ", driver);
    if(event->button & LIBDRV_HID_BTN_RIGHT)
        printf("[%s] Mouse Right Button ", driver);

    // Wheel up/down/left/right
    if(event->button & LIBDRV_HID_BTN_WHEEL)
    {
        if(event->wheel == LIBDRV_HID_WHEEL_UP)
            printf("[%s] Mouse Wheel Up x %u", driver, event->wheel_diff);
        else if(event->wheel == LIBDRV_HID_WHEEL_DOWN)
            printf("[%s] Mouse Wheel Down x %u", driver, event->wheel_diff);
        else if(event->wheel == LIBDRV_HID_WHEEL_LEFT)
            printf("[%s] Mouse Wheel Left", driver);
        else if(event->wheel == LIBDRV_HID_WHEEL_RIGHT)
            printf("[%s] Mouse Wheel Right", driver);
    }

    printf("\n");
}