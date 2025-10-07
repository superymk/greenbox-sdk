// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef TOOL_HYPERCALL_H
#define TOOL_HYPERCALL_H

#ifndef __ASSEMBLY__
#include <os/os_headers.h>
#include <app_types.h>
#include <stdio.h>

#include <secbase_hypercalls.h>
#include <secbase_errno.h>

extern ulong_t hypercall_noarg(ulong_t arg_ax);

extern ulong_t hypercall_1arg(ulong_t arg_ax, ulong_t arg_cx);

extern ulong_t hypercall_2args(ulong_t arg_ax, ulong_t arg_cx, ulong_t arg_dx);

extern ulong_t hypercall_3args(ulong_t arg_ax, ulong_t arg_cx, ulong_t arg_dx,
									  ulong_t arg_si);

extern ulong_t hypercall_ret4(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx,
									 ulong_t *arg_si, ulong_t *arg_di);

extern ulong_t hypercall_ret1(ulong_t arg_ax, ulong_t *arg_cx);
extern ulong_t hypercall_ret2(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx);
extern ulong_t hypercall_ret3(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx, ulong_t *arg_si);

#endif // __ASSEMBLY__
#endif // TOOL_HYPERCALL_H