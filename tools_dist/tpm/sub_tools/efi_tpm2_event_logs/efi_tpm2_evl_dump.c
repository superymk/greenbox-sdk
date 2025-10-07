#include <windows.h>
#include <tbs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Tbs.lib")

// Function to create intermediate directories
static int CreateDirectories(const char* path)
{
    char temp[MAX_PATH];
    strncpy_s(temp, MAX_PATH, path, MAX_PATH);
    temp[MAX_PATH - 1] = '\0';

    // Remove the file name part from the path
    char* lastBackslash = strrchr(temp, '\\');
    if (lastBackslash)
    {
        *lastBackslash = '\0';
    }
    else
    {
        // No backslash found, assume current directory
        return 0;
    }

    size_t len = strlen(temp);
    if (len == 0)
        return 0;

    // Handle drive letter (e.g., "C:")
    size_t start = 0;
    if (len >= 2 && temp[1] == ':')
    {
        start = 2;
    }

    for (size_t i = start; i < len; i++)
    {
        if (temp[i] == '\\')
        {
            temp[i] = '\0';
            if (!CreateDirectoryA(temp, NULL))
            {
                DWORD error = GetLastError();
                if (error != ERROR_ALREADY_EXISTS)
                {
                    return -1;
                }
            }
            temp[i] = '\\';
        }
    }

    // Create the final directory
    if (!CreateDirectoryA(temp, NULL))
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            return -1;
        }
    }

    return 0;
}

// Function to check if a path is absolute
static int IsAbsolutePath(const char* path)
{
    if ((strlen(path) >= 3 && path[1] == ':' && path[2] == '\\') ||
        (strlen(path) >= 2 && path[0] == '\\' && path[1] == '\\'))
    {
        return 1;
    }
    return 0;
}

static int cmd_efi_tpm2_event_logs_dump(const char* outputFilePath)
{
    // Initialize TBS context
    TBS_CONTEXT_PARAMS2 contextParams;
    ZeroMemory(&contextParams, sizeof(TBS_CONTEXT_PARAMS2));
    contextParams.version = TPM_VERSION_20;
    contextParams.includeTpm20 = 1;

    TBS_HCONTEXT hContext;
    TBS_RESULT status = Tbsi_Context_Create(&contextParams, &hContext);
    if (status != TBS_SUCCESS) {
        printf("Failed to create TBS context. Error: 0x%X\n", status);
        return -1;
    }

    // Prepare to retrieve the log
    BYTE* logData = NULL;
    UINT32 logSize = 0;

    // First call to get the size of the log
    status = Tbsi_Get_TCG_Log(hContext, NULL, &logSize);
    if (status != TBS_SUCCESS) {
        printf("Failed to get log size. Error: 0x%X\n", status);
        Tbsip_Context_Close(hContext);
        return -1;
    }

    // Allocate buffer for the log
    logData = (BYTE*)malloc(logSize);
    if (!logData) {
        printf("Failed to allocate memory for log.\n");
        Tbsip_Context_Close(hContext);
        return -1;
    }

    // Retrieve the log
    status = Tbsi_Get_TCG_Log(hContext, logData, &logSize);
    if (status != TBS_SUCCESS) {
        printf("Failed to retrieve TPM log. Error: 0x%X\n", status);
        free(logData);
        Tbsip_Context_Close(hContext);
        return -1;
    }

    // Create intermediate directories
    if (CreateDirectories(outputFilePath) != 0)
    {
        printf("Failed to create intermediate directories for path: %s\n", outputFilePath);
        free(logData);
        Tbsip_Context_Close(hContext);
        return -1;
    }

    // Open the file for writing as binary
    HANDLE hFile = CreateFileA(
        outputFilePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create or open the file: %s. Error: %d\n", outputFilePath, GetLastError());
        free(logData);
        Tbsip_Context_Close(hContext);
        return -1;
    }

    DWORD bytesWritten = 0;
    BOOL writeResult = WriteFile(
        hFile,
        logData,
        logSize,
        &bytesWritten,
        NULL
    );

    if (!writeResult || bytesWritten != logSize)
    {
        printf("Failed to write the complete log data to the file. Error: %d\n", GetLastError());
        CloseHandle(hFile);
        free(logData);
        Tbsip_Context_Close(hContext);
        return -1;
    }

    // Cleanup
    CloseHandle(hFile);
    free(logData);
    Tbsip_Context_Close(hContext);
    return 0;
}

static void print_usage(void)
{
    printf("UEFI TPM2.X Event Log Dumper:\n");
    printf("Author: Miao Yu, Virgil Gligor\n");
    printf("\t efi_tpm2_event_logs.exe <AbsoluteOutputFilePath>\n");
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        print_usage();
        return -1;
    }

    const char* outputFilePath = argv[1];

    // Check if the path is absolute
    if (!IsAbsolutePath(outputFilePath))
    {
        printf("Please provide an absolute file path.\n");
        return -1;
    }

    int status = cmd_efi_tpm2_event_logs_dump(outputFilePath);

    return status;
}
