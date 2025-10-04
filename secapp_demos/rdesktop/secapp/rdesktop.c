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

#include "rdesktop.h"
#include "rdesktop_hid.h"
#include "rdesktop_canvas.h"

static int realWidth, realHeight, bytesPerPixel, rowStride;
static char *sdlPixels = NULL;

static rfbClient *cl;
// #define NUM_ARGS 2
#define NUM_ARGS 6

__attribute__((__unused__))
static rfbCredential *_get_credential(rfbClient *cl, int credentialType)
{
	rfbCredential *c = malloc(sizeof(rfbCredential));

	if (!c)
	{
		return NULL;
	}
	c->userCredential.username = malloc(RFB_BUF_SIZE);
	if (!c->userCredential.username)
		goto err;
	
	c->userCredential.password = malloc(RFB_BUF_SIZE);
	if (!c->userCredential.password)
		goto err;

	if (credentialType != rfbCredentialTypeUser)
	{
		rfbClientErr("something else than username and password required for authentication\n");
		goto err;
	}

	rfbClientLog("username and password required for authentication!\n");
	cred_username_get(c->userCredential.username);
	cred_password_get(c->userCredential.password);

	// /* remove trailing newlines */
	// c->userCredential.username[strcspn(c->userCredential.username, "\n")] = 0;
	// c->userCredential.password[strcspn(c->userCredential.password, "\n")] = 0;

	return c;


err:
	if(c->userCredential.password)
		free(c->userCredential.password);
	if(c->userCredential.username)
		free(c->userCredential.username);
	free(c);
	return NULL;
}


__attribute__((__unused__))
static char * _get_password(rfbClient *client)
{
	char* result = NULL;

	// Show the login screen
	wnd_login_show();

	// Get the password
	// The password returned must be malloced (e.g., must not be a global variable)!
	result = (char*)malloc(RD_PASSWORD_LENGTH_MAX);
	if(!result)
		return NULL;

	cred_password_get(result);

	return result;
}

static void _rdesktop_init_resource(vaddr_t fb_vaddr, uint32_t win_width, uint32_t win_height)
{
	/* 16-bit: cl=rfbGetClient(5,3,2); */
	realWidth = win_width;
	realHeight = win_height;
	bytesPerPixel = 4;
	rowStride = realWidth;
	sdlPixels = (char *)fb_vaddr;

	rdesktop_hid_init(realWidth, realHeight, bytesPerPixel, rowStride, sdlPixels);
	rdesktop_canvas_init(realWidth, realHeight, bytesPerPixel, rowStride, sdlPixels);
}

static char _compress_level_str[4] = {0};
static char _quality_level_str[4] = {0};

static SAP_STATUS _rdesktop_init_vnc(SAP_CONFIG* conf, rfbClient **out_cl)
{
	SAP_STATUS status = SUCCESS;
	rfbBool rfb_status = TRUE;
	int argc = NUM_ARGS;
	char *argv[NUM_ARGS];

	cl = rfbGetClient(8, 3, 4);
	if (!cl)
	{
		SAP_ERR("RFB get client error!\n");
		return EINTERNAL;
	}

	// Overwrite RFBClient's AppData settings.
	//// Tell the RFB server to not render the mouse pointer.
	cl->appData.useRemoteCursor = TRUE;
	//// Force use JPEG compression.
	cl->appData.enableJPEG = TRUE;

	cl->GotFrameBufferUpdate = rdesktop_canvas_update;
	cl->canHandleNewFBSize = FALSE;
	// cl->MallocFrameBuffer= resize;
	/*
	cl->HandleKeyboardLedState=kbd_leds;
	cl->HandleTextChat=text_chat;
	cl->GotXCutText = got_selection;*/

	// Use RGB 24-bit format
	cl->format.redShift = LIBDTM_RED_BIT_SHIFT;
	cl->format.greenShift = LIBDTM_GREEN_BIT_SHIFT;
	cl->format.blueShift = LIBDTM_BLUE_BIT_SHIFT;
	SetFormatAndEncodings(cl);

	cl->GetPassword = _get_password;
	cl->GetCredential = _get_credential;

	argv[0] = "./secapp";

	argv[1] = "-compress";
	sprintf(_compress_level_str, "%d", conf->display_info.compress);
	argv[2] = _compress_level_str;

	argv[3] = "-quality";
	sprintf(_quality_level_str, "%d", conf->display_info.quality);
	argv[4] = _quality_level_str;
	argv[5] = conf->logon_info.url;

	SAP_DEBUG("RDesktop is connecting to %s\n", conf->logon_info.url);
	SAP_DEBUG("RDesktop is using compress level %d and quality level %d\n", conf->display_info.compress, conf->display_info.quality);

	rfb_status = rfbInitClient(cl, &argc, argv);
	if(rfb_status != TRUE)
	{
		SAP_ERR("RFB Client init error!\n");
		status = EINTERNAL;
		goto rfb_init_err;
	}

	// rfb_status = SendScaleSetting(cl, 10);
	// if(rfb_status != TRUE)
	// {
	// 	SAP_ERR("RFB Client set caling error!\n");
	// 	status = EINTERNAL;
	// 	goto rfb_scale_err;
	// }

	// On success
	*out_cl = cl;
	return SUCCESS;

// rfb_scale_err:
rfb_init_err:
	// No need to call <rfbClientCleanup(client)>. If <rfbInitClient> calls it before returning FALSE 
	return status;
}

#define SERVER_CONNECT_RETRY_MAX	(3)
#define SERVER_CONNECT_RETRY_WAIT_SEC	(1)
SAP_STATUS rdesktop_init(SAP_CONFIG* conf, vaddr_t fb_vaddr, uint32_t win_width, uint32_t win_height, rfbClient **out_cl)
{
	SAP_STATUS status = SUCCESS;
	int retry_connect = 0;

	// Check: <conf> and <out_cl> must be valid
	if (!conf || !out_cl)
		return EINVAL;
		
	_rdesktop_init_resource(fb_vaddr, win_width, win_height);

	// Connect to the VNC server (Retry for maximum <SERVER_CONNECT_RETRY_MAX> times).
	wnd_login_mark_first_load();
	while(retry_connect < SERVER_CONNECT_RETRY_MAX)
	{
		status = _rdesktop_init_vnc(conf, out_cl);
		if(PASS(status))
			break;
		else
		{
			SAP_ERR("RDesktop Client init error on try %d!\n", (retry_connect + 1));
			wnd_login_unmark_first_load();
		}

		retry_connect++;
		_temp_wait_in_wnd_login("Incorrect Connection Password!", SERVER_CONNECT_RETRY_WAIT_SEC * MSEC_PER_SEC); // Add a delay so we will not be banned by the VNC server
	}

	if(retry_connect >= SERVER_CONNECT_RETRY_MAX)
	{
		SAP_ERR("RDesktop Client init error!\n");
		return EINTERNAL;
	}

	// On success
	return SUCCESS;
}

void rdesktop_cleanup(rfbClient *cl)
{
	/*
	  just in case we're running in listenLoop:
	  close viewer window by restarting SDL video subsystem
	*/
	if (cl)
	{
		rfbClientCleanup(cl);
		cl = NULL; /* rfbInitClient has already freed the client struct */
	}
}