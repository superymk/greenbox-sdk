// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_PS2_KBD_H
#define LIBDRV_HID_PS2_KBD_H

#include <palddk.h>
#include <ps2_def.h>
#include <iosm_ps2_sp_intfs.h>
#include "ps2.h"

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <stdbool.h>

// #include <kernel/isr.h>

// #include <stdint.h>
// #include <stdbool.h>

// #include <kernel/uapi/uapi_kbd.h>

// "SSC" means "Scan Code Set" here, i.e. the set of bytes corresponding to
// physical keyboard keys.
#define KBD_SSC_CMD 0xF0
#define KBD_SSC_GET 0x00
#define KBD_SSC_2 0x02
#define KBD_SSC_3 0x03

/* AT keyboard. */
#define KB_COMMAND	0x64	/* I/O port for commands on AT */
#define KB_STATUS	0x64	/* I/O port for status on AT */
#define KB_ACK		0xFA	/* keyboard ack response */
#define KB_AUX_BYTE	0x20	/* Auxiliary Device Output Buffer Full */
#define KB_OUT_FULL	0x01	/* status bit set when keypress char pending */
#define KB_IN_FULL	0x02	/* status bit set when not ready to receive */
#define KBC_RD_RAM_CCB	0x20	/* Read controller command byte */
#define KBC_WR_RAM_CCB	0x60	/* Write controller command byte */
#define KBC_DI_AUX	0xA7	/* Disable Auxiliary Device */
#define KBC_EN_AUX	0xA8	/* Enable Auxiliary Device */
#define KBC_DI_KBD	0xAD	/* Disable Keybard Interface */
#define KBC_EN_KBD	0xAE	/* Enable Keybard Interface */
#define LED_CODE	0xED	/* command to keyboard to set LEDs */

// Possible keyboard transmission states
enum {
    // Next byte could be a one byte scancode, a release indicator preceding
    // a one-byte scancode, or a multibyte scancode indicator
    KBD_NORMAL,
    // Next byte is the one-byte scancode released
    KBD_RELEASE_SHORT,
    // Next bytes should be part of a multibyte scancode
    KBD_CONTINUE
};

typedef struct {
    uint32_t keycode;
    bool pressed;
    char repr;
} kbd_event_t;

typedef struct {
    uint32_t state;
    uint8_t scancode[8];
    uint32_t current;
    bool alt;
    bool alt_gr;
    bool shift;
    bool super;
    bool control;
} kbd_context_t;

void init_kbd(uint32_t dev);

/// @brief PS2 keyboard driver by handling IRQ1 interrupts
/// @param  
void kbd_handler(void);

/// @brief PS2 keyboard driver by polling PS2 ports
/// @param ps2_status 
void kbd_handler_poll(uint32_t ps2_status);

bool kbd_process_byte(kbd_context_t* ctx, uint8_t sc, kbd_event_t* event);
bool kbd_is_valid_scancode(uint8_t* bytes, uint32_t len, uint32_t* keycode);
bool kbd_is_key_pressed(uint32_t keycode);
char kbd_make_shift(char c);
char ps2_kbd_keycode_to_char(uint32_t keycode, bool shift);


#endif // __ASSEMBLY__
#endif // LIBDRV_HID_PS2_KBD_H