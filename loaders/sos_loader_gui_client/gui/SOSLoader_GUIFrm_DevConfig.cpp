#include "SOSLoader_GUIFrm_DevConfig.h"

extern "C"
{
    #include "util_file.h"
}
#include "sl_gui_utils.h"
#include "sl_gui_utils_cpp.h"
#include "SOSLoader_GUIFrm_Main.h"

extern "C"
{
    #include <loader_sa_intfs.h>
    #include <app_var.h>
}

SOSLoader_GUIFrm_DevConfig::SOSLoader_GUIFrm_DevConfig(wxWindow *parent)
    : Frm_DevConfig(parent)
{
    _view_selectors_init();

    // Init the device config list on the left.
    uint32_t col_ID_width = 200;
    this->m_dev_config_type_list->ClearAll();
    this->m_dev_config_type_list->InsertColumn(0, _("Device"), wxLIST_FORMAT_LEFT, col_ID_width);
    this->m_dev_config_type_list->InsertItem(0, _("Keyboard and Mouse"));
    this->m_dev_config_type_list->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void SOSLoader_GUIFrm_DevConfig::btn_reset_users_profilesOnButtonClick(wxCommandEvent &event)
{
    // Initialize the message dialog with custom buttons
    wxMessageDialog dialog(
        this, 
        "WILL RESET ALL GreenBox users secrets. The operation cannot be undone. Proceed?", // Message
        "Confirmation",             // Title
        wxYES_NO | wxICON_QUESTION   // Style: Yes and No buttons with a question icon
    );

    // Customize the labels of the Yes and No buttons
    dialog.SetYesNoLabels("Continue", "Cancel");

    // Show the dialog modally and capture the user's response
    int response = dialog.ShowModal();

    // Check the response and return true if "Continue" was clicked
    if (response == wxID_YES)
    {

#if defined(__WINDOWS__)
    #define TPMTOOLS_RELATIVE_FILEPATH L".\\tools\\tpmtools\\gb_tpm.exe"

    // Delete SecBase's sealed secrets
    ShellExecuteAndWaitW(L"runas",
                 TPMTOOLS_RELATIVE_FILEPATH,
                 L"--secbase-seal-delete",
                 SL_GUI_UTILS::ConvertCharToLPCWSTR(FileUtils::getCurrentExecutablePath()).c_str(),
                 SW_HIDE);

    // Invoke SecBase to re-create physical TPM's keys.
    ShellExecuteAndWaitW(L"runas",
                 TPMTOOLS_RELATIVE_FILEPATH,
                 L"--secbase-create-keys",
                 SL_GUI_UTILS::ConvertCharToLPCWSTR(FileUtils::getCurrentExecutablePath()).c_str(),
                 SW_HIDE);

#endif // __WINDOWS__

        // Prompt user to reboot the computer to erase secrets in SecBase.
        {
            wxString info;
            info.Printf("Reboot computer to finish the operation.\n");
            wxMessageBox(info);
        }
    }
}

void SOSLoader_GUIFrm_DevConfig::btn_config_okOnButtonClick(wxCommandEvent &event)
{
    SL_STATUS status = SL_SUCCESS;
    SL_CFG dev_configs;
    int usb_kbd_selection = this->m_hid_kbd_selector->GetSelection();
    int usb_mouse_selection = this->m_hid_mouse_selector->GetSelection();
    int disp_resolution_selection = this->m_disp_resolution_selector->GetSelection();

    // Check: Device selections must be valid
    if ((usb_kbd_selection == wxNOT_FOUND) || (usb_mouse_selection == wxNOT_FOUND))
    {
        wxString err;
        err.Printf("USB Keyboard or mouse selection is invalid!\n");
        wxMessageBox(err);

        return;
    }

    if (disp_resolution_selection == wxNOT_FOUND)
    {
        wxString err;
        err.Printf("Window resolution is invalid!\n");
        wxMessageBox(err);

        return;
    }

    // Check and generate device configurations
    memset(&dev_configs, 0, sizeof(SL_CFG));

    //// Display
    strcpy(dev_configs.disp_info.type, "Insecure");
    dev_configs.disp_info.disp_width = avail_resolutions[disp_resolution_selection].width;
    dev_configs.disp_info.disp_height = avail_resolutions[disp_resolution_selection].height;

    //// USB3
    {
        USBPDEV_ADDR kbd_addr = _hid_usbkbd_device_list.devices[usb_kbd_selection].usb_addr;
        USBPDEV_ADDR mouse_addr = _hid_usbmouse_device_list.devices[usb_mouse_selection].usb_addr;

        dev_configs.usb3_info.usb3_enable = true;
        dev_configs.usb3_info.hid_dev1_addr = kbd_addr.dev_usbaddr;
        dev_configs.usb3_info.hid_dev1_hub_addr = kbd_addr.hub_usbaddr;
        dev_configs.usb3_info.hid_dev1_hub_port = kbd_addr.hub_port;
        dev_configs.usb3_info.hid_dev1_bus_id = kbd_addr.bus_id;

        //// Check: All USB devices must be under the same bus
        if (mouse_addr.dev_usbaddr)
        {
            if (mouse_addr.bus_id != kbd_addr.bus_id)
            {
                wxString err;
                err.Printf("USB: Keyboard and mouse must be under the same bus!\n");
                wxMessageBox(err);

                return;
            }
        }

        //// Check: All USB devices must be directly connected to the USB host controller
        if (kbd_addr.dev_usbaddr && (kbd_addr.hub_usbaddr != USB_HC_ROOTHUB_USB_ADDRESS))
        {
            wxString err;
            err.Printf("USB: Keyboard must be directly connected to the USB host controller!\n");
            wxMessageBox(err);

            return;
        }

        if (mouse_addr.dev_usbaddr && (mouse_addr.hub_usbaddr != USB_HC_ROOTHUB_USB_ADDRESS))
        {
            wxString err;
            err.Printf("USB: Mouse must be directly connected to the USB host controller!\n");
            wxMessageBox(err);

            return;
        }

        //// Generate device configurations
        if (!usbpdev_addr_equal(&mouse_addr, &kbd_addr))
        {
            dev_configs.usb3_info.hid_dev2_addr = mouse_addr.dev_usbaddr;
            dev_configs.usb3_info.hid_dev2_hub_addr = mouse_addr.hub_usbaddr;
            dev_configs.usb3_info.hid_dev2_hub_port = mouse_addr.hub_port;
            dev_configs.usb3_info.hid_dev2_bus_id = mouse_addr.bus_id;
        }
        else
        {
            dev_configs.usb3_info.hid_dev2_addr = 0;
            dev_configs.usb3_info.hid_dev2_hub_addr = 0;
            dev_configs.usb3_info.hid_dev2_hub_port = 0;
            dev_configs.usb3_info.hid_dev2_bus_id = 0;
        }
    }

    // Save the device configurations
    status = _model_save_configs(&dev_configs);
    if (SL_FAIL(status))
    {
        wxString err;
        err.Printf("Save device configurations error! status:%lu\n", status);
        wxMessageBox(err);

        return;
    }

    // Close the window
    this->Close();

    // If the SecOS config file exists, then enable the RUN button. Otherwise, disable both RUN and STOP button.
    if (FileUtils::isFileExist(SECOS_CONFIG_FILE_RELATIVE_FILENAMEPATH))
    {
        this->parent_window->ui_enable_run_button_on_secos_conf_exist();
    }
    else
    {
        this->parent_window->ui_disable_run_stop_buttons_on_secos_conf_not_exist();
    }
}

void SOSLoader_GUIFrm_DevConfig::btn_config_cancelOnButtonClick(wxCommandEvent &event)
{
    // Close the window
    this->Close();
}

/********* Private Functions *********/
#include "util_file.h"

extern "C"
{
#include <loader_sa_intfs.h>
// #include <sl_headers.h>
#include <app_var.h>
}
void SOSLoader_GUIFrm_DevConfig::_view_selectors_init(void)
{
    _view_hid_selectors_init();

    _view_disp_selectors_init();
}

void SOSLoader_GUIFrm_DevConfig::_view_hid_kbd_selector_update(void)
{
}

SL_STATUS SOSLoader_GUIFrm_DevConfig::_model_save_configs(SL_CFG *dev_configs)
{
    SL_STATUS status = SL_SUCCESS;
    cJSON *root_json = NULL;
    char *root_json_str = NULL;
    

    // Check: Parameters must be valid
    if (!dev_configs)
        return SL_EINVAL;

    root_json = cJSON_CreateObject();
    if (!root_json)
        goto err;

    status = _model_save_configs_display(root_json, &dev_configs->disp_info);
    if (SL_FAIL(status))
        goto err;

    status = _model_save_configs_usb3(root_json, &dev_configs->usb3_info);
    if (SL_FAIL(status))
        goto err;

    root_json_str = cJSON_Print(root_json);
    if(!root_json_str)
        goto err;

    // Save configs
    {
        char filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

        strncpy(filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(filepath, SECOS_CONFIG_FILE_RELATIVE_FILENAMEPATH);

        status = FileUtils::writeFile(filepath, (uint8_t *)root_json_str, strlen(root_json_str));
        cJSON_free(root_json_str);
        if (SL_FAIL(status))
            goto err;
    }

    // On success
    return SL_SUCCESS;

err:
    if (root_json)
        cJSON_Delete(root_json);
    return SL_EINTERNAL;
}

/********* Deeper Internal Functions *********/
#define HID_SELECTOR_CHARS_PER_ENTRY_MAX (100)
void SOSLoader_GUIFrm_DevConfig::_view_hid_selectors_init(void)
{
    SL_STATUS status = SL_SUCCESS;

    // Init <m_hid_kbd_selector>
    {
        memset(&_hid_usbkbd_device_list, 0, sizeof(struct usbhid_device_list));

        this->m_hid_kbd_selector->Clear();

        status = usbhid_list_all_keyboards(&_hid_usbkbd_device_list);
        if (SL_PASS(status))
        {
            // Fill <m_hid_kbd_selector> with choices
            uint32_t i = 0;

            FOREACH_S(i, _hid_usbkbd_device_list.num_devices, USBHID_DEVICE_LIST_ENTRIES_MAX, 0, 1)
            {
                struct usbhid_device *dev_info = &_hid_usbkbd_device_list.devices[i];

                if (dev_info)
                {
                    char item_str[HID_SELECTOR_CHARS_PER_ENTRY_MAX] = {0};

                    snprintf(item_str, HID_SELECTOR_CHARS_PER_ENTRY_MAX, "[Bus %03u Dev %03u] %s",
                             dev_info->usb_addr.bus_id, dev_info->usb_addr.dev_usbaddr,
                             dev_info->name);
                    item_str[HID_SELECTOR_CHARS_PER_ENTRY_MAX - 1] = '\0';

                    this->m_hid_kbd_selector->Append(item_str);
                }
            }

            // Select the first one by default
            if (this->m_hid_kbd_selector->GetCount())
            {
                this->m_hid_kbd_selector->SetSelection(0);
            }
        }
    }

    // Init <m_hid_mouse_selector>
    {
        memset(&_hid_usbmouse_device_list, 0, sizeof(struct usbhid_device_list));

        this->m_hid_mouse_selector->Clear();

        status = usbhid_list_all_mouses(&_hid_usbmouse_device_list);
        if (SL_PASS(status))
        {
            // Fill <m_hid_mouse_selector> with choices
            uint32_t i = 0;

            FOREACH_S(i, _hid_usbmouse_device_list.num_devices, USBHID_DEVICE_LIST_ENTRIES_MAX, 0, 1)
            {
                struct usbhid_device *dev_info = &_hid_usbmouse_device_list.devices[i];

                if (dev_info)
                {
                    char item_str[HID_SELECTOR_CHARS_PER_ENTRY_MAX] = {0};

                    snprintf(item_str, HID_SELECTOR_CHARS_PER_ENTRY_MAX, "[Bus %03u Dev %03u] %s",
                             dev_info->usb_addr.bus_id, dev_info->usb_addr.dev_usbaddr,
                             dev_info->name);
                    item_str[HID_SELECTOR_CHARS_PER_ENTRY_MAX - 1] = '\0';

                    this->m_hid_mouse_selector->Append(item_str);
                }
            }

            // Select the first one by default
            if (this->m_hid_mouse_selector->GetCount())
            {
                this->m_hid_mouse_selector->SetSelection(0);
            }
        }
    }
}

#define SL_USB3_INFO_ENABLE_COMMENT "Set 1 to enable, set 0 to disable"
#define SL_USB3_INFO_HID_DEV_HUBPORT_COMMENT "USB port. Port number starts from 0, not 1."
SL_STATUS SOSLoader_GUIFrm_DevConfig::_model_save_configs_usb3(cJSON *root_json, SL_USB3_INFO *dev_cfgs)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    info_top_json = cJSON_CreateObject();
    if (!info_top_json)
        goto err;

    {
        // Serialize <usb3_enable>
        node_json = cJSON_CreateNumber((dev_cfgs->usb3_enable) ? 1 : 0);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "usb3_enable", node_json);

        // Serialize <type_comment>
        node_json = cJSON_CreateString(SL_USB3_INFO_ENABLE_COMMENT);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "usb3_enable_comment", node_json);
    }

    {
        // Serialize <hid_dev1_addr>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev1_addr);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev1_addr", node_json);

        // Serialize <hid_dev1_hub_addr>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev1_hub_addr);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev1_hub_addr", node_json);

        // Serialize <hid_dev1_hub_port>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev1_hub_port);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev1_hub_port", node_json);

        // Serialize <hid_dev1_hub_port_comment>
        node_json = cJSON_CreateString(SL_USB3_INFO_HID_DEV_HUBPORT_COMMENT);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev1_hub_port_comment", node_json);

        // Serialize <hid_dev1_bus_id>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev1_bus_id);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev1_bus_id", node_json);
    }

    {
        // Serialize <hid_dev2_addr>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev2_addr);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev2_addr", node_json);

        // Serialize <hid_dev2_hub_addr>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev2_hub_addr);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev2_hub_addr", node_json);

        // Serialize <hid_dev2_hub_port>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev2_hub_port);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev2_hub_port", node_json);

        // Serialize <hid_dev2_hub_port_comment>
        node_json = cJSON_CreateString(SL_USB3_INFO_HID_DEV_HUBPORT_COMMENT);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev2_hub_port_comment", node_json);

        // Serialize <hid_dev2_bus_id>
        node_json = cJSON_CreateNumber(dev_cfgs->hid_dev2_bus_id);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "hid_dev2_bus_id", node_json);
    }

    // On success
    cJSON_AddItemToObject(root_json, "usb3", info_top_json);
    return SL_SUCCESS;

err:
    if (info_top_json)
        cJSON_Delete(info_top_json);
    return SL_EINTERNAL;
}

#define WINDOW_RESOLUTION_SELECTOR_CHARS_PER_ENTRY_MAX (20)
void SOSLoader_GUIFrm_DevConfig::_view_disp_selectors_init(void)
{
    // SL_STATUS status = SL_SUCCESS;

    // Init <m_disp_resolution_selector>
    {
        uint32_t i = 0;
        uint32_t cnt = sizeof(avail_resolutions) / sizeof(avail_resolutions[0]);

        this->m_disp_resolution_selector->Clear();

        // Fill <m_disp_resolution_selector> with choices
        FOREACH_S(i, cnt, cnt, 0, 1)
        {
            struct disp_resolution *disp_info = &avail_resolutions[i];

            if (disp_info)
            {
                char item_str[WINDOW_RESOLUTION_SELECTOR_CHARS_PER_ENTRY_MAX] = {0};

                snprintf(item_str, WINDOW_RESOLUTION_SELECTOR_CHARS_PER_ENTRY_MAX, "%u x %u",
                         disp_info->width, disp_info->height);
                item_str[WINDOW_RESOLUTION_SELECTOR_CHARS_PER_ENTRY_MAX - 1] = '\0';

                this->m_disp_resolution_selector->Append(item_str);
            }

            // Select the first one by default
            if (this->m_disp_resolution_selector->GetCount())
            {
                this->m_disp_resolution_selector->SetSelection(0);
            }
        }
    }
}

#define SL_DISPLAY_INFO_TYPE_COMMENT "type must be 'IOSM-FB', 'Insecure', or 'NONE'"
SL_STATUS SOSLoader_GUIFrm_DevConfig::_model_save_configs_display(cJSON *root_json, SL_DISPLAY_INFO *dev_cfgs)
{
    cJSON *info_top_json = NULL;
    cJSON *node_json = NULL;

    info_top_json = cJSON_CreateObject();
    if (!info_top_json)
        goto err;

    {
        // Serialize <type>
        node_json = cJSON_CreateString(dev_cfgs->type);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "type", node_json);

        // Serialize <type_comment>
        node_json = cJSON_CreateString(SL_DISPLAY_INFO_TYPE_COMMENT);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "type_comment", node_json);

        // Serialize <disp_width>
        node_json = cJSON_CreateNumber(dev_cfgs->disp_width);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "disp_width", node_json);

        // Serialize <disp_height>
        node_json = cJSON_CreateNumber(dev_cfgs->disp_height);
        if (node_json == NULL)
            goto err;
        cJSON_AddItemToObject(info_top_json, "disp_height", node_json);
    }

    // On success
    cJSON_AddItemToObject(root_json, "display", info_top_json);
    return SL_SUCCESS;

err:
    if (info_top_json)
        cJSON_Delete(info_top_json);
    return SL_EINTERNAL;
}