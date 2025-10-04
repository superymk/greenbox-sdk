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

#ifndef SECOS_COMMON_DEFS_H
#define SECOS_COMMON_DEFS_H

#define INVALID_SP_ID                   0

//! The invalid handler of the IOMMU PageTable
#define IOMMU_PT_INVALID	            0xFFFFFFFF


// Default ID (sp_id/app_id)
// The reserved ID for Sec* components in OS and SecOS
#define SECOS_ID					0
#define SECBASE_ID					1
#define OS_SYS_ID					2

#define SOS_RESERVED_IDS            4


#ifndef __ASSEMBLY__

typedef uint32_t cpu_idx; 	// cpu index
typedef uint8_t intr_t;

#endif // __ASSEMBLY__
#endif // SECOS_COMMON_DEFS_H

