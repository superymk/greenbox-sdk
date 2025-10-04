#include "SOSLoader_GUIFrm_Main.h"

extern "C"
{
#include <loader_sa_intfs.h>
#include "util_file.h"
#include "sosl_log.h"
}
#include "sl_gui_utils.h"
#include "sl_gui_utils_cpp.h"

#if defined(__WINDOWS__)
#include <cwchar>
#include "../loader/win/win_task_mgr.h"
#endif // __WINDOWS__

#if defined(__WINDOWS__)

#define SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH_CHARSTR ".\\sos\\app_srv_dwm.exe"

/// @brief Returns 0 iff the SOSLoader GUI can run the DesktopMgr app with admin priviledge.
/// [NOTE] These checks are for availablity purpose, not for security purpose. If SOSLoader GUI cannot start DesktopMgr
/// app after running SecOS, users lose their trusted path devices (e.g., keyboard, mouse, display) and hence cannot use
/// their computers at all.
/// [NOTE] These checks cannot be for security purpose at all. If attackers may change the scheduled task of DesktopMgr
/// app and point the task to a malware, attackers do not need to start GreenBox at all to gain admin priviledge to
/// compromise the untrusted OS/Apps. Attackers could also restores the path after their attacks to remain stealthy.
/// [NOTE] How to protect file integrity of privileged GreenBox applications so attackers cannot use them to escalate
/// privledges? The solution is to use permissions of file systems; e.g., ordinary users are not allowed to modify
/// GreenBox executable files.
/// @param
/// @return
static SL_STATUS _win_check_dtm_app_task_avail(void)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    const wchar_t *task_name = GreenBox_DesktopMgr_App_Task;
    wchar_t cur_exe_wpath[MAX_PATH] = {0};
    wchar_t cur_exe_wargs[MAX_PATH] = {0};
    bool is_admin_priviledge = false;

    // [TODO][Issue 143] Check the existence of DesktopMgr App only, once we are able to run it with the user priviledge.

    ret = win_task_read_elevated(task_name, cur_exe_wpath, cur_exe_wargs, &is_admin_priviledge);
    if (ret)
    {
        FileUtils::log("[DesktopMgr-App] Read DesktopMgr-App task failed! ret:%d\n", ret);
        return SL_EINTERNAL;
    }

    // Check: the absolute file path of the task must point to the benign DesktopMgr-App.
    {
        const char *cur_exe_path = SL_GUI_UTILS::ConvertWCharToChar(cur_exe_wpath).c_str();
        char calc_exe_path[MAX_PATH_LEN] = {0};
        bool is_filepath_equal = false;

        strncpy(calc_exe_path, FileUtils::getCurrentExecutablePath(), MAX_PATH_LEN);
        strcat(calc_exe_path, SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH_CHARSTR);

        status = FileUtils::win_is_filepath_equal(cur_exe_path, calc_exe_path, &is_filepath_equal);
        if (SL_FAIL(status) || !is_filepath_equal)
        {
            FileUtils::log("[DesktopMgr-App] Check file path of DesktopMgr-App task error! status:%lu, is_filepath_equal:%u\n", status, is_filepath_equal);
            return SL_EINTERNAL;
        }
    }

    // Check: DesktopMgr app must be able to run with the admin priviledge in order to inject keys to admin applications.
    if (!is_admin_priviledge)
    {
        FileUtils::log("[DesktopMgr-App] Priviledge check failed\n");
        return SL_EINTERNAL;
    }

    // On success
    return SL_SUCCESS;
}

static int _win_run_dtm(void)
{
    int ret = 0;

    // [TODO][Issue 143] On Win10, the untrusted loader of DesktopMgr has to run with admin privilege to inject
    // key/mouse presses to all Windows processes. Need to refactor hid-bridge-backend to reduce the privilege of
    // DesktopMgr.
    // ShellExecute(NULL,
    //              L"runas",
    //              SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH,
    //              NULL,
    //              SL_GUI_UTILS::ConvertCharToLPCWSTR(FileUtils::getCurrentExecutablePath()).c_str(),
    //              SW_SHOWNORMAL);

    // [TODO][Issue 143] Remove the task run of DesktopMgr App once we are able to run it with the user priviledge.
    ret = win_task_run_scheduled(GreenBox_DesktopMgr_App_Task);
    if (ret)
    {
        FileUtils::log("[DesktopMgr-App] Run failed! ret:%d\n", ret);
        return 1;
    }

    // On success
    return 0;
}

#else

static SL_STATUS _linux_check_dtm_app_task_avail(void)
{
    FileUtils::log("[DesktopMgr-App] <_linux_check_dtm_app_task_avail> Not implemented!\n");
    
    // return SL_EINVAL;
    return SL_SUCCESS;
}

#endif // __WINDOWS__

static SL_STATUS _check_dtm_app_task_avail(void)
{
#if defined(__WINDOWS__)
    return _win_check_dtm_app_task_avail();
#else
    return _linux_check_dtm_app_task_avail();
#endif // __WINDOWS__
}

SL_STATUS SOSLoader_GUIFrm_Main::_load_icon(void)
{
#if defined(__WXMSW__)
    // Use the icon embedded in the executable.
    SetIcon(wxICON(IDI_APP_ICON));
#else
    // For non-Windows platforms, you might load from a file instead.
    SetIcon(wxIcon("./gui_res/home.ico", wxBITMAP_TYPE_ICO));
#endif

    // On success
    return SL_SUCCESS;
}

SOSLoader_GUIFrm_Main::SOSLoader_GUIFrm_Main(wxWindow *parent)
    : Frm_Main(parent)
{
    (void)FileUtils::log_init();
    // Load window icon
    (void)_load_icon();
}

void SOSLoader_GUIFrm_Main::Frm_MainOnClose(wxCloseEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;

    status = client.call_sl_fini();
    if (SL_FAIL(status))
    {
        wxString err;
        err.Printf("SOSLoader finalize error! status:%lu", status);
        wxMessageBox(err);
    }

    // Release SecOS loader lock.
    ret = sl_frm_main_instance_lock_release();
    if (ret)
    {
        wxString err;
        err.Printf("SOSLoader finalize error: Failed to release SOSLoader lock");
        wxMessageBox(err);
    }

    // Finalize the log subsystem.
    FileUtils::log_fini();

    Destroy();
}

void SOSLoader_GUIFrm_Main::Frm_MainOnShow(wxShowEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;

    // Grab lock to ensure the singleton instance of the untrusted loader process.
    ret = sl_frm_main_instance_lock_grab();
    if (ret)
    {
        wxString err;
        err.Printf("Another SOSLoader is running. Exiting!");
        wxMessageBox(err);

        Destroy();
    }

    status = client.call_sl_init();
    // status = sl_init();
    if (SL_FAIL(status))
    {
        wxString err;
        err.Printf("SOSLoader init error! status:%lu", status);
        wxMessageBox(err);
    }

    // If the SecOS config file exists, then enable the RUN button. Otherwise, disable both RUN and STOP button.
    if (FileUtils::isFileExist(SECOS_CONFIG_FILE_RELATIVE_FILENAMEPATH))
    {
        this->btn_run_sos->Enable(true);
    }
    else
    {
        this->btn_run_sos->Enable(false);
        this->btn_stop_sos->Enable(false);
    }
}

void SOSLoader_GUIFrm_Main::m_control_monitorOnMenuSelection(wxCommandEvent &event)
{
    // Run GreenBox Control Monitor (currently available on Windows only)

#if defined(__WINDOWS__)
#define GB_CONTROL_MONITOR_RELATIVE_FILEPATH L".\\tools\\control_monitor\\control_monitor.exe"

    ShellExecute(NULL,
                 L"runas",
                 GB_CONTROL_MONITOR_RELATIVE_FILEPATH,
                 NULL,
                 SL_GUI_UTILS::ConvertCharToLPCWSTR(FileUtils::getCurrentExecutablePath()).c_str(),
                 SW_SHOWNORMAL);

#endif // __WINDOWS__
}

void SOSLoader_GUIFrm_Main::m_sp_mgrOnMenuSelection(wxCommandEvent &event)
{
    if (!sl_frm_spmgr_status_get())
    {
        SOSLoader_GUIFrm_SPMgr *sp_mgr_frame = NULL;

        sl_frm_spmgr_status_set(true);

        sp_mgr_frame = new SOSLoader_GUIFrm_SPMgr(this);
        ;
        sp_mgr_frame->Show(true);
    }
}

void SOSLoader_GUIFrm_Main::m_aboutOnMenuSelection(wxCommandEvent &event)
{
    // TODO: Implement m_aboutOnMenuSelection
}

void SOSLoader_GUIFrm_Main::m_log_showOnMenuSelection(wxCommandEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;

    status = client.call_sl_dump_dmesg(this);
    if (SL_FAIL(status))
    {
        wxString err;
        err.Printf("SOSLoader dump dmesg error! status:%lu", status);
        wxMessageBox(err);
    }
}

void SOSLoader_GUIFrm_Main::btn_config_sosOnButtonClick(wxCommandEvent &event)
{
    // SL_STATUS status = SL_SUCCESS;
    // struct usbhid_device_list device_list = {0};

    // status = usbhid_list_all_keyboards(&device_list);
    // if (SL_FAIL(status))
    // {
    //     wxString err;
    //     err.Printf("Config SecOS and SecServices error! status:%lu\n", status);
    //     wxMessageBox(err);
    // }

    SOSLoader_GUIFrm_DevConfig *frm_dev_config = NULL;

    frm_dev_config = new SOSLoader_GUIFrm_DevConfig(this);
    frm_dev_config->parent_window = this;
    frm_dev_config->Show(true);
}

void SOSLoader_GUIFrm_Main::btn_config_sosOnEnterWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_config_sosOnEnterWindow
}

void SOSLoader_GUIFrm_Main::btn_config_sosOnLeaveWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_config_sosOnLeaveWindow
}

void SOSLoader_GUIFrm_Main::btn_run_sosOnButtonClick(wxCommandEvent &event)
{
    // SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    bool enable_stop_sos_btn = false;

    // Check: The DesktopMgr task must be good.
    status = _check_dtm_app_task_avail();
    if (SL_FAIL(status))
    {
        FileUtils::log("[DesktopMgr-App] Check failed! status:%lu\n", status);
        return;
    }

    // Run SecOS (including IOSMs) and DesktopMgr.
    status = this->_run_sos_and_dtm();
    if (SL_FAIL(status))
    {
        bool need_return_after_error = false;

        if ((status == SL_EINVAL) || (status == SL_EINVAL_CONFIGS))
        {
            // SecOS and SecServices are not booted yet
            enable_stop_sos_btn = false;
            need_return_after_error = false; // No need to return after SecOS and services error
        }
        else
        {
            // Encounters error on SecOS and services startup. So we must terminate them.
            wxCommandEvent event;
            this->btn_stop_sosOnButtonClick(event);
            need_return_after_error = true; // Need to return after SecOS and services error
        }

        // Display error message
        wxString err;
        _sos_run_error_string(status, &err);
        wxMessageBox(err);

        // Return if <need_return_after_error> is true
        if (need_return_after_error)
            return;
    }
    else
    {
        // If succeed, then enable <btn_stop_sos>
        enable_stop_sos_btn = true;
    }

    // Enable <btn_stop_sos> and disable <btn_run_sos> if desired
    if (enable_stop_sos_btn)
    {
        this->btn_stop_sos->Enable(true);
        this->btn_run_sos->Enable(false);
    }
}

void SOSLoader_GUIFrm_Main::btn_run_sosOnEnterWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_run_sosOnEnterWindow
}

void SOSLoader_GUIFrm_Main::btn_run_sosOnLeaveWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_run_sosOnLeaveWindow
}

void SOSLoader_GUIFrm_Main::btn_stop_sosOnButtonClick(wxCommandEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;

    // Stop the DesktopMgr probe timer
    m_dtm_probe_timer.Stop();

    status = client.call_sl_terminate_and_unregister_secos();
    if (SL_FAIL(status))
    {
        wxString err;
        err.Printf("SOSLoader terminates SecOS error! status:%lu\n", status);
        wxMessageBox(err);
    }

    // Enable <btn_run_sos> and disable <btn_stop_sos>
    this->btn_stop_sos->Enable(false);
    this->btn_run_sos->Enable(true);
}

void SOSLoader_GUIFrm_Main::btn_stop_sosOnEnterWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_stop_sosOnEnterWindow
}

void SOSLoader_GUIFrm_Main::btn_stop_sosOnLeaveWindow(wxMouseEvent &event)
{
    // TODO: Implement btn_stop_sosOnLeaveWindow
}

/// @brief Return true in <*out_dtm_sp_exist> if the DesktopMgr SecProcess exists.
/// @param
/// @return
static int _dtm_sp_detect(bool *out_dtm_sp_exist)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    struct sp_info_id_name_resp *resp_buf = NULL;
    uint32_t i = 0;

    *out_dtm_sp_exist = false;

    resp_buf = (struct sp_info_id_name_resp *)malloc(sizeof(struct sp_info_id_name_resp));
    if (!resp_buf)
    {
        ret = 1;
        goto out;
    }

    status = client.call_sl_secprocesses_get_ids_and_names(resp_buf);
    if (SL_FAIL(status))
    {
        ret = 1;
        goto out;
    }

    // Find the DesktopMgr SecProcess in <resp_buf>
    for (i = 0; i < resp_buf->num_info && i < SP_INFO_GET_IDS_NAMES_MAX_RESULTS; i++)
    {
        if (str_equal(resp_buf->info[i].sp_name, TRUSTED_SP_DesktopMgr_NAME))
        {
            // Found the DesktopMgr SecProcess
            *out_dtm_sp_exist = true;
            break;
        }
    }

    // On success
    ret = 0;

out:
    if (resp_buf)
        free(resp_buf);

    return ret;
}

/// @brief Return true in <*out_dtm_alive> if DesktopMgr is alive (i.e., the flag NS_DTM_SECPROCESS_IS_EXIT says so and
/// the DesktopMgr SecProcess exists).
/// @param out_dtm_alive
/// @return Return 0 on success.
///         Return 1 on error.
static SL_STATUS _query_shared_params_dtm_state(bool *out_dtm_alive)
{
    OSSTATUS sa_status = OS_SUCCESS;
    int ret = 0;
    uint64_t sa_dtm_state = DTM_SECPROCESS_TERMINATED;
    bool dtm_sp_exist = false;

    // Check: <out_dtm_alive> must be valid
    if (!out_dtm_alive)
        return SL_EINVAL;

    // Check the flag at <NS_DTM_SECPROCESS_IS_EXIT> in SecAgent.
    // [NOTE] This function does not need to check the existence of DesktopMgr loader, because the DesktopMgr loader
    // sets the flag to be DTM_SECPROCESS_TERMINATED before it exits normally or on any error.
    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DTM_SECPROCESS_IS_EXIT, &sa_dtm_state);
    if (SA_FAIL(sa_status))
    {
        FileUtils::log("Failed to get the parameter <NS_DTM_SECPROCESS_IS_EXIT>! SecAgent status:%d\n", sa_status);
        *out_dtm_alive = false;
        return SL_EINTERNAL;
    }
    else if (sa_dtm_state != DTM_SECPROCESS_NOT_TERMINATED)
    {
        // DesktopMgr SecProcess has been marked to be terminated.
        *out_dtm_alive = false;
        return SL_SUCCESS;
    }

    // Check the existence of DesktopMgr SecProcess.
    // DesktopMgr loader may exit before the SecProcess
    ret = _dtm_sp_detect(&dtm_sp_exist);
    if (ret || !dtm_sp_exist)
    {
        // DesktopMgr SecProcess does not exist or failed to detect it.
        *out_dtm_alive = false;
        return SL_SUCCESS;
    }

    // DesktopMgr SecProcess exists
    *out_dtm_alive = true;
    return SL_SUCCESS;
}

void SOSLoader_GUIFrm_Main::m_dtm_probe_timerOnTimer(wxTimerEvent &event)
{
    SL_STATUS ret = SL_SUCCESS;
    // int ret = 0;
    bool dtm_alive = true;

    ret = _query_shared_params_dtm_state(&dtm_alive);
    if (SL_FAIL(ret))
    {
        // Failed to get the parameter <NS_DTM_SECPROCESS_IS_EXIT>. GreenBox must be terminated now to release HID devices.
        // Terminate GreenBox by emulating to click the <Stop> button
        wxCommandEvent event;
        this->btn_stop_sosOnButtonClick(event);
    }
    else if (!dtm_alive)
    {
        // DesktopMgr is not alive. GreenBox must be terminated now to release HID devices.
        // Terminate GreenBox by emulating to click the <Stop> button
        wxCommandEvent event;
        this->btn_stop_sosOnButtonClick(event);
    }
}

SOSLoader_GUIFrm_Main::~SOSLoader_GUIFrm_Main(void)
{
}

#if defined(__WINDOWS__)
#define SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH L".\\sos\\app_srv_dwm.exe"
#else
#define SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH "./sos/app_srv_dwm"
#endif

void _execute_dtm()
{
#if defined(__WINDOWS__)
    int ret = 0;

    ret = _win_run_dtm();
    if (ret)
    {
        return;
    }
#else
    system(SRV_DESKTOPMGR_LOADER_RELATIVE_FILEPATH);
#endif
}

SL_STATUS SOSLoader_GUIFrm_Main::_run_sos_and_dtm()
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    bool probe_timer_started = false;
    bool dtm_alive = true;

    // Check system() command processor
    if (!system(NULL))
        return SL_EINVAL;

    // Run SecOS
    status = client.call_sl_load_and_run_secos(this);
    if (SL_FAIL(status))
    {
        if ((status == SL_EINVAL) || (status == SL_EINVAL_CONFIGS))
        {
            // The caller will perform specific handling for these errors

            // Clear the SOSLoader progress bar
            this->SetSOSLoaderProgressValue(0);

            return status;
        }
        else
        {
            // Clear the SOSLoader progress bar
            this->SetSOSLoaderProgressValue(0);

            return status;
        }
    }

    // Run DesktopMgr
    dtm_thread = new std::thread(_execute_dtm);

    //// Poll the status of the DesktopMgr SecProcess: It should run within 7000ms.
    const auto timeout   = std::chrono::milliseconds(7000);
    const auto sleepStep = std::chrono::milliseconds(20);   // small backoff to avoid busy-loop
    const auto t0        = std::chrono::steady_clock::now();

    for (;;)
    {
        status = _query_shared_params_dtm_state(&dtm_alive);
        if (SL_PASS(status) && dtm_alive)
        {
            // Success path: proceed with the original steps
            // Start the DesktopMgr probe timer
            probe_timer_started = m_dtm_probe_timer.Start(500);

            // Clear the SOSLoader progress bar
            this->SetSOSLoaderProgressValue(0);
            break;
        }

        // Timeout?
        if (std::chrono::steady_clock::now() - t0 >= timeout) {
            FileUtils::log("Timeout to run DesktopMgr!\n");
            return SL_ETIMEOUT; // early exit on failure to meet the deadline
        }

        // Keep UI responsive while we wait.
        ::wxYield();

        // Brief sleep to prevent a tight spin.
        std::this_thread::sleep_for(sleepStep);
    }

    // On success
    return SL_SUCCESS;
}

void SOSLoader_GUIFrm_Main::ui_disable_run_stop_buttons_on_secos_conf_not_exist(void)
{
    this->btn_run_sos->Enable(false);
    this->btn_stop_sos->Enable(false);
}

void SOSLoader_GUIFrm_Main::ui_enable_run_button_on_secos_conf_exist(void)
{
    this->btn_run_sos->Enable(true);
}

void SOSLoader_GUIFrm_Main::SetSOSLoaderProgressValue(uint32_t value)
{
    if (value > 100)
        return;

    this->g_sos_boot->SetRange(100);
    this->g_sos_boot->SetValue(value);
}

void SOSLoader_GUIFrm_Main::UpdateSOSLoaderProgress(void)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint64_t num_components_to_boot = 0;
    uint64_t num_components_booted = 0;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_GB_COMPONENTS_NUM, &num_components_to_boot);
    if (SA_FAIL(sa_status))
    {
        // Return on errors
        return;
    }

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_GB_COMPONENTS_BOOTED_NUM, &num_components_booted);
    if (SA_FAIL(sa_status))
    {
        // Return on errors
        return;
    }

    // Update SOSLoader progress bar
    {
        // Return on invalid numbers
        if (!num_components_to_boot)
        {
            this->SetSOSLoaderProgressValue(0);
            return;
        }

        if (num_components_booted > num_components_to_boot)
        {
            this->SetSOSLoaderProgressValue(100);
            return;
        }

        uint32_t value = (uint32_t)(num_components_booted * 100 / num_components_to_boot);

        if (value > 100)
        {
            // Invalid result
            return;
        }

        this->SetSOSLoaderProgressValue(value);
    }
}

void SOSLoader_GUIFrm_Main::_sos_run_error_string(SL_STATUS status, wxString *out_err_str)
{
    switch (status)
    {
    case SL_EINVAL_CONFIGS:
        out_err_str->Printf("Invalid configurations! Please re-set configurations.\n");
        break;
    default:
        out_err_str->Printf("SOSLoader runs SecOS and SecServices error! status:%lu\n", status);
        break;
    }
}