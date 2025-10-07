// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>
#include <dtm_sp_intfs.h>

#include <stdio.h>

#include <gui/dtm/libdtm.h>
#include "comm_dtm.h"

enum SECSAP_STATUS
{
    SRV_UNINITIALIZED = 0,
    SRV_INITIALIZED = 1
};

/********* Forward Declaration *********/
extern void libdtm_ops(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm);
extern void libdtm_hid_listener_mgr_init(void);
extern void libdtm_hid_listener_mgr_fini(void);

static SAP_STATUS _libdtm_unreference_framebuffer(shandle_t h_fb, size_t fb_size, vaddr_t fb_vaddr);
static SAP_STATUS _libdtm_unreference_ringbuffer(shandle_t h_rb_ctl, shandle_t h_rb_buf, vaddr_t rb_ctl_vaddr, vaddr_t rb_buf_vaddr);




/********* Data *********/
static enum SECSAP_STATUS srv_status = SRV_UNINITIALIZED;
static spid_t sp_dtm_spid = INVALID_SP_ID;

static shandle_t _h_fb = INVALID_HANDLE;
static vaddr_t _fb_vaddr = INVALID_VADDR;
static size_t _fb_size = 0;
static uint32_t _fb_max_width = 0;
static uint32_t _fb_max_height = 0;
static uint32_t _fb_cur_width = 0;
static uint32_t _fb_cur_height = 0;

static shandle_t _h_rb_ctl = INVALID_HANDLE;
static vaddr_t _rb_ctl = INVALID_VADDR;
static shandle_t _h_rb_buf = INVALID_HANDLE;
static vaddr_t _rb_buf = INVALID_VADDR;
static struct rb_channel dtm_rb_channel;



/********* Private Functions *********/
static SAP_STATUS _libdtm_reference_framebuffer(shandle_t h_fb, size_t fb_size, vaddr_t* out_fb_vaddr)
{
    SAP_STATUS status = SUCCESS;
    vaddr_t fb_vaddr = INVALID_VADDR;

    if((h_fb == INVALID_HANDLE) || (!out_fb_vaddr))
        return EINVAL;

    // Map
	fb_vaddr = mm_pick_vaddr(fb_size);
	if (fb_vaddr == INVALID_VADDR)
	{
		SAP_ERR("Initialize frame buffer: Failed to map the the frame buffer (handle:%u)!\n", h_fb);
		status = EINTERNAL;
		goto err;
	}

    status = sos_sys_obj_reference_object(h_fb, fb_vaddr, OA_RW_CACHE);
	if (FAIL(status))
	{
		SAP_ERR("Initialize frame buffer: Failed to reference the the frame buffer (handle:%u) with size 0x%lX! status:%lu\n", h_fb, fb_size, status);
		status = EINTERNAL;
		goto err;
	}

    // On success
    *out_fb_vaddr = fb_vaddr;
    return SUCCESS;


err:
    (void)_libdtm_unreference_framebuffer(h_fb, fb_size, fb_vaddr);
    return status;
}

static SAP_STATUS _libdtm_unreference_framebuffer(shandle_t h_fb, size_t fb_size, vaddr_t fb_vaddr)
{
    VKSTATUS sos_status = SUCCESS;

    if(fb_vaddr != INVALID_VADDR)
        mm_return_vaddr(fb_vaddr, fb_size);
    
    if(h_fb != INVALID_HANDLE)
    {
        sos_status = sos_sys_obj_unreference_object(h_fb);
        if (FAIL(sos_status))
            SAP_ERR("Finalize frame buffer: Failed to unreference the frame buffer (handle:%u)! status:%lu\n", h_fb, sos_status);
    }

    // On success
	return SUCCESS;
}

static SAP_STATUS _libdtm_reference_ringbuffer(shandle_t h_rb_ctl, shandle_t h_rb_buf, vaddr_t* out_rb_ctl_vaddr, vaddr_t* out_rb_buf_vaddr)
{
    SAP_STATUS status = SUCCESS;
    vaddr_t rb_ctl = INVALID_VADDR;
	vaddr_t rb_buf = INVALID_VADDR;
    size_t rb_ctl_size = DTM_EVENT_RB_CONTROL_PAGES * PAGE_SIZE_4K;
	size_t rb_buf_size = DTM_EVENT_RB_BUFFER_PAGES * PAGE_SIZE_4K;

    if((h_rb_ctl == INVALID_HANDLE) || (h_rb_buf == INVALID_HANDLE) || (!out_rb_ctl_vaddr) || (!out_rb_buf_vaddr))
        return EINVAL;

    // Map the control of the ring buffer
	rb_ctl = mm_pick_vaddr(rb_ctl_size);
	if (rb_ctl == INVALID_VADDR)
	{
		SAP_ERR("Initialize ring buffer control: Failed to map the the memory (handle:%u)!\n", h_rb_ctl);
		status = EINTERNAL;
		goto err;
	}

    status = sos_sys_obj_reference_object(h_rb_ctl, rb_ctl, OA_RW_CACHE);
	if (FAIL(status))
	{
		SAP_ERR("Initialize ring buffer control: Failed to reference the the memory (handle:%u) with size 0x%lX! status:%lu\n", h_rb_ctl, rb_ctl_size, status);
		status = EINTERNAL;
		goto err;
	}

    // Map the buffer of the ring buffer
	rb_buf = mm_pick_vaddr(rb_buf_size);
	if (rb_buf == INVALID_VADDR)
	{
		SAP_ERR("Initialize ring buffer buffer: Failed to map the the memory (handle:%u)!\n", h_rb_buf);
		status = EINTERNAL;
		goto err;
	}

    status = sos_sys_obj_reference_object(h_rb_buf, rb_buf, OA_RW_CACHE);
	if (FAIL(status))
	{
		SAP_ERR("Initialize ring buffer buffer: Failed to reference the the memory (handle:%u) with size 0x%lX! status:%lu\n", h_rb_buf, rb_buf_size, status);
		status = EINTERNAL;
		goto err;
	}

    // On success
    *out_rb_ctl_vaddr = rb_ctl;
    *out_rb_buf_vaddr = rb_buf;
    return SUCCESS;


err:
    (void)_libdtm_unreference_ringbuffer(h_rb_ctl, h_rb_buf, rb_ctl, rb_buf);
    return status;
}

static SAP_STATUS _libdtm_unreference_ringbuffer(shandle_t h_rb_ctl, shandle_t h_rb_buf, vaddr_t rb_ctl_vaddr, vaddr_t rb_buf_vaddr)
{
    VKSTATUS sos_status = SUCCESS;

    // Unmap the buffer of the ring buffer
    if(rb_buf_vaddr != INVALID_VADDR)
        mm_return_vaddr(rb_buf_vaddr, DTM_EVENT_RB_BUFFER_PAGES * PAGE_SIZE_4K);
    
    if(h_rb_buf != INVALID_HANDLE)
    {
        sos_status = sos_sys_obj_unreference_object(h_rb_buf);
        if (FAIL(sos_status))
            SAP_ERR("Finalize ring buffer buffer: Failed to unreference the memory (handle:%u)! status:%lu\n", h_rb_buf, sos_status);
    }

    // Unmap the control of the ring buffer
    if(rb_ctl_vaddr != INVALID_VADDR)
        mm_return_vaddr(rb_ctl_vaddr, DTM_EVENT_RB_CONTROL_PAGES * PAGE_SIZE_4K);
    
    if(h_rb_ctl != INVALID_HANDLE)
    {
        sos_status = sos_sys_obj_unreference_object(h_rb_ctl);
        if (FAIL(sos_status))
            SAP_ERR("Finalize ring buffer control: Failed to unreference the memory (handle:%u)! status:%lu\n", h_rb_ctl, sos_status);
    }

    // On success
	return SUCCESS;
}




/********* Public Functions *********/
SAP_STATUS libdtm_init(uint32_t win_init_width, uint32_t win_init_height)
{
    SAP_STATUS status = SUCCESS;

    // Find the SecProcess ID of DesktopMgr
    sp_dtm_spid = sos_sys_sp_query_id(SP_SERVICE, TRUSTED_SP_DesktopMgr_NAME, true);
    if (sp_dtm_spid == INVALID_SP_ID)
    {
        SAP_ERR("Failed to get the SecProcess ID of DesktopMgr!\n");
        status = EINTERNAL;
        goto err;
    }

    // Init listener manager
    libdtm_hid_listener_mgr_init();

    // Init SP-SP communication with the DesktopMgr
    status = comm_dtm_init(sp_dtm_spid);
    if (FAIL(status))
    {
        SAP_ERR("Failed to create SP-SP communication channels with DesktopMgr! status:%lu\n", status);
        status = EINTERNAL;
        goto err;
    }

    // Register window
    status = dtm_register_window(win_init_width, win_init_height, &_h_fb, &_fb_size, &_fb_max_width, &_fb_max_height, &_h_rb_ctl, &_h_rb_buf);
    if (FAIL(status))
    {
        SAP_ERR("Failed to create main window! status:%lu\n", status);
        status = EINTERNAL;
        goto err;
    }
    _fb_cur_width = win_init_width;
    _fb_cur_height = win_init_height;

    // Reference the frame buffer object
    status = _libdtm_reference_framebuffer(_h_fb, _fb_size, &_fb_vaddr);
    if (FAIL(status))
    {
        SAP_ERR("Failed to reference frame buffer! status:%lu\n", status);
        status = EINTERNAL;
        goto err;
    }

    // Reference the ring buffer object
    status = _libdtm_reference_ringbuffer(_h_rb_ctl, _h_rb_buf, &_rb_ctl, &_rb_buf);
    if (FAIL(status))
    {
        SAP_ERR("Failed to reference ring buffer! status:%lu\n", status);
        status = EINTERNAL;
        goto err;
    }
    //// Create the ring buffer channel
    status = rb_init(&dtm_rb_channel, _rb_ctl, _rb_buf, sizeof(DTM_EVENT), MAX_DTM_EVENTS_IN_RINGBUFFER);
    if (FAIL(status))
    {
        SAP_ERR("Failed to create ring buffer channel! status:%lu\n", status);
        status = EINTERNAL;
        goto err;
    }

    // Return successfully
    srv_status = SRV_INITIALIZED;
    return SUCCESS;


err:
    SAP_ERR("Initialize DesktopMgr client error! status:%lu\n", status);
    libdtm_fini();
    return status;
}

void libdtm_fini(void)
{
    SAP_STATUS status = SUCCESS;

    if (sp_dtm_spid == INVALID_SP_ID)
        SAP_ERR("Finalize DesktopMgr client error! Cannot communicate with DesktopMgr\n");

    srv_status = SRV_UNINITIALIZED;

    // Unreference ring buffer
    status = rb_fini(&dtm_rb_channel);
    if(FAIL(status))
        SAP_ERR("Finalize DesktopMgr client error! Failed to finalize ring buffer channel: status:%lu\n", status);

    status = _libdtm_unreference_ringbuffer(_h_rb_ctl, _h_rb_buf, _rb_ctl, _rb_buf);
    if(FAIL(status))
        SAP_ERR("Finalize DesktopMgr client error! Failed to unreference ring buffer: status:%lu\n", status);

    // Unreference frame buffer
    status = _libdtm_unreference_framebuffer(_h_fb, _fb_size, _fb_vaddr);
    if(FAIL(status))
        SAP_ERR("Finalize DesktopMgr client error! Failed to unreference frame buffer: status:%lu\n", status);

    status = dtm_unregister_window();
    if (FAIL(status))
        SAP_ERR("Finalize DesktopMgr client error! Failed to notify DesktopMgr: status:%lu\n", status);

    comm_dtm_fini();

    // Finalize listener manager
    libdtm_hid_listener_mgr_fini();

    // Clear the private values
    sp_dtm_spid = INVALID_SP_ID;
    _h_fb = INVALID_HANDLE;
    _fb_vaddr = INVALID_VADDR;
    _fb_size = 0;
    _fb_max_width = 0;
    _fb_max_height = 0;
    _fb_cur_width = 0;
    _fb_cur_height = 0;

    _h_rb_ctl = INVALID_HANDLE;
    _rb_ctl = INVALID_VADDR;
    _h_rb_buf = INVALID_HANDLE;
    _rb_buf = INVALID_VADDR;
    memset(&dtm_rb_channel, 0, sizeof(struct rb_channel));
}

SAP_STATUS libdtm_get_window_info(WINDOW_INFO* out_info)
{
    if(!out_info)
        return EINVAL;
    
    if(srv_status == SRV_UNINITIALIZED)
        return EINVAL;

    out_info->fb_vaddr = _fb_vaddr;
    out_info->fb_width = _fb_cur_width;
    out_info->fb_height = _fb_cur_height;
    return SUCCESS;
}

SAP_STATUS libdtm_on_dtm_event(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm)
{
    // Check: Input parameters must be valid
    if(!msg || !comm)
        return ENOTHANDLED;

    if (sp_dtm_spid == INVALID_SP_ID)
        return ENOTHANDLED;

    // Do not handle the event if the caller SecProcess is not DesktopMgr
    if(msg->header.sender_id != sp_dtm_spid)
        return ENOTHANDLED;

    libdtm_ops(msg, comm);

    return SUCCESS;
}

SAP_STATUS libdtm_pull_dtm_event(DTM_EVENT* out_event)
{
    SAP_STATUS status = SUCCESS;
    bool is_empty = false;

    if(!out_event)
        return EINVAL;

    status = rb_poll_msg(&dtm_rb_channel, out_event, &is_empty, true);
    if (FAIL(status))
    {
        if(is_empty)
            return FAILED;
        else
        {
            return EINTERNAL;
        }
    }

    return SUCCESS;
}

extern void libdtm_hid_call_kbd_listeners(struct kbd_key_event* event);
extern void libdtm_hid_call_mouse_listeners(struct mouse_event* event);

SAP_STATUS libdtm_handle_dtm_events(void)
{
    SAP_STATUS status = SUCCESS;
    DTM_EVENT event;

    while(1)
    {
        status = libdtm_pull_dtm_event(&event);
        if(PASS(status))
        {
            if(event.event_type == EVENT_TYPE_KBD)
            {
                libdtm_hid_call_kbd_listeners(&event.kbd_event);
            }
            else if (event.event_type == EVENT_TYPE_MOUSE)
                libdtm_hid_call_mouse_listeners(&event.mouse_event);
        }
        else if (status == FAILED)
        {
            // No more events
            return SUCCESS;
        }
        else
        {
            // error
            SAP_WARN("<libdtm_handle_dtm_events> Pull event error! status:%lu\n", status);
            break;
        }
    }

    return EINTERNAL;
}