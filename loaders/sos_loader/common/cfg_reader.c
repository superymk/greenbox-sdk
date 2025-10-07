// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h>
#include <string.h>
#include <errno.h> // for errno
// #include <pthread.h>
#include <stdlib.h>
#include <sl_headers.h>
#include <dev/usb.h>

#define USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN  (5)    // length is 4 (e.g., "1d6b"). +1 for '\0'

static char *cfg_buffer = NULL;

static SL_STATUS _cfg_read(const char *filepath)
{
    SL_STATUS status = SL_SUCCESS;
    FILE *file = NULL;
    long fileSize;
    size_t read_size = 0;

    // Open the file in binary read mode
    file = fopen_normalized_path(filepath, "rb");

    if (file == NULL)
    {
        SL_ERR("SOSLoader: Open configuration error!\n");
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
        SL_ERR("SOSLoader: Memory allocation for configuration error!\n");
        status = SL_EINTERNAL;
        goto out;
    }

    // Read the entire file into the cfg_buffer
    read_size = fread(cfg_buffer, 1, fileSize, file);
    if(read_size != fileSize)
    {
        SL_ERR("SOSLoader: Read configuration error!\n");
        status = SL_EINTERNAL;
        goto out;
    }

    cfg_buffer[fileSize] = '\0'; // Null-terminate the string

    // On success
    status = SL_SUCCESS;

out:
    // Close the file
    if(file)
        fclose(file);
    return status;
}

static SL_STATUS _cfg_parse_display(cJSON *root_json, SL_CFG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !root_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(root_json, "display");
    if (!info_top_json)
        return SL_EINVAL;

    node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "type");

    if (cJSON_IsString(node_json))
    {
        strncpy(out_cfg->disp_info.type, node_json->valuestring, TYPE_NAME_MAX_LEN);
        out_cfg->disp_info.type[TYPE_NAME_MAX_LEN] = '\0';
    }
    else
    {
        memset(out_cfg->disp_info.type, 0, TYPE_NAME_MAX_LEN);
        out_cfg->disp_info.type[TYPE_NAME_MAX_LEN] = '\0';
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "disp_width");

        if (cJSON_IsNumber(node_json))
            out_cfg->disp_info.disp_width = (uint8_t)node_json->valuedouble;
        else
            out_cfg->disp_info.disp_width = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "disp_height");

        if (cJSON_IsNumber(node_json))
            out_cfg->disp_info.disp_height = (uint8_t)node_json->valuedouble;
        else
            out_cfg->disp_info.disp_height = 0;
    }

    return SL_SUCCESS;
}

__attribute__((unused)) static SL_STATUS _cfg_parse_ps2(cJSON *root_json, SL_CFG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !root_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(root_json, "ps2");
    if (!info_top_json)
        return SL_EINVAL;

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "ps2_enable");

        if (cJSON_IsNumber(node_json))
            out_cfg->ps2_info.ps2_enable = (((uint8_t)node_json->valuedouble) == 1) ? true : false;
        else
            out_cfg->ps2_info.ps2_enable = false;
    }

    return SL_SUCCESS;
}

__attribute__((unused)) static SL_STATUS _cfg_parse_usb2(cJSON *cfg_json, SL_CFG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !cfg_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(cfg_json, "usb2");
    if (!info_top_json)
        return SL_EINVAL;

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "usb2_enable");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.usb2_enable = (((uint8_t)node_json->valuedouble) == 1) ? true : false;
        else
            out_cfg->usb2_info.usb2_enable = false;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "kbd_dev_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.kbd_dev_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.kbd_dev_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "kbd_hub_port");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.kbd_hub_port = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.kbd_hub_port = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "mouse_dev_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.mouse_dev_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.mouse_dev_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "mouse_hub_port");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.mouse_hub_port = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.mouse_hub_port = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "restore_dev_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.restore_dev_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.restore_dev_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "restore_hub_port");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb2_info.restore_hub_port = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb2_info.restore_hub_port = 0;
    }

    return SL_SUCCESS;
}

static SL_STATUS _cfg_parse_usb3(cJSON *cfg_json, SL_CFG *out_cfg)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    if (!out_cfg || !cfg_json)
        return SL_EINVAL;

    info_top_json = cJSON_GetObjectItemCaseSensitive(cfg_json, "usb3");
    if (!info_top_json)
        return SL_EINVAL;

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "usb3_enable");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.usb3_enable = (((uint8_t)node_json->valuedouble) == 1) ? true : false;
        else
            out_cfg->usb3_info.usb3_enable = false;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev1_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev1_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_hub_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev1_hub_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev1_hub_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_hub_port");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev1_hub_port = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev1_hub_port = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_bus_id");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev1_bus_id = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev1_bus_id = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_vendor_id");

        if (cJSON_IsString(node_json))
        {
            SL_STATUS status = SL_SUCCESS;
            char str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN] = {0};

            strncpy(str, node_json->valuestring, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN);
            str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1] = '\0';

            status = str_to_byte_array(str, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1, (uint8_t*)&out_cfg->usb3_info.hid_dev1_vendor_id);
            if(SL_FAIL(status))
                SL_ERR("SOSLoader: Parse keyboard's vendor ID error!\n");
        }
        else
            out_cfg->usb3_info.hid_dev1_vendor_id = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev1_product_id");

        if (cJSON_IsString(node_json))
        {
            SL_STATUS status = SL_SUCCESS;
            char str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN] = {0};

            strncpy(str, node_json->valuestring, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN);
            str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1] = '\0';

            status = str_to_byte_array(str, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1, (uint8_t*)&out_cfg->usb3_info.hid_dev1_product_id);
            if(SL_FAIL(status))
                SL_ERR("SOSLoader: Parse keyboard's product ID error!\n");
        }
        else
            out_cfg->usb3_info.hid_dev1_product_id = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev2_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev2_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_hub_addr");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev2_hub_addr = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev2_hub_addr = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_hub_port");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev2_hub_port = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev2_hub_port = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_bus_id");

        if (cJSON_IsNumber(node_json))
            out_cfg->usb3_info.hid_dev2_bus_id = (uint8_t)node_json->valuedouble;
        else
            out_cfg->usb3_info.hid_dev2_bus_id = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_vendor_id");

        if (cJSON_IsString(node_json))
        {
            SL_STATUS status = SL_SUCCESS;
            char str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN] = {0};

            strncpy(str, node_json->valuestring, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN);
            str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1] = '\0';

            status = str_to_byte_array(str, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1, (uint8_t*)&out_cfg->usb3_info.hid_dev2_vendor_id);
            if(SL_FAIL(status))
                SL_ERR("SOSLoader: Parse mouse's vendor ID error!\n");
        }
        else
            out_cfg->usb3_info.hid_dev2_vendor_id = 0;
    }

    {
        node_json = cJSON_GetObjectItemCaseSensitive(info_top_json, "hid_dev2_product_id");

        if (cJSON_IsString(node_json))
        {
            SL_STATUS status = SL_SUCCESS;
            char str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN] = {0};

            strncpy(str, node_json->valuestring, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN);
            str[USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1] = '\0';

            status = str_to_byte_array(str, USB_DEVICE_VENDOR_PRODUCT_ID_STRING_LEN - 1, (uint8_t*)&out_cfg->usb3_info.hid_dev2_product_id);
            if(SL_FAIL(status))
                SL_ERR("SOSLoader: Parse mouse's product ID error!\n");
        }
        else
            out_cfg->usb3_info.hid_dev2_product_id = 0;
    }

    return SL_SUCCESS;
}

static void _cfg_calc_components_to_boot(SL_CFG *out_cfg)
{
    uint32_t num_components = 2; // SecOS and SECSRV-INIT always boots

    if(out_cfg->need_iosm_pcie)
        num_components++;
    if(out_cfg->need_iosm_ps2)
        num_components++;
    if(out_cfg->need_iosm_usb2)
        num_components++;
    if(out_cfg->need_iosm_usb3)
        num_components++;
    if(out_cfg->need_iosm_fb)
        num_components++;
    
    out_cfg->num_components_to_boot = num_components;
}

static SL_STATUS _cfg_parse(SL_CFG *out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    cJSON *cfg_json = cJSON_Parse(cfg_buffer);

    if (!out_cfg || !cfg_json)
        return SL_EINVAL;

    status = _cfg_parse_display(cfg_json, out_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("SOSLoader: Parse 'display' configuration error! status:%lu\n", status);
        status = SL_EINTERNAL;
        goto err;
    }

    // status = _cfg_parse_ps2(cfg_json, out_cfg);
    // if (SL_FAIL(status))
    // {
    //     SL_ERR("SOSLoader: Parse 'ps2' configuration error! status:%lu\n", status);
    //     status = SL_EINTERNAL;
    //     goto err;
    // }

    // status = _cfg_parse_usb2(cfg_json, out_cfg);
    // if (SL_FAIL(status))
    // {
    //     SL_ERR("SOSLoader: Parse 'usb2' configuration error! status:%lu\n", status);
    //     status = SL_EINTERNAL;
    //     goto err;
    // }

    status = _cfg_parse_usb3(cfg_json, out_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("SOSLoader: Parse 'usb3' configuration error! status:%lu\n", status);
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

static SL_STATUS _cfg_deduct(SL_CFG *out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    
    // Check: Parameters must be valid
    if(!out_cfg)
        return SL_EINVAL;

    // We always need <out_cfg->need_iosm_pcie> because PCIe is the barebone bus
    out_cfg->need_iosm_pcie = true;

    // Decide <out_cfg->need_iosm_ps2>
    out_cfg->need_iosm_ps2 = false;
    if(out_cfg->ps2_info.ps2_enable)
        out_cfg->need_iosm_ps2 = true;

    // Decide <out_cfg->need_iosm_usb2>
    out_cfg->need_iosm_usb2 = false;
    if(out_cfg->usb2_info.usb2_enable)
        out_cfg->need_iosm_usb2 = true;

    // Decide <out_cfg->need_iosm_usb3>
    out_cfg->need_iosm_usb3 = false;
    if(out_cfg->usb3_info.usb3_enable)
        out_cfg->need_iosm_usb3 = true;

    // Config additional parameters for IOSM-USB3
    if(out_cfg->need_iosm_usb3)
    {
        uint16_t usb_bus_id = out_cfg->usb3_info.hid_dev1_bus_id;

        status = usb_hc_get_pci_address((uint32_t)usb_bus_id, out_cfg->usb3_info.xhci_hc_pci_addr_str);
        if(SL_FAIL(status))
        {
            SL_ERR("SOSLoader: Failed to get the PCI address of the USB host controller (bus:%u)! status:%lu\n", usb_bus_id, status);
            status = SL_EINTERNAL;
            goto out;
        }
        SL_INFO("PCI address of the USB host controller (bus:%u): %s\n", usb_bus_id, out_cfg->usb3_info.xhci_hc_pci_addr_str);

#if defined(__WINDOWS__)
        status = usb_hc_get_vendor_product_string((uint32_t)usb_bus_id, out_cfg->usb3_info.xhci_hc_vendor_product_str);
        if(SL_FAIL(status))
        {
            SL_ERR("SOSLoader: Failed to get the vendor and product string of the USB host controller (bus:%u)! status:%lu\n", usb_bus_id, status);
            status = SL_EINTERNAL;
            goto out;
        }
        SL_INFO("Vendor and Product string of the USB host controller (bus:%u): %s\n", usb_bus_id, out_cfg->usb3_info.xhci_hc_vendor_product_str);
#endif // __WINDOWS__
    }
        
    // Decide <out_cfg->need_iosm_fb>
    out_cfg->need_iosm_fb = false;
    if(!strcmp(out_cfg->disp_info.type, "IOSM-FB"))
    {
        out_cfg->need_iosm_fb = true;
    }

    // On success
    status = SL_SUCCESS;

out:
    // Calculate the number of components to boot by SOSLoader
    _cfg_calc_components_to_boot(out_cfg);

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

/********* Public Functions *********/
SL_STATUS sl_cfg_read(SL_CFG *out_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    char cfg_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};

    strncpy(cfg_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(cfg_file_path, "sos_conf.json");

    // Check: Parameters must be valid
    if (!out_cfg)
        return SL_EINVAL;

    // Read the config file
    status = _cfg_read(cfg_file_path);
    if (SL_FAIL(status))
    {
        SL_ERR("SOSLoader: Read config file error! status:%lu\n", status);
        return SL_EINVAL_CONFIGS;
    }

    status = _cfg_parse(out_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("SOSLoader: Parse config file error! status:%lu\n", status);
        return SL_EINVAL_CONFIGS;
    }

    status = _cfg_deduct(out_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("SOSLoader: Deduct config file error! status:%lu\n", status);
        return SL_EINVAL_CONFIGS;
    }

    _cfg_free();
    mfence();

    return SL_SUCCESS;
}