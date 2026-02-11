// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_INTEL_GPU_GEN9_H
#define SECAGENT_INTEL_GPU_GEN9_H

#include "intel_gpu.h"

#ifndef __ASSEMBLY__

/// @brief Pause Gen9 GPU
/// @param gpu GPU device
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS i965_gen9_gpu_pause(struct gpu_pdev *gpu);

/// @brief Resume Gen9 GPU
/// @param gpu GPU device
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS i965_gen9_gpu_resume(struct gpu_pdev *gpu);

#endif //__ASSEMBLY__
#endif // SECAGENT_INTEL_GPU_GEN9_H