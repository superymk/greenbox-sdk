// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOSLoader_GUI_WIN_TASK_MGR_H
#define SOSLoader_GUI_WIN_TASK_MGR_H

#include <windows.h>
#include <tchar.h>

#define GreenBox_DesktopMgr_App_Task  L"GreenBox_DesktopMgr_App"
#define GreenBox_DesktopMgr_App_Author  L"GreenBox_Dev_Team"

/// @brief Create a scheduled task with elevated privileges.
/// @param task_name 
/// @param exe_path 
/// @param exe_args 
/// @return 
extern int win_task_create_elevated(const wchar_t *task_name, const wchar_t *exe_path, const wchar_t *exe_args);

/// @brief Run the scheduled task immediately.
/// @param task_name 
/// @return 
extern int win_task_run_scheduled(const wchar_t *task_name);

/// @brief Read the task's executablepath, arguments, and elevation status.
/// @param task_name 
/// @param out_exe_path 
/// @param out_exe_args 
/// @param out_is_elevated 
/// @return 
extern int win_task_read_elevated(const wchar_t *task_name, wchar_t *out_exe_path, wchar_t *out_exe_args, bool *out_is_elevated);

/// @brief Modify the task's executable path, arguments, and elevation status
/// @param task_name 
/// @param exe_path 
/// @param exe_args 
/// @param is_elevated 
/// @return 
extern int win_task_modify_elevated(const wchar_t *task_name, const wchar_t *exe_path, const wchar_t *exe_args, bool is_elevated);

/// @brief Delete the scheduled task.
/// @param task_name 
/// @return 
extern int win_task_delete(const wchar_t *task_name);

#endif // SOSLoader_GUI_WIN_TASK_MGR_H