// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once
#include <lvgl/lvgl.h>

#include <rfiles_header.h>
#include <app_sp_intfs.h>

typedef void (*gdrive_user_code_cb_t)(const char *user_code,
                                      const char *verification_url,
                                      const char *qr_uri,
                                      void *user_ctx);

/* [ADD] Token polling (runs the blocking loop off the UI thread/process).
 * Contract: returns 0 if successfully started; when done, cb(status) is invoked on a non-UI thread.
 * The backend MUST use lv_async_call(...) to bounce results back to UI thread if it needs to touch UI.
 */
typedef void (*gdrive_login_done_cb_t)(SAP_STATUS sap_status, void *user_ctx);

/* Close notification so the main window can restore its empty state. */
typedef void (*wnd_login_closed_cb_t)(void *user_ctx);

/* Create and present the login window inside `parent`. Returns the root object. */
lv_obj_t *wnd_rfiles_login_create(lv_obj_t *parent,
                                  wnd_login_closed_cb_t on_closed,
                                  void *user_ctx);

/* Programmatically close/destroy the login window. Safe to call multiple times. */
void wnd_rfiles_login_close(lv_obj_t *login_root);

/* (Optional) Helper to query if an object is a login root. */
bool wnd_rfiles_login_is_root(lv_obj_t *obj);

extern SAP_STATUS wnd_rfiles_login_init_cfgs(SAP_CONFIG *in_sap_cfg);
