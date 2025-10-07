// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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