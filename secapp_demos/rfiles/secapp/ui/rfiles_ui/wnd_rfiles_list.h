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

#pragma once
#include <lvgl/lvgl.h>

#include <rfiles_header.h>
#include <app_sp_intfs.h>

/// @brief Init Window-RFiles-List with the given SecProcess configurations.
extern SAP_STATUS wnd_rfiles_list_init_cfgs(SAP_CONFIG* sap_cfg);

extern void wnd_rfiles_list_show(lv_obj_t *parent);