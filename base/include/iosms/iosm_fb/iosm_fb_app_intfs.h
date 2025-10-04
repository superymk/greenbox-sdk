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

#ifndef IOSM_FB_APP_INTFS_H
#define IOSM_FB_APP_INTFS_H

/// @brief IOSM-FB's DMA memory size
#define IOSM_FB_GPU_DMA_MEMORY      MB(32)

#ifndef __ASSEMBLY__
#include <iosm_pcie_app_intfs.h>

/// @brief Configurations of Framebuffer device separation
struct fb_conf
{
    union pci_bdf			fb_bdf;

    uint32_t                os_enabled_surfaces_num;
    uint32_t                os_pixel_bytes;
};


#endif // __ASSEMBLY__
#endif // IOSM_FB_APP_INTFS_H