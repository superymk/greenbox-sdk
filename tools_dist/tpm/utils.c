// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <toolsddk.h>
#include "utils.h"
#include <log.h>
#include "app_os_if.h"

int app_touch_mem(void *ptr, size_t len)
{
	int i;

	for(i = 0; i < len; i += PAGE_SIZE_4K) 
	{
		volatile unsigned int *addr = (unsigned int*)(ptr + i);
		volatile unsigned int data = *addr;

		// Write non-zero values to each page to prevent OS mapping empty pages to the same physical memory page.
		*addr = (i + 1);
		mfence();
		*addr = data;
		mfence();

		(void) data; (void)addr;
	}
	return 0;
}

/*********** [TODO] Get Executable Directory  **********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WINDOWS__)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

#define MAX_PATH_OUTPUT_LEN	(MAX_PATH_LEN + 1) // Need one more character to append the last "directory delimiter"

static char exePath[MAX_PATH_OUTPUT_LEN] = {0};
char* apploader_get_exe_directory()
{
	char* lastSlash;

	memset(exePath, 0, MAX_PATH_OUTPUT_LEN);

    // if (exePath == NULL) {
    //     printf("Memory allocation error");
    //     return NULL;
    // }

#if defined(_WIN32) || defined(__WINDOWS__)
    // Windows specific
    if (GetModuleFileName(NULL, exePath, MAX_PATH_LEN) == 0) {
        printf("GetModuleFileName failed");
        return NULL;
    }

	// Extract the directory path
    lastSlash = strrchr(exePath, '\\');
	if (lastSlash != NULL) {
        *lastSlash = '\\';
		*(lastSlash + 1) = '\0';
    }
#elif defined(__APPLE__)
    uint32_t bufsize = MAX_PATH_LEN;
    if (_NSGetExecutablePath(exePath, &bufsize) != 0) {
        printf("_NSGetExecutablePath failed");
        return NULL;
    }

	// Extract the directory path
    lastSlash = strrchr(exePath, '/');
	if (lastSlash != NULL) {
        *lastSlash = '/';
		*(lastSlash + 1) = '\0';
    }
#else
    // Linux specific
    ssize_t count = readlink("/proc/self/exe", exePath, MAX_PATH_LEN);
    if (count == -1 || count >= MAX_PATH_LEN) {
        printf("readlink failed");
        return NULL;
    }

	// Null-terminate the string
    exePath[count] = '\0';

	// Extract the directory path
    lastSlash = strrchr(exePath, '/');
	if (lastSlash != NULL) {
        *lastSlash = '/';
		*(lastSlash + 1) = '\0';
    }
#endif

    return exePath;
}

void print_hex(const char *label, const void *data, size_t length)
{
    const uint8_t *byte_ptr = (const uint8_t *)data;
    size_t index;

    for (index = 0; index < length; index++) {
        if (index % 16 == 0 && label != NULL) {
            printf("\n%s", label);
        }
        printf("%02x ", byte_ptr[index]);
    }
    printf("\n");
}

void gb_get_root_absolute_path(char* out_root_path)
{
    if(!out_root_path)
        return;

#if defined(_WIN32) || defined(__WINDOWS__)
    // Windows specific
    sprintf(out_root_path, "%s..\\..\\", apploader_get_exe_directory());
#else 
    // Linux specific
    sprintf(out_root_path, "%s/../../", apploader_get_exe_directory());
#endif
}

void gb_get_tpmtools_userdata_absolute_path(char* out_userdata_tpm_path)
{
    if(!out_userdata_tpm_path)
        return;

#if defined(_WIN32) || defined(__WINDOWS__)
    // Windows specific
    sprintf(out_userdata_tpm_path, "%s..\\..\\userdata\\tools\\tpmtools\\", apploader_get_exe_directory());
#else 
    // Linux specific
    sprintf(out_userdata_tpm_path, "%s/../../userdata/tools/tpmtools/", apploader_get_exe_directory());
#endif
}

void press_continue(const char* prompt)
{
	printf("%s", prompt);
	while (true)
	{ 
		char c = getchar();
		if (c =='\n' || c== EOF) 
			break;

		// Put a sleep in order to yield CPU
		// [NOTE] sched_yield() does not reduce CPU usage somehow...
		// msleep(100);
	}
}

bool secagent_test_presence(void)
{
    OSSTATUS sa_status = OS_SUCCESS;
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    cmd.id = CMD_SECAGENT_PRESENCE_REQ;

    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        TOOL_ERR("<secagent_test_presence> error with code %d!\n", ret);
        return false;
    }

    sa_status = cmd.status;
    if (SA_FAIL(sa_status))
    {
        TOOL_ERR("<secagent_test_presence> SecAgent error: %llu!\n", (unsigned long long)sa_status);
        return false;
    }

    return true;
}