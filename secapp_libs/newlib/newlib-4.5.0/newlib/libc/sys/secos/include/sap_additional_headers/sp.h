// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SAP_SECPROCESS_H
#define SAP_SECPROCESS_H


#ifndef __ASSEMBLY__




/// @brief The ID of the SecProcess.
/// [NOTE] <spid_t> must be 32-bit long. This is the assumption of driver IDs in IOSM-USB2 and IOSM-FB.
typedef uint32_t 	spid_t; 

/// @brief Reentrant information for SecProcess signals
typedef struct
{
	vaddr_t* sig_ret_ip; // The resumed instruction pointer after the current signal returns
} SP_SIGNAL_REENTRY_INFO;

/// @brief Execute the code at the Instruction Pointer <resume_ip> after the signal returns
/// @param sig_r_info 
/// @param resume_ip 
extern void sp_signal_set_resume_ip(SP_SIGNAL_REENTRY_INFO* sig_r_info, vaddr_t resume_ip);

/// @brief Check if the current SecProcess has received signals from untrusted applications to handle. If so, handle all
/// of them. SecApps are required to call this function frequently to handle untrusted application's signals on time.
/// @param  
extern void sp_poll_and_handle_app_signals(void);

/// @brief Yield the remaining CPU slice of the current SecProcess.
extern void sp_yield(void);

#endif // __ASSEMBLY__
#endif // SAP_SECPROCESS_H