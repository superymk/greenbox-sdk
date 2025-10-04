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

#ifndef SECAGENT_INTEL_GPU_GEN9_H
#define SECAGENT_INTEL_GPU_GEN9_H

#include "intel_gpu.h"

#ifndef __ASSEMBLY__


extern OSSTATUS i965_gen9_gpu_pause(struct gpu_pdev *gpu);
extern OSSTATUS i965_gen9_gpu_resume(struct gpu_pdev *gpu);

#endif //__ASSEMBLY__
#endif // SECAGENT_INTEL_GPU_GEN9_H