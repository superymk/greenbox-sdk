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

#include <sa_headers.h>
#include "intel_gpu.h"
#include "ilk.h"
#include "gen9.h"

// Const information initialization goes here
static struct intel_gpu_intl ilk_m_drv =
    {
        .gen = I965_GEN_5,
        .num_surfaces = ILK_MAX_SURFACE_NUMS,
        .has_hw_overlay = true,
        .has_fb_compression = false,

        .gpu_pause_op = i965_ilk_gpu_pause,
        .gpu_resume_op = i965_ilk_gpu_resume,
};

static struct intel_gpu_intl kabylk_m_drv =
    {
        .gen = I965_GEN_9_5,
        .num_surfaces = KABYLK_MAX_SURFACE_NUMS,
        .has_hw_overlay = true,
        .has_fb_compression = false,

        .gpu_pause_op = i965_gen9_gpu_pause,
        .gpu_resume_op = i965_gen9_gpu_resume,
};

OSSTATUS intel_gpu_probe(struct gpu_pdev *gpu)
{
    OSSTATUS status = OS_SUCCESS;
    struct pci_device *pci = NULL;

    // Sanity checks
    if (!gpu)
        return E_INV_ARG;
    pci = &gpu->pci;

    switch (pci->device)
    {
    case INTEL_GPU_ILK_ID:
    {
        gpu->private = &ilk_m_drv;
        status = OS_SUCCESS;
        break;
    }

    // Kaby Lake
    case INTEL_GPU_KABYLK_ULT_GT1_ID:
    case INTEL_GPU_KABYLK_GT1_5_ID:

    case INTEL_GPU_KABYLK_ULX_GT1_ID:
    case INTEL_GPU_KABYLK_ULX_GT1_5_ID:

    case INTEL_GPU_KABYLK_DT_GT1_ID:
    case INTEL_GPU_KABYLK_HALO_GT1_ID:
    case INTEL_GPU_KABYLK_SRV_GT1_ID:
    case INTEL_GPU_KABYLK_HALO_GT1_5_ID:

    case INTEL_GPU_KABYLK_ULT_GT2_ID:
    case INTEL_GPU_KABYLK_ULT_GT2F_ID:

    case INTEL_GPU_KABYLK_ULX_GT2_ID:

    case INTEL_GPU_KABYLK_DT_GT2_ID:
    case INTEL_GPU_KABYLK_MOBILE_GT2_ID:
    case INTEL_GPU_KABYLK_SRV_GT2_ID:
    case INTEL_GPU_KABYLK_HALO_GT2_ID:
    case INTEL_GPU_KABYLK_WKS_GT2_ID:

    case INTEL_GPU_KABYLK_ULT_GT3_ID:
    case INTEL_GPU_KABYLK_DT_GT3_ID:
    case INTEL_GPU_KABYLK_MOBILE_GT3_ID:

    case INTEL_GPU_KABYLK_HALO_GT4_ID:

    case INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID1:
    case INTEL_GPU_KABYLK_AML_KBL_ULX_GT2_ID2:
    {
        gpu->private = &kabylk_m_drv;
        status = OS_SUCCESS;
        break;
    }
    default:
    {
        // Unknown GPU to IOSM-FB
        DRV_ERR("Intel GPU Probe Failed: Unknown GPU!\n");
        status = E_DEV_UNKNOWN;
        break;
    }
    }

    // On success
    return status;
}

OSSTATUS intel_gpu_pause(struct gpu_pdev *gpu)
{
    struct pci_device *pci = NULL;
    struct intel_gpu_intl *i965_gpu = NULL;

    // Sanity checks
    if (!gpu)
        return EINVAL;
    pci = &gpu->pci;
    i965_gpu = (struct intel_gpu_intl *)gpu->private;

    return i965_gpu->gpu_pause_op(gpu);
}

OSSTATUS intel_gpu_resume(struct gpu_pdev *gpu)
{
    struct pci_device *pci = NULL;
    struct intel_gpu_intl *i965_gpu = NULL;

    // Sanity checks
    if (!gpu)
        return EINVAL;
    pci = &gpu->pci;
    i965_gpu = (struct intel_gpu_intl *)gpu->private;

    return i965_gpu->gpu_resume_op(gpu);
}