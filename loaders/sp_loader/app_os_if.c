// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h> //fprintf, printf
#include <string.h>
#include <pthread.h>

extern int _do_os_sound_mute(uint8_t* out_already_muted);
extern int _do_os_sound_unmute(uint8_t already_muted);

static void* _owner_lock = NULL;

int os_sound_mute(const void* owner_lock, bool* out_already_muted)
{
    int ret = 0;
    OSSTATUS sa_status = OS_SUCCESS;
    bool lock_got = false;

    // Check: params must be valid
    if(!owner_lock)
        return 1;

    // Check: <os_sound_mute> may be nested, only the first caller can actually mute/unmute the OS.
    if(_owner_lock)
        return 0;

    sa_status = secagent_nsdom_global_spinlock_try_acquire(GLOBAL_SPINLOCK_OS_SOUND, &lock_got);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("OS Sound Mute: Failed to get global lock! sa_status:%lu\n", sa_status);
        return 1;
    }

    // <os_sound_mute> may be nested, only the first caller can actually mute/unmute the OS.
    if(lock_got)
    {
        // Update the owner after acquiring the spin lock.
        _owner_lock = (void*)owner_lock;

        ret = _do_os_sound_mute((uint8_t*)out_already_muted);

        sa_status = secagent_nsdom_global_spinlock_release(GLOBAL_SPINLOCK_OS_SOUND);
        if(SA_FAIL(sa_status))
        {
            APP_ERR("OS Sound Mute: Failed to release global lock! sa_status:%lu\n", sa_status);
            return 1;
        }

        return ret;
    }
    else
    {
        // Someone else is adjusting OS sound volume. Do nothing here.
        return 0;
    }

    // Should not hit
    return 1;
}

int os_sound_unmute(const void* owner_lock, bool already_muted)
{
    int ret = 0;
    OSSTATUS sa_status = OS_SUCCESS;
    bool lock_got = false;

    // Check: params must be valid
    if(!owner_lock)
        return 1;

    // Check: <os_sound_unmute> may be nested, only the first caller can actually mute/unmute the OS.
    if(_owner_lock != owner_lock)
        return 0;

    sa_status = secagent_nsdom_global_spinlock_try_acquire(GLOBAL_SPINLOCK_OS_SOUND, &lock_got);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("OS Sound Mute: Failed to get global lock! sa_status:%lu\n", sa_status);
        return 1;
    }

    // <os_sound_mute> may be nested, only the first caller can actually mute/unmute the OS.
    if(lock_got)
    {
        _owner_lock = NULL;

        ret = _do_os_sound_unmute((uint8_t)already_muted);

        sa_status = secagent_nsdom_global_spinlock_release(GLOBAL_SPINLOCK_OS_SOUND);
        if(SA_FAIL(sa_status))
        {
            APP_ERR("OS Sound Mute: Failed to release global lock! sa_status:%lu\n", sa_status);
            return 1;
        }

        return ret;
    }
    else
    {
        // Someone else is adjusting OS sound volume. Do nothing here.
        return 0;
    }

    // Should not hit
    return 1;
}