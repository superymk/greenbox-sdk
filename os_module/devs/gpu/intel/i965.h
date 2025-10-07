// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_INTEL_I965_GPU_H
#define SECAGENT_INTEL_I965_GPU_H

#include <device/gpu/intel/i965_regs.h>

/// @brief Intel IronLake GPUs support 2 screens at most
#define ILK_MAX_SURFACE_NUMS		(2)

/// @brief Intel IronLake GPUs support 3 screens at most
#define KABYLK_MAX_SURFACE_NUMS		(3)

/// @brief Intel I965 GPUs support 3 screens at most
#define I965_MAX_SURFACE_NUMS       (3)

#ifndef __ASSEMBLY__

#define IS_I965_ILK(gpu_pci) (gpu_pci->device == INTEL_GPU_ILK_ID)

#define IS_I965_KABYLK(gpu_pci) (                                \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULT_GT1_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_GT1_5_ID) ||            \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULX_GT1_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULX_GT1_5_ID) ||        \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_DT_GT1_ID) ||           \
    (gpu_pci->device == INTEL_GPU_KABYLK_HALO_GT1_ID) ||         \
    (gpu_pci->device == INTEL_GPU_KABYLK_SRV_GT1_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_HALO_GT1_5_ID) ||       \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULT_GT2_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULT_GT2F_ID) ||         \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULX_GT2_ID) ||          \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_DT_GT2_ID) ||           \
    (gpu_pci->device == INTEL_GPU_KABYLK_MOBILE_GT2_ID) ||       \
    (gpu_pci->device == INTEL_GPU_KABYLK_SRV_GT2_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_HALO_GT2_ID) ||         \
    (gpu_pci->device == INTEL_GPU_KABYLK_WKS_GT2_ID) ||          \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_ULT_GT3_ID) ||          \
    (gpu_pci->device == INTEL_GPU_KABYLK_DT_GT3_ID) ||           \
    (gpu_pci->device == INTEL_GPU_KABYLK_MOBILE_GT3_ID) ||       \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_HALO_GT4_ID) ||         \
                                                                 \
    (gpu_pci->device == INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID1) || \
    (gpu_pci->device == INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID2))


#endif //__ASSEMBLY__
#endif // SECAGENT_INTEL_I965_GPU_H