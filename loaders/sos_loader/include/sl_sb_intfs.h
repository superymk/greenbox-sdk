/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#ifndef SOS_LD_SECBASE_INTERFACE_H
#define SOS_LD_SECBASE_INTERFACE_H

#include <secbase_hypercalls.h>

#ifndef __ASSEMBLY__

//! Resume SecOS's execution
//!
//! [NOTE] The internal hypercall of this function must be issued from red application (not red OS kernel). Otherwise,
//! SecAgent got blocked due to SecOS preempts red OS kernel's execution. Because red OS kernels' assume their kernel 
//! modules handle ioctls fast, red OS will complain SecAgent for being busy for a long time, and even throw exceptions.
extern void sos_resume(void);

extern void hypercall_test(ulong_t arg1);
extern void hypercall_test2(ulong_t arg1);
extern void hypercall_test3(ulong_t arg1);
extern int hypercall_read_dmesg(uint8_t* dmesg_buf, size_t* out_size);

#endif // __ASSEMBLY__
#endif // SOS_LD_SECBASE_INTERFACE_H