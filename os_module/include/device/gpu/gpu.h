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

#ifndef GPU_H
#define GPU_H

#include <device/gpu/gpu_utils.h>
#include <device/gpu/intel/i965_regs.h>

#ifndef __ASSEMBLY__

struct gpu_pdev;

typedef OSSTATUS (*GPU_PROBE_OP)(struct gpu_pdev *gpu);
typedef OSSTATUS (*GPU_PAUSE_OP)(struct gpu_pdev *gpu);
typedef OSSTATUS (*GPU_RESUME_OP)(struct gpu_pdev *gpu);

struct gpu_pdev
{
    /// @brief True iff the GPU probe function has initialized this struct.
    bool is_probed;

    /// @brief The PCI configuration space of the physical GPU device.
    struct pci_device pci;

    /// @brief Pointer to GPU-Specific info
    void *private;
};

#endif // __ASSEMBLY__
#endif // GPU_H