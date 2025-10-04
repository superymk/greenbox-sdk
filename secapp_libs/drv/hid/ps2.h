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

#ifndef LIBDRV_HID_PS2_H
#define LIBDRV_HID_PS2_H

#include <palddk.h>
#include <ps2_def.h>
#include <iosm_ps2_sp_intfs.h>

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <stdbool.h>

// Status Byte
#define PS2_STS_MOBF        (1 << 5)
#define PS2_STS_OBF         (1 << 0)    // kbd data in buffer

// Configuration byte
#define PS2_CFG_FIRST_PORT 1
#define PS2_CFG_SECOND_PORT (1 << 1)
#define PS2_CFG_SYSTEM_FLAG (1 << 2)
#define PS2_CFG_FIRST_CLOCK (1 << 4)
#define PS2_CFG_SECOND_CLOCK (1 << 5)
#define PS2_CFG_TRANSLATION (1 << 6)
#define PS2_CFG_MUST_BE_ZERO (1 << 7)

// Controller commands
#define PS2_DISABLE_FIRST 0xAD
#define PS2_DISABLE_SECOND 0xA7
#define PS2_ENABLE_FIRST 0xAE
#define PS2_ENABLE_SECOND 0xA8
#define PS2_READ_CONFIG 0x20
#define PS2_WRITE_CONFIG 0x60
#define PS2_SELF_TEST 0xAA
#define PS2_TEST_FIRST 0xAB
#define PS2_TEST_SECOND 0xA9
#define PS2_WRITE_SECOND 0xD4

// Controller responses
#define PS2_SELF_TEST_OK 0x55
#define PS2_TEST_OK 0x00

// Device commands
#define PS2_DEV_RESET 0xFF
#define PS2_DEV_IDENTIFY 0xF2
#define PS2_DEV_ENABLE_SCAN 0xF4
#define PS2_DEV_DISABLE_SCAN 0xF5

// Device responses
#define PS2_DEV_ACK 0xFA
#define PS2_DEV_RESET_ACK 0xAA

// Timeout for PS2 Ack
#define PS2_ACK_TIMEOUT_MS      (25)

typedef enum 
{
    PS2_MOUSE = 0x00,
    PS2_MOUSE_SCROLL_WHEEL = 0x03,
    PS2_MOUSE_FIVE_BUTTONS = 0x04,
    PS2_KEYBOARD,
    PS2_KEYBOARD_TRANSLATED,
    PS2_TOUCHPAD,
    PS2_TOUCHPAD_THINKPAD,
    PS2_DEVICE_UNKNOWN,
} PS2_DEV_TYPE;

typedef struct
{
    // True if this device exist
    bool present;
    // The controller in which this device is plugged in
    uint32_t num;
    uint32_t type;
    // Raw identifiers
    uint8_t first_id_byte;
    uint8_t second_id_byte; // Validity depends on device
} ps2_device_t;

/// @brief Performs discovery and initialisation of PS/2 controllers and devices.
/// A lot of redundancy here because of poorly-indexed identifiers.
/// [NOTE] This function properly finalize its states on errors. Its caller function does not need to call 
/// <libhid_ps2_fini> again.
extern SAP_STATUS libhid_ps2_init(void);
extern void libhid_ps2_fini(void);
extern SAP_STATUS ps2_intr_handler(intr_t intr);
extern void ps2_intr_poll(void);

PS2_DEV_TYPE ps2_identify_device(uint32_t device_num);
PS2_DEV_TYPE ps2_identity_bytes_to_type(uint8_t a, uint8_t b);

bool ps2_wait_write();
bool ps2_wait_read();
bool ps2_read(uint32_t port, uint8_t* out_val);
bool ps2_write(uint32_t port, uint8_t b);
bool ps2_write_device(uint32_t device, uint8_t b);

/// @brief Poll PS2 Ack or return ETIMEOUT after <timeout_ms> is elapsed
/// @param timeout_ms 
/// @return 
extern SAP_STATUS ps2_expect_ack(uint32_t timeout_ms);
bool ps2_can_read();

extern uint8_t inportb(uint16_t port);
extern void outportb(uint16_t port, uint8_t value);

// [TODO] A dirty hack to prevent hanging problems of PS2 devices
extern void _ps2_temp_trigger(void);


#endif // __ASSEMBLY__
#endif // LIBDRV_HID_PS2_H