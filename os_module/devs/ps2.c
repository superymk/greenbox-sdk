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

#include <sa_headers.h>
#include <device/ps2.h>

#define PS2_CMD 	0x64
#define PS2_STATUS 	0x64
#define PS2_DATA 	0x60

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

#define PS2_TIMEOUT 500

/* Loops until the controller's input buffer is empty or our timer has expired.
 * Call this function before writing to the controller's command port, PS2_CMD.
 */
static bool ps2_wait_write(void)
{
    int timer = PS2_TIMEOUT;

    while ((os->inb(PS2_STATUS) & 2) && timer-- > 0)
    {
        // asm("pause");
    }

    return timer != 0;
}

/* Loops until data is available in the controller's output buffer or until our
 * timer has expired.
 * Call this function before reading from the controller's data port, PS2_DATA.
 */
static bool ps2_wait_read(void)
{
    int timer = PS2_TIMEOUT;

    while (!(os->inb(PS2_STATUS) & 1) && timer-- >= 0)
    {
        // asm("pause");
    }

    return timer != 0;
}

/* Returns the first byte of data available from `port`.
 * Returns `(uint8_t) -1` on read error, and also when this value was
 * legitimately read. TODO: room for improvement.
 */
static bool ps2_read(uint16_t port, uint8_t* out_val)
{
    if(!out_val)
        return false;

    if (ps2_wait_read())
    {
        *out_val = os->inb(port);
        return true;
    }

    return false;
}

/* Tries to write a byte on the specified port, and returns whether the
 * operation was successful.
 */
static bool ps2_write(uint16_t port, uint8_t b)
{
    if (ps2_wait_write())
    {
        os->outb(b, port);
        return true;
    }

    DRV_ERR("PS2 write failed\n");

    return false;
}

// static void ps2_resume(void)
// {
//     // Send interrupts for IRQ1 and IRQ12.
//     // Why this is sufficient? If there is no data from PS2 devices, red OS takes the interrupts as spurious interrupts. 
//     // If there is data, red OS can process the data normally and put devices to work.

//     // [TODO] Need a smart way to decide the interrupt vector of IRQ1 and IRQ12.
//     os->intr_trigger_interrupt(49);
//     os->intr_trigger_interrupt(60);
// }


static void _drain_ps2_dev_buffers(void)
{
    uint8_t val = 0;

    // After sending the command, the input buffers of device 1 are clear
    ps2_write(PS2_CMD, PS2_TEST_FIRST);
    // After dropping the response, the output buffers of device 1 are clear
    (void)ps2_read(PS2_DATA, &val);

    // After sending the command, the input buffers of device 2 are clear
    ps2_write(PS2_CMD, PS2_TEST_SECOND);
    // After dropping the response, the output buffers of device 2 are clear
    (void)ps2_read(PS2_DATA, &val);
}


OSSTATUS do_CMD_IOSM_PS2_DRAIN_BUFFERS(struct tz_cmd_packet *cmd_packet)
{
    (void)cmd_packet;
    _drain_ps2_dev_buffers();

    return OS_SUCCESS;
}