// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/sched.h> //find_task_by_pid_type
#include <linux/slab.h>

#include <linux/version.h>
// #include <asm/siginfo.h>	//siginfo

// #if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
// 	#include <linux/sched/signal.h> // send_sig_info
// 	typedef struct kernel_siginfo LINUX_STRUCT_SIGINFO;
// #else
// 	#include <linux/sched.h> // send_sig_info
// 	typedef struct siginfo LINUX_STRUCT_SIGINFO;
// #endif

#include "linux_intr.h"

static struct myprivate *myp = NULL;
static int sos_os_irq = INVALID_IRQ;
static intr_t sos_os_intr = INVALID_INTR;
static lapic_id_t sos_os_intr_os_lapic_id = INVALID_LAPIC_ID;

struct myprivate
{
	// int scancode;
	struct work_struct task;
};

void linux_intr_trigger_interrupt(uint32_t vector)
{
	switch (vector)
	{
	case 0x20:
		asm volatile("int $0x20;"
					 :
					 :
					 :);
		return;
	case 0x21:
		asm volatile("int $0x21;"
					 :
					 :
					 :);
		return;
	case 0x22:
		asm volatile("int $0x22;"
					 :
					 :
					 :);
		return;
	case 0x23:
		asm volatile("int $0x23;"
					 :
					 :
					 :);
		return;
	case 0x24:
		asm volatile("int $0x24;"
					 :
					 :
					 :);
		return;
	case 0x25:
		asm volatile("int $0x25;"
					 :
					 :
					 :);
		return;
	case 0x26:
		asm volatile("int $0x26;"
					 :
					 :
					 :);
		return;
	case 0x27:
		asm volatile("int $0x27;"
					 :
					 :
					 :);
		return;
	case 0x28:
		asm volatile("int $0x28;"
					 :
					 :
					 :);
		return;
	case 0x29:
		asm volatile("int $0x29;"
					 :
					 :
					 :);
		return;
	case 0x2A:
		asm volatile("int $0x2A;"
					 :
					 :
					 :);
		return;
	case 0x2B:
		asm volatile("int $0x2B;"
					 :
					 :
					 :);
		return;
	case 0x2C:
		asm volatile("int $0x2C;"
					 :
					 :
					 :);
		return;
	case 0x2D:
		asm volatile("int $0x2D;"
					 :
					 :
					 :);
		return;
	case 0x2E:
		asm volatile("int $0x2E;"
					 :
					 :
					 :);
		return;
	case 0x2F:
		asm volatile("int $0x2F;"
					 :
					 :
					 :);
		return;

	case 0x30:
		asm volatile("int $0x30;"
					 :
					 :
					 :);
		return;
	case 0x31:
		asm volatile("int $0x31;"
					 :
					 :
					 :);
		return;
	case 0x32:
		asm volatile("int $0x32;"
					 :
					 :
					 :);
		return;
	case 0x33:
		asm volatile("int $0x33;"
					 :
					 :
					 :);
		return;
	case 0x34:
		asm volatile("int $0x34;"
					 :
					 :
					 :);
		return;
	case 0x35:
		asm volatile("int $0x35;"
					 :
					 :
					 :);
		return;
	case 0x36:
		asm volatile("int $0x36;"
					 :
					 :
					 :);
		return;
	case 0x37:
		asm volatile("int $0x37;"
					 :
					 :
					 :);
		return;
	case 0x38:
		asm volatile("int $0x38;"
					 :
					 :
					 :);
		return;
	case 0x39:
		asm volatile("int $0x39;"
					 :
					 :
					 :);
		return;
	case 0x3A:
		asm volatile("int $0x3A;"
					 :
					 :
					 :);
		return;
	case 0x3B:
		asm volatile("int $0x3B;"
					 :
					 :
					 :);
		return;
	case 0x3C:
		asm volatile("int $0x3C;"
					 :
					 :
					 :);
		return;
	case 0x3D:
		asm volatile("int $0x3D;"
					 :
					 :
					 :);
		return;
	case 0x3E:
		asm volatile("int $0x3E;"
					 :
					 :
					 :);
		return;
	case 0x3F:
		asm volatile("int $0x3F;"
					 :
					 :
					 :);
		return;

	case 0x40:
		asm volatile("int $0x40;"
					 :
					 :
					 :);
		return;
	case 0x41:
		asm volatile("int $0x41;"
					 :
					 :
					 :);
		return;
	case 0x42:
		asm volatile("int $0x42;"
					 :
					 :
					 :);
		return;
	case 0x43:
		asm volatile("int $0x43;"
					 :
					 :
					 :);
		return;
	case 0x44:
		asm volatile("int $0x44;"
					 :
					 :
					 :);
		return;
	case 0x45:
		asm volatile("int $0x45;"
					 :
					 :
					 :);
		return;
	case 0x46:
		asm volatile("int $0x46;"
					 :
					 :
					 :);
		return;
	case 0x47:
		asm volatile("int $0x47;"
					 :
					 :
					 :);
		return;
	case 0x48:
		asm volatile("int $0x48;"
					 :
					 :
					 :);
		return;
	case 0x49:
		asm volatile("int $0x49;"
					 :
					 :
					 :);
		return;
	case 0x4A:
		asm volatile("int $0x4A;"
					 :
					 :
					 :);
		return;
	case 0x4B:
		asm volatile("int $0x4B;"
					 :
					 :
					 :);
		return;
	case 0x4C:
		asm volatile("int $0x4C;"
					 :
					 :
					 :);
		return;
	case 0x4D:
		asm volatile("int $0x4D;"
					 :
					 :
					 :);
		return;
	case 0x4E:
		asm volatile("int $0x4E;"
					 :
					 :
					 :);
		return;
	case 0x4F:
		asm volatile("int $0x4F;"
					 :
					 :
					 :);
		return;

	case 0x50:
		asm volatile("int $0x50;"
					 :
					 :
					 :);
		return;
	case 0x51:
		asm volatile("int $0x51;"
					 :
					 :
					 :);
		return;
	case 0x52:
		asm volatile("int $0x52;"
					 :
					 :
					 :);
		return;
	case 0x53:
		asm volatile("int $0x53;"
					 :
					 :
					 :);
		return;
	case 0x54:
		asm volatile("int $0x54;"
					 :
					 :
					 :);
		return;
	case 0x55:
		asm volatile("int $0x55;"
					 :
					 :
					 :);
		return;
	case 0x56:
		asm volatile("int $0x56;"
					 :
					 :
					 :);
		return;
	case 0x57:
		asm volatile("int $0x57;"
					 :
					 :
					 :);
		return;
	case 0x58:
		asm volatile("int $0x58;"
					 :
					 :
					 :);
		return;
	case 0x59:
		asm volatile("int $0x59;"
					 :
					 :
					 :);
		return;
	case 0x5A:
		asm volatile("int $0x5A;"
					 :
					 :
					 :);
		return;
	case 0x5B:
		asm volatile("int $0x5B;"
					 :
					 :
					 :);
		return;
	case 0x5C:
		asm volatile("int $0x5C;"
					 :
					 :
					 :);
		return;
	case 0x5D:
		asm volatile("int $0x5D;"
					 :
					 :
					 :);
		return;
	case 0x5E:
		asm volatile("int $0x5E;"
					 :
					 :
					 :);
		return;
	case 0x5F:
		asm volatile("int $0x5F;"
					 :
					 :
					 :);
		return;

	case 0x60:
		asm volatile("int $0x60;"
					 :
					 :
					 :);
		return;
	case 0x61:
		asm volatile("int $0x61;"
					 :
					 :
					 :);
		return;
	case 0x62:
		asm volatile("int $0x62;"
					 :
					 :
					 :);
		return;
	case 0x63:
		asm volatile("int $0x63;"
					 :
					 :
					 :);
		return;
	case 0x64:
		asm volatile("int $0x64;"
					 :
					 :
					 :);
		return;
	case 0x65:
		asm volatile("int $0x65;"
					 :
					 :
					 :);
		return;
	case 0x66:
		asm volatile("int $0x66;"
					 :
					 :
					 :);
		return;
	case 0x67:
		asm volatile("int $0x67;"
					 :
					 :
					 :);
		return;
	case 0x68:
		asm volatile("int $0x68;"
					 :
					 :
					 :);
		return;
	case 0x69:
		asm volatile("int $0x69;"
					 :
					 :
					 :);
		return;
	case 0x6A:
		asm volatile("int $0x6A;"
					 :
					 :
					 :);
		return;
	case 0x6B:
		asm volatile("int $0x6B;"
					 :
					 :
					 :);
		return;
	case 0x6C:
		asm volatile("int $0x6C;"
					 :
					 :
					 :);
		return;
	case 0x6D:
		asm volatile("int $0x6D;"
					 :
					 :
					 :);
		return;
	case 0x6E:
		asm volatile("int $0x6E;"
					 :
					 :
					 :);
		return;
	case 0x6F:
		asm volatile("int $0x6F;"
					 :
					 :
					 :);
		return;

	case 0x70:
		asm volatile("int $0x70;"
					 :
					 :
					 :);
		return;
	case 0x71:
		asm volatile("int $0x71;"
					 :
					 :
					 :);
		return;
	case 0x72:
		asm volatile("int $0x72;"
					 :
					 :
					 :);
		return;
	case 0x73:
		asm volatile("int $0x73;"
					 :
					 :
					 :);
		return;
	case 0x74:
		asm volatile("int $0x74;"
					 :
					 :
					 :);
		return;
	case 0x75:
		asm volatile("int $0x75;"
					 :
					 :
					 :);
		return;
	case 0x76:
		asm volatile("int $0x76;"
					 :
					 :
					 :);
		return;
	case 0x77:
		asm volatile("int $0x77;"
					 :
					 :
					 :);
		return;
	case 0x78:
		asm volatile("int $0x78;"
					 :
					 :
					 :);
		return;
	case 0x79:
		asm volatile("int $0x79;"
					 :
					 :
					 :);
		return;
	case 0x7A:
		asm volatile("int $0x7A;"
					 :
					 :
					 :);
		return;
	case 0x7B:
		asm volatile("int $0x7B;"
					 :
					 :
					 :);
		return;
	case 0x7C:
		asm volatile("int $0x7C;"
					 :
					 :
					 :);
		return;
	case 0x7D:
		asm volatile("int $0x7D;"
					 :
					 :
					 :);
		return;
	case 0x7E:
		asm volatile("int $0x7E;"
					 :
					 :
					 :);
		return;
	case 0x7F:
		asm volatile("int $0x7F;"
					 :
					 :
					 :);
		return;

	case 0x80:
		asm volatile("int $0x80;"
					 :
					 :
					 :);
		return;
	case 0x81:
		asm volatile("int $0x81;"
					 :
					 :
					 :);
		return;
	case 0x82:
		asm volatile("int $0x82;"
					 :
					 :
					 :);
		return;
	case 0x83:
		asm volatile("int $0x83;"
					 :
					 :
					 :);
		return;
	case 0x84:
		asm volatile("int $0x84;"
					 :
					 :
					 :);
		return;
	case 0x85:
		asm volatile("int $0x85;"
					 :
					 :
					 :);
		return;
	case 0x86:
		asm volatile("int $0x86;"
					 :
					 :
					 :);
		return;
	case 0x87:
		asm volatile("int $0x87;"
					 :
					 :
					 :);
		return;
	case 0x88:
		asm volatile("int $0x88;"
					 :
					 :
					 :);
		return;
	case 0x89:
		asm volatile("int $0x89;"
					 :
					 :
					 :);
		return;
	case 0x8A:
		asm volatile("int $0x8A;"
					 :
					 :
					 :);
		return;
	case 0x8B:
		asm volatile("int $0x8B;"
					 :
					 :
					 :);
		return;
	case 0x8C:
		asm volatile("int $0x8C;"
					 :
					 :
					 :);
		return;
	case 0x8D:
		asm volatile("int $0x8D;"
					 :
					 :
					 :);
		return;
	case 0x8E:
		asm volatile("int $0x8E;"
					 :
					 :
					 :);
		return;
	case 0x8F:
		asm volatile("int $0x8F;"
					 :
					 :
					 :);
		return;

	case 0x90:
		asm volatile("int $0x90;"
					 :
					 :
					 :);
		return;
	case 0x91:
		asm volatile("int $0x91;"
					 :
					 :
					 :);
		return;
	case 0x92:
		asm volatile("int $0x92;"
					 :
					 :
					 :);
		return;
	case 0x93:
		asm volatile("int $0x93;"
					 :
					 :
					 :);
		return;
	case 0x94:
		asm volatile("int $0x94;"
					 :
					 :
					 :);
		return;
	case 0x95:
		asm volatile("int $0x95;"
					 :
					 :
					 :);
		return;
	case 0x96:
		asm volatile("int $0x96;"
					 :
					 :
					 :);
		return;
	case 0x97:
		asm volatile("int $0x97;"
					 :
					 :
					 :);
		return;
	case 0x98:
		asm volatile("int $0x98;"
					 :
					 :
					 :);
		return;
	case 0x99:
		asm volatile("int $0x99;"
					 :
					 :
					 :);
		return;
	case 0x9A:
		asm volatile("int $0x9A;"
					 :
					 :
					 :);
		return;
	case 0x9B:
		asm volatile("int $0x9B;"
					 :
					 :
					 :);
		return;
	case 0x9C:
		asm volatile("int $0x9C;"
					 :
					 :
					 :);
		return;
	case 0x9D:
		asm volatile("int $0x9D;"
					 :
					 :
					 :);
		return;
	case 0x9E:
		asm volatile("int $0x9E;"
					 :
					 :
					 :);
		return;
	case 0x9F:
		asm volatile("int $0x9F;"
					 :
					 :
					 :);
		return;

	case 0xA0:
		asm volatile("int $0xA0;"
					 :
					 :
					 :);
		return;
	case 0xA1:
		asm volatile("int $0xA1;"
					 :
					 :
					 :);
		return;
	case 0xA2:
		asm volatile("int $0xA2;"
					 :
					 :
					 :);
		return;
	case 0xA3:
		asm volatile("int $0xA3;"
					 :
					 :
					 :);
		return;
	case 0xA4:
		asm volatile("int $0xA4;"
					 :
					 :
					 :);
		return;
	case 0xA5:
		asm volatile("int $0xA5;"
					 :
					 :
					 :);
		return;
	case 0xA6:
		asm volatile("int $0xA6;"
					 :
					 :
					 :);
		return;
	case 0xA7:
		asm volatile("int $0xA7;"
					 :
					 :
					 :);
		return;
	case 0xA8:
		asm volatile("int $0xA8;"
					 :
					 :
					 :);
		return;
	case 0xA9:
		asm volatile("int $0xA9;"
					 :
					 :
					 :);
		return;
	case 0xAA:
		asm volatile("int $0xAA;"
					 :
					 :
					 :);
		return;
	case 0xAB:
		asm volatile("int $0xAB;"
					 :
					 :
					 :);
		return;
	case 0xAC:
		asm volatile("int $0xAC;"
					 :
					 :
					 :);
		return;
	case 0xAD:
		asm volatile("int $0xAD;"
					 :
					 :
					 :);
		return;
	case 0xAE:
		asm volatile("int $0xAE;"
					 :
					 :
					 :);
		return;
	case 0xAF:
		asm volatile("int $0xAF;"
					 :
					 :
					 :);
		return;

	case 0xB0:
		asm volatile("int $0xB0;"
					 :
					 :
					 :);
		return;
	case 0xB1:
		asm volatile("int $0xB1;"
					 :
					 :
					 :);
		return;
	case 0xB2:
		asm volatile("int $0xB2;"
					 :
					 :
					 :);
		return;
	case 0xB3:
		asm volatile("int $0xB3;"
					 :
					 :
					 :);
		return;
	case 0xB4:
		asm volatile("int $0xB4;"
					 :
					 :
					 :);
		return;
	case 0xB5:
		asm volatile("int $0xB5;"
					 :
					 :
					 :);
		return;
	case 0xB6:
		asm volatile("int $0xB6;"
					 :
					 :
					 :);
		return;
	case 0xB7:
		asm volatile("int $0xB7;"
					 :
					 :
					 :);
		return;
	case 0xB8:
		asm volatile("int $0xB8;"
					 :
					 :
					 :);
		return;
	case 0xB9:
		asm volatile("int $0xB9;"
					 :
					 :
					 :);
		return;
	case 0xBA:
		asm volatile("int $0xBA;"
					 :
					 :
					 :);
		return;
	case 0xBB:
		asm volatile("int $0xBB;"
					 :
					 :
					 :);
		return;
	case 0xBC:
		asm volatile("int $0xBC;"
					 :
					 :
					 :);
		return;
	case 0xBD:
		asm volatile("int $0xBD;"
					 :
					 :
					 :);
		return;
	case 0xBE:
		asm volatile("int $0xBE;"
					 :
					 :
					 :);
		return;
	case 0xBF:
		asm volatile("int $0xBF;"
					 :
					 :
					 :);
		return;

	case 0xC0:
		asm volatile("int $0xC0;"
					 :
					 :
					 :);
		return;
	case 0xC1:
		asm volatile("int $0xC1;"
					 :
					 :
					 :);
		return;
	case 0xC2:
		asm volatile("int $0xC2;"
					 :
					 :
					 :);
		return;
	case 0xC3:
		asm volatile("int $0xC3;"
					 :
					 :
					 :);
		return;
	case 0xC4:
		asm volatile("int $0xC4;"
					 :
					 :
					 :);
		return;
	case 0xC5:
		asm volatile("int $0xC5;"
					 :
					 :
					 :);
		return;
	case 0xC6:
		asm volatile("int $0xC6;"
					 :
					 :
					 :);
		return;
	case 0xC7:
		asm volatile("int $0xC7;"
					 :
					 :
					 :);
		return;
	case 0xC8:
		asm volatile("int $0xC8;"
					 :
					 :
					 :);
		return;
	case 0xC9:
		asm volatile("int $0xC9;"
					 :
					 :
					 :);
		return;
	case 0xCA:
		asm volatile("int $0xCA;"
					 :
					 :
					 :);
		return;
	case 0xCB:
		asm volatile("int $0xCB;"
					 :
					 :
					 :);
		return;
	case 0xCC:
		asm volatile("int $0xCC;"
					 :
					 :
					 :);
		return;
	case 0xCD:
		asm volatile("int $0xCD;"
					 :
					 :
					 :);
		return;
	case 0xCE:
		asm volatile("int $0xCE;"
					 :
					 :
					 :);
		return;
	case 0xCF:
		asm volatile("int $0xCF;"
					 :
					 :
					 :);
		return;

	case 0xD0:
		asm volatile("int $0xD0;"
					 :
					 :
					 :);
		return;
	case 0xD1:
		asm volatile("int $0xD1;"
					 :
					 :
					 :);
		return;
	case 0xD2:
		asm volatile("int $0xD2;"
					 :
					 :
					 :);
		return;
	case 0xD3:
		asm volatile("int $0xD3;"
					 :
					 :
					 :);
		return;
	case 0xD4:
		asm volatile("int $0xD4;"
					 :
					 :
					 :);
		return;
	case 0xD5:
		asm volatile("int $0xD5;"
					 :
					 :
					 :);
		return;
	case 0xD6:
		asm volatile("int $0xD6;"
					 :
					 :
					 :);
		return;
	case 0xD7:
		asm volatile("int $0xD7;"
					 :
					 :
					 :);
		return;
	case 0xD8:
		asm volatile("int $0xD8;"
					 :
					 :
					 :);
		return;
	case 0xD9:
		asm volatile("int $0xD9;"
					 :
					 :
					 :);
		return;
	case 0xDA:
		asm volatile("int $0xDA;"
					 :
					 :
					 :);
		return;
	case 0xDB:
		asm volatile("int $0xDB;"
					 :
					 :
					 :);
		return;
	case 0xDC:
		asm volatile("int $0xDC;"
					 :
					 :
					 :);
		return;
	case 0xDD:
		asm volatile("int $0xDD;"
					 :
					 :
					 :);
		return;
	case 0xDE:
		asm volatile("int $0xDE;"
					 :
					 :
					 :);
		return;
	case 0xDF:
		asm volatile("int $0xDF;"
					 :
					 :
					 :);
		return;

	case 0xE0:
		asm volatile("int $0xE0;"
					 :
					 :
					 :);
		return;
	case 0xE1:
		asm volatile("int $0xE1;"
					 :
					 :
					 :);
		return;
	case 0xE2:
		asm volatile("int $0xE2;"
					 :
					 :
					 :);
		return;
	case 0xE3:
		asm volatile("int $0xE3;"
					 :
					 :
					 :);
		return;
	case 0xE4:
		asm volatile("int $0xE4;"
					 :
					 :
					 :);
		return;
	case 0xE5:
		asm volatile("int $0xE5;"
					 :
					 :
					 :);
		return;
	case 0xE6:
		asm volatile("int $0xE6;"
					 :
					 :
					 :);
		return;
	case 0xE7:
		asm volatile("int $0xE7;"
					 :
					 :
					 :);
		return;
	case 0xE8:
		asm volatile("int $0xE8;"
					 :
					 :
					 :);
		return;
	case 0xE9:
		asm volatile("int $0xE9;"
					 :
					 :
					 :);
		return;
	case 0xEA:
		asm volatile("int $0xEA;"
					 :
					 :
					 :);
		return;
	case 0xEB:
		asm volatile("int $0xEB;"
					 :
					 :
					 :);
		return;
	case 0xEC:
		asm volatile("int $0xEC;"
					 :
					 :
					 :);
		return;
	case 0xED:
		asm volatile("int $0xED;"
					 :
					 :
					 :);
		return;
	case 0xEE:
		asm volatile("int $0xEE;"
					 :
					 :
					 :);
		return;
	case 0xEF:
		asm volatile("int $0xEF;"
					 :
					 :
					 :);
		return;

	case 0xF0:
		asm volatile("int $0xF0;"
					 :
					 :
					 :);
		return;
	case 0xF1:
		asm volatile("int $0xF1;"
					 :
					 :
					 :);
		return;
	case 0xF2:
		asm volatile("int $0xF2;"
					 :
					 :
					 :);
		return;
	case 0xF3:
		asm volatile("int $0xF3;"
					 :
					 :
					 :);
		return;
	case 0xF4:
		asm volatile("int $0xF4;"
					 :
					 :
					 :);
		return;
	case 0xF5:
		asm volatile("int $0xF5;"
					 :
					 :
					 :);
		return;
	case 0xF6:
		asm volatile("int $0xF6;"
					 :
					 :
					 :);
		return;
	case 0xF7:
		asm volatile("int $0xF7;"
					 :
					 :
					 :);
		return;
	case 0xF8:
		asm volatile("int $0xF8;"
					 :
					 :
					 :);
		return;
	case 0xF9:
		asm volatile("int $0xF9;"
					 :
					 :
					 :);
		return;
	case 0xFA:
		asm volatile("int $0xFA;"
					 :
					 :
					 :);
		return;
	case 0xFB:
		asm volatile("int $0xFB;"
					 :
					 :
					 :);
		return;
	case 0xFC:
		asm volatile("int $0xFC;"
					 :
					 :
					 :);
		return;
	case 0xFD:
		asm volatile("int $0xFD;"
					 :
					 :
					 :);
		return;
	case 0xFE:
		asm volatile("int $0xFE;"
					 :
					 :
					 :);
		return;
	case 0xFF:
		asm volatile("int $0xFF;"
					 :
					 :
					 :);
		return;

	default:
		return;
	}
}

void linux_intr_local_irq_enable(void)
{
	local_irq_enable();
}

void linux_intr_local_irq_disable(void)
{
	local_irq_disable();
}

/*
 * This will get called by the kernel as soon as it's safe
 * to do everything normally allowed by kernel modules.
 */
static void irqhandler_bh(struct work_struct *taskp)
{
	// struct myprivate *myp = container_of(taskp, struct myprivate, task);
	/*printk(KERN_INFO "Scan Code %x %s.\n",
		   (int)*((char *)scancode) & 0x7F,
		   *((char *)scancode) & 0x80 ? "Released" : "Pressed");*/

	SACOMM_DEBUG("<comm_sos_handle_queued_msgs> start!\n");
	comm_sos_handle_queued_msgs();
	SACOMM_DEBUG("<comm_sos_handle_queued_msgs> finished!\n");
}

/*
 * This function services keyboard interrupts. It reads the relevant
 * information from the keyboard and then puts the non time critical
 * part into the work queue. This will be run when the kernel considers it safe.
 */
static irqreturn_t irqhandler_th(int irq, void *dev_id)
{
	struct myprivate *myp = (struct myprivate *)dev_id;

	schedule_work(&myp->task);

	// [TODO] It is a shared interrupt. So we should consider about the IRQ_NONE case.
	return IRQ_HANDLED;
}

// #include <linux/of_platform.h>

static OSSTATUS _register_irq(int irq)
{
	int ret;

	ret = request_irq(irq,
					irqhandler_th, // our handler
					// IRQF_NOBALANCING | IRQF_PERCPU | IRQF_NO_THREAD,
					IRQF_NOBALANCING | IRQF_NO_THREAD,
					"secagent",
					myp);

	if (ret != 0)
		return E_INTERNAL_ERROR;

	return OS_SUCCESS;
}

static OSSTATUS _linux_intr_register(int irq)
{
	myp = kmalloc(sizeof(struct myprivate), GFP_KERNEL);

	INIT_WORK(&myp->task, irqhandler_bh);

	return _register_irq(irq);
}

// clean up OS interrupt helper functions
static void _linux_intr_unregister(int irq)
{
	/*
	 * This is only here for completeness. It's totally irrelevant, since
	 * we don't have a way to restore the normal keyboard interrupt so the
	 * computer is completely useless and has to be rebooted.
	 */
	free_irq(irq, myp);
	if (myp)
	{
		kfree(myp);
		myp = NULL;
	}
}

// [TODO-Important][Issue 105] Adhoc IRQ start search number for Linux
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
	#define IRQ_SEARCH_START	(5)		// Ugly hack: if we registered a handler for IRQ 2, the kernel may not bind it 
						// to any core. So we do not want to handle a low number IRQ. 5 is just an ugly magic number 
#else
	#define IRQ_SEARCH_START	(19)
#endif


static int _linux_find_sos_os_irq(void)
{
	OSSTATUS status = OS_SUCCESS;
	int irq = 0;

	FOREACH_S(irq, 23, 23, IRQ_SEARCH_START, 1)
	{
		status = _linux_intr_register(irq);
		if(SA_PASS(status))
		{
			_linux_intr_unregister(irq);
			return irq;
		}
	}

	return INVALID_IRQ;
}

static OSSTATUS _linux_register_sos_os_irq(void)
{
    OSSTATUS status = OS_SUCCESS;

    sos_os_irq = _linux_find_sos_os_irq();
    if(sos_os_irq == INVALID_IRQ)
	{
		DRV_ERR("Register SecOS-OS IRQ Error: No free IRQ!\n"); 
		goto find_irq_err;
	}

    status = _linux_intr_register(sos_os_irq);
	if(SA_FAIL(status))
	{
		DRV_ERR("Register SecOS-OS IRQ Error: Failed to claim IRQ:%d\n", sos_os_irq); 
		goto irq_register_err;
	}

    // On success
    return OS_SUCCESS;


irq_register_err:
find_irq_err:
	sos_os_irq = INVALID_IRQ;
    return E_INTERNAL_ERROR;
}

OSSTATUS linux_sos_os_intr_probe_and_register(void)
{
	OSSTATUS status = OS_SUCCESS;
	// paddr_t ioapic_paddr;
	uint32_t irq_affinity = 0;
	uint32_t os_cpu_id = 0;
	bool result = false;
	bool is_dest_physical_mode = false;

	if(sos_os_irq != INVALID_IRQ)
    {
		DRV_ERR("Probe SecOS-OS IRQ Error! Double probe without unregistering the previous SecOS-OS interrupt vector\n"); 
		return E_INV_ARG;
	}

	// ioapic_paddr = ioapic_get_base();
	// ioapic_read(ioapic_paddr);
	
	//// Probe available IRQs
	status = _linux_register_sos_os_irq();
	if(SA_FAIL(status))
	{
		DRV_ERR("Probe SecOS-OS IRQ Error!\n"); 
		return E_INTERNAL_ERROR;
	}

	sos_os_intr = ioapic_get_vector(sos_os_irq);

	// Calculate the OS's dedicated processor for SecOS-OS communication.
	//// Calculate IRQ affinity 
	irq_affinity = ioapic_get_irq_affinity(sos_os_irq, &is_dest_physical_mode);

	DRV_INFO("Assign IRQ for SecOS-OS communication. IRQ:%u, Interrupt vector:%u\n", sos_os_irq, sos_os_intr);

    // // [Superymk]
    // {
    //     DRV_ERR("[Superymk] Assign IRQ for SecOS-OS communication. irq_affinity:0x%08X\n", irq_affinity);
    // }
    // // [Superymk] End

	if(is_dest_physical_mode)
	{
		// The destination mode of the IOAPIC's IRQ entry is physical, so <irq_affinity> is the destination APIC ID.
		
		// The destination CPU must not be the current processor, because SecOS will run on this processor. In other words,
		// the current processor must not be dedicated to the red OS.
		if(irq_affinity == os->processor_id())
		{
			DRV_ERR("Probe SecOS-OS IRQ Error! No CPU is dedicated for red OS\n");
			return E_INTERNAL_ERROR;
		}

		sos_os_intr_os_lapic_id = irq_affinity;
	}
	else
	{
		// The destination mode of the IOAPIC's IRQ entry is logical, so <irq_affinity> is the set of CPUs that can receive the IRQ.

		// Exclude the current processor from <irq_affinity>, because SecOS will run on this processor. In other words,
		// the current processor must not be dedicated to the red OS.
		// How the SecAgent knows the current processor is dedicated to the SecOS? SecOS loader runs on the desired processor 
		// before calling the ioctl.
		uint32_t irq_affinity_mask = irq_affinity;

		irq_affinity_mask &= ~(1 << os->processor_id());
		if (!ulong_test_highest_bit((ulong_t)irq_affinity_mask, &os_cpu_id)) {
			DRV_ERR("Probe SecOS-OS IRQ Error! No CPU is dedicated for red OS\n");
			return E_INTERNAL_ERROR;
		}

		result = __x86_os_cpu_lapic_mapping_find(os_cpu_id, &sos_os_intr_os_lapic_id);
		if(!result)
		{
			DRV_ERR("Probe SecOS-OS IRQ Error! Cannot find LAPIC ID of the OS's CPU:%u\n", os_cpu_id); 
			return E_INTERNAL_ERROR;
		}
	}

    DRV_INFO("Dedicated OS CPU LAPIC ID for SecOS-OS communication. LAPIC ID:%u\n", sos_os_intr_os_lapic_id);

	// On success
	return OS_SUCCESS;
}

void linux_sos_os_intr_unregister(void)
{
    if(sos_os_irq != INVALID_IRQ)
	{
		_linux_intr_unregister(sos_os_irq);
		sos_os_irq = INVALID_IRQ;
	}
}

intr_t linux_sos_os_intr_get_vector(void)
{
	return sos_os_intr;
}

#define TIMER_INTERRUPT_VECTOR_LINUX        (0xEC)
intr_t linux_intr_timer(void)
{
	return TIMER_INTERRUPT_VECTOR_LINUX;
}

lapic_id_t linux_sos_os_intr_get_os_lapic_id(void)
{
	return sos_os_intr_os_lapic_id;
}

void linux_os_intr_remove_affinity(void)
{
	OSSTATUS status = OS_SUCCESS;

	status = ioapic_remove_allintrs_affinity(1); // Remove the interrupt affinity from CPU 1
	if(SA_FAIL(status))
	{
		DRV_ERR("Failed to remove affinity in IOAPIC of CPU 1\n");
	}
}

void linux_os_intr_restore_affinity(void)
{
	// Do nothing
}
