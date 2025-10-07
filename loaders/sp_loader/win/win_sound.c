// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
#include <appddk.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdarg.h>

#define INITGUID
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <objbase.h>

#include <intrin.h>

#include <app_types.h>
#include <secloader_status.h>

static int _set_mute(BOOL mute)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioEndpointVolume *pEndpointVolume = NULL;
    int result = 1;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoInitializeEx failed: 0x%llx\n", (ulong_t)hr);
        return 1;
    }

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &IID_IMMDeviceEnumerator, (void **)&pEnumerator);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoCreateInstance(Enumerator) failed: 0x%llx\n", (ulong_t)hr);
        result = 1;
        goto cleanup;
    }

    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        fprintf(stderr, "GetDefaultAudioEndpoint failed: 0x%llx\n", (ulong_t)hr);
        result = 1;
        goto cleanup;
    }

    hr = pDevice->lpVtbl->Activate(pDevice, &IID_IAudioEndpointVolume,
                                   CLSCTX_ALL, NULL, (void **)&pEndpointVolume);
    if (FAILED(hr))
    {
        fprintf(stderr, "Device Activate failed: 0x%llx\n", (ulong_t)hr);
        result = 1;
        goto cleanup;
    }

    hr = pEndpointVolume->lpVtbl->SetMute(pEndpointVolume, mute, NULL);
    if (FAILED(hr))
    {
        fprintf(stderr, "SetMute failed: 0x%llx\n", (ulong_t)hr);
        result = 1;
        goto cleanup;
    }

    // On success
    result = 0;

cleanup:
    if (pEndpointVolume)
        pEndpointVolume->lpVtbl->Release(pEndpointVolume);
    if (pDevice)
        pDevice->lpVtbl->Release(pDevice);
    if (pEnumerator)
        pEnumerator->lpVtbl->Release(pEnumerator);
    CoUninitialize();
    return result;
}

static int _get_mute(BOOL *muted)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioEndpointVolume *pEndpointVolume = NULL;
    int result = 1;

    if (muted == NULL) {
        return 1; // invalid pointer
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoInitializeEx failed: 0x%llx\n", (ulong_t)hr);
        return 1;
    }

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &IID_IMMDeviceEnumerator, (void **)&pEnumerator);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoCreateInstance(Enumerator) failed: 0x%llx\n", (ulong_t)hr);
        goto cleanup;
    }

    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        fprintf(stderr, "GetDefaultAudioEndpoint failed: 0x%llx\n", (ulong_t)hr);
        goto cleanup;
    }

    hr = pDevice->lpVtbl->Activate(pDevice, &IID_IAudioEndpointVolume,
                                   CLSCTX_ALL, NULL, (void **)&pEndpointVolume);
    if (FAILED(hr))
    {
        fprintf(stderr, "Device Activate failed: 0x%llx\n", (ulong_t)hr);
        goto cleanup;
    }

    BOOL currentMute = FALSE;
    hr = pEndpointVolume->lpVtbl->GetMute(pEndpointVolume, &currentMute);
    if (FAILED(hr))
    {
        fprintf(stderr, "GetMute failed: 0x%llx\n", (ulong_t)hr);
        goto cleanup;
    }

    *muted = currentMute;
    result = 0; // success

cleanup:
    if (pEndpointVolume)
        pEndpointVolume->lpVtbl->Release(pEndpointVolume);
    if (pDevice)
        pDevice->lpVtbl->Release(pDevice);
    if (pEnumerator)
        pEnumerator->lpVtbl->Release(pEnumerator);
    CoUninitialize();
    return result;
}

/********* Public Functions *********/
int _do_os_sound_mute(uint8_t* out_already_muted)
{
    int ret = 0;
    BOOL _already_muted = FALSE;

    // Check: params must be valid
    if(!out_already_muted)
        return 1;

    // Do nothing if the system has already been muted before calling <os_sound_mute>.
    ret = _get_mute(&_already_muted);
    if(ret)
        return 1;
    *out_already_muted = (_already_muted == TRUE) ? true : false;
    
    if(_already_muted)
        return 0;

    // Mute the OS
    ret = _set_mute(TRUE);
    if(ret)
        return 1;

    // On success
    return 0;
}

int _do_os_sound_unmute(uint8_t already_muted)
{
    int ret = 0;

    // Do nothing if the system has already been muted before calling <os_sound_mute>.
    if(already_muted)
        return 0;

    ret = _set_mute(FALSE);

    if(ret)
        return 1;

    // On success
    return 0;
}