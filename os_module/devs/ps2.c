// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

/// @brief Wait until controller input buffer is empty
/// @return True if ready before timeout
static bool ps2_wait_write(void)
{
    int timer = PS2_TIMEOUT;

    // Loops until the controller's input buffer is empty or our timer has expired.
    // Call this function before writing to the controller's command port, PS2_CMD.
    while ((os->inb(PS2_STATUS) & 2) && timer-- > 0)
    {
        // asm("pause");
    }

    return timer != 0;
}

/// @brief Wait until controller output buffer has data
/// @return True if data available before timeout
static bool ps2_wait_read(void)
{
    int timer = PS2_TIMEOUT;

    // Loops until data is available in the controller's output buffer or until our
    // timer has expired.
    // Call this function before reading from the controller's data port, PS2_DATA.
    while (!(os->inb(PS2_STATUS) & 1) && timer-- >= 0)
    {
        // asm("pause");
    }

    return timer != 0;
}

/// @brief Returns the first byte of data available from PS/2 controller
/// @param port I/O port
/// @param out_val Output value
/// @return True on success, false on timeout
static bool ps2_read(uint16_t port, uint8_t* out_val)
{
    if (ps2_wait_read())
    {
        *out_val = os->inb(port);
        return true;
    }

    return false;
}

/// @brief Tries to write a byte on the specified port of the PS/2 controller, and returns whether the operation was 
/// successful.
/// @param port I/O port
/// @param b Value to write
/// @return True on success, false on timeout
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


/// @brief Drain PS/2 device buffers
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