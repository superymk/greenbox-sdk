// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include <app_sp_intfs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// #include <sys/ipc/varbuf.h>
// #include <sys/ipc/comm.h>

/// @brief SecProcess constant properties (e.g., name, type).
const SP_CONST_PARAMETERS sp_constant_properties __attribute__((section(".version_rodata"))) = {
	.version_number = GREENBOX_VERSION_NUMBER,	// SecProcess version number

	.sp_type = SP_APPLICATION,			// SecProcess type
	"SecApp-RichGUI",					// SecProcess name

	.is_trusted = false,				// Is the SecProcess a trusted one?
	.sap_params = 
	{
		._reserved = 0					// Is the SecProcess a INIT SecProcess?
	}
};

static bool RUN = true;

static SAP_CONFIG sap_conf = {0};
static uint32_t win_phy_width = 0;
static uint32_t win_phy_height = 0;

static void _print_welcome_message(void)
{
	printf("******************************\n");
	printf("Starting the Demo - Rich GUI\n");
	printf("******************************\n");
}




/********* Private functions *********/
static SAP_STATUS _init_libgui(WINDOW_INFO* info)
{
	SAP_STATUS status = SUCCESS;
	struct libgui_config cfg;
	
	cfg.fb_vaddr = info->fb_vaddr;
	cfg.fb_width = info->fb_width;
	cfg.fb_height = info->fb_height;

	cfg.lines_per_wheel = 3;

	status = libgui_init(&cfg, true);

	lv_demo_widgets();

	return status;
}

static SAP_STATUS _read_cfgs(SP_UTASK *params)
{
	// SAP_STATUS status = SUCCESS;
	SAP_CONFIG* in_sap_conf = (SAP_CONFIG*)params->exec_params_secure;

	// Copy <in_sap_conf> to prevent TOCTTOU attacks
	memcpy(&sap_conf, in_sap_conf, sizeof(SAP_CONFIG));

	// Read <window_info>
	win_phy_width = in_sap_conf->window_info.win_logical_width * in_sap_conf->window_info.win_scale;
	win_phy_height = in_sap_conf->window_info.win_logical_height * in_sap_conf->window_info.win_scale;

	return SUCCESS;
}




/********* Public functions *********/
// void print_stack_pointer() {
//   void* p = NULL;
//   SAP_ERR("%p\n", (void*)&p);
// }

// void sp_main(SP_UTASK* params)
// {
// 	print_stack_pointer();
// 	(void)win_cur_width;(void)win_cur_height;(void)_init_libgui; (void)_print_welcome_message;(void)RUN;
// }

void sp_main(SP_UTASK* params) 
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
	if(FAIL(status))
	{
		SAP_ERR("Init main window error! status:%lu\n", status);
		goto out;
	}

	status = libdtm_get_window_info(&win_info);
	if(FAIL(status))
	{
		SAP_ERR("Get main window's info error! status:%lu\n", status);
		goto out;
	}

	// Init GUI
	status = _init_libgui(&win_info);
	if(FAIL(status))
	{
		SAP_ERR("Init GUI error! status:%lu\n", status);
		goto out;
	}

	libdtm_hid_register_kbd_listener(libgui_kbd_listener);
	libdtm_hid_register_mouse_listener(libgui_mouse_listener);

	// Main loop
	RUN = true;

	while(RUN)
	{
		// Pull and handle all DesktopMgr events (e.g., key presses)
		libdtm_handle_dtm_events();

		// For GUI
		libgui_cycle();

		// cpu_relax();
		sp_yield();
	}


out:
	sp_exit(params);
	return;
}

void sp_exit(SP_UTASK* sp_params)
{
	libdtm_hid_unregister_kbd_listener(libgui_kbd_listener);
	libdtm_hid_unregister_mouse_listener(libgui_mouse_listener);

	// Fini GUI
	libgui_fini();

	// Finalize main window
	libdtm_fini();

	SAP_INFO("Rich GUI Demo Exited!\n");
}


void sp_handle_intr(intr_t vector)
{
	// Empty function
}

static SAP_STATUS _on_sp_request(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm)
{
	// SAP_STATUS status = SUCCESS;

	// // Handle DesktopMgr events
	// status = libdtm_on_dtm_event(msg, comm);
	// if(FAIL(status) && status != ENOTHANDLED)
	// 	return status;

	// return status;

	return SUCCESS;
}

void sp_handle_sp_request(SP_MESSAGE* msg)
{
	SAP_STATUS status = SUCCESS;

	switch(msg->header.cmd)
	{
		case REQ_SP_SPECIFIC_CALL:
		{
			spid_t caller_sp_id = msg->header.sender_id;
			COMM_ISP_SHAREDMEM* comm = NULL;

			SAP_DEBUG("SecProcess (id:%u) sends a request with sub-command %u\n", caller_sp_id, msg->header.sub_cmd);

			// Find the communication channel associated with the caller SecProcess.
			comm = comm_isp_sharedmem_find(caller_sp_id);
			if(!comm)
			{
				SAP_ERR("SecProcess (id:%u) sends commands to this SecProcess before creating communication channels\n", caller_sp_id);
				break;
			}

			// Handle the incoming request
			status = _on_sp_request(msg, comm);
			if(FAIL(status))
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

bool sp_handle_app_request(struct ipc_msg* msg)
{
	// Sanity checks
	if(!msg)
		return false;

	// Process events
	return true;
}

void sp_handle_exception(void)
{
	sp_exit(current_sp);
}