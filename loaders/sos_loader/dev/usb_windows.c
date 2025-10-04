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

#if defined(__WINDOWS__)

#include <dev/usb.h>

#include <appddk.h>
#include <sl_headers.h>
#include <libusb.h>

#include <string.h>
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <wchar.h>
#include <initguid.h>
#include <devpkey.h>
#include <stdio.h>

static SL_STATUS _do_win_reset_device(WCHAR* instanceID);

#define QUERY_AND_REMOVAL_TIMEOUT_MS        (5000)

#define USBDEV_STATUS_QUERY_TIMEOUT_MS (2000)  // Maximum time to wait for status query (2 seconds)
#define USBDEV_STATUS_POLL_INTERVAL_MS (500)  // Status polling interval (0.5 seconds)

/**
 * @brief Converts a wide-character (WCHAR) string to a multi-byte (char) string using UTF-8 encoding.
 *
 * @param wstr     Pointer to the null-terminated wide-character string to be converted.
 * @param str_len  Size of the output buffer in bytes.
 * @param out_str  Pointer to the buffer that will receive the converted multi-byte string.
 *
 * @return int     Returns 0 on successful conversion.
 *                Returns 1 if an error occurs (e.g., insufficient buffer size, conversion failure).
 */
__attribute__((unused))
static int ConvertWCharToChar(WCHAR* wstr, size_t str_len, char* out_str)
{
    if (wstr == NULL || out_str == NULL) {
        SL_ERR("Error: NULL pointer provided to ConvertWCharToChar.\n");
        return 1;
    }

    // Step 1: Determine the required buffer size for the converted string
    int requiredSize = WideCharToMultiByte(
        CP_UTF8,                // Code page: UTF-8
        0,                      // Conversion flags
        wstr,                   // Source wide-character string
        -1,                     // Indicates that the string is null-terminated
        NULL,                   // No output buffer to calculate required size
        0,                      // Size of the output buffer
        NULL,                   // Default character (unused)
        NULL                    // Whether a default character was used (unused)
    );

    if (requiredSize == 0) {
        DWORD error = GetLastError();
        SL_ERR("WideCharToMultiByte failed to calculate buffer size. Error Code: %lu\n", error);
        return 1;
    }

    // Step 2: Check if the provided buffer is large enough
    if ((size_t)requiredSize > str_len) {
        SL_ERR("Error: Provided buffer is too small. Required size: %d bytes, Provided size: %zu bytes.\n", requiredSize, str_len);
        return 1;
    }

    // Step 3: Perform the actual conversion
    int bytesWritten = WideCharToMultiByte(
        CP_UTF8,                // Code page: UTF-8
        0,                      // Conversion flags
        wstr,                   // Source wide-character string
        -1,                     // Indicates that the string is null-terminated
        out_str,                // Destination buffer for the multi-byte string
        (int)str_len,           // Size of the destination buffer
        NULL,                   // Default character (unused)
        NULL                    // Whether a default character was used (unused)
    );

    if (bytesWritten == 0) {
        DWORD error = GetLastError();
        SL_ERR("WideCharToMultiByte failed during conversion. Error Code: %lu\n", error);
        return 1;
    }

    // On success
    return 0;
}

static SL_STATUS _usb_hc_get_roothub_vendor_product_id(uint32_t usb_bus_id, uint16_t* out_vendor_id, uint16_t* out_product_id)
{
    libusb_device **devs;
    libusb_context *ctx = NULL; // libusb session context
    int r; // for results
    ssize_t dev_cnt; // number of usb devices found
    
    r = libusb_init(&ctx); // open session
    if (r < 0) 
        return SL_EINTERNAL;

    // get a list of all found USB devices
    dev_cnt = libusb_get_device_list(ctx, &devs);
    if (dev_cnt < 0) 
        return SL_EINTERNAL;

    // Get the vendor ID and product ID of the host controller
    {
        uint32_t i = 0;

        FOREACH_S(i, dev_cnt, dev_cnt, 0, 1)
        {
            libusb_device *dev = devs[i];
            struct libusb_device_descriptor dev_desc;
            uint8_t dev_bus_id = 0;
            uint8_t dev_usbaddr = 0;

            // Get the device name string
            r = libusb_get_device_descriptor(dev, &dev_desc);
            if (r < 0) 
            {
                // Error: USB device handle not received
                return SL_EINTERNAL;
            }

            // Get the vendor ID and product ID
            dev_bus_id = libusb_get_bus_number(dev);
            dev_usbaddr = libusb_get_device_address(dev);

            if((dev_bus_id == usb_bus_id) && (dev_usbaddr == USB_HC_ROOTHUB_USB_ADDRESS))
            {
                *out_vendor_id = dev_desc.idVendor;
                *out_product_id = dev_desc.idProduct;

                // On success
                return SL_SUCCESS;
            }
        }
    }

    // Not found the root hub for the bus
    return SL_EINVAL;
}

/// @brief Return the PCI address of the PCI device, given the device's vendor ID and product ID. The format is 
/// "XXXX:XX:XX.X" ("%04x:%02x:%02x.%u").
/// @param vendor_id 
/// @param product_id 
/// @param out_pci_addr_str 
/// @return 
static SL_STATUS _pci_find_device_bdf(uint16_t vendor_id, uint16_t product_id, char* out_pci_addr_str)
{
    SL_STATUS status = SL_SUCCESS;
    HDEVINFO deviceInfoSet;
    SP_DEVINFO_DATA deviceInfoData;
    LPTSTR id_str = NULL;
    LPTSTR bdf_str = NULL;
    DWORD i = 0;
    char search_str[USB_HC_WINDOWS_VENDOR_PRODUCT_STR_LENGTH_MAX];
    bool found_device = false;

    // Use snprintf to format and store the vendor and product IDs into search_str
    snprintf(search_str, sizeof(search_str), "VEN_%04X&DEV_%04X", vendor_id, product_id);

    // Find <search_str> in all PCI devices' info.
    //// Get a list of devices matching the PCI device setup class
    deviceInfoSet = SetupDiGetClassDevs(NULL, TEXT("PCI"), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return SL_EINVAL;
    }
    
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    while (SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData) && !found_device) 
    {
        DWORD DataT;
        DWORD id_str_size = 0;

        while (!SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &DataT, (PBYTE)id_str, id_str_size, &id_str_size)) 
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (id_str) 
                    LocalFree(id_str);
                id_str = (LPTSTR)LocalAlloc(LPTR, id_str_size);
            } else {
                SL_ERR("Find PCI device error: Error getting device property!\n");
                status = SL_EINTERNAL;
                goto out;
            }
        }

        if(strstr(id_str, search_str))
        {
            // Found the PCI device corresponding to <search_str>
            DWORD bdf_str_size = 0;
            uint8_t result_format_str_len = 13; // The length of "XXXX:XX:XX.X" with a trailing '\0'
            uint32_t bus = 0, device = 0, function = 0;

            found_device = true;
            
            // First call gets the <bdf_str> size.
            while (!SetupDiGetDeviceRegistryProperty(
                deviceInfoSet,
                &deviceInfoData,
                SPDRP_LOCATION_INFORMATION,
                &DataT,
                (PBYTE)bdf_str,
                bdf_str_size,
                &bdf_str_size))
            {
                
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
                {
                    // Change the <bdf_str> size.
                    if (bdf_str)
                        LocalFree(bdf_str);
                
                    // Allocate the <bdf_str>.
                    bdf_str = (LPTSTR)LocalAlloc(LPTR, bdf_str_size);
                } else 
                {
                    // Handle unknown error.
                    status = SL_EINTERNAL;
                    goto out;
                }
            }

            // Get the string of the device's PCI address.
            if (sscanf(bdf_str, "PCI bus %u, device %u, function %u", &bus, &device, &function) != 3)
            {
                SL_ERR("Find PCI device error: Unable to parse BDF string %s!\n", bdf_str);
                status = SL_EINVAL;
                goto out;
            }

            // Output the result to <out_pci_addr_str> with the format "XXXX:XX:XX.X" ("%04x:%02x:%02x.%u")
            snprintf(out_pci_addr_str, result_format_str_len, "%04x:%02x:%02x.%u", 0, bus, device, function);

            // printf("Device Location Information: %s %s\n", id_str, bdf_str);
        }

        i++;
    }
    
    // On success
    status = SL_SUCCESS;

out:
    if (id_str) 
        LocalFree(id_str);
    if (bdf_str)
        LocalFree(bdf_str);
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return status;
}

SL_STATUS usb_hc_get_pci_address(uint32_t usb_bus_id, char* out_pci_addr_str)
{
    SL_STATUS status = SL_SUCCESS;
    uint16_t vendor_id, product_id;

    status = _usb_hc_get_roothub_vendor_product_id(usb_bus_id, &vendor_id, &product_id);
    if(SL_FAIL(status))
        return status;

    status = _pci_find_device_bdf(vendor_id, product_id, out_pci_addr_str);
    if(SL_FAIL(status))
        return status;

    // On success
    return SL_SUCCESS;
}

SL_STATUS usb_hc_get_vendor_product_string(uint32_t usb_bus_id, char* out_usb_hc_vendor_product_str)
{
    SL_STATUS status = SL_SUCCESS;
    uint16_t vendor_id, product_id;

    status = _usb_hc_get_roothub_vendor_product_id(usb_bus_id, &vendor_id, &product_id);
    if(SL_FAIL(status))
        return status;

    snprintf(out_usb_hc_vendor_product_str, USB_HC_WINDOWS_VENDOR_PRODUCT_STR_LENGTH_MAX, "VEN_%04X&DEV_%04X", vendor_id, product_id);

    // On success
    return SL_SUCCESS;
}

// SL_STATUS usb_hc_reset(const char* pci_addr_str)
// {
//     SL_STATUS status = SL_SUCCESS;
//     char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
//     const char* exe_filename = "resetusb.bat";
        // const union sha_digest _file_win_resetusb_bat_sha256 = {
        //     .sha2_256_digest = {
        //         0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        //         0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        //         0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        //         0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
        //     }
        // };
//     int ret = 0;

//     if(!pci_addr_str)
//         return SL_EINVAL;
    
//     // Reset the USB host controller 
//     strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
//     strcat(exe_filepath, exe_filename);
//     ret = os_execute_program_wait_exit(false, exe_filepath, &_file_win_resetusb_bat_sha256, exe_filename, pci_addr_str, NULL);
//     if(ret)
//     {
//         SL_ERR("Failed to reset the USB host controller %s!\n", pci_addr_str);
//         status = SL_EINTERNAL;
//     }
//     else
//     {
//         SL_INFO("USB host controller is reset\n");
//         status = SL_SUCCESS;
//     }

//     // Return status
//     return status;
// }

static bool _win_enum_pci_devices(DEVINST devInst, const WCHAR* searchPattern, WCHAR* instanceID, ULONG instanceIDLength) 
{
    DEVINST childDevInst;
    WCHAR deviceID[MAX_DEVICE_ID_LEN];
    CONFIGRET cr = CR_SUCCESS;

    // Enumerate this device
    cr = CM_Get_Device_IDW(devInst, deviceID, MAX_DEVICE_ID_LEN, 0);
    if (cr == CR_SUCCESS && wcsstr(deviceID, searchPattern) != NULL) {
        if (wcslen(deviceID) < instanceIDLength) {
            wcscpy(instanceID, deviceID);
            return true; // Found the device
        }
    }

    // Enumerate child devices
    cr = CM_Get_Child(&childDevInst, devInst, 0);
    while (cr == CR_SUCCESS) {
        if (_win_enum_pci_devices(childDevInst, searchPattern, instanceID, instanceIDLength)) {
            return true; // Found the device in a child
        }
        cr = CM_Get_Sibling(&childDevInst, childDevInst, 0);
    }

    return false; // Device not found in this branch
}

static bool _usb_hc_get_instanceID(const char* searchPattern, WCHAR* instanceID, ULONG instanceIDLength) 
{
    WCHAR searchPatternW[MAX_DEVICE_ID_LEN];
    DEVINST rootDevInst;
    CONFIGRET cr = CR_SUCCESS;
    bool found = false;

    // Convert searchPattern from char to WCHAR
    if (MultiByteToWideChar(CP_ACP, 0, searchPattern, -1, searchPatternW, MAX_DEVICE_ID_LEN) == 0) {
        SL_ERR("Error converting searchPattern to wide character string.\n");
        return false;
    }

    // Locate the root of the device tree
    cr = CM_Locate_DevNodeW(&rootDevInst, NULL, CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        SL_ERR("CM_Locate_DevNodeW failed with error: %u\n", cr);
        return false;
    }

    // Start enumeration from the root device
    found = _win_enum_pci_devices(rootDevInst, searchPatternW, instanceID, instanceIDLength);
    return found;
}

static SL_STATUS _do_win_disable_device(WCHAR* instanceID)
{
    DEVINST usb_hc;
    CONFIGRET cr;
    WCHAR veto_wname[MAX_PATH] = {0};  // Buffer to store veto name (wide string)
    PNP_VETO_TYPE veto_type = PNP_VetoTypeUnknown;  // To store the type of veto
    ULONG cm_status = 0, cm_problem = 0;
    bool is_usb_hc_disabled = false;

    // Get device instance handle from its instance ID
    cr = CM_Locate_DevNodeW(&usb_hc, instanceID, CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        SL_ERR("Disable USB Host Controller: Failed to get USB HC handle! error:0x%08X\n", cr);
        return SL_EINTERNAL;
    }

    // Check: USB host controller exists.
    cr = CM_Get_DevNode_Status(&cm_status, &cm_problem, usb_hc, 0);
    if (cr != CR_SUCCESS) 
    {
        SL_ERR("Disable USB Host Controller: Failed to get the USB HC! error:0x%08X\n", cr);
        return SL_EINTERNAL;
    }

    // Query and Remove the SubTree
    cr = CM_Query_And_Remove_SubTreeW(usb_hc, &veto_type, veto_wname, MAX_PATH, CM_REMOVE_UI_NOT_OK | CM_REMOVE_NO_RESTART);
    if(cr == CR_SUCCESS)
    {
        // Check if the device is disabled
        cr = CM_Get_DevNode_Status(&cm_status, &cm_problem, usb_hc, 0);
        if((cr == CR_SUCCESS) && (!(cm_status & DN_STARTED)))
        {
            is_usb_hc_disabled = true;
        }
    }
    else
    {
        SL_ERR("Disable USB Host Controller: Failed to disable USB devices by removing device sub-tree! error:0x%08X\n", cr);
        if(cr == CR_REMOVE_VETOED)
        {
            SL_ERR("Disable USB Host Controller: CM_Query_And_Remove_SubTreeW vetoed by: Type=%d, Name='%S'\n", veto_type, veto_wname);
        }
    }

    // If the device has not been disabled successfully yet, try eject it.
    if(!is_usb_hc_disabled)
    {
        cr = CM_Request_Device_EjectW(usb_hc, &veto_type, veto_wname, MAX_PATH, 0);
        if (cr != CR_SUCCESS) 
        {
            SL_ERR("Disable USB Host Controller: Failed to disable the USB HC! error:0x%08X\n", cr);
            if(cr == CR_REMOVE_VETOED)
            {
                SL_ERR("Disable USB Host Controller: CM_Request_Device_EjectW vetoed by: Type=%d, Name='%S'\n", veto_type, veto_wname);
            }
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}


static SL_STATUS _win_disable_device(WCHAR* instanceID)
{
    SL_STATUS status = SL_SUCCESS;
    uint32_t i = 0;

    // Try to disable the device for 3 times.
    for(i = 0; i < 3; i++)
    {
        status = _do_win_disable_device(instanceID);
        if(SL_PASS(status))
        {
            SL_INFO("Disable USB Devices (Try %u) ... Success\n", i);
            break;
        }
        else
        {
            SL_INFO("Disable USB Devices (Try %u) ... Failed\n", i);
            continue;
        }

        msleep(1000);
    }

    return status;
}

#define RESET_TIMEOUT_MS 1000  // Maximum time to wait for reset (1 seconds)
#define RESET_POLL_INTERVAL_MS 500  // Polling interval (0.5 seconds)

static SL_STATUS _do_win_reset_device(WCHAR* instanceID)
{
    DEVINST usb_hc;
    CONFIGRET cr;
    WCHAR veto_wname[MAX_PATH] = {0};  // Buffer to store veto name (wide string)
    PNP_VETO_TYPE veto_type = PNP_VetoTypeUnknown;  // To store the type of veto

    // Get device instance handle from its instance ID
    cr = CM_Locate_DevNodeW(&usb_hc, instanceID, CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        SL_ERR("CM_Locate_DevNodeW failed! error:0x%08X\n", cr);
        return SL_EINTERNAL;
    }

    // Query and Remove the SubTree
    cr = CM_Query_And_Remove_SubTreeW(usb_hc, &veto_type, veto_wname, 0, CM_REMOVE_UI_NOT_OK);
    if (cr != CR_SUCCESS)
    {
        SL_ERR("CM_Query_And_Remove_SubTreeW failed! error:0x%08X\n", cr);
        if(cr == CR_REMOVE_VETOED)
        {
            SL_ERR("Removal vetoed by: Type=%d, Name='%S'\n", veto_type, veto_wname);
        }
        return SL_EINTERNAL;
    }

    // At this point, the device subtree starting at the specified device instance
    // has been removed. Now, you might want to rescan to reinstall the device.
    cr = CM_Reenumerate_DevNode(usb_hc, 0);
    if (cr != CR_SUCCESS) {
        SL_ERR("CM_Reenumerate_DevNode failed with error:0x%08X\n", cr);
        return SL_EINTERNAL;
    }

    // Sleep for a short period to allow re-enumeration to start
    msleep(500);  // 0.5 second

    // Initialize variables for polling
    {
        ULONG status, problem;
        volatile DWORD elapsed = 0;
        BOOL isReset = FALSE;

        while (elapsed < USBDEV_STATUS_QUERY_TIMEOUT_MS)
        {
            // Get the current status of the device node
            cr = CM_Get_DevNode_Status(&status, &problem, usb_hc, 0);
            if (cr != CR_SUCCESS) 
            {
                SL_ERR("CM_Get_DevNode_Status failed with error: %lu\n", cr);
                return SL_EINTERNAL;
            }

            // Check if the device is active (not disabled or removed)
            if((status & DN_STARTED) && (status & DN_DRIVER_LOADED)) 
            {
                isReset = TRUE;
                break;
            }

            // Wait for the next polling interval
            msleep(USBDEV_STATUS_POLL_INTERVAL_MS);
            elapsed += USBDEV_STATUS_POLL_INTERVAL_MS;
        }

        if (isReset) {
            return SL_SUCCESS;
        } else {
            SL_ERR("Device reset timed out or failed to reach the desired state.\n");
            return SL_EINTERNAL;
        }
    }

    // Should not hit.
    return SL_EINTERNAL;
}

static SL_STATUS _win_reset_device(WCHAR* instanceID)
{
    SL_STATUS status = SL_SUCCESS;
    uint32_t i = 0;

    // Try to disable the device for three times.
    for(i = 0; i < 3; i++)
    {
        status = _do_win_reset_device(instanceID);
        if(SL_PASS(status))
        {
            SL_INFO("Reset Device (Try %u) ... Success\n", i);
            break;
        }
        else
        {
            SL_INFO("Reset Device (Try %u) ... Failed\n", i);
            continue;
        }
    }

    return status;
}

SL_STATUS usb_hc_reset_by_vendor_product_str(char* usb_hc_vendor_product_str)
{
    SL_STATUS status = SL_SUCCESS;
    WCHAR instanceID[MAX_DEVICE_ID_LEN] = {0};

    // Get device ID
    if (!_usb_hc_get_instanceID(usb_hc_vendor_product_str, instanceID, MAX_DEVICE_ID_LEN))
    {
        SL_ERR("Reset USB HC Error: Not found or an error occurred for %s\n", usb_hc_vendor_product_str);
        return SL_EINTERNAL;
    }

    // Reset the device <instanceID>
    status = _win_reset_device(instanceID);
    if(SL_FAIL(status))
        return status;

    // On success
    return SL_SUCCESS;
}

SL_STATUS usb_hc_disable_by_vendor_product_str(char* usb_hc_vendor_product_str)
{
    SL_STATUS status = SL_SUCCESS;
    WCHAR instanceID[MAX_DEVICE_ID_LEN] = {0};

    // Get device ID
    if (!_usb_hc_get_instanceID(usb_hc_vendor_product_str, instanceID, MAX_DEVICE_ID_LEN))
    {
        SL_ERR("Disable Device Error: Not found or an error occurred for %s\n", usb_hc_vendor_product_str);
        return SL_EINTERNAL;
    }
    SL_DEBUG("Disabling USB Host Controller (Device ID): %S\n", instanceID);

    // Disable the device <instanceID>
    status = _win_disable_device(instanceID);
    if(SL_FAIL(status))
    {
        // Reset USB HC and devices to restart any disabled USB devices.
        (void)_do_win_reset_device(instanceID);
        return status;
    }

    // On success
    return SL_SUCCESS;
}

#endif // __WINDOWS__