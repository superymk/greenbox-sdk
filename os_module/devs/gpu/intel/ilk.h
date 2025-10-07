// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_INTEL_GPU_ILK_H
#define SECAGENT_INTEL_GPU_ILK_H

#include "intel_gpu.h"

#ifndef __ASSEMBLY__

extern OSSTATUS i965_ilk_gpu_pause(struct gpu_pdev *gpu);
extern OSSTATUS i965_ilk_gpu_resume(struct gpu_pdev *gpu);

#endif //__ASSEMBLY__
#endif // SECAGENT_INTEL_GPU_ILK_H