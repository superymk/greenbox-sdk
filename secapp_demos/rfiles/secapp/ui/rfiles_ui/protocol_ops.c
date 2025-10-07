// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "wnd_rfiles_login.h"

#include "../../protocols/gdrive_limited.h"
#include "../../rfiles.h"

static device_user_code_t duc = {0};
static bool duc_initialized = false;

static gdrive_tokens_poll_state_t* gdrive_poll_state = NULL;

void gdrive_cancel_device_code_flow(void)
{
    duc_initialized = false;

    gdrive_fini();
}

int gdrive_begin_device_code_flow_async(gdrive_user_code_cb_t cb, void *user_ctx)
{
    SAP_STATUS status = SUCCESS;
    
    status = gdrive_init();
    if (FAIL(status))
    {
        SAP_ERR("[GDrive] Init error! status:%lu\n", status);
        return 1;
    }

    status = gdrive_login_query_device_user_code(&duc);
    if (FAIL(status))
    {
        SAP_ERR("[GDrive] Get device/user code error! status:%lu\n", status);
        return 1;
    }

    // Invoke the UI callback
    duc_initialized = true;
    cb(duc.user_code, duc.verification_uri, duc.verification_uri_with_code, user_ctx);



    // On success
    return 0;
}

int gdrive_begin_token_poll_async(gdrive_login_done_cb_t cb, void *user_ctx)
{
    SAP_STATUS status = SUCCESS;

    if (!duc_initialized)
    {
        SAP_ERR("[GDrive] Login failed!\n");
        return 1;
    }

    if(!gdrive_poll_state)
    {
        gdrive_poll_state = gdrive_login_tokens_poll_state_init(&duc);
    }

    status = gdrive_login_poll_tokens_once(&duc, gdrive_poll_state);
    if (FAIL(status))
    {
        if (status != EAGAIN)
        {
            SAP_ERR("[GDrive] Poll login tokens error! status:%lu\n", status);
        }
    }

    // Invoke the UI callback
    cb(status, user_ctx);

    // On success
    return 0;
}

void gdrive_cancel_token_poll(void)
{
    if (gdrive_poll_state)
    {
        gdrive_login_tokens_poll_state_fini(gdrive_poll_state);
        gdrive_poll_state = NULL;
    }
}