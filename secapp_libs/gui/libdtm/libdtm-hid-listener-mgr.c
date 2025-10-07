// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>
#include <gui/dtm/libdtm.h>

#define MAX_KBD_LISTENERS           (4)
#define MAX_MOUSE_LISTENERS         (4)

static KBD_LISTENER kbd_listeners[MAX_KBD_LISTENERS] = {0};
static MOUSE_LISTENER mouse_listeners[MAX_MOUSE_LISTENERS] = {0};




/********* Private functions *********/
static void _hid_listener_mgr_clear(void)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_KBD_LISTENERS, MAX_KBD_LISTENERS, 0, 1)
    {
        kbd_listeners[i] = NULL;
    }

    FOREACH_S(i, MAX_MOUSE_LISTENERS, MAX_MOUSE_LISTENERS, 0, 1)
    {
        mouse_listeners[i] = NULL;
    }
}




/********* Internal functions *********/
void libdtm_hid_call_kbd_listeners(struct kbd_key_event* event)
{
    uint32_t i = 0;

    // Sanity checks
    if(!event)
        return;

    if(event->action == LIBDRV_HID_KBD_NO_EVENT)
        return;

    FOREACH_S(i, MAX_KBD_LISTENERS, MAX_KBD_LISTENERS, 0, 1)
    {
        if(kbd_listeners[i])
            kbd_listeners[i](event);
    }
}

void libdtm_hid_call_mouse_listeners(struct mouse_event* event)
{
    uint32_t i = 0;

    // Sanity checks
    if(!event)
        return;

    if(event->action == LIBDRV_HID_MOUSE_NO_EVENT)
        return;

    FOREACH_S(i, MAX_MOUSE_LISTENERS, MAX_MOUSE_LISTENERS, 0, 1)
    {
        if(mouse_listeners[i])
            mouse_listeners[i](event);
    }
}




/********* Public functions *********/
void libdtm_hid_listener_mgr_init(void)
{
    _hid_listener_mgr_clear();
}

void libdtm_hid_listener_mgr_fini(void)
{
    _hid_listener_mgr_clear();
}

SAP_STATUS libdtm_hid_register_kbd_listener(KBD_LISTENER listener)
{
    uint32_t i = 0;

    // Sanity checks
    if(!listener)
        return EINVAL;

    // Return SUCCESS if <listener> has been registered.
    FOREACH_S(i, MAX_KBD_LISTENERS, MAX_KBD_LISTENERS, 0, 1)
    {
        if(kbd_listeners[i] == listener)
        {
            return SUCCESS;
        }
    }

    // Register <listener>
    FOREACH_S(i, MAX_KBD_LISTENERS, MAX_KBD_LISTENERS, 0, 1)
    {
        if(kbd_listeners[i] == NULL)
        {
            kbd_listeners[i] = listener;
            return SUCCESS;
        }
    }

    return FAILED;
}

SAP_STATUS libdtm_hid_unregister_kbd_listener(KBD_LISTENER listener)
{
    uint32_t i = 0;

    // Sanity checks
    if(!listener)
        return EINVAL;

    FOREACH_S(i, MAX_KBD_LISTENERS, MAX_KBD_LISTENERS, 0, 1)
    {
        if(kbd_listeners[i] == listener)
        {
            // Remove all entries of <listener>
            kbd_listeners[i] = NULL;
        }
    }

    return SUCCESS;
}

SAP_STATUS libdtm_hid_register_mouse_listener(MOUSE_LISTENER listener)
{
    uint32_t i = 0;

    // Sanity checks
    if(!listener)
        return EINVAL;

    // Return SUCCESS if <listener> has been registered.
    FOREACH_S(i, MAX_MOUSE_LISTENERS, MAX_MOUSE_LISTENERS, 0, 1)
    {
        if(mouse_listeners[i] == listener)
        {
            return SUCCESS;
        }
    }

    // Register <listener>
    FOREACH_S(i, MAX_MOUSE_LISTENERS, MAX_MOUSE_LISTENERS, 0, 1)
    {
        if(mouse_listeners[i] == NULL)
        {
            mouse_listeners[i] = listener;
            return SUCCESS;
        }
    }

    return FAILED;
}

SAP_STATUS libdtm_hid_unregister_mouse_listener(MOUSE_LISTENER listener)
{
    uint32_t i = 0;

    // Sanity checks
    if(!listener)
        return EINVAL;

    FOREACH_S(i, MAX_MOUSE_LISTENERS, MAX_MOUSE_LISTENERS, 0, 1)
    {
        if(mouse_listeners[i] == listener)
        {
            // Remove all entries of <listener>
            mouse_listeners[i] = NULL;
        }
    }

    return SUCCESS;
}