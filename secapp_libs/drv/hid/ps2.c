// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>
#include <palddk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ps2.h"
#include "iosm_ps2_client.h"

#include "ps2-kbd.h"
#include "ps2-mouse.h"

// #include <kernel/ps2.h>
// #include <kernel/com.h>
// #include <kernel/mouse.h>
// #include <kernel/kbd.h>
// #include <kernel/sys.h>

// #include <stdio.h>



// SecOS's and SecBase's quirks may affect how ps2.c handle interrupts
static SECOS_QUIRKS_INFO sos_quirks_info = {0};

// Describes the available PS/2 controllers
// static bool controllers[] = {true, true};
static shandle_t h_irq1 = INVALID_HANDLE;
static shandle_t h_irq12 = INVALID_HANDLE;
static intr_t ps2_kbd_intr = INVALID_INTR;
static intr_t ps2_mouse_intr = INVALID_INTR;

/// @brief Are we clearing PS2 devices' buffers?
static bool is_draining_buffers = false;
/// @brief Is it ready to receive interrupts?
static bool is_intr_initialized = false;

#define IRQ1            (1)
#define IRQ12           (12)

/********* Private Functions *********/
static SAP_STATUS _ps2_own_irqs(void)
{
    VKSTATUS status = SUCCESS;
    SYS_OBJ_INFO info = {0};
    SYS_PARAMS_OBJECT_CREATE_ROOT_OBJECT_FLAGS flags = {.obj_type = OBJ_IRQ, .ns_access_type = OBJECT_ACCESS_NS_ACCESS_MEDIATED};

    // Own the IRQ1 and IRQ12
	h_irq1 = sos_sys_obj_create(IRQ1, 1, flags);
	if(h_irq1 == INVALID_HANDLE)
	{
		SAP_ERR("Create IRQ1 object error!\n");
		status = EINTERNAL;
		goto create_irq1_err;
	}

    h_irq12 = sos_sys_obj_create(IRQ12, 1, flags);
	if(h_irq12 == INVALID_HANDLE)
	{
		SAP_ERR("Create IRQ12 object error!\n");
		status = EINTERNAL;
		goto create_irq12_err;
	}

    // Get the interrupt vector of the keyboard and mouse
    status = sos_sys_obj_get_info(h_irq1, &info);
    if(FAIL(status))
    {
        SAP_ERR("Query interrupt vector of IRQ1 error!\n");
        status = EINTERNAL;
        goto query_irq1_err;
    }
    ps2_kbd_intr = info.irq_info.intr;

    status = sos_sys_obj_get_info(h_irq12, &info);
    if(FAIL(status))
    {
        SAP_ERR("Query interrupt vector of IRQ12 error!\n");
        status = EINTERNAL;
        goto query_irq12_err;
    }
    ps2_mouse_intr = info.irq_info.intr;
    
    SAP_INFO("PS2 keyboard interrupt vector:%u, mouse interrupt vector:%u\n", ps2_kbd_intr, ps2_mouse_intr);

    // On success
    return SUCCESS;


query_irq12_err:
    ps2_kbd_intr = INVALID_INTR;
query_irq1_err:
    if(h_irq12 != INVALID_HANDLE)
        sos_sys_obj_destroy(h_irq12);
create_irq12_err:
    if(h_irq1 != INVALID_HANDLE)
        sos_sys_obj_destroy(h_irq1);
create_irq1_err:
    return status;
}

static void _ps2_release_irqs(void)
{
    ps2_kbd_intr = INVALID_INTR;
    ps2_mouse_intr = INVALID_INTR;

    if(h_irq12 != INVALID_HANDLE)
	{
		sos_sys_obj_destroy(h_irq12);
		h_irq12 = INVALID_HANDLE;
	}

	if(h_irq1 != INVALID_HANDLE)
	{
		sos_sys_obj_destroy(h_irq1);
		h_irq1 = INVALID_HANDLE;
	}
}

static SAP_STATUS _iosm_ps2_init(void)
{
    SAP_STATUS status = SUCCESS;
    VKSTATUS status2 = SUCCESS;

    // Read the quirks of SecOS and SecBase
    status2 = sos_sys_quirks_get_info(&sos_quirks_info);
    if(FAIL(status2))
    {
        SAP_ERR("Init IOSM-PS2 client error! Read quirks error!\n");
        status = EINTERNAL;
        goto iosm_ps2_err;
    }

    // Talk to IOSM-PS2, and
    // Register the current SecProcess as the sole user of the PS2 ports
    status = init_iosm_ps2_client();
    if(FAIL(status))
    {
        SAP_ERR("Init IOSM-PS2 client error! status:%lu\n", status);
        status = EINTERNAL;
        goto iosm_ps2_err;
    }
    
    // On success
    return SUCCESS;

iosm_ps2_err:
    return status;
}

static void _iosm_ps2_fini(void)
{
    fini_libps2_client();
}

static void _drain_ps2_dev_buffers(void)
{
    uint8_t val = 0;

    is_draining_buffers = true;

    // After sending the command, the input buffers of device 1 are clear
    ps2_write(PS2_CMD, PS2_TEST_FIRST);
    // After dropping the response, the output buffers of device 1 are clear
    (void)ps2_read(PS2_DATA, &val);

    // After sending the command, the input buffers of device 2 are clear
    ps2_write(PS2_CMD, PS2_TEST_SECOND);
    // After dropping the response, the output buffers of device 2 are clear
    (void)ps2_read(PS2_DATA, &val);

    is_draining_buffers = false;
}

/* Performs discovery and initialisation of PS/2 controllers and devices.
 * A lot of redundancy here because of poorly-indexed identifiers.
 */
// __attribute__((unused))
// static SAP_STATUS _init_ps2_devices(void)
// {
//     SAP_INFO("initializing PS/2 devices\n");

//     bool dual_channel = true;

//     sos_sys_disable_intr();

//     // Disable both PS/2 device ports
//     // Even if only one is present, disabling the second is harmless
//     ps2_write(PS2_CMD, PS2_DISABLE_FIRST);
//     ps2_write(PS2_CMD, PS2_DISABLE_SECOND);

//     // Flush output bufffer: if the controller had anything to say, ignore it
//     inportb(PS2_DATA);

//     // Get the controller configuration byte
//     ps2_write(PS2_CMD, PS2_READ_CONFIG);
//     uint8_t config = ps2_read(PS2_DATA);

//     // Check the basics
//     config |= PS2_CFG_SYSTEM_FLAG;

//     if (config & PS2_CFG_MUST_BE_ZERO)
//     {
//         SAP_ERR("invalid bit set in configuration byte\n");
//         return EINVAL;
//     }

//     // Disable interrupts and scan code translation
//     config &= ~(PS2_CFG_FIRST_PORT | PS2_CFG_SECOND_PORT | PS2_CFG_TRANSLATION);

//     // Update configuration byte
//     ps2_write(PS2_CMD, PS2_WRITE_CONFIG);
//     ps2_write(PS2_DATA, config);

//     // Controller self-test
//     ps2_write(PS2_CMD, PS2_SELF_TEST);

//     if (ps2_read(PS2_DATA) != PS2_SELF_TEST_OK)
//     {
//         SAP_ERR("controller failed self-test\n");

//         controllers[0] = false;
//         controllers[1] = false;

//         return EINTERNAL;
//     }

//     // The last write may have reset our controller:
//     // better reset our configuration byte just in case
//     ps2_write(PS2_CMD, PS2_WRITE_CONFIG);
//     ps2_write(PS2_DATA, config);

//     // Check if we _really_ have two channels
//     ps2_write(PS2_CMD, PS2_ENABLE_SECOND);
//     ps2_write(PS2_CMD, PS2_READ_CONFIG);
//     config = ps2_read(PS2_DATA);

//     if (config & PS2_CFG_SECOND_CLOCK)
//     {
//         SAP_ERR("only one PS/2 controller\n");
//         dual_channel = false;
//         controllers[1] = false;
//     }
//     else
//     {
//         ps2_write(PS2_CMD, PS2_DISABLE_SECOND); // Re-disable the second controller
//     }

//     // Test the controllers
//     ps2_write(PS2_CMD, PS2_TEST_FIRST);

//     if (ps2_read(PS2_DATA) != PS2_TEST_OK)
//     {
//         SAP_ERR("first PS/2 port failed testing\n");
//         controllers[0] = false;
//         return EINTERNAL;
//     }

//     if (dual_channel)
//     {
//         ps2_write(PS2_CMD, PS2_TEST_SECOND);

//         if (ps2_read(PS2_DATA) != PS2_TEST_OK)
//         {
//             SAP_ERR("second PS/2 port failed testing\n");
//             controllers[1] = false;
//             return EINTERNAL;
//         }
//     }

//     // Enable available ports
//     if (controllers[0])
//     {
//         ps2_write(PS2_CMD, PS2_ENABLE_FIRST);
//         config |= PS2_CFG_FIRST_PORT;
//         config &= ~PS2_CFG_FIRST_CLOCK;
//     }

//     if (controllers[1])
//     {
//         ps2_write(PS2_CMD, PS2_ENABLE_SECOND);
//         config |= PS2_CFG_SECOND_PORT;
//         config &= ~PS2_CFG_SECOND_CLOCK;
//     }

//     // Enable interrupts from detected controllers
//     ps2_write(PS2_CMD, PS2_WRITE_CONFIG);
//     ps2_write(PS2_DATA, config);

//     // Reset devices
//     for (uint32_t i = 0; i < 2; i++)
//     {
//         if (!controllers[i])
//         {
//             continue;
//         }

//         ps2_write_device(i, PS2_DEV_RESET);
//         uint8_t ret = ps2_read(PS2_DATA);

//         usleep(1000);

//         // If it fails, disable the device's port
//         if (ret != PS2_DEV_ACK || ps2_read(PS2_DATA) != PS2_DEV_RESET_ACK)
//         {
//             SAP_ERR("failed to reset device %d\n", i);

//             controllers[i] = false;
//             config &= ~(i == 0 ? PS2_CFG_FIRST_PORT : PS2_CFG_SECOND_PORT);

//             ps2_write(PS2_CMD, PS2_WRITE_CONFIG);
//             ps2_write(PS2_DATA, config);
//             return EINTERNAL;
//         }

//         // For some reason, mice send an additional 0x00 byte
//         if (ps2_can_read())
//         {
//             ps2_read(0x60);
//         }
//     }

//     for (uint32_t i = 0; i < 2; i++)
//     {
//         if (controllers[i])
//         {
//             uint32_t type = ps2_identify_device(i);

//             switch (type)
//             {
//             case PS2_KEYBOARD:
//             case PS2_KEYBOARD_TRANSLATED:
//                 SAP_INFO("keyboard\n");
//                 init_kbd(i);
//                 break;
//             case PS2_MOUSE:
//             case PS2_MOUSE_SCROLL_WHEEL:
//             case PS2_MOUSE_FIVE_BUTTONS:
//                 SAP_INFO("mouse\n");
//                 init_mouse(i);
//             }
//         }
//     }

//     sos_sys_enable_intr();

//     // On success
//     return SUCCESS;
// }



/********* Public Functions *********/
uint8_t inportb(uint16_t port)
{
    VKSTATUS status = SUCCESS;
    uint32_t result = 0;

    status = sos_sys_portio_read(port, PIO_SIZE_BYTE, &result);
    if(FAIL(status))
        SAP_ERR("PIO (%u) read error! status:%lu\n", (uint32_t)port, status);

    return (uint8_t)result;
}

void outportb(uint16_t port, uint8_t value)
{
    VKSTATUS status = SUCCESS;

    status = sos_sys_portio_write(port, value, PIO_SIZE_BYTE);
    if(FAIL(status))
        SAP_ERR("PIO (%u) write error! status:%lu\n", (uint32_t)port, status);
}

SAP_STATUS libhid_ps2_init(void)
{
    SAP_STATUS status = SUCCESS;

    // Init IOSM-PS2 client
    status = _iosm_ps2_init();
    if(FAIL(status))
    {
        SAP_ERR("[PS2] Init IOSM-PS2 client error!\n");
        status = EINTERNAL;
        goto init_iosm_ps2_err;
    }

    // Own PS2 IRQs
    status = _ps2_own_irqs();
    if(FAIL(status))
    {
        SAP_ERR("[PS2] Own PS2 IRQs error!\n");
        status = EINTERNAL;
        goto own_ps2_irqs_err;
    }

    // Empty PS2 devices' buffers
    _drain_ps2_dev_buffers();

    // Probe the PS2 mouse
    status = ps2_mouse_init();
    if(FAIL(status))
    {
        SAP_ERR("[PS2] Fail to initialize PS2 mouse!\n");
        status = EINTERNAL;
        goto init_mouse_err;
    }

    // ps2_touchpad_probe();

    // Invoke PS2 devices interrupts because they can ensure the devices continue to work
    is_intr_initialized = true;
    ps2_intr_handler(ps2_kbd_intr);
    ps2_intr_handler(ps2_mouse_intr);

    // Probe and init PS2 devices
    // status = _init_ps2_devices();
    // if(FAIL(status))
    // {
    //     SAP_ERR("[PS2] Probe and init PS2 devices error!\n");
    //     status = EINTERNAL;
    //     goto init_ps2_devs_err;
    // }
    
    // On success
    SAP_INFO("PS2 devices init success\n");
    return SUCCESS;


// init_ps2_devs_err:
    _ps2_release_irqs();
init_mouse_err:
own_ps2_irqs_err:
    _iosm_ps2_fini();
init_iosm_ps2_err:
    return status;
}

void libhid_ps2_fini(void)
{
    SAP_STATUS status = SUCCESS;
    
    is_intr_initialized = false;

    // Finalize PS2 mouse
    status = ps2_mouse_fini();
    if(FAIL(status))
        SAP_ERR("[PS2] Fail to finalize PS2 mouse!\n");

    // Empty PS2 devices' buffers
    _drain_ps2_dev_buffers();

    _ps2_release_irqs();
    _iosm_ps2_fini();
}

/* Asks the device to identify itself, returns an enum value.
 * Useful as some devices's identities are several bytes long.
 */
PS2_DEV_TYPE ps2_identify_device(uint32_t num)
{
    SAP_STATUS status1 = SUCCESS;
    bool status = true;
    uint8_t first_id_byte = 0;
    uint8_t second_id_byte = 0;
    PS2_DEV_TYPE result = 0;

    // Disables scanning
    ps2_write_device(num, PS2_DEV_DISABLE_SCAN); 
    status1 = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if(FAIL(status1))
    {
        SAP_ERR("Identify PS2 mouse type error: Failed to disable mouse port!\n");
        return PS2_DEVICE_UNKNOWN;
    }

    // Identify
    ps2_write_device(num, PS2_DEV_IDENTIFY); 
    status1 = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if(FAIL(status1))
    {
        SAP_ERR("Identify PS2 mouse type error: Failed to identify mouse!\n");
        return PS2_DEVICE_UNKNOWN;
    }

    // [TODO] Ugly hack: Remove the check of "<first_id_byte> must not be PS2_DEV_ACK"
    do
    {
        status = ps2_read(PS2_DATA, &first_id_byte);
        if(!status)
            return PS2_DEVICE_UNKNOWN;
    }while(first_id_byte == PS2_DEV_ACK);

    status = ps2_read(PS2_DATA, &second_id_byte);
    if(!status)
        return PS2_DEVICE_UNKNOWN;

    result = ps2_identity_bytes_to_type(first_id_byte, second_id_byte);

    // Enables scanning
    ps2_write_device(num, PS2_DEV_ENABLE_SCAN); 
    status1 = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if(FAIL(status1))
    {
        SAP_ERR("Identify PS2 mouse type error: Failed to resume mouse!\n");
        return PS2_DEVICE_UNKNOWN;
    }

    return result;
}

/* Converts the (one or two) bytes sent by a PS/2 device in response to an
 * `identity` request to a code representing the device type.
 */
PS2_DEV_TYPE ps2_identity_bytes_to_type(uint8_t first, uint8_t second)
{
    if (first == 0x00 || first == 0x03 || first == 0x04)
    {
        return first; // PS2_MOUSE* match the first byte
    }
    else if (first == 0xAB)
    {
        if (second == 0x41 || second == 0xC1)
        {
            return PS2_KEYBOARD_TRANSLATED;
        }
        else if (second == 0x83)
        {
            return PS2_KEYBOARD;
        }
    }

    return PS2_DEVICE_UNKNOWN;
}

// Number of iteration in our PS/2 IO spin loops
#define PS2_TIMEOUT 500

/* Loops until the controller's input buffer is empty or our timer has expired.
 * Call this function before writing to the controller's command port, PS2_CMD.
 */
bool ps2_wait_write()
{
    int timer = PS2_TIMEOUT;

    while ((inportb(PS2_STATUS) & 2) && timer-- > 0)
    {
        asm("pause");
    }

    return timer != 0;
}

/* Loops until data is available in the controller's output buffer or until our
 * timer has expired.
 * Call this function before reading from the controller's data port, PS2_DATA.
 */
bool ps2_wait_read()
{
    int timer = PS2_TIMEOUT;

    while (!(inportb(PS2_STATUS) & 1) && timer-- >= 0)
    {
        asm("pause");
    }

    return timer != 0;
}

/* Returns the first byte of data available from `port`.
 * Returns `(uint8_t) -1` on read error, and also when this value was
 * legitimately read. TODO: room for improvement.
 */
bool ps2_read(uint32_t port, uint8_t* out_val)
{
    if(!out_val)
        return false;

    if (ps2_wait_read())
    {
        *out_val = inportb(port);
        return true;
    }

    return false;
}

/* Tries to write a byte on the specified port, and returns whether the
 * operation was successful.
 */
bool ps2_write(uint32_t port, uint8_t b)
{
    if (ps2_wait_write())
    {
        outportb(port, b);
        return true;
    }

    SAP_ERR("write failed\n");

    return false;
}

/* Write a byte to the specified `device` input buffer.
 * This function is used to send command to devices.
 */
bool ps2_write_device(uint32_t device, uint8_t b)
{
    if (device != 0)
    {
        if (!ps2_write(PS2_CMD, PS2_WRITE_SECOND))
        {
            return false;
        }
    }

    return ps2_write(PS2_DATA, b);
}

#define BLOCK_WAIT_USLEEP_PER_ROUND	(100)

SAP_STATUS ps2_expect_ack(uint32_t timeout_ms)
{
    uint8_t ret = 0;
    bool status = true;
    volatile uint32_t timeout_round = timeout_ms * 1000 / BLOCK_WAIT_USLEEP_PER_ROUND;

    do
    {
        status = ps2_read(PS2_DATA, &ret);
        if(!status)
            return EINTERNAL;

        if (ret == PS2_DEV_ACK)
            break;
        
        usleep(BLOCK_WAIT_USLEEP_PER_ROUND);
        timeout_round--;
    } while (timeout_round != 0);
    
    if(!timeout_round)
        return ETIMEOUT;

    return SUCCESS;
}

bool ps2_can_read()
{
    return inportb(PS2_CMD) & 1;
}

static bool ps2_parse_ready_on_kbd_or_aux(uint32_t* out_ps2_st, bool* out_is_mb_full)
{
    uint32_t st;

    st = inportb(PS2_STATUS);
    if((st & PS2_STS_OBF) == 0)
        return false;
        
    if(out_ps2_st)
        *out_ps2_st = st;
    if(out_is_mb_full)
        *out_is_mb_full = !!(st & PS2_STS_MOBF);

    return true;
}

void ps2_intr_poll(void)
{
    uint32_t ps2_st = 0;
    bool is_mb_full = false;

    if(!ps2_parse_ready_on_kbd_or_aux(&ps2_st, &is_mb_full))
        return;

    if(!is_mb_full)
        kbd_handler_poll(ps2_st);
    else
        mouse_handle_polling(ps2_st);
}

SAP_STATUS ps2_intr_handler(intr_t intr)
{
    // If the code is draining devices' buffers, we ignore all interrupts (which are largely due to data ready).
    if(is_draining_buffers)
        return SUCCESS;

    // If the code is not ready to receive interrupts, then ignore them.
    if(!is_intr_initialized)
        return SUCCESS;

    // Handle PS2 interrupts according to SecOS's and SecBase's quirks
    if(sos_quirks_info.quirks_support_ns_dom_intr_interception)
    {
        // SecOS and SecBase supports interrupt interception of the NS domain, so they calls this handler only if there
        // is a real PS2 interrupt
        if(intr == ps2_kbd_intr)
            kbd_handler();
        else if (intr == ps2_mouse_intr)
            mouse_handle_interrupt();
    }
    else
    {
        // SecOS and SecBase do not support interrupt interception of the NS domain. So they will inject spurious PS2
        // interrupts to this handler to avoid missing PS2 interrupts.
        ps2_intr_poll();
    }

    // On success
    return SUCCESS;
}

