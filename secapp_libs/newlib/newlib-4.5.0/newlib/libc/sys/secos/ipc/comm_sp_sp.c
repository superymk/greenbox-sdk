// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <palddk.h>
#include <string.h>
#include <stdio.h>
#include <sap_headers_libc.h>
#ifdef __i386__
#include <sys/machine/i386/atomic.h>
#elif defined __amd64__
#include <sys/machine/amd64/atomic.h>
#endif

// #include <sddk.h>
// #include <arch.h>
// #include <glibc/arch/x86/atomic.h>

/********* Forward Declaration *********/
extern SP_UTASK *current_sp;

/********* Data *********/
#define COMM_ISP_INVALID_MSG_ID     0
#define COMM_ISP_START_MSG_ID       1
static uint32_t _next_msg_id = COMM_ISP_START_MSG_ID;
static uint32_t _wait_reply_on_msg_id = COMM_ISP_INVALID_MSG_ID;

#define ALL_COMM_ISPS_MAX_NUM       16           
static COMM_ISP_SHAREDMEM* _all_comm_isps[ALL_COMM_ISPS_MAX_NUM] = {NULL}; // Records of all active <COMM_ISP_SHAREDMEM> info so far




/********* Data for Handling Accidentally Closed IPC Servers *********/
// Because IPC servers of shared memory allocate the memory, accidentally termination of an IPC server will cause its 
// IPC clients encounter #PF because of the dangling memory references of the shared memory left in the IPC clients.
// Thus, IPC clients should always use <comm_isp_read_buffer> and <comm_isp_write_buffer> to access shared memory and 
// handle the #PF in an elegant manner.

// The active COMM_ISP_SHAREDMEM which is accessing memory buffer
static COMM_ISP_SHAREDMEM* _comm_isp_accessing_buffer = NULL;

// Only one <COMM_ISP_SHAREDMEM> can be in the middle of <comm_isp_read_buffer> or <comm_isp_write_buffer>
__LOCK_INIT(static, _comm_isp_accessing_buffer_lock);



/********* Private Functions *********/
static uint32_t read32(const vaddr_t addr)
{
    return *(volatile const uint32_t *)addr;
}

static uint32_t _get_new_msg_id(void)
{
    uint32_t result = _next_msg_id;

    atomic_increase(&_next_msg_id);

    // <_next_msg_id> must skip the value COMM_ISP_INVALID_MSG_ID
    if(_next_msg_id == COMM_ISP_INVALID_MSG_ID)
        _next_msg_id = COMM_ISP_START_MSG_ID;

    return result;
}

/// @brief Prepare the header of a <COMM_ISP_SEND_PARAMS> message
static void _comm_isp_prepare_request(COMM_ISP_SEND_PARAMS* msg, bool req_response)
{
	msg->req_id = _get_new_msg_id();

	if(req_response)
		msg->req_response = true;
	else
		msg->req_response = false;
}


static COMM_ISP_SEND_PARAMS* _comm_isp_create_msg(bool req_response)
{
	COMM_ISP_SEND_PARAMS* msg = NULL;

	msg = (COMM_ISP_SEND_PARAMS*)smalloc(sizeof(COMM_ISP_SEND_PARAMS));
	if(!msg)
	{
		SP_COMM_ERR("Create communication message error: No memory!\n");
		return NULL;
	}

	// Prepare the msg
	_comm_isp_prepare_request(msg, req_response);

	return msg;
}

static void _comm_isp_free_msg(COMM_ISP_SEND_PARAMS* msg)
{
	if(msg)
		sfree(msg, sizeof(COMM_ISP_SEND_PARAMS));
}

static SAP_STATUS _is_target_sp_responsed(spid_t target_sp_id, uint32_t wait_reply_on_msg_id, volatile bool* result, void* out_reply_msg_data, size_t* out_reply_msg_size)
{
    uint32_t i = 0;

    // Sanity checks
    if(!current_sp)
    {
        *result = false;
        return EINVAL;
    }

    if(!result)
    {
        *result = false;
        return EINVAL;
    }

    if(wait_reply_on_msg_id == COMM_ISP_INVALID_MSG_ID)
    {
        *result = false;
        return EINVAL;
    }

    // [NOTE] This function does not ask for the lock of <current_sp->events>. Otherwise SecOS could encounter a deadlock in
    // <sp_send> when this function holds the lock and is interruptable.
    FOREACH_S(i, SECAPP_MAX_NUM_EVENTS, SECAPP_MAX_NUM_EVENTS, 0, 1)
	{
		SP_MESSAGE* msg = &current_sp->events[i];

		if(msg->header.sender_id == target_sp_id && msg->header.handled == SAP_EVENT_NOT_HANDLED)
        {
            // Got a message from the target SecProcess
            if(msg->header.ack_req_id == wait_reply_on_msg_id)
            {
                size_t reply_msg_size = 0;

                // The message is the reply on request message with the ID <wait_reply_on_msg_id>
                //// Mark the message is handled, so we do not handle it later.
                msg->header.handled = SAP_EVENT_HANDLED;
                *result = true;
                reply_msg_size = min(msg->header.data_size, SECPROCESS_MSG_DATA_SIZE);

                if(reply_msg_size)
                {
                    if(out_reply_msg_data)
                        memcpy(out_reply_msg_data, msg->data, reply_msg_size);
                }

                if(out_reply_msg_size)
                    *out_reply_msg_size = reply_msg_size;
                    
                return SUCCESS;
            }
        }
	}

    *result = false;
    return SUCCESS;
}

/// @brief Close an Inter-SecProcess shared memory channel.
/// Requirement: <comm> != NULL
/// Requirement: comm->role == COMM_ISP_ROLE_OWNER
static void _isp_sharedmem_close_owner(COMM_ISP_SHAREDMEM* comm)
{
    SAP_STATUS status = SUCCESS;

    // Destroy the Inter-SecProcess shared memory channel
    status = (SAP_STATUS)sos_sys_comm_isp_destroy(comm->h_comm);
    if(FAIL(status))
        SP_COMM_ERR("Inter-SecProcess communication (%u <--> %u): Failed to destroy the channel!\n", comm->owner_spid, comm->user_spid);

    // Free memory
    if(comm->comm_buf)
        free_pages((void*)comm->comm_buf, BYTES_TO_PAGE_4K(comm->comm_buf_size));
}

/// @brief Close an Inter-SecProcess shared memory channel.
/// Requirement: <comm> != NULL
/// Requirement: comm->role == COMM_ISP_ROLE_USER
static void _isp_sharedmem_close_user(shandle_t h_comm, vaddr_t comm_buf_base, size_t comm_buf_size)
{
    sos_sys_obj_unreference_object(h_comm);
    mm_return_vaddr(comm_buf_base, comm_buf_size);
}

/// @brief Close an Inter-SecProcess shared memory channel.
/// Requirement: <comm> != NULL
static void _isp_sharedmem_clear(COMM_ISP_SHAREDMEM* comm)
{
    comm->role = COMM_ISP_ROLE_INVALID;
    comm->h_comm = INVALID_HANDLE;
    comm->comm_buf = INVALID_VADDR;
    comm->comm_buf_size = 0;
    comm->owner_spid = INVALID_SP_ID;
    comm->user_spid = INVALID_SP_ID;

    comm->buffer_access_err_handler = NULL;

    sfree(comm, sizeof(COMM_ISP_SHAREDMEM));
}




/********* Public Functions *********/
/// @brief Create an Inter-SecProcess shared memory channel.
///
/// @return Return a valid pointer on succss.
///         Return NULL on errors.
COMM_ISP_SHAREDMEM* comm_isp_sharedmem_create(spid_t target_sp_id, uint32_t num_pages)
{
    VKSTATUS sos_sts = SUCCESS;
    shandle_t h_comm = INVALID_HANDLE;
    void* comm_buf = NULL;
    COMM_ISP_SHAREDMEM* comm = NULL;

    // Sanity checks
    if(target_sp_id == INVALID_SP_ID)
        return NULL;

    // Allocate memory
    comm_buf = alloc_pages(num_pages);
    if(!comm_buf)
    {
        SP_COMM_ERR("Inter-SecProcess shared memory communication (%u <--> %u): Failed to allocate memory!\n", current_sp->sp_id, target_sp_id);
        goto err_ret;
    }

    // Create the Inter-SecProcess shared memory channel
    h_comm = sos_sys_comm_isp_create(target_sp_id, current_sp->h_heap, (vaddr_t)comm_buf, num_pages);
    if(h_comm == INVALID_HANDLE)
    {
        SP_COMM_ERR("Inter-SecProcess shared memory communication (%u <--> %u): Failed to create the channel!\n", current_sp->sp_id, target_sp_id);
        goto err_ret;
    }

    // Reference the shared memory with cache disabled
    sos_sts = sos_sys_obj_re_reference_object(h_comm, (vaddr_t)comm_buf, OA_RW_NO_CACHE);
    if(FAIL(sos_sts))
    {
        SP_COMM_ERR("Inter-SecProcess shared memory communication (%u <--> %u): Failed to reference the channel with cache disabled!\n", current_sp->sp_id, target_sp_id);
        goto err_ret;
    }

    // Allocate <COMM_ISP_SHAREDMEM>
    comm = (COMM_ISP_SHAREDMEM*)smalloc(sizeof(COMM_ISP_SHAREDMEM));
	if(!comm)
	{
		SP_COMM_ERR("Inter-SecProcess shared memory communication (%u <--> %u): Failed to allocate structure of the channel!\n", current_sp->sp_id, target_sp_id);
		goto err_ret;
	}

    comm->role = COMM_ISP_ROLE_OWNER;
    comm->h_comm = h_comm;
    comm->comm_buf = (vaddr_t)comm_buf;
    comm->comm_buf_size = PAGE_4K_TO_BYTES(num_pages);
    comm->owner_spid = current_sp->sp_id;
    comm->user_spid = target_sp_id;

    comm->buffer_access_err_handler = NULL;
    
    return comm;

err_ret:
    if(comm)
        _isp_sharedmem_clear(comm);

    if(h_comm != INVALID_HANDLE)
        (void)sos_sys_comm_isp_destroy(h_comm);

    if(comm_buf)
        free_pages(comm_buf, num_pages);
        
    return NULL;
}

/// @brief The caller SecProcess accepts to establish the given Inter-SecProcess shared memory channel <h_comm> 
COMM_ISP_SHAREDMEM* comm_isp_sharedmem_accept(spid_t requester_spid, shandle_t h_comm)
{
    SYS_OBJ_INFO ret_info;
    VKSTATUS sos_status = SUCCESS;
    COMM_ISP_SHAREDMEM* comm = NULL;
    vaddr_t comm_buf_base = INVALID_VADDR;
    size_t comm_buf_size = 0;

    // Sanity checks
    if(requester_spid == INVALID_SP_ID)
		return NULL;
    if(h_comm == INVALID_HANDLE)
		return NULL;

    // Check: Does the SecProcess <requester_spid> own the object <h_comm>?
    sos_status = sos_sys_obj_get_info(h_comm, &ret_info);
    if(FAIL(sos_status))
    {
        SP_COMM_ERR("Inter-SecProcess Shared Memory Communication Accept Channel Error: Failed to get the owner of the Inter-Process communication channel (handle:%u)! status:%lu\n", h_comm, sos_status);
        return NULL;
    }

    if((ret_info.owner_spid != requester_spid) || (ret_info.obj_type != OBJ_MEM_SDOM))
    {
        SP_COMM_ERR("Inter-SecProcess Shared Memory Communication Accept Channel Error: The requester SecProcess (id:%u) does not own the Inter-Process communication channel (handle:%u)!\n", requester_spid, h_comm);
        return NULL;
    }
    
    // Map the communication channel object in the current SecProcess.
    comm_buf_size = ret_info.resource_size;
    comm_buf_base = mm_pick_vaddr(comm_buf_size);
    sos_status = sos_sys_obj_reference_object(h_comm, comm_buf_base, OA_RW_NO_CACHE);
    if(FAIL(sos_status))
    {
        SP_COMM_ERR("Inter-SecProcess Shared Memory Communication Accept Channel Error: Failed to map the Inter-Process communication channel (handle:%u)! status:%lu\n", h_comm, sos_status);
        return NULL;
    }

    // Record the communication channel.
    comm = (COMM_ISP_SHAREDMEM*)smalloc(sizeof(COMM_ISP_SHAREDMEM));
	if(!comm)
	{
		SP_COMM_ERR("Inter-SecProcess Shared Memory Communication Accept Channel Error: No memory to record the Inter-SP communication channel!\n");
        goto malloc_err;
	}

	comm->role = COMM_ISP_ROLE_USER;
	comm->owner_spid = requester_spid;
	comm->user_spid = current_sp->sp_id;
	comm->h_comm = h_comm;
	comm->comm_buf = comm_buf_base;
	comm->comm_buf_size = comm_buf_size;

    comm->buffer_access_err_handler = NULL;
	
	return comm;

malloc_err:
    _isp_sharedmem_close_user(h_comm, comm_buf_base, comm_buf_size);
    return NULL;
}

/// @brief Close an Inter-SecProcess shared memory channel. 
void comm_isp_sharedmem_close(COMM_ISP_SHAREDMEM* comm)
{
	if(!comm)
		return;

    if (comm->role == COMM_ISP_ROLE_OWNER)
		_isp_sharedmem_close_owner(comm);
	else if(comm->role == COMM_ISP_ROLE_USER)
        _isp_sharedmem_close_user(comm->h_comm, comm->comm_buf, comm->comm_buf_size);
	else
        SP_COMM_ERR("Inter-SecProcess Shared Memory Communication Close Error: Invalid Inter-SP communication channel!\n");

	_isp_sharedmem_clear(comm);
}

/// @brief Issue a synchronous call to a SecProcess via messaging provided by SecOS.
/// SecProcess can issue a call only after the previous call is returned.
SAP_STATUS comm_isp_msg_call(
    spid_t target_sp_id, 
    bool req_mailbox_response, 
    uint32_t sub_cmd, 
    void* in_data, 
    size_t in_data_sz,
    void* out_data, 
    size_t* out_data_size
)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SEND_PARAMS* req_params = NULL;
    size_t trimmed_in_data_sz = 0;

    // Sanity checks
    if(_wait_reply_on_msg_id != COMM_ISP_INVALID_MSG_ID)
    {
        SP_COMM_ERR("Inter-SecProcess Message Call error: Previous call has not returned!\n");
        return EINVAL;
    }

    // Create message
    req_params = _comm_isp_create_msg(req_mailbox_response);
	if(!req_params)
    {
        status = EINTERNAL;
        goto out;
    }

    // Limit <in_data_sz> to be <= SECPROCESS_MSG_DATA_SIZE
    trimmed_in_data_sz = min(in_data_sz, SECPROCESS_MSG_DATA_SIZE);

    // Send message
    req_params->target_spid = target_sp_id;
    req_params->data_size = trimmed_in_data_sz;
    req_params->ack_req_id = 0;
    req_params->sub_cmd = sub_cmd;
    req_params->data = (vaddr_t)in_data;

    sos_sys_comm_isp_call(req_params);

    // Block waiting for the response
    if(req_mailbox_response)
    {
        SAP_STATUS resp_status = SUCCESS;
        volatile bool is_responsed = false;

        do
        {
            resp_status = _is_target_sp_responsed(target_sp_id, req_params->req_id, &is_responsed, out_data, out_data_size);
            if(FAIL(resp_status))
            {
                SP_COMM_ERR("Failed to check responses of the target SecProcess!\n");
                return EINTERNAL;
            }
        } while(!is_responsed);

        // Now we do not wait on the request
        _wait_reply_on_msg_id = COMM_ISP_INVALID_MSG_ID;
    }

    // Success
    status = SUCCESS;

out:
    if(req_params)
        _comm_isp_free_msg(req_params);
    return status;
}

/// @brief Send Inter-SecProcess call response to the caller via messaging provided by SecOS.
SAP_STATUS comm_isp_msg_send_response(spid_t target_sp_id, uint32_t req_msg_id, void* in_data, size_t in_data_sz)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SEND_PARAMS* resp_params = NULL;
    size_t trimmed_in_data_sz = 0;

    // Create message
    resp_params = _comm_isp_create_msg(false);
	if(!resp_params)
    {
        status = EINTERNAL;
        goto out;
    }

    // Limit <in_data_sz> to be <= SECPROCESS_MSG_DATA_SIZE
    trimmed_in_data_sz = min(in_data_sz, SECPROCESS_MSG_DATA_SIZE);

    // Send message
    resp_params->target_spid = target_sp_id;
    resp_params->data_size = trimmed_in_data_sz;
    resp_params->ack_req_id = req_msg_id;
    resp_params->sub_cmd = 0;
    resp_params->data = (vaddr_t)in_data;

    sos_sys_comm_isp_send(resp_params);

    // Success
    status = SUCCESS;

out:
    if(resp_params)
        _comm_isp_free_msg(resp_params);
    return status;
}




/********* Management of All <COMM_ISP_SHAREDMEM> Info *********/
static int _all_comm_isps_find_slot(COMM_ISP_SHAREDMEM* comm)
{
	uint32_t i = 0;

	// Find slot
	FOREACH_S(i, ALL_COMM_ISPS_MAX_NUM, ALL_COMM_ISPS_MAX_NUM, 0, 1)
	{
		COMM_ISP_SHAREDMEM* o = _all_comm_isps[i];

		if(o == comm)
		{
			// Found!
			return i;
		}
	}

	return -1;
}

/// @brief Clear all record entries.
void comm_isp_sharedmem_mgr_clear(void)
{
    uint32_t i = 0;

	// Iterate slot
	FOREACH_S(i, ALL_COMM_ISPS_MAX_NUM, ALL_COMM_ISPS_MAX_NUM, 0, 1)
    {
        _all_comm_isps[i] = NULL;
    }
}

/// @brief Register <comm> so the SecProcess can use a SecProcess ID to find it later.
SAP_STATUS comm_isp_sharedmem_register(COMM_ISP_SHAREDMEM* comm)
{
	int slot = 0;

	if(!comm)
        return EINVAL;

	slot = _all_comm_isps_find_slot(NULL);
	if(slot == -1)
		return EINTERNAL;

	_all_comm_isps[slot] = comm;

	return SUCCESS;
}

/// @brief Unregister <comm>.
void comm_isp_sharedmem_unregister(COMM_ISP_SHAREDMEM* comm)
{
	int slot = 0;

	if(!comm)
        return;

	slot = _all_comm_isps_find_slot(comm);
	if(slot == -1)
		return;

	_all_comm_isps[slot] = NULL;
}

/// @brief Return the <COMM_ISP_SHAREDMEM> whose owner or user SecProcess has the given <target_spid>.
COMM_ISP_SHAREDMEM* comm_isp_sharedmem_find(spid_t target_spid)
{
    uint32_t i = 0;

	// Find slot
	FOREACH_S(i, ALL_COMM_ISPS_MAX_NUM, ALL_COMM_ISPS_MAX_NUM, 0, 1)
	{
		COMM_ISP_SHAREDMEM* comm = _all_comm_isps[i];
        if(comm)
        {
            if((comm->owner_spid == target_spid) || (comm->user_spid == target_spid))
                return comm;
        }
        else
            continue;
    }

    return NULL;
}

/// @brief Close all registered Inter-SecProcess shared memory channels.
void comm_isp_sharedmem_unregister_and_close_all(void)
{
    uint32_t i = 0;

	// Iterate slot
	FOREACH_S(i, ALL_COMM_ISPS_MAX_NUM, ALL_COMM_ISPS_MAX_NUM, 0, 1)
    {
        COMM_ISP_SHAREDMEM* comm = _all_comm_isps[i];
        if(comm)
            comm_isp_sharedmem_close(comm);
        else
            continue;
    }

    // Clear all entries in <_all_comm_isps>.
    comm_isp_sharedmem_mgr_clear();
}

/// @brief Close the given registered Inter-SecProcess shared memory channels.
void comm_isp_sharedmem_unregister_and_close(COMM_ISP_SHAREDMEM* comm)
{
    // Prevent double free
	if(_all_comm_isps_find_slot(comm) == -1)
		return;

    comm_isp_sharedmem_unregister(comm);
    comm_isp_sharedmem_close(comm);
}

SAP_STATUS comm_isp_read_buffer(COMM_ISP_SHAREDMEM* comm, void* dest, offset_t offset, size_t size)
{
    vaddr_t vaddr;
    bool dest_perm = false;

    // Check: <comm> and <dest> must be valid
    if(!comm || !dest)
        return EINVAL;

    dest_perm = sos_sys_mm_check_allow_rw((vaddr_t)dest, size);
    if(!dest_perm)
        return EINVAL;

    // Check: <offset> must <= comm->comm_buf_size
    if(offset > comm->comm_buf_size)
        return EINVAL;
    // Check: <offset + size> must <= comm->comm_buf_size
    if((offset + size) > comm->comm_buf_size)
        return EINVAL;

    vaddr = comm->comm_buf + offset;

    if(comm->role == COMM_ISP_ROLE_OWNER)
    {
        // Copy read result
        memcpy(dest, (void*)vaddr, size);
    }
    else if (comm->role == COMM_ISP_ROLE_USER)
    {
        // Try to read the buffer and handle any page fault here
        __libc_lock_acquire(&_comm_isp_accessing_buffer_lock);
        _comm_isp_accessing_buffer = comm;
        comm->buffer_access_err_handler = &&read_buf_err;
        mfence();

        // Copy read result
        memcpy(dest, (void*)vaddr, size);

        //// Hit here if the shared memory still exists. Thus, no #PF happens.
        _comm_isp_accessing_buffer = NULL;
        comm->buffer_access_err_handler = NULL;
        __libc_lock_release(&_comm_isp_accessing_buffer_lock);

        // On success
        return SUCCESS;


    read_buf_err:
        //// Hit here if the shared memory is accidently freed. Thus, #PF happens.
        _comm_isp_accessing_buffer = NULL;
        comm->buffer_access_err_handler = NULL;
        __libc_lock_release(&_comm_isp_accessing_buffer_lock);
        return EFAULT;
    }

    // Invalid <comm->role>
    return EINVAL;
}

SAP_STATUS comm_isp_write_buffer(COMM_ISP_SHAREDMEM* comm, void* src, offset_t offset, size_t size)
{
    vaddr_t vaddr;
    bool src_perm = false;

    // Check: <comm> and <src> must be valid
    if(!comm || !src)
        return EINVAL;

    src_perm = sos_sys_mm_check_allow_rw((vaddr_t)src, size);
    if(!src_perm)
        return EINVAL;

    // Check: <offset> must <= comm->comm_buf_size
    if(offset > comm->comm_buf_size)
        return EINVAL;
    // Check: <offset + size> must <= comm->comm_buf_size
    if((offset + size) > comm->comm_buf_size)
        return EINVAL;

    vaddr = comm->comm_buf + offset;

    if(comm->role == COMM_ISP_ROLE_OWNER)
    {
        // Copy write result
        memcpy((void*)vaddr, src, size);
    }
    else if (comm->role == COMM_ISP_ROLE_USER)
    {
        // Try to read the buffer and handle any page fault here
        __libc_lock_acquire(&_comm_isp_accessing_buffer_lock);
        _comm_isp_accessing_buffer = comm;
        comm->buffer_access_err_handler = &&write_buf_err;
        barrier();

        // Copy write result
        memcpy((void*)vaddr, src, size);

        //// Hit here if the shared memory still exists. Thus, no #PF happens.
        _comm_isp_accessing_buffer = NULL;
        comm->buffer_access_err_handler = NULL;
        __libc_lock_release(&_comm_isp_accessing_buffer_lock);

        // On success
        return SUCCESS;


    write_buf_err:
        //// Hit here if the shared memory is accidently freed. Thus, #PF happens.
        _comm_isp_accessing_buffer = NULL;
        comm->buffer_access_err_handler = NULL;
        __libc_lock_release(&_comm_isp_accessing_buffer_lock);
        return EFAULT;
    }

    // Invalid <comm->role>
    return EINVAL;
}

COMM_ISP_SHAREDMEM* comm_isp_get_active_access_desc(void)
{
    return _comm_isp_accessing_buffer;
}