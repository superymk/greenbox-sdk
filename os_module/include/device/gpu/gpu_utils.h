// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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