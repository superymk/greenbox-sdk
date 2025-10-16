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

/*
 * @XMHF_LICENSE_HEADER_START@
 *
 * eXtensible, Modular Hypervisor Framework (XMHF)
 * Copyright (c) 2009-2012 Carnegie Mellon University
 * Copyright (c) 2010-2012 VDG Inc.
 * All Rights Reserved.
 *
 * Developed by: XMHF Team
 *               Carnegie Mellon University / CyLab
 *               VDG Inc.
 *               http://xmhf.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of Carnegie Mellon or VDG Inc, nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @XMHF_LICENSE_HEADER_END@
 */

#ifndef TRUSTVISOR_H
#define TRUSTVISOR_H

#define SECBASE_MAX_CPUS 256

// Isolated Execution Environment ID. 0 is reserved for mHV. 1 is for the NS domain. They must be the same with 
// <DOMAIN_UXMHF64>, <DOMAIN_NONSECURE> and <DOMAIN_OTHER> defined in uxmhf64-commons.h
#define DOMAIN_MHV	(DOMAIN_UXMHF64)
#define DOMAIN_UNTRUSTED (DOMAIN_NONSECURE)
#define DOMAIN_SECOS (DOMAIN_OTHER)

#ifndef __ASSEMBLY__

/*
 * structs for pal-registration descriptor
 */

enum tv_pal_section_type
{
	TV_SECTION_TYPE_INVALID = 0,
	
	// SecApp
	TV_PAL_SECTION_CODE = 1,
	TV_PAL_SECTION_IMAGE_DATA = 2,
	TV_PAL_SECTION_RODATA = 3,
	TV_PAL_SECTION_DATA = 4,
	TV_PAL_SECTION_SHARED = 5,
	// TV_PAL_GPU_ISOLATED = 6, // [TODO] refer to ticket-53. This is just a workaround

	// SecOS
	TV_VK_SECTION_IMAGE = 11,
	TV_VK_SECTION_DATA = 12,
	TV_VK_SECTION_SHARED = 13,
	SOS_SECTION_SIGNATURE = 14,

	TV_PLATFORM_MEMORY = 21, // TV_PLATFORM_MEMORY are memory that must not be zeroed when assigning back to the NS domain.
							// For example, MMIO is TV_PLATFORM_MEMORY
};

struct section_meminfo
{
	enum tv_pal_section_type type;
	ulong_t start_addr;
	ulong_t dest_gpaddr;
	ulong_t page_num; /* size of section in pages */
};

#define TV_MAX_SECTIONS 24 /* max sections that are allowed in pal registration */
struct hv_reg_sections
{
	uint32_t num_sections;
	struct section_meminfo sections[TV_MAX_SECTIONS];
};

/* parameter type */
enum tv_pal_param_type
{
	TV_PAL_PM_INTEGER = 1,
	TV_PAL_PM_POINTER = 2,
};

struct tv_pal_param
{
	enum tv_pal_param_type type; /* 1: integer ;  2:pointer*/
	uint32_t size;
};

#define TV_MAX_PARAMS 10
struct hv_reg_params
{
	uint32_t num_params;
	struct tv_pal_param params[TV_MAX_PARAMS];
};

#endif // __ASSEMBLY__
#endif // TRUSTVISOR_H

/* Local Variables: */
/* mode:c           */
/* indent-tabs-mode:nil */
/* c-basic-offset:2      */
/* End:             */
