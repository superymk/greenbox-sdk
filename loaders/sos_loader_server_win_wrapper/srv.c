// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <app_types.h>
#include "./hash/hash.h"

#define SERVICE_NAME "GreenBox_SecOS_Loader"

// Service status variables
static SERVICE_STATUS        g_ServiceStatus = {0};
static SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
static HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

// Process information for sos_loader.exe
static PROCESS_INFORMATION g_ProcessInfo = {0};

const union sha_digest sos_loader_sha256 = {
    .sha2_256_digest = {
        SOS_LOADER_SHA256
    }
};

#define MAX_PATH_LEN MAX_PATH
#define MAX_PATH_OUTPUT_LEN	(MAX_PATH_LEN + 1) // Need one more character to append the last "directory delimiter"
static char exePath[MAX_PATH_OUTPUT_LEN] = {0};
static char* _srv_wrapper_get_absolute_filepath(void)
{
    char* lastSlash;

    memset(exePath, 0, MAX_PATH_OUTPUT_LEN);

    // Windows specific
    wchar_t wideExePath[MAX_PATH_OUTPUT_LEN] = { 0 };

    // Use wide-character string function and buffer
    if (GetModuleFileNameW(NULL, wideExePath, MAX_PATH_OUTPUT_LEN) == 0) {
        printf("GetModuleFileName failed");
        return NULL;
    }

    // Convert wide character path back to multibyte character set
    if (WideCharToMultiByte(CP_ACP, 0, wideExePath, -1, exePath, MAX_PATH_OUTPUT_LEN, NULL, NULL) == 0) {
        printf("WideCharToMultiByte failed");
        return NULL;
    }

    // Extract the directory path
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash != NULL) {
        *lastSlash = '\\';
        *(lastSlash + 1) = '\0';
    }

    return exePath;
}

/// @brief Utility function to read the entire file into memory
static int read_file(const char *filepath, uint8_t **data, size_t *len) 
{
    HANDLE hFile = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file %s: %d\n", filepath, GetLastError());
        return -1;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        printf("Error getting file size: %d\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    *data = (uint8_t *)malloc(fileSize.QuadPart);
    if (*data == NULL) {
        printf("Memory allocation failed\n");
        CloseHandle(hFile);
        return -1;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, *data, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != fileSize.QuadPart) {
        printf("Error reading file: %d\n", GetLastError());
        free(*data);
        CloseHandle(hFile);
        return -1;
    }

    *len = (size_t)fileSize.QuadPart;
    CloseHandle(hFile);
    return 0;
}

// Function to log errors (optional)
void LogError(const char* message)
{
    HANDLE hEventLog = RegisterEventSource(NULL, SERVICE_NAME);
    if (hEventLog != NULL)
    {
        const char* messages[1] = { message };
        ReportEventA(
            hEventLog,               // Event log handle
            EVENTLOG_ERROR_TYPE,     // Event type
            0,                       // Category
            0,                       // Event identifier
            NULL,                    // No security identifier
            1,                       // Size of lpszStrings array
            0,                       // No binary data
            messages,                // Array of strings
            NULL                     // No binary data
        );
        DeregisterEventSource(hEventLog);
    }
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
        case SERVICE_CONTROL_STOP:
            if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
                break;

            // Update service status to stopping
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

            // Signal the service to stop
            SetEvent(g_ServiceStopEvent);
            break;
        default:
            break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    const char *sos_loader_filename = "sos_loader.exe";
    char exe_filepath[MAX_PATH_OUTPUT_LEN] = {0};

    strncpy(exe_filepath, _srv_wrapper_get_absolute_filepath(), MAX_PATH_LEN);
    strcat(exe_filepath, sos_loader_filename);


    // Register the service control handler
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_StatusHandle == NULL)
    {
        LogError("RegisterServiceCtrlHandler failed.");
        return;
    }

    // Initialize service status
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        LogError("SetServiceStatus failed during start pending.");
    }

    // Create a stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        LogError("CreateEvent failed.");

        // Error handling
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Update service status to running
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        LogError("SetServiceStatus failed during running state.");
    }

    // Check: the SOSLoader exe file matches its hash
    {
        uint8_t* file_data;
        size_t file_size;
        if (read_file(exe_filepath, &file_data, &file_size) != 0)
        {
            goto hash_check_err;
        }

        uint8_t file_hash[SHA256_DIGEST_LENGTH];
        if (sha2_256_mem(file_data, file_size, file_hash) != 0)
        {
            free(file_data);
            LogError("Hash computation failed");
            goto hash_check_err;
        }

        free(file_data);

        if (memcmp(file_hash, sos_loader_sha256.sha2_256_digest, SHA256_DIGEST_LENGTH) != 0)
        {
            LogError("Executable file hash does not match\n");
            goto hash_check_err;
        }
    }

    // Start sos_loader.exe
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&g_ProcessInfo, sizeof(g_ProcessInfo)); // Initialize the global PROCESS_INFORMATION

    if (!CreateProcess(
            NULL,
            exe_filepath,       // Command line
            NULL,              // Process handle not inheritable
            NULL,              // Thread handle not inheritable
            FALSE,             // Set handle inheritance to FALSE
            0,                 // Creation flags
            NULL,              // Use parent's environment block
            _srv_wrapper_get_absolute_filepath(),              // Use parent's starting directory 
            &si, 
            &g_ProcessInfo))   // Use the global PROCESS_INFORMATION
    {
        char errorMsg[256];
        sprintf(errorMsg, "Failed to start sos_loader.exe. Error: %d", GetLastError());
        LogError(errorMsg);

        // Failed to start the process
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        CloseHandle(g_ServiceStopEvent);
        return;
    }

    // Optionally, log that sos_loader.exe has started successfully

    // **Wait for the stop event to be signaled**
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);

    // **Service Stop Initiated**

    // Optionally, log that a stop has been requested

    // **Terminate sos_loader.exe**
    if (g_ProcessInfo.hProcess != NULL)
    {
        // Attempt to terminate the process gracefully
        // If sos_loader.exe has a graceful shutdown mechanism, implement it here
        if (!TerminateProcess(g_ProcessInfo.hProcess, 0))
        {
            LogError("Failed to terminate sos_loader.exe.");
        }
        else
        {
            // Optionally, log that termination was successful
        }

        // Wait until the process has terminated
        WaitForSingleObject(g_ProcessInfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(g_ProcessInfo.hProcess);
        CloseHandle(g_ProcessInfo.hThread);
    }

    // Update service status to stopped
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        LogError("SetServiceStatus failed during stopped state.");
    }

    // Close the stop event
    CloseHandle(g_ServiceStopEvent);

    // On success
    return;

hash_check_err:
    // Error handling
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = GetLastError();
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    return;
}

int main(int argc, char* argv[])
{
    // Define the service table
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    // Start the service
    if (!StartServiceCtrlDispatcher(ServiceTable))
    {
        printf("StartServiceCtrlDispatcher failed (%d)\n", GetLastError());
        return 1;
    }

    return 0;
}
