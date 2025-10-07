// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <palddk.h>
// #include <sap_mm.h>
// #include <pal_usb.h>

#include <sap_headers_libc.h>
#include <sys/secapp_libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc/comm.h>

// External headers in other SecProcess libs
extern COMM_ISP_SHAREDMEM* comm_isp_get_active_access_desc(void);

// The main signal handler entry for SecProcess at ring3
extern void sp_signal_handler(void);

// The global programming entry of SecProcess for 3rd party.
extern void sp_main(SP_UTASK* sp_params);
extern void sp_exit(SP_UTASK* sp_params);
extern void sp_handle_sp_request(SP_MESSAGE* event);
extern bool sp_handle_app_request(struct ipc_msg* msg);
extern void sp_handle_intr(intr_t vector);
extern void sp_handle_exception(void);

SP_UTASK* current_sp = NULL;

/// @brief If the value is 1, then the SecProcess has received signals from untrusted applications to handle. Otherwise,
/// the value is 0.
static uint32_t _app_signal_has_pending = 0;

void sp_handle_app_signal(void)
{

	// Process all messages received from red applications
	sp_comm_app_process_all_app_msgs();
}

static void _sp_msg_update_handle_status(volatile SP_MESSAGE* msg, uint32_t status)
{
	if((status == SAP_EVENT_HANDLE_STATUS_INVALID) || (status == SAP_EVENT_NOT_HANDLED) || 
		(status == SAP_EVENT_HANDLING) || (status == SAP_EVENT_HANDLED))
	{
		msg->header.handled = status;
	}
}

void sp_signal_set_resume_ip(SP_SIGNAL_REENTRY_INFO* sig_r_info, vaddr_t resume_ip)
{
	*sig_r_info->sig_ret_ip = resume_ip;
}

static void _sp_handle_sp_event(SP_SIGNAL_REENTRY_INFO* sig_r_info, SP_MESSAGE* msg)
{
	SAP_DEBUG("[SecProcess:%u] Received event. cmd:%u\n", current_sp->sp_id, msg->header.cmd);
	
	// Set <msg> to "handling"
	_sp_msg_update_handle_status(msg, SAP_EVENT_HANDLING);

	switch(msg->header.cmd)
	{
		case REQ_OBJ_USER_MEM_ACCESS:
		{
			MEMOBJ_USER_ACCESS_INFO* memobj_access_info = (MEMOBJ_USER_ACCESS_INFO*)msg->data;

			// If this SecProcess causes #PF and traps into its handler, then dump the backtrace and exit.
			// if(memobj_access_info->requester == current_sp->sp_id)
			{
				COMM_ISP_SHAREDMEM* active_comm_isp_sharedmem = comm_isp_get_active_access_desc();

				if(active_comm_isp_sharedmem)
				{
					// If the #PF is caused by accidentally termination of IPC shared memory servers (i.e., caused by 
					// <comm_isp_read_buffer> or <comm_isp_write_buffer>), then we should go to its error handling and
					// continue.
					vaddr_t err_handler_vaddr = (vaddr_t)active_comm_isp_sharedmem->buffer_access_err_handler;

					// Execute error handling code after the signal returns
					sp_signal_set_resume_ip(sig_r_info, err_handler_vaddr);
				}
				else
				{
					TRAPPF_MEM_ACCESS_TYPE access_type = memobj_access_info->access_type;
					vaddr_t stack_bottom = memobj_access_info->owner_sp_stack_bottom;
					vaddr_t stack_top = memobj_access_info->owner_sp_stack_top;

					if(access_type == TRAPPED_READ)
						SAP_ERR("Page Fault is caught: Inst 0x%lX reads memory 0x%lX\n", memobj_access_info->trapped_inst, memobj_access_info->access_vaddr);
					else if(access_type == TRAPPED_WRITE)
						SAP_ERR("Page Fault is caught: Inst 0x%lX writes memory 0x%lX\n", memobj_access_info->trapped_inst, memobj_access_info->access_vaddr);
					else if(access_type == TRAPPED_EXECUTE)
						SAP_ERR("Page Fault is caught: Inst 0x%lX executes memory 0x%lX\n", memobj_access_info->trapped_inst, memobj_access_info->access_vaddr);

					_dbg_print_backtrace_select(memobj_access_info->fn_stack_frame_bp, stack_top, stack_bottom);

					sp_handle_exception();

					// Set <msg> to be handled, because the SecProcess will exit.
					_sp_msg_update_handle_status(msg, SAP_EVENT_HANDLED);

					// Exit
					sos_sys_sp_exit(EFAULT);
				}
			}

			break;
		}
		case REQ_MHV_TRAP_MEM_READ:
		case REQ_MHV_TRAP_MEM_WRITE:
		case REQ_MHV_TRAP_PIO_READ: 
		case REQ_MHV_TRAP_PIO_WRITE:
		case REQ_MHV_TRAP_INTR_ACCESS:
		{
			SAP_ERR("Unexpected command %u in the received message, sender is SecProcess (id:%u)!\n", msg->header.cmd, msg->header.sender_id);
			break;
		}
		default:
		{
			// SecProcess specific events
			// VKSTATUS sos_status = SUCCESS;

			// sos_status = sos_sys_sp_scheduling_disable();
			// if(FAIL(sos_status))
			// 	SAP_ERR("[SecProcess:%u] Failed to disable SecProcess scheduling! status:%lu\n", current_sp->sp_id, sos_status);

			sp_handle_sp_request(msg);
			
			// sos_status = sos_sys_sp_scheduling_enable();
			// if(FAIL(sos_status))
			// 	SAP_ERR("[SecProcess:%u] Failed to enable SecProcess scheduling! status:%lu\n", current_sp->sp_id, sos_status);

			SAP_DEBUG("[SecProcess:%u] Handled SecService specific event. cmd:%u\n", current_sp->sp_id, msg->header.cmd);
			break;
		}
	}

	SAP_DEBUG("[SecProcess:%u] Handled event. cmd:%u\n", current_sp->sp_id, msg->header.cmd);
	// Set <msg> to be handled
	_sp_msg_update_handle_status(msg, SAP_EVENT_HANDLED);
}


static void _sp_handle_sp_events(SP_SIGNAL_REENTRY_INFO* sig_r_info)
{
	uint32_t i = 0;

	FOREACH_S(i, SECAPP_MAX_NUM_EVENTS, SECAPP_MAX_NUM_EVENTS, 0, 1)
	{
		SP_MESSAGE* event = &current_sp->events[i];

		if(event->header.handled == SAP_EVENT_HANDLING)
		{
			SAP_ERR("Error: Found interrupted SecProcess Event (from caller SecOS/SecProcess ID:%u)!\n", event->header.sender_id);
		}

		if(event->header.handled == SAP_EVENT_NOT_HANDLED)
			_sp_handle_sp_event(sig_r_info, event);
	}
}

// Finialize the allocated resources before terminating the SecProcess
static void _do_sp_exit(SP_UTASK *sp_params)
{
	(void)sp_params;
	
	sp_mm_fini();

	pal_mm_destroy_pool();
	libc_fini();

	// [TODO] We may want to use trap model to detect SecProcess exit
	sos_sys_sp_exit(SUCCESS);
}




/************ Public Functions **********/
extern void sp_entry(SP_UTASK* sp_params) __attribute__((section(".text.entry"))) __attribute__((cdecl));

/// @brief SecOS signals SecApps (SecProcesses that are schedulable) with interrupts enabled. So this function may be
/// preempted and re-entered at any time. Thus, this function must be thread-safe and is better to be fast. For example, 
/// this function and its subfunctions must avoid spinlocks to prevent deadlocks caused by re-entry. SecProcesses
/// should cache signals that take time to handle, or it may lead to race conditions.
///
/// [NOTE] This function has different assumptions from the <sp_signal_handler_entry> function for SecServices. The 
/// signal handler of SecServices always have interrupt disabled and SecServices are not schedulable. Thus, those 
/// handlers must handle all signals, not just the thread-safe or fast ones. Otherwise, SecOS won't schedule SecServices
/// to handle slow ones.
///
/// [NOTE] _cdecl is needed for any functions called in assembly code, because assembly code need to know the calling 
/// conventions.
/// @param sig_ret_ip 
/// @param is_intr 
/// @param info1 
extern void sp_signal_handler_entry(vaddr_t* sig_ret_ip, ulong_t is_intr, ulong_t info1) __attribute__((cdecl));

// [TODO] We should move this function into glibc, it acts as _tmain
void arch_sp_entry(SP_UTASK* sp_params)
{
	SAP_STATUS status = SUCCESS;

	current_sp = sp_params;

	// We can safely use libc functions after <libc_init>
	libc_init((uint32_t)sp_params->sp_id, (void*)sp_params->mem_pool, 
		(uint32_t)sp_params->mem_pool_size, 
		(void*)sp_params->sp_app_comm, (uint32_t)sp_params->sp_app_comm_size,
		(void*)sp_params->sharedmem_start, (uint32_t)sp_params->sharedmem_size
	);

	// Allocate DMA & memory pool 
	if (pal_mm_create_pool(sp_params)) 
	{
		SAP_ERR("Init SecProcess Error: Failed to create DMA pool!\n");
		goto mm_pool_error;
	}

	// Init Remappable Memory Region
	status = sp_mm_init(sp_params->remappable_base);
	if(FAIL(status))
	{
		SAP_ERR("Init SecProcess Error: Failed to init Remappable Memory Region! status:%lu\n", status);
		goto sp_mm_init_error;
	}

	sos_sys_sp_register_sig_handler((vaddr_t)sp_signal_handler);
	
	sp_main(sp_params);

	sp_mm_fini();


sp_mm_init_error:
	pal_mm_destroy_pool();
	libc_fini();
mm_pool_error:
	// Exit
	// [TODO] We may want to use trap model to detect SecProcess exit
	sos_sys_sp_exit(status);
}

/// @brief SecOS signals SecApps (SecProcesses that are schedulable) with interrupts enabled. So this function may be
/// preempted and re-entered at any time. Thus, this function must be thread-safe and is better to be fast. For example, 
/// this function and its subfunctions must avoid spinlocks to prevent deadlocks caused by re-entry. SecProcesses
/// should cache signals that take time to handle, or it may lead to race conditions.
///
/// [NOTE] This function has different assumptions from the <sp_signal_handler_entry> function for SecServices. The 
/// signal handler of SecServices always have interrupt disabled and SecServices are not schedulable. Thus, those 
/// handlers must handle all signals, not just the thread-safe or fast ones. Otherwise, SecOS won't schedule SecServices
/// to handle slow ones.
/// @param sig_ret_ip 
/// @param is_intr 
/// @param info1 
void sp_signal_handler_entry(vaddr_t* sig_ret_ip, ulong_t is_intr, ulong_t info1)
{
	SP_SIGNAL_REENTRY_INFO sig_r_info = {0};

	// Initialize <sig_r_info>
	sig_r_info.sig_ret_ip = sig_ret_ip;

	// Disable interrupts to execute in serial
	// sos_sys_disable_intr();

	//if(!is_intr)
		// printf("[Superymk] <sp_signal_handler_entry - Entry> SecProcess:%u, is_intr:%u, info1:%u\n", current_sp->sp_id, is_intr, info1);

	if(is_intr)
	{
		intr_t vector = info1;
		sp_handle_intr(vector);
	}
	else if(!is_intr && info1 == SIGNAL_EVENT_LIST_MAINTANCE)
		_sp_handle_sp_events(&sig_r_info);
	else if(!is_intr && info1 == SIGNAL_CALLED_BY_SECOS_SECPROCESS)
		_sp_handle_sp_events(&sig_r_info);
	else if(!is_intr && info1 == SIGNAL_REQEXIT_BY_APP)
	{
		sp_exit(current_sp);
        _do_sp_exit(current_sp);
	}
	else if(!is_intr && info1 == SIGNAL_CALLED_BY_APP)
	{
		// Handle signals by untrusted applications in the SecApp's main loop (if any), not in the signal handler. It is
		// because SecProcess-App communications uses ring buffer and spinlocks, while <sp_signal_handler_entry> and its
		// subfunctions should avoid spinlocks.
		//
		// SecApps are required to call <sp_poll_and_handle_app_signals> to handle untrusted application's signals timely.
		sp_x86_atomic_set(&_app_signal_has_pending, 1);
	}
		

	// Enable interrupts
	// sos_sys_enable_intr();
}

void sp_poll_and_handle_app_signals(void)
{
	uint32_t app_signal_pending_val = 0;

	app_signal_pending_val = sp_x86_atomic_get(&_app_signal_has_pending);
	if(app_signal_pending_val)
	{
		sp_x86_atomic_set(&_app_signal_has_pending, 0);
		sp_handle_app_signal();
	}
}


// void do_handle_signal(sig_no_t sig_no) 
// {
// 	//printf("<do_handle_signal> Signal Number:0x%X\n", sig_no);
// 	pal_handle_signal(sig_no);
// }