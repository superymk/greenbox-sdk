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

#include <sap_headers.h>
#include <palddk.h>

#include "rfiles.h"
#include "protocols/gdrive_limited.h"

#include "ui/rfiles_ui/wnd_rfiles.h"
#include "ui/rfiles_ui/wnd_rfiles_list.h"

/// @brief SecProcess constant properties (e.g., name, type).
const SP_CONST_PARAMETERS sp_constant_properties __attribute__((section(".version_rodata"))) = {
    .version_number = GREENBOX_VERSION_NUMBER, // SecProcess version number

    .sp_type = SP_APPLICATION, // SecProcess type
    "SecApp-RFiles",         // SecProcess name

    .is_trusted = false, // Is the SecProcess a trusted one?
    .sap_params =
    {
        ._reserved = 0 // Is the SecProcess a INIT SecProcess?
    }
};

/********* RFiles Configurations *********/
static SAP_CONFIG sap_conf = {0};
static uint32_t win_phy_width = 0;
static uint32_t win_phy_height = 0;

/********* Private functions *********/
static void _print_welcome_message(void)
{
    printf("******************************\n");
    printf("Starting the Demo - Remote Files\n");
    printf("******************************\n");
}

static SAP_STATUS _check_cfgs(SAP_CONFIG *conf)
{
    // // Check: compress level and quality level must be in [0, 10). If not, set them to be 5
    // if (!VALUE_IN_RANGE(conf->display_info.compress, 0, 10))
    //     conf->display_info.compress = 5;
    // if (!VALUE_IN_RANGE(conf->display_info.quality, 0, 10))
    //     conf->display_info.quality = 5;

    // On success
    return SUCCESS;
}

__attribute__((unused)) static SAP_STATUS _read_cfgs(SP_UTASK *params)
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

    // Pass the configs to the window_rfiles_list.
    status = wnd_rfiles_list_init_cfgs(&sap_conf);
    if (FAIL(status))
    {
        SAP_ERR("Failed to pass configs to Window-RFiles-List! status:%lu\n", status);
        return EINTERNAL;
    }

    // On success
    return SUCCESS;
}

__attribute__((unused)) static SAP_STATUS _init_libgui(WINDOW_INFO *info)
{
    SAP_STATUS status = SUCCESS;
    struct libgui_config cfg;

    cfg.fb_vaddr = info->fb_vaddr;
    cfg.fb_width = info->fb_width;
    cfg.fb_height = info->fb_height;

    cfg.lines_per_wheel = 3;

    status = libgui_init(&cfg, true);

    return status;
}

/********* Public functions *********/
void sp_main(SP_UTASK *params)
{
    SAP_STATUS status = SUCCESS;
    WINDOW_INFO win_info;

    _print_welcome_message();

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

    // Init RFiles UI
    wnd_rfiles_show();

    // Main loop
    while (true)
    {
        SAP_STATUS status = SUCCESS;

        // Pull and handle all DesktopMgr events (e.g., key presses)
        status = libdtm_handle_dtm_events();
        if (FAIL(status))
        {
            SAP_WARN("Handle DesktopMgr events error! status:%lu\n", status);
        }

        // For GUI
        libgui_cycle();

        // Release CPU
        sp_yield();
    }

out:
    sp_exit(params);
    return;
}

void sp_exit(SP_UTASK *sp_params)
{
    libdtm_hid_unregister_kbd_listener(libgui_kbd_listener);
	libdtm_hid_unregister_mouse_listener(libgui_mouse_listener);

    // Finalize Remote Files Window
    wnd_rfiles_close();

    // Finalize GUI
    libgui_fini();

    // Finalize main window
    libdtm_fini();

    SAP_INFO("Demo - Remote Files Exited!\n");
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

void sp_handle_exception(void)
{
    sp_exit(current_sp);
}







// // [Superymk]
// void sp_main(SP_UTASK *params)
// {
//     _print_welcome_message();


//     // [Superymk]
//     {
//         gdrive_test();
//     }
//     // [Superymk] End

//     // Main loop
//     while (true)
//     {
//         // Release CPU
//         sp_yield();
//     }

//     sp_exit(params);
//     return;
// }

// void sp_exit(SP_UTASK *sp_params)
// {
//     SAP_INFO("Demo - Remote Files Exited!\n");
// }

// // [Superymk] End
