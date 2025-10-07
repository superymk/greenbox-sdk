// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <app_sp_intfs.h>
#include "config_mgr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h> // for errno
// #include <pthread.h>
#include <stdlib.h>

static char *cfg_buffer = NULL;

static SL_STATUS _cfg_read(const char *filepath)
{
    SL_STATUS status = SL_SUCCESS;
    FILE *file;
    long fileSize;
    size_t read_size = 0;

    // Open the file in binary read mode
    file = fopen_normalized_path(filepath, "rb");

    if (file == NULL)
    {
        APP_ERR("SAPLoader: Open configuration error!\n");
        return SL_EINTERNAL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);

    // Allocate memory for the cfg_buffer
    cfg_buffer = (char *)malloc(fileSize + 1);

    if (cfg_buffer == NULL)
    {
        APP_ERR("SAPLoader: Memory allocation for configuration error!\n");
        return SL_EINTERNAL;
    }

    // Read the entire file into the cfg_buffer
    read_size = fread(cfg_buffer, 1, fileSize, file);
    if(read_size == fileSize)
    {
        status = SL_SUCCESS;
    }
    else
    {
        status = SL_EIO;
    }

    cfg_buffer[fileSize] = '\0'; // Null-terminate the string

    // Close the file
    fclose(file);

    return status;
}

static void _cfg_free(void)
{
    if (cfg_buffer)
    {
        free(cfg_buffer);
        cfg_buffer = NULL;
    }
}




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




/********* Private Functions of JSON Configurations*********/
static SL_STATUS _cfg_parse_logon(cJSON *root_json, SAP_CONFIG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !root_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(root_json, "logon");
    if (!info_top_json)
        return SL_EINVAL;

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "url");

        if (cJSON_IsString(node_json))
        {
            strncpy(out_cfg->logon_info.url, node_json->valuestring, SAP_LOGON_URL_LENGTH_MAX);
            out_cfg->logon_info.url[SAP_LOGON_URL_LENGTH_MAX] = '\0';
        }
        else
        {
            memset(out_cfg->logon_info.url, 0, SAP_LOGON_URL_LENGTH_MAX);
            out_cfg->logon_info.url[SAP_LOGON_URL_LENGTH_MAX] = '\0';
        }
    }

    return SL_SUCCESS;
}

#define RDESKTOP_DISPLAY_DEFAULT_COMPRESS_LEVEL (5)
#define RDESKTOP_DISPLAY_DEFAULT_QUALITY_LEVEL  (5)
static SL_STATUS _cfg_parse_display(cJSON *root_json, SAP_CONFIG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !root_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(root_json, "display");
    if (!info_top_json)
        return SL_EINVAL;

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "compress");

        if (cJSON_IsNumber(node_json))
            out_cfg->display_info.compress = (uint8_t)node_json->valuedouble;
        else
            out_cfg->display_info.compress = RDESKTOP_DISPLAY_DEFAULT_COMPRESS_LEVEL;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "quality");

        if (cJSON_IsNumber(node_json))
            out_cfg->display_info.quality = (uint8_t)node_json->valuedouble;
        else
            out_cfg->display_info.quality = RDESKTOP_DISPLAY_DEFAULT_QUALITY_LEVEL;
    }

    return SL_SUCCESS;
}

static SL_STATUS _cfg_parse(SAP_CONFIG *out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    cJSON *cfg_json = cJSON_Parse(cfg_buffer);

    if (!out_cfg || !cfg_json)
        return SL_EINVAL;

    status = _cfg_parse_logon(cfg_json, out_cfg);
    if (SL_FAIL(status))
    {
        APP_ERR("SOSLoader: Parse 'logon' configuration error! status:%lu\n", status);
        status = SL_EINTERNAL;
        goto err;
    }

    status = _cfg_parse_display(cfg_json, out_cfg);
    if (SL_FAIL(status))
    {
        APP_ERR("SOSLoader: Parse 'display' configuration error! status:%lu\n", status);
        status = SL_EINTERNAL;
        goto err;
    }

    // On success
    cJSON_Delete(cfg_json);
    return SL_SUCCESS;

err:
    cJSON_Delete(cfg_json);
    return status;
}




/********* Public Functions *********/
SL_STATUS loader_params_read(SAP_CONFIG* out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    char cfg_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};

    strncpy(cfg_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(cfg_file_path, SP_CONFIG_JSON_FILE_NAME);

    // Check: Parameters must be valid
    if (!out_cfg)
        return SL_EINVAL;

    // Read the config file
    status = _cfg_read(cfg_file_path);
    if (SL_FAIL(status))
    {
        APP_ERR("SAPLoader: Read config file error! status:%lu\n", status);
        return SL_EINVAL_CONFIGS;
    }

    // Read window configurations
    status = _cfg_read_window_configs(out_cfg);
    if (SL_FAIL(status))
    {
        APP_ERR("SAPLoader: Read windows config error! status:%d\n", status);
        return SL_EINVAL_CONFIGS;
    }

    status = _cfg_parse(out_cfg);
    if (SL_FAIL(status))
    {
        APP_ERR("SAPLoader: Parse config file error! status:%lu\n", status);
        return SL_EINVAL_CONFIGS;
    }

    _cfg_free();
    mfence();

    // On success
    return SL_SUCCESS;
}