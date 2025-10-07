// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <app_sp_intfs.h>
#include "config_mgr.h"

/********* Private Functions of Window Configurations *********/
static int _query_shared_params_window_scaling(float* out_window_scale)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint64_t data = 0;
    
    // Check: <out_window_scale> must be valid
    if(!out_window_scale)
        return 1;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DISP_SCALING, &data);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("Failed to get the parameter <NS_DISP_SCALING>! SecAgent status:%d\n", sa_status);
        return 1;
    }

    // On success
    *out_window_scale = (float)data;
    return 0;
}

static int _query_shared_params_window_width(uint32_t* out_window_width)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint64_t data = 0;
    
    // Check: <out_window_width> must be valid
    if(!out_window_width)
        return 1;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DISP_WIDTH, &data);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("Failed to get the parameter <NS_DISP_WIDTH>! SecAgent status:%d\n", sa_status);
        return 1;
    }

    // On success
    *out_window_width = (uint32_t)data;
    return 0;
}

static int _query_shared_params_window_height(uint32_t* out_window_height)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint64_t data = 0;
    
    // Check: <out_window_height> must be valid
    if(!out_window_height)
        return 1;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DISP_HEIGHT, &data);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("Failed to get the parameter <NS_DISP_HEIGHT>! SecAgent status:%d\n", sa_status);
        return 1;
    }

    // On success
    *out_window_height = (uint32_t)data;
    return 0;
}

static SL_STATUS _cfg_read_window_configs(SAP_CONFIG *out_cfg)
{
    int ret = 0;
    float window_scale = 0;
    uint32_t window_width = 0;
    uint32_t window_height = 0;

    ret = _query_shared_params_window_scaling(&window_scale);
    if(ret)
    {
        APP_ERR("Failed to get the parameter <window_scale>!\n");
        return SL_EINVAL_CONFIGS;
    }
    window_scale = window_scale / WINDOW_SCALE_FACTOR; // Divide <WINDOW_SCALE_FACTOR> is needed to be consistent with 
                                // this value's setting. Otherwise, the float to uint64_t conversion loses all decimals 
                                // of the float value.

    ret = _query_shared_params_window_width(&window_width);
    if(ret)
    {
        APP_ERR("Failed to get the parameter <window_width>!\n");
        return SL_EINVAL_CONFIGS;
    }

    ret = _query_shared_params_window_height(&window_height);
    if(ret)
    {
        APP_ERR("Failed to get the parameter <window_height>!\n");
        return SL_EINVAL_CONFIGS;
    }


    out_cfg->window_info.win_logical_height = window_height;
    out_cfg->window_info.win_logical_width = window_width;
    out_cfg->window_info.win_scale = window_scale;

    // On success
    return SL_SUCCESS;
}




/********* Public Functions *********/
SL_STATUS loader_params_read(SAP_CONFIG* out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    
    // Check: Parameters must be valid
    if (!out_cfg)
        return SL_EINVAL;

    // Read window configurations
    status = _cfg_read_window_configs(out_cfg);
    if (SL_FAIL(status))
    {
        APP_ERR("SAPLoader: Read windows config error! status:%d\n", status);
        return SL_EINVAL_CONFIGS;
    }


    // On success
    return SL_SUCCESS;
}