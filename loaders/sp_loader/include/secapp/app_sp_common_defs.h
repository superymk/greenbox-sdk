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
#ifndef APP_SECAPP_COMMON_DEFS
#define APP_SECAPP_COMMON_DEFS

// Sub types of trusted devices
// USB
#define DEV_USB_KEYBOARD		1
#define DEV_USB_MOUSE			2


#define IOSM_MAX_DEVICE_CFG_OBJECTS		4

#ifndef __ASSEMBLY__

//! @brief The type of the address region (e.g., Main memory region, MMIO region  
//! or Port I/O (PIO) region)
enum address_region_type
{
	REGION_MAIN_MEMORY = 1,
	REGION_MMIO = 2,
	REGION_PIO = 3
};

struct dev_config_object
{
	addr_t 						start;
	size_t						size;

	enum address_region_type	region_type;
};

//! @brief Boot params For I/O Separation Modules
typedef struct _iosm_boot_params
{
	struct dev_config_object 	dev_cfg_objs[IOSM_MAX_DEVICE_CFG_OBJECTS];
	uint32_t					num_cfg_objs;

	uint32_t					parent_iosm_id;

	// Block accesses to all dev_cfg_objs from the NS domain
	bool						block_ns_access;
} IOSM_PROC_PARAMS;

//! @brief Boot params for SecApps. SecApps are user's secure sensitive 
//! applications
typedef struct _sp_proc_params
{
	// Trusted Path Devices
	uint32_t 					num_devices;
	DEVICE_REG_INFO				tp_devs[PAL_MAX_DEVICES];

	// Trusted Display
	bool						need_tdisp;
} SAP_PROC_PARAMS; 

//! SecProcess specific boot parameters
typedef union
{
	IOSM_PROC_PARAMS 	iosm_params;
	//SAP_PROC_PARAMS 	sp_params;
} SP_PROC_PARAMS;

#endif // __ASSEMBLY__


#endif // APP_SECAPP_COMMON_DEFS
*/