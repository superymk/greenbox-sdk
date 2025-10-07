// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>
#include <libdrv_hid_headers.h>

#define MAX_KBD_LISTENERS           (4)
#define MAX_MOUSE_LISTENERS         (4)

static KBD_LISTENER kbd_listeners[MAX_KBD_LISTENERS] = {0};
static MOUSE_LISTENER mouse_listeners[MAX_MOUSE_LISTENERS] = {0};

static List *kbd_event_list = NULL;
__LOCK_INIT(static, e_kbd_list_lock);
static List *mouse_event_list = NULL;
__LOCK_INIT(static, e_mouse_list_lock);




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

void libdrv_hid_call_kbd_listeners(struct kbd_key_event* event)
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

void libdrv_hid_call_mouse_listeners(struct mouse_event* event)
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




/********* Internal functions *********/
void libdrv_hid_kbd_event_enqueue(struct kbd_key_event* event)
{
    // Sanity checks
    if(!event)
        return;

    if(event->action == LIBDRV_HID_KBD_NO_EVENT)
        return;

    __libc_lock_acquire(&e_kbd_list_lock);
    stl_list_enqueue(kbd_event_list, event);
    __libc_lock_release(&e_kbd_list_lock);
}

// struct kbd_key_event* libdrv_hid_kbd_event_dequeue(void)
// {
//     struct kbd_key_event* e = NULL;

//     __libc_lock_acquire(&e_kbd_list_lock);
//     e = stl_list_dequeue(kbd_event_list);
//     __libc_lock_release(&e_kbd_list_lock);

//     return e;
// }

void libdrv_hid_mouse_event_enqueue(struct mouse_event* event)
{
    // Sanity checks
    if(!event)
        return;

    if(event->action == LIBDRV_HID_KBD_NO_EVENT)
        return;

    __libc_lock_acquire(&e_mouse_list_lock);
    stl_list_enqueue(mouse_event_list, event);
    __libc_lock_release(&e_mouse_list_lock);
}

// struct mouse_event* libdrv_hid_mouse_event_dequeue(void)
// {
//     struct mouse_event* e = NULL;

//     __libc_lock_acquire(&e_mouse_list_lock);
//     e = stl_list_dequeue(mouse_event_list);
//     __libc_lock_release(&e_mouse_list_lock);

//     return e;
// }

void libdrv_hid_dispatch_kbd_events(bool called_by_secsrv)
{
    struct kbd_key_event* e = NULL;

    while (!is_list_empty(kbd_event_list))
    {
        if(called_by_secsrv)
        {
            // Disable interrupts, or the caller (e.g., DesktopMgr) may deadlock itself due to contending for the <e_kbd_list_lock>
            sos_sys_disable_intr();
        }
        else 
        {
            // The caller is a SecApp, which does not allowed to call <sos_sys_disable_intr>. 
            // The SecApp needs to be careful to not deadlock itself.
        }

        __libc_lock_acquire(&e_kbd_list_lock);
        e = stl_list_dequeue(kbd_event_list);
        __libc_lock_release(&e_kbd_list_lock);

        if(called_by_secsrv)
        {
            sos_sys_enable_intr();
        }

        libdrv_hid_call_kbd_listeners(e);

        // Free the memory of <e>
        sfree(e, sizeof(struct kbd_key_event));
    }
}

void libdrv_hid_dispatch_mouse_events(bool called_by_secsrv)
{
    struct mouse_event* e = NULL;

    while (!is_list_empty(mouse_event_list))
    {
        if(called_by_secsrv)
        {
            // Disable interrupts, or the caller (e.g., DesktopMgr) may deadlock itself due to contending for the <e_kbd_list_lock>
            sos_sys_disable_intr();
        }
        else 
        {
            // The caller is a SecApp, which does not allowed to call <sos_sys_disable_intr>. 
            // The SecApp needs to be careful to not deadlock itself.
        }

        __libc_lock_acquire(&e_mouse_list_lock);
        e = stl_list_dequeue(mouse_event_list);
        __libc_lock_release(&e_mouse_list_lock);
        
        if(called_by_secsrv)
        {
            sos_sys_enable_intr();
        }

        libdrv_hid_call_mouse_listeners(e);

        // Free the memory of <e>
        sfree(e, sizeof(struct mouse_event));
    }
}




/********* Public functions *********/
SAP_STATUS hid_listener_mgr_init(void)
{
    kbd_event_list = stl_list_create();
    if(!kbd_event_list)
    {
        SAP_ERR("No memory to create keyboard event list!\n");
        return ENOMEM;
    }

    mouse_event_list = stl_list_create();
    if(!mouse_event_list)
    {
        SAP_ERR("No memory to create mouse event list!\n");
        return ENOMEM;
    }

    _hid_listener_mgr_clear();

    return SUCCESS;
}

void hid_listener_mgr_fini(void)
{
    _hid_listener_mgr_clear();

    if (kbd_event_list)
	{
		stl_list_clear_destroy(kbd_event_list);
		kbd_event_list = NULL;
	}

    if (mouse_event_list)
	{
		stl_list_clear_destroy(mouse_event_list);
		mouse_event_list = NULL;
	}
}

SAP_STATUS libdrv_hid_register_kbd_listener(KBD_LISTENER listener)
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

SAP_STATUS libdrv_hid_unregister_kbd_listener(KBD_LISTENER listener)
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

SAP_STATUS libdrv_hid_register_mouse_listener(MOUSE_LISTENER listener)
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

SAP_STATUS libdrv_hid_unregister_mouse_listener(MOUSE_LISTENER listener)
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