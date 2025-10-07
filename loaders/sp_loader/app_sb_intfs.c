// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <secbase_errno.h>
#include <secbase_hypercalls.h>

#define cpuid(op, eax, ebx, ecx, edx)                                                 \
	({                                                                                \
		__asm__ __volatile__("cpuid"                                                  \
							 : "=a"(*(eax)), "=b"(*(ebx)), "=c"(*(ecx)), "=d"(*(edx)) \
							 : "0"(op), "2"(0));                                      \
	})

#define CPU_VENDOR_INTEL 	0xAB
#define CPU_VENDOR_AMD 		0xCD
#define CPU_VENDOR_UNKNOWN      0x00

#define AMD_STRING_DWORD1 0x68747541
#define AMD_STRING_DWORD2 0x69746E65
#define AMD_STRING_DWORD3 0x444D4163

#define INTEL_STRING_DWORD1	0x756E6547
#define INTEL_STRING_DWORD2	0x49656E69
#define INTEL_STRING_DWORD3	0x6C65746E	

static inline uint32_t _get_cpu_vendor(void)
{
	uint32_t dummy;
	uint32_t vendor_dword1, vendor_dword2, vendor_dword3;

	cpuid(0, &dummy, &vendor_dword1, &vendor_dword3, &vendor_dword2);
	if (vendor_dword1 == AMD_STRING_DWORD1 && vendor_dword2 == AMD_STRING_DWORD2 && vendor_dword3 == AMD_STRING_DWORD3)
		return CPU_VENDOR_AMD;
	else if (vendor_dword1 == INTEL_STRING_DWORD1 && vendor_dword2 == INTEL_STRING_DWORD2 && vendor_dword3 == INTEL_STRING_DWORD3)
		return CPU_VENDOR_INTEL;
	else
		return CPU_VENDOR_UNKNOWN;

	return 0; /* never reached */
}

static inline ulong_t hypcall_4args(ulong_t arg_ax, ulong_t arg_cx, ulong_t arg_dx,
									ulong_t arg_si, ulong_t arg_di)
{
	switch (_get_cpu_vendor())
	{
	case CPU_VENDOR_INTEL:
		__asm__ __volatile__(
			"vmcall\n\t"
			: "=a"(arg_ax)
			: "a"(arg_ax), "c"(arg_cx), "d"(arg_dx),
			  "S"(arg_si), "D"(arg_di));
		break;
	case CPU_VENDOR_AMD:
		__asm__ __volatile__(
			"vmcall\n\t"
			: "=a"(arg_ax)
			: "a"(arg_ax), "c"(arg_cx), "d"(arg_dx),
			  "S"(arg_si), "D"(arg_di));
		break;
	default:
		arg_ax = -1;
	}

	return arg_ax;
}

static inline ulong_t hypercall_noarg(ulong_t arg_ax)
{
	return hypcall_4args(arg_ax, 0, 0, 0, 0);
}

static inline ulong_t hypercall_1arg(ulong_t arg_ax, ulong_t arg_cx)
{
	return hypcall_4args(arg_ax, arg_cx, 0, 0, 0);
}

static inline ulong_t hypercall_2args(ulong_t arg_ax, ulong_t arg_cx, ulong_t arg_dx)
{
	return hypcall_4args(arg_ax, arg_cx, arg_dx, 0, 0);
}

static inline ulong_t hypercall_3args(ulong_t arg_ax, ulong_t arg_cx, ulong_t arg_dx,
									  ulong_t arg_si)
{
	return hypcall_4args(arg_ax, arg_cx, arg_dx, arg_si, 0);
}

static inline ulong_t hypercall_ret4(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx,
									 ulong_t *arg_si, ulong_t *arg_di)
{
	switch (_get_cpu_vendor())
	{
	case CPU_VENDOR_INTEL:
		__asm__ __volatile__(
			"vmcall\n\t"
			: "=a"(arg_ax), "=c"(*arg_cx), "=d"(*arg_dx),
			"=S"(*arg_si), "=D"(*arg_di)
			: "a"(arg_ax), "c"(*arg_cx), "d"(*arg_dx),
			"S"(*arg_si), "D"(*arg_di));
		break;
	case CPU_VENDOR_AMD:
		__asm__ __volatile__(
			"vmmcall\n\t"
			: "=a"(arg_ax), "=c"(*arg_cx), "=d"(*arg_dx),
			"=S"(*arg_si), "=D"(*arg_di)
			: "a"(arg_ax), "c"(*arg_cx), "d"(*arg_dx),
			"S"(*arg_si), "D"(*arg_di));
		break;
	default:
		arg_ax = -1;
	}

	return arg_ax;
}

static inline ulong_t hypercall_ret1(ulong_t arg_ax, ulong_t *arg_cx)
{
	ulong_t reserved1 = 0;
	ulong_t reserved2 = 0;
	ulong_t reserved3 = 0;

	return hypercall_ret4(arg_ax, arg_cx, &reserved1, &reserved2, &reserved3);
}

static inline ulong_t hypercall_ret2(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx)
{
	ulong_t reserved1 = 0;
	ulong_t reserved2 = 0;

	return hypercall_ret4(arg_ax, arg_cx, arg_dx, &reserved1, &reserved2);
}

static inline ulong_t hypercall_ret3(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx,
									 ulong_t *arg_si)
{
	ulong_t reserved1 = 0;

	return hypercall_ret4(arg_ax, arg_cx, arg_dx, arg_si, &reserved1);
}

int hypercall_secbase_get_boot_state(NS_HC_PARAMS_GET_SECBASE_BOOT_STATE *out_result)
{
    MHV_STATUS sb_status = MHV_SUCCESS;

    // Check: params must be valid
    if (!out_result)
        return -1;

    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_GET_SECBASE_BOOT_STATE, (ulong_t)out_result);
    if (MHV_FAIL(sb_status))
    {
        printf("<hypercall_secbase_get_boot_state> Hypercall error: %u\n", (uint32_t)sb_status);
        return -1;
    }

    // On success
    return 0;
}

int hypercall_secbase_get_tpm_pcrs(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT *out_result)
{
    MHV_STATUS sb_status = MHV_SUCCESS;

    // Check: params must be valid
    if (!out_result)
        return 1;

    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_TPM_GET_SECBASE_MEASUREMENT, (ulong_t)out_result);
    if (MHV_FAIL(sb_status))
    {
        printf("<secbase_get_tpm_pcrs> Hypercall error: %u\n", (uint32_t)sb_status);
        return 1;
    }

    // On success
    return 0;
}

int hypercall_secbase_get_unique_machine_id(uint8_t *out_machine_id)
{
    MHV_STATUS sb_status = MHV_SUCCESS;

    // Check: params must be valid
    if (!out_machine_id)
        return -1;

    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_GET_UNIQUE_MACHINE_ID, (ulong_t)out_machine_id);
    if (MHV_FAIL(sb_status))
    {
        printf("<hypercall_secbase_get_unique_machine_id> Hypercall error: %u\n", (uint32_t)sb_status);
        return -1;
    }

    // On success
    return 0;
}