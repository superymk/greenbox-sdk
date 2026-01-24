// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_INTEL_GPU_H
#define SECAGENT_INTEL_GPU_H

#include <device/gpu/gpu.h>
#include "i965.h"

#define INTEL_GPU_VENDOR_ID (0x8086)
#define INTEL_GPU_ILK_ID (0x0046)

// Kabylake GPUs
#define INTEL_GPU_KABYLK_ULT_GT1_ID (0x5906) /* ULT GT1 */
#define INTEL_GPU_KABYLK_GT1_5_ID (0x5913)   /* ULT GT1.5 */

#define INTEL_GPU_KABYLK_ULX_GT1_ID (0x590E)   /* ULX GT1 */
#define INTEL_GPU_KABYLK_ULX_GT1_5_ID (0x5915) /* ULX GT1.5 */

#define INTEL_GPU_KABYLK_DT_GT1_ID (0x5902)     /* DT  GT1 */
#define INTEL_GPU_KABYLK_HALO_GT1_ID (0x5908)   /* Halo GT1 */
#define INTEL_GPU_KABYLK_SRV_GT1_ID (0x590A)    /* SRV GT1 */
#define INTEL_GPU_KABYLK_HALO_GT1_5_ID (0x590B) /* Halo GT1 */

#define INTEL_GPU_KABYLK_ULT_GT2_ID (0x5916)  /* ULT GT2 */
#define INTEL_GPU_KABYLK_ULT_GT2F_ID (0x5921) /* ULT GT2F */

#define INTEL_GPU_KABYLK_ULX_GT2_ID (0x591E) /* ULX GT2 */

#define INTEL_GPU_KABYLK_DT_GT2_ID (0x5912)     /* DT  GT2 */
#define INTEL_GPU_KABYLK_MOBILE_GT2_ID (0x5917) /* Mobile GT2 */
#define INTEL_GPU_KABYLK_SRV_GT2_ID (0x591A)    /* SRV GT2 */
#define INTEL_GPU_KABYLK_HALO_GT2_ID (0x591B)   /* Halo GT2 */
#define INTEL_GPU_KABYLK_WKS_GT2_ID (0x591D)    /* WKS GT2 */

#define INTEL_GPU_KABYLK_ULT_GT3_ID (0x5926)    /* ULT GT3 */
#define INTEL_GPU_KABYLK_DT_GT3_ID (0x5923)     /* ULT GT3 */
#define INTEL_GPU_KABYLK_MOBILE_GT3_ID (0x5927) /* ULT GT3 */

#define INTEL_GPU_KABYLK_HALO_GT4_ID (0x593B) /* Halo GT4 */

#define INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID1 (0x591C) /* ULX GT2 */
#define INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID2 (0x87C0) /* ULX GT2 */

#ifndef __ASSEMBLY__

#define I965_GEN_INVALID (0)
#define I965_GEN_1 (1)
#define I965_GEN_2 (2)
#define I965_GEN_3 (3)
#define I965_GEN_4 (4)
#define I965_GEN_5 (5)
#define I965_GEN_5_5 (6)
#define I965_GEN_6 (7)
#define I965_GEN_7 (8)
#define I965_GEN_7_5 (9)
#define I965_GEN_8 (10)
#define I965_GEN_9 (11)
#define I965_GEN_9_5 (12)
#define I965_GEN_11 (13)
#define I965_GEN_12 (14)

#define IS_HASWELL(gpu) (INTEL_GPU(gpu)->is_haswell)

/// @brief Specific/Internal information of Intel GPUs
struct intel_gpu_intl
{
    /*** Const information ***/
    uint8_t gen;
    uint32_t num_surfaces;

    bool has_hw_overlay;
    bool has_fb_compression;

    GPU_PAUSE_OP gpu_pause_op;
    GPU_RESUME_OP gpu_resume_op;
};

/// @brief Probe Intel GPU
/// @param gpu GPU device
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS intel_gpu_probe(struct gpu_pdev* gpu);

/// @brief Pause Intel GPU
/// @param gpu GPU device
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS intel_gpu_pause(struct gpu_pdev *gpu);

/// @brief Resume Intel GPU
/// @param gpu GPU device
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS intel_gpu_resume(struct gpu_pdev *gpu);

#endif //__ASSEMBLY__
#endif // SECAGENT_INTEL_GPU_H