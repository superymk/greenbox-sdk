// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <windows.h>
#include <taskschd.h>
#include <comdef.h>
#include <stdio.h>

#include "win_task_mgr.h"

// Link against the Taskschd.lib and Comsuppw.lib
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")

static HRESULT _initialize_com(void)
{
    HRESULT hr = S_OK;

    // 1. Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == RPC_E_CHANGED_MODE) 
    {
        // COM is already initialized with a different threading model (likely STA)
        // You can either:
        // 1. Use CoInitializeEx with COINIT_APARTMENTTHREADED, or
        // 2. Remove this call if the initialization is already done.
        // For example:
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    if (FAILED(hr))
        return hr;

    // 2. Set COM security levels
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);
    if (hr == RPC_E_TOO_LATE)
    {
        return S_OK;
    }
    else if (FAILED(hr))
    {
        CoUninitialize();
        return hr;
    }

    // On success
    return S_OK;
}

int win_task_create_elevated(const wchar_t *task_name, const wchar_t *exe_path, const wchar_t *exe_args)
{
    HRESULT hr = S_OK;
    ITaskService *pService = NULL;
    ITaskFolder *pRootFolder = NULL;
    ITaskDefinition *pTask = NULL;
    IRegistrationInfo *pRegInfo = NULL;
    ITaskSettings *pSettings = NULL;
    IPrincipal *pPrincipal = NULL;
    IActionCollection *pActionCollection = NULL;
    IAction *pAction = NULL;
    IExecAction *pExecAction = NULL;

    // 1. Initialize COM
    hr = _initialize_com();
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeEx failed: 0x%08lx\n", hr);
        return -1;
    }

    // 3. Create an instance of the Task Service
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&pService);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create ITaskService instance: 0x%08lx\n", hr);
        CoUninitialize();
        return -1;
    }

    // 4. Connect to the task service
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        wprintf(L"ITaskService::Connect failed: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 5. Get the pointer to the root TaskFolder
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Root Folder: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 6. Create the task definition
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create a task definition: 0x%08lx\n", hr);
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 7. Get the registration info for setting the author
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (SUCCEEDED(hr))
    {
        pRegInfo->put_Author(_bstr_t(GreenBox_DesktopMgr_App_Author));
        pRegInfo->Release();
    }

    // 8. Get the settings for the task
    hr = pTask->get_Settings(&pSettings);
    if (SUCCEEDED(hr))
    {
        pSettings->put_StartWhenAvailable(VARIANT_TRUE);
        
        // Disable the "Start the task only if the computer is on AC power" option.
        pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
        pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);

        pSettings->Release();
    }

    // 9. Create the principal for the task
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr))
    {
        pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
        pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }

    // 10. Create the action for the task
    hr = pTask->get_Actions(&pActionCollection);
    if (SUCCEEDED(hr))
    {
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        if (SUCCEEDED(hr))
        {
            hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
            if (SUCCEEDED(hr))
            {
                pExecAction->put_Path(_bstr_t(exe_path));
                pExecAction->put_Arguments(_bstr_t(exe_args));
                pExecAction->Release();
            }
            pAction->Release();
        }
        pActionCollection->Release();
    }

    // 11. Register the task
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(task_name),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(L""), // No user
        _variant_t(L""), // No password
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(L""),
        NULL);

    if (FAILED(hr))
    {
        wprintf(L"Registration of task failed: 0x%08lx\n", hr);
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }
    else
    {
        wprintf(L"Task \"%s\" successfully registered.\n", task_name);
    }

    // Cleanup
    pTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return 0;
}

int win_task_run_scheduled(const wchar_t *task_name)
{
    HRESULT hr = S_OK;
    ITaskService *pService = NULL;
    ITaskFolder *pRootFolder = NULL;
    IRegisteredTask *pRegisteredTask = NULL;

    // 1. Initialize COM
    hr = _initialize_com();
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeEx failed: 0x%08lx\n", hr);
        return -1;
    }

    // 3. Create an instance of the Task Service
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&pService);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create ITaskService instance: 0x%08lx\n", hr);
        CoUninitialize();
        return -1;
    }

    // 4. Connect to the task service
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        wprintf(L"ITaskService::Connect failed: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 5. Get the pointer to the root TaskFolder
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Root Folder: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 6. Get the registered task
    hr = pRootFolder->GetTask(_bstr_t(task_name), &pRegisteredTask);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get the registered task: 0x%08lx\n", hr);
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 7. Run the task
    hr = pRegisteredTask->Run(_variant_t(), NULL);
    if (FAILED(hr))
    {
        wprintf(L"Failed to run the task: 0x%08lx\n", hr);
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }
    else
    {
        wprintf(L"Task \"%s\" started successfully.\n", task_name);
    }

    // Cleanup
    pRegisteredTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return 0;
}

int win_task_read_elevated(const wchar_t *task_name, wchar_t *out_exe_path, wchar_t *out_exe_args, bool *out_is_elevated)
{
    HRESULT hr = S_OK;
    ITaskService *pService = NULL;
    ITaskFolder *pRootFolder = NULL;
    IRegisteredTask *pRegisteredTask = NULL;
    ITaskDefinition *pTaskDefinition = NULL;

    // 1. Initialize COM
    hr = _initialize_com();
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeEx failed: 0x%08lx\n", hr);
        return -1;
    }

    // 3. Create an instance of the Task Service
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&pService);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create ITaskService instance: 0x%08lx\n", hr);
        CoUninitialize();
        return -1;
    }

    // 4. Connect to the task service
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        wprintf(L"ITaskService::Connect failed: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 5. Get the pointer to the root TaskFolder
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Root Folder: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 6. Get the registered task
    hr = pRootFolder->GetTask(_bstr_t(task_name), &pRegisteredTask);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get the registered task: 0x%08lx\n", hr);
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 7. Get the task definition
    hr = pRegisteredTask->get_Definition(&pTaskDefinition);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Task Definition: 0x%08lx\n", hr);
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 8. Get the actions of the task
    IActionCollection *pActionCollection = NULL;
    hr = pTaskDefinition->get_Actions(&pActionCollection);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Actions collection: 0x%08lx\n", hr);
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    LONG actionCount = 0;
    hr = pActionCollection->get_Count(&actionCount);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Actions count: 0x%08lx\n", hr);
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    if (actionCount == 0)
    {
        wprintf(L"No actions found in the task.\n");
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // Get the first action (assuming Exec Action)
    IAction *pAction = NULL;
    hr = pActionCollection->get_Item(1, &pAction);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Action: 0x%08lx\n", hr);
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    IExecAction *pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
    if (FAILED(hr))
    {
        wprintf(L"Action is not an Exec Action: 0x%08lx\n", hr);
        pAction->Release();
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // Get the executable path
    BSTR bstrExePath = NULL;
    hr = pExecAction->get_Path(&bstrExePath);
    if (FAILED(hr))
    {
        wprintf(L"Failed to get exe path: 0x%08lx\n", hr);
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // Get the arguments
    BSTR bstrExeArgs = NULL;
    hr = pExecAction->get_Arguments(&bstrExeArgs);
    if (FAILED(hr))
    {
        wprintf(L"Failed to get exe arguments: 0x%08lx\n", hr);
        SysFreeString(bstrExePath);
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // Copy the exe path to the output buffer
    if (bstrExePath != NULL && out_exe_path != NULL)
    {
        wcscpy_s(out_exe_path, MAX_PATH, bstrExePath);
    }
    else
    {
        wprintf(L"Invalid buffer for exe path.\n");
        SysFreeString(bstrExePath);
        SysFreeString(bstrExeArgs);
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pTaskDefinition->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // Copy the exe arguments to the output buffer
    if (bstrExeArgs != NULL && out_exe_args != NULL)
    {
        wcscpy_s(out_exe_args, MAX_PATH, bstrExeArgs);
    }
    else
    {
        out_exe_args[0] = L'\0'; // No arguments
    }

    // Get the principal to check if it's elevated
    IPrincipal *pPrincipal = NULL;
    hr = pTaskDefinition->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr))
    {
        TASK_RUNLEVEL_TYPE runLevel;
        hr = pPrincipal->get_RunLevel(&runLevel);
        if (SUCCEEDED(hr))
        {
            *out_is_elevated = (runLevel == TASK_RUNLEVEL_HIGHEST) ? true : false;
        }
        else
        {
            wprintf(L"Failed to get run level: 0x%08lx\n", hr);
            *out_is_elevated = false;
        }
        pPrincipal->Release();
    }
    else
    {
        wprintf(L"Failed to get principal: 0x%08lx\n", hr);
        *out_is_elevated = true;
    }

    // Cleanup
    SysFreeString(bstrExePath);
    SysFreeString(bstrExeArgs);
    pExecAction->Release();
    pAction->Release();
    pActionCollection->Release();
    pTaskDefinition->Release();
    pRegisteredTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return 0;
}

int win_task_modify_elevated(const wchar_t *task_name, const wchar_t *exe_path, const wchar_t *exe_args, bool *is_elevated)
{
    HRESULT hr = S_OK;
    ITaskService *pService = NULL;
    ITaskFolder *pRootFolder = NULL;
    IRegisteredTask *pRegisteredTask = NULL;
    ITaskDefinition *pTask = NULL;
    IActionCollection *pActionCollection = NULL;
    IAction *pAction = NULL;
    IExecAction *pExecAction = NULL;
    IPrincipal *pPrincipal = NULL;

    // 1. Initialize COM
    hr = _initialize_com();
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeEx failed: 0x%08lx\n", hr);
        return -1;
    }

    // 3. Create an instance of the Task Service
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&pService);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create ITaskService instance: 0x%08lx\n", hr);
        CoUninitialize();
        return -1;
    }

    // 4. Connect to the task service
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        wprintf(L"ITaskService::Connect failed: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 5. Get the pointer to the root TaskFolder
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Root Folder: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 6. Get the registered task
    hr = pRootFolder->GetTask(_bstr_t(task_name), &pRegisteredTask);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get the registered task: 0x%08lx\n", hr);
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 7. Get the task definition
    hr = pRegisteredTask->get_Definition(&pTask);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Task Definition: 0x%08lx\n", hr);
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 8. Get the actions of the task
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Actions collection: 0x%08lx\n", hr);
        pTask->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }

    if (exe_path != NULL || exe_args != NULL)
    {
        // Assume modifying the first action
        hr = pActionCollection->get_Item(1, &pAction);
        if (FAILED(hr))
        {
            wprintf(L"Cannot get Action: 0x%08lx\n", hr);
            pActionCollection->Release();
            pTask->Release();
            pRegisteredTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            return -1;
        }

        hr = pAction->QueryInterface(IID_IExecAction, (void **)&pExecAction);
        if (FAILED(hr))
        {
            wprintf(L"Action is not an Exec Action: 0x%08lx\n", hr);
            pAction->Release();
            pActionCollection->Release();
            pTask->Release();
            pRegisteredTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            return -1;
        }

        // Modify the executable path if provided
        if (exe_path != NULL)
        {
            hr = pExecAction->put_Path(_bstr_t(exe_path));
            if (FAILED(hr))
            {
                wprintf(L"Failed to set new exe path: 0x%08lx\n", hr);
            }
        }

        // Modify the arguments if provided
        if (exe_args != NULL)
        {
            hr = pExecAction->put_Arguments(_bstr_t(exe_args));
            if (FAILED(hr))
            {
                wprintf(L"Failed to set new exe arguments: 0x%08lx\n", hr);
            }
        }

        pExecAction->Release();
        pAction->Release();
    }

    // 9. Modify the principal's run level
    if (is_elevated != NULL)
    {
        hr = pTask->get_Principal(&pPrincipal);
        if (SUCCEEDED(hr))
        {
            if (*is_elevated)
            {
                hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
            }
            else
            {
                hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_LUA);
            }

            if (FAILED(hr))
            {
                wprintf(L"Failed to set RunLevel: 0x%08lx\n", hr);
            }

            pPrincipal->Release();
        }
        else
        {
            wprintf(L"Failed to get Principal: 0x%08lx\n", hr);
        }
    }

    // 10. Register the updated task
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(task_name),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(L""), // No user
        _variant_t(L""), // No password
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(L""),
        NULL);

    if (FAILED(hr))
    {
        wprintf(L"Failed to register updated task: 0x%08lx\n", hr);
        pActionCollection->Release();
        pTask->Release();
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }
    else
    {
        wprintf(L"Task \"%s\" successfully updated.\n", task_name);
    }

    // Cleanup
    pActionCollection->Release();
    pTask->Release();
    pRegisteredTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return 0;
}

int win_task_delete(const wchar_t *task_name)
{
    HRESULT hr = S_OK;
    ITaskService *pService = NULL;
    ITaskFolder *pRootFolder = NULL;

    // 1. Initialize COM
    hr = _initialize_com();
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeEx failed: 0x%08lx\n", hr);
        return -1;
    }

    // 3. Create an instance of the Task Service
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&pService);
    if (FAILED(hr))
    {
        wprintf(L"Failed to create ITaskService instance: 0x%08lx\n", hr);
        CoUninitialize();
        return -1;
    }

    // 4. Connect to the task service
    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        wprintf(L"ITaskService::Connect failed: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 5. Get the pointer to the root TaskFolder
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        wprintf(L"Cannot get Root Folder: 0x%08lx\n", hr);
        pService->Release();
        CoUninitialize();
        return -1;
    }

    // 6. Delete the task
    hr = pRootFolder->DeleteTask(_bstr_t(task_name), 0);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            wprintf(L"Task \"%s\" does not exist.\n", task_name);
        }
        else
        {
            wprintf(L"Failed to delete the task: 0x%08lx\n", hr);
        }
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return -1;
    }
    else
    {
        wprintf(L"Task \"%s\" successfully deleted.\n", task_name);
    }

    // Cleanup
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return 0;
}
