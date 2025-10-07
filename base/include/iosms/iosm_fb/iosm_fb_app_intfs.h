// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
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