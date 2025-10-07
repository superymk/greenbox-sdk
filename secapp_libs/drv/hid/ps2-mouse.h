// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_PS2_MOUSE_H
#define LIBDRV_HID_PS2_MOUSE_H

#include <palddk.h>
#include <ps2_def.h>
#include <iosm_ps2_sp_intfs.h>
#include "ps2.h"

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <stdbool.h>

// #include <kernel/ps2.h>
// #include <kernel/irq.h>

// #include <stdint.h>
// #include <stdbool.h>




/********* PS2 Mouse Definitions *********/
// Mouse-specific commands
#define MOUSE_SET_SAMPLE 0xF3
#define MOUSE_SET_RESOLUTION 0xE8
#define MOUSE_ENABLE_SCALING 0xE7
#define MOUSE_DISABLE_SCALING 0xE7

// Bits of the first mouse data packet
#define MOUSE_Y_OVERFLOW (1 << 7)
#define MOUSE_X_OVERFLOW (1 << 6)
#define MOUSE_Y_NEG (1 << 5)
#define MOUSE_X_NEG (1 << 4)
#define MOUSE_MIDDLE (1 << 2)
#define MOUSE_RIGHT (1 << 1)
#define MOUSE_LEFT (1 << 0)

#define PS2_MOUSE_BYTE0_BIT3_MUST_SET (3)
#define PS2_TOUCHPAD_BYTE0_BIT7_MUST_SET (7)
#define PS2_TOUCHPAD_BYTE0_BIT6_MUST_CLEAR (6)
#define PS2_TOUCHPAD_BYTE0_BIT3_MUST_CLEAR (3)

// Bits of the fourth mouse data packet
#define MOUSE_UNUSED_A (1 << 7)
#define MOUSE_UNUSED_B (1 << 6)

// Extra
#define MOUSE_EXTRA_BIT_SIGN    (1 << 4)

typedef struct {
    uint32_t x, y;
    int delta_x;
    int delta_y;
    int wheel;
    bool left_pressed;
    bool right_pressed;
    bool middle_pressed;
} ps2_mouse_t;

typedef struct {
    uint32_t x, y;
    bool left_pressed;
    bool right_pressed;
    bool middle_pressed;
} ps2_touchpad_t;



/********* PS2 Touchpad Definitions *********/
#define PS2_TOUCHPAD_BYTE0_LEFT             (1 << 0)
#define PS2_TOUCHPAD_BYTE0_RIGHT            (1 << 1)

#define PS2_TOUCHPAD_THINKPAD_BYTE1_LEFT             (1 << 0)
#define PS2_TOUCHPAD_THINKPAD_BYTE1_RIGHT            (1 << 1)
#define PS2_TOUCHPAD_THINKPAD_BYTE1_MIDDLE           (1 << 2)

#define PS2_TOUCHPAD_THINKPAD_BYTE3_BTN_PRESS        (1 << 0)

extern SAP_STATUS ps2_mouse_init(void);
extern SAP_STATUS ps2_mouse_fini(void);

// void init_mouse(uint32_t dev);
// void ps2_mouse_handle_packet();

/// @brief PS2 mouse driver by handling IRQ1 interrupts
/// @param  
void mouse_handle_interrupt(void);

/// @brief PS2 mouse driver by polling PS2 ports
/// @param ps2_status 
extern void mouse_handle_polling(uint32_t ps2_status);

// void mouse_set_sample_rate(uint8_t rate);
// void mouse_set_resolution(uint8_t level);
// void mouse_set_scaling(bool enabled);
// void mouse_enable_scroll_wheel();
// void mouse_enable_five_buttons();

int mouse_get_x();
int mouse_get_y();
bool mouse_left_button();
bool mouse_right_button();

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_PS2_MOUSE_H