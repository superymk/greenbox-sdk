// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// Server Command: x11vnc -display :0 -rfbport 8000 -v -shared -many -repeat -debug_keyboard -debug_pointer

#include <sap_headers.h>
#include <palddk.h>

#include <app_sp_intfs.h>

#include "rdesktop.h"
#include "rdesktop_hid.h"
#include "rdesktop_rfb.h"

/// @brief SecProcess constant properties (e.g., name, type).
const SP_CONST_PARAMETERS sp_constant_properties __attribute__((section(".version_rodata"))) = {
    .version_number = GREENBOX_VERSION_NUMBER, // SecProcess version number

    .sp_type = SP_APPLICATION, // SecProcess type
    "SecApp-RDesktop",         // SecProcess name

    .is_trusted = false, // Is the SecProcess a trusted one?
    .sap_params =
    {
        ._reserved = 0 // Is the SecProcess a INIT SecProcess?
    }
};

extern void sp_kbd_listener(struct kbd_key_event *event);
extern void sp_mouse_listener(struct mouse_event *event);

struct event
{
    int event_type;
    void *event;
};
#define EVENT_KBD 1
#define EVENT_MOUSE 2
static List *event_list = NULL;
__LOCK_INIT(static, e_list_lock);

static rfbClient *cl = NULL;

/********* RDesktop Configurations *********/
static SAP_CONFIG sap_conf = {0};
static uint32_t win_phy_width = 0;
static uint32_t win_phy_height = 0;

/********* Private functions *********/
static void _print_welcome_message(void)
{
    printf("******************************\n");
    printf("Starting the Demo - Remote Desktop\n");
    printf("******************************\n");
}

static SAP_STATUS _check_cfgs(SAP_CONFIG *conf)
{
    // Check: compress level and quality level must be in [0, 10). If not, set them to be 5
    if (!VALUE_IN_RANGE(conf->display_info.compress, 0, 10))
        conf->display_info.compress = 5;
    if (!VALUE_IN_RANGE(conf->display_info.quality, 0, 10))
        conf->display_info.quality = 5;

    // On success
    return SUCCESS;
}

static SAP_STATUS _read_cfgs(SP_UTASK *params)
{
    SAP_STATUS status = SUCCESS;
    SAP_CONFIG *in_sap_conf = (SAP_CONFIG *)params->exec_params_secure;

    // Copy <in_sap_conf> to prevent TOCTTOU attacks
    memcpy(&sap_conf, in_sap_conf, sizeof(SAP_CONFIG));

    // Check: Configurations must be valid
    status = _check_cfgs(&sap_conf);
    if (FAIL(status))
    {
        SAP_ERR("Invalid SecApp configurations! status:%lu\n", status);
        return EINTERNAL;
    }

    // Read <window_info>
    win_phy_width = in_sap_conf->window_info.win_logical_width * in_sap_conf->window_info.win_scale;
    win_phy_height = in_sap_conf->window_info.win_logical_height * in_sap_conf->window_info.win_scale;

    // On success
    return SUCCESS;
}

static SAP_STATUS _init_libgui(WINDOW_INFO *info)
{
    SAP_STATUS status = SUCCESS;
    struct libgui_config cfg;

    cfg.fb_vaddr = info->fb_vaddr;
    cfg.fb_width = info->fb_width;
    cfg.fb_height = info->fb_height;

    cfg.lines_per_wheel = 3;

    status = libgui_init(&cfg, true);

    //// Display the main window
    wnd_init_main();

    return status;
}

/********* Public functions *********/
void sp_main(SP_UTASK *params)
{
    SAP_STATUS status = SUCCESS;
    WINDOW_INFO win_info;
    int i = 0;

    _print_welcome_message();

    event_list = stl_list_create();

    // Read configs
    status = _read_cfgs(params);
    if (FAIL(status))
    {
        SAP_ERR("Read configs error! status:%lu\n", status);
        goto out;
    }

    // Init main window
    status = libdtm_init(win_phy_width, win_phy_height);
    if (FAIL(status))
    {
        SAP_ERR("Init main window error! status:%lu\n", status);
        goto out;
    }

    status = libdtm_get_window_info(&win_info);
    if (FAIL(status))
    {
        SAP_ERR("Get main window's info error! status:%lu\n", status);
        goto out;
    }

    // Init GUI
    status = _init_libgui(&win_info);
    if (FAIL(status))
    {
        SAP_ERR("Init GUI error! status:%lu\n", status);
        goto out;
    }

    //// Init keyboard and mouse event handlers
    libdtm_hid_register_kbd_listener(libgui_kbd_listener);
    libdtm_hid_register_mouse_listener(libgui_mouse_listener);

    // wnd_login_show("aaa");

    // Init RFB Client - Remote Desktop Canvas Window
    status = wnd_rdesktop_canvas_init(win_phy_width, win_phy_height);
    if (FAIL(status))
    {
        SAP_ERR("RFB Client (Remote Desktop Canvas Window) init failed! status:%lu\n", status);
        goto out;
    }

    // Init RFB Client - Login Window
    status = rdesktop_init(&sap_conf, (vaddr_t)wnd_rdesktop_get_fb(), win_phy_width, win_phy_height, &cl);
    if (FAIL(status))
    {
        SAP_ERR("RFB Client (Login Window) init failed! status:%lu\n", status);
        goto out;
    }

    // Add RDesktop keyboard and mouse handlers to forward HID events to the RDesktop server.
    libdtm_hid_register_kbd_listener(sp_kbd_listener);
    libdtm_hid_register_mouse_listener(sp_mouse_listener);

    // Destroy the Login window and show the RDesktop window
    wnd_destroy_ScreenLogin();

    wnd_rdesktop_show();

    // Main loop
    while (true)
    {
        // Pull and handle all DesktopMgr events (e.g., key presses)
        status = libdtm_handle_dtm_events();
        if (FAIL(status))
        {
            SAP_WARN("Handle DesktopMgr events error! status:%lu\n", status);
        }

        // For GUI
        libgui_cycle();

        // I/O logic
        while (!is_list_empty(event_list))
        {
            struct event *e = NULL;

            __libc_lock_acquire(&e_list_lock);
            e = stl_list_dequeue(event_list);
            __libc_lock_release(&e_list_lock);

            if (e->event_type == EVENT_KBD)
                rdesktop_handle_kbd_event(cl, e->event);
            else if (e->event_type == EVENT_MOUSE)
                rdesktop_handle_mouse_event(cl, e->event);

            free(e->event);
        }

        // VNC logic
        i = WaitForMessage(cl, 50);
        if (i < 0)
        {
            rdesktop_cleanup(cl);
            break;
        }
        else if (i)
        {
            if (HandleRFBServerMessage(cl))
            {
                // Do nothing.
            }
            else
            {
                rdesktop_cleanup(cl);
                break;
            }
        }

        // wnd_rdesktop_refresh_all();

        // Release CPU
        sp_yield();
    }

out:
    sp_exit(params);
    return;
}

void sp_exit(SP_UTASK *sp_params)
{
    libdtm_hid_unregister_kbd_listener(sp_kbd_listener);
    libdtm_hid_unregister_mouse_listener(sp_mouse_listener);

    // Finalize Remote Desktop Canvas Window
    wnd_rdesktop_canvas_fini();

    // Finalize GUI
    libgui_fini();

    // Finalize main window
    libdtm_fini();

    if (event_list)
    {
        stl_list_clear_destroy(event_list);
        event_list = NULL;
    }

    SAP_INFO("Demo - Remote Desktop Exited!\n");
}

void sp_handle_intr(intr_t vector)
{
    // Empty function
}

static SAP_STATUS _on_sp_request(SP_MESSAGE *msg, COMM_ISP_SHAREDMEM *comm)
{
    // SAP_STATUS status = SUCCESS;

    // // Handle DesktopMgr events
    // status = libdtm_on_dtm_event(msg, comm);
    // if (FAIL(status) && status != ENOTHANDLED)
    // 	return status;

    // return status;

    return SUCCESS;
}

void sp_handle_sp_request(SP_MESSAGE *msg)
{
    SAP_STATUS status = SUCCESS;

    switch (msg->header.cmd)
    {
    case REQ_SP_SPECIFIC_CALL:
    {
        spid_t caller_sp_id = msg->header.sender_id;
        COMM_ISP_SHAREDMEM *comm = NULL;

        SAP_DEBUG("SecProcess (id:%u) sends a request with sub-command %u\n", caller_sp_id, msg->header.sub_cmd);

        // Find the communication channel associated with the caller SecProcess.
        comm = comm_isp_sharedmem_find(caller_sp_id);
        if (!comm)
        {
            SAP_ERR("SecProcess (id:%u) sends commands to this SecProcess before creating communication channels\n", caller_sp_id);
            break;
        }

        // Handle the incoming request
        status = _on_sp_request(msg, comm);
        if (FAIL(status))
        {
            SAP_ERR("Failed to handle request from SecProcess (id:%u)! status:%lu\n", caller_sp_id, status);
            break;
        }

        break;
    }
    case REQ_OBJ_USER_MEM_ACCESS:
        break;
    default:
        break;
    }
}

bool sp_handle_app_request(struct ipc_msg *msg)
{
    // Sanity checks
    if (!msg)
        return false;

    // Process events
    return true;
}

void sp_kbd_listener(struct kbd_key_event *e)
{
    struct event *new_event = NULL;
    struct kbd_key_event *new_kbd_event = NULL;

    new_event = (struct event *)malloc(sizeof(struct event));
    if (!new_event)
        goto err;
    new_kbd_event = (struct kbd_key_event *)malloc(sizeof(struct kbd_key_event));
    if (!new_kbd_event)
        goto err;
    memcpy(new_kbd_event, e, sizeof(struct kbd_key_event));

    new_event->event = new_kbd_event;
    new_event->event_type = EVENT_KBD;

    __libc_lock_acquire(&e_list_lock);
    stl_list_enqueue(event_list, new_event);
    __libc_lock_release(&e_list_lock);

    return;

err:
    if (new_event)
        free(new_event);
    if (new_kbd_event)
        free(new_kbd_event);
}

void sp_mouse_listener(struct mouse_event *e)
{
    struct event *new_event = NULL;
    struct mouse_event *new_mouse_event = NULL;

    new_event = (struct event *)malloc(sizeof(struct event));
    if (!new_event)
        goto err;
    new_mouse_event = (struct mouse_event *)malloc(sizeof(struct mouse_event));
    if (!new_mouse_event)
        goto err;
    memcpy(new_mouse_event, e, sizeof(struct mouse_event));

    new_event->event = new_mouse_event;
    new_event->event_type = EVENT_MOUSE;

    __libc_lock_acquire(&e_list_lock);
    stl_list_enqueue(event_list, new_event);
    __libc_lock_release(&e_list_lock);

    return;

err:
    if (new_event)
        free(new_event);
    if (new_mouse_event)
        free(new_mouse_event);
}

void sp_handle_exception(void)
{
    sp_exit(current_sp);
}