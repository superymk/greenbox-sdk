// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once
#include <lvgl/lvgl.h>

#include <rfiles_header.h>
#include <app_sp_intfs.h>

/// @brief Init Window-RFiles-List with the given SecProcess configurations.
extern SAP_STATUS wnd_rfiles_list_init_cfgs(SAP_CONFIG* sap_cfg);

extern void wnd_rfiles_list_show(lv_obj_t *parent);