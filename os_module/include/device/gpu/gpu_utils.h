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

#ifndef GPU_UTILS_H
#define GPU_UTILS_H

#include <utils.h>



#ifndef __ASSEMBLY__

static inline uint32_t gpu_read_reg(vaddr_t gpu_regs, uint32_t offset)
{
    vaddr_t vaddr = gpu_regs + offset;

    return sa_read32(vaddr);
}

static inline void gpu_write_reg(vaddr_t gpu_regs, uint32_t offset, uint32_t val)
{
    vaddr_t vaddr = gpu_regs + offset;

    sa_write32(vaddr, val);
}

#endif // __ASSEMBLY__
#endif // GPU_UTILS_H