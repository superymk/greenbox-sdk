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
#include <device/pci.h>
#include <device/gpu/gpu.h>

#include "./intel/intel_gpu.h"

struct gpu_vendor_sel
{
    uint32_t vendor_id;

    /// @brief Fill GPU-specific info according to <device_id>.
    GPU_PROBE_OP gpu_probe_op;
    GPU_PAUSE_OP gpu_pause_op;
    GPU_RESUME_OP gpu_resume_op;
};

#define GPU_VENDOR_ENTRY(v_id, probe_gpu_op, pause_gpu_op, resume_gpu_op) \
    {                                                                     \
        .vendor_id = (v_id),                                              \
        .gpu_probe_op = (probe_gpu_op),                                   \
        .gpu_pause_op = (pause_gpu_op),                                   \
        .gpu_resume_op = (resume_gpu_op)                                  \
}

static struct gpu_vendor_sel list_gpus[] = {
    GPU_VENDOR_ENTRY(INTEL_GPU_VENDOR_ID, intel_gpu_probe, intel_gpu_pause, intel_gpu_resume),
};

static struct gpu_pdev g_gpu_pdev = {.is_probed = false};

static OSSTATUS _gpu_probe(struct gpu_pdev *gpu)
{
    uint32_t i = 0;
    struct pci_device *pci = &gpu->pci;
    union pci_bdf gpu_bdf = pci->bdf;
    size_t num_entries = C_ARRAY_SIZE(list_gpus);

    FOREACH_S(i, num_entries, num_entries, 0, 1)
    {
        struct gpu_vendor_sel *gpu_list_entry = &list_gpus[i];

        if (pci->vendor == gpu_list_entry->vendor_id)
        {
            // Found the operation for the given GPU <gpu>.
            gpu_list_entry->gpu_probe_op(gpu);

            return OS_SUCCESS;
        }
    }

    // Unknown GPU
    DRV_ERR("[GPU (%02X:%02X:%02X)]: Failed to probe GPU: Unknown GPU!\n", gpu_bdf.bus, gpu_bdf.dev, gpu_bdf.fn);

    return E_DEV_UNKNOWN;
}

static OSSTATUS _gpu_pause(struct gpu_pdev *gpu)
{
    uint32_t i = 0;
    struct pci_device *pci = &gpu->pci;
    union pci_bdf gpu_bdf = pci->bdf;
    size_t num_entries = C_ARRAY_SIZE(list_gpus);

    FOREACH_S(i, num_entries, num_entries, 0, 1)
    {
        struct gpu_vendor_sel *gpu_list_entry = &list_gpus[i];

        if (pci->vendor == gpu_list_entry->vendor_id)
        {
            // Found the operation for the given GPU <gpu>.
            gpu_list_entry->gpu_pause_op(gpu);

            return OS_SUCCESS;
        }
    }

    // Unknown GPU
    DRV_ERR("[GPU (%02X:%02X:%02X)]: Failed to pause GPU: Unknown GPU!\n", gpu_bdf.bus, gpu_bdf.dev, gpu_bdf.fn);

    return E_DEV_UNKNOWN;
}

static OSSTATUS _gpu_resume(struct gpu_pdev *gpu)
{
    uint32_t i = 0;
    struct pci_device *pci = &gpu->pci;
    union pci_bdf gpu_bdf = pci->bdf;
    size_t num_entries = C_ARRAY_SIZE(list_gpus);

    FOREACH_S(i, num_entries, num_entries, 0, 1)
    {
        struct gpu_vendor_sel *gpu_list_entry = &list_gpus[i];

        if (pci->vendor == gpu_list_entry->vendor_id)
        {
            // Found the operation for the given GPU <gpu>.
            gpu_list_entry->gpu_resume_op(gpu);

            return OS_SUCCESS;
        }
    }

    // Unknown GPU
    DRV_ERR("[GPU (%02X:%02X:%02X)]: Failed to resume GPU: Unknown GPU!\n", gpu_bdf.bus, gpu_bdf.dev, gpu_bdf.fn);

    return E_DEV_UNKNOWN;
}

/********* Public Functions *********/
OSSTATUS do_CMD_IOSM_FB_PROBE_GPU(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    union pci_bdf gpu_pci_bdf = {.raw = (uint16_t)cmd_packet->arg1}; // arg1 - The BDF of the GPU PCIe device
    struct pci_device *pci = &g_gpu_pdev.pci;
    union pci_bdf gpu_bdf = pci->bdf;

    // Prevent double probe
    if (g_gpu_pdev.is_probed)
    {
        DRV_ERR("Probe GPU failed: SecAgent has probed the GPU [%02X:%02X:%02X]!\n", gpu_bdf.bus, gpu_bdf.dev, gpu_bdf.fn);
        return E_INV_ARG;
    }

    // Clear <g_gpu_pdev>
    memset(&g_gpu_pdev, 0, sizeof(struct gpu_pdev));

    // Read GPU PCI device
    status = pci_read_cfg_space(gpu_pci_bdf, &g_gpu_pdev.pci);
    if (SA_FAIL(status))
    {
        DRV_ERR("Probe GPU failed: Failed to read GPU PCI configuration space of device [%02X:%02X:%02X]! status:%lu\n",
                gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn, status);
        return E_IO;
    }

    // Probe GPU
    status = _gpu_probe(&g_gpu_pdev);
    if (SA_FAIL(status))
    {
        DRV_ERR("Probe GPU failed: Failed to probe GPU [%02X:%02X:%02X]! status:%lu\n",
                gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn, status);
        return status;
    }

    // On success
    g_gpu_pdev.is_probed = true;
    return OS_SUCCESS;
}

OSSTATUS do_CMD_IOSM_FB_PAUSE_GPU(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    union pci_bdf gpu_pci_bdf = {.raw = (uint16_t)cmd_packet->arg1}; // arg1 - The BDF of the GPU PCIe device
    struct pci_device *pci = &g_gpu_pdev.pci;

    // The GPU must be probed
    if (!g_gpu_pdev.is_probed || !pci_bdf_equal(gpu_pci_bdf, pci->bdf))
    {
        DRV_ERR("Pause GPU failed: SecAgent has not probed the GPU [%02X:%02X:%02X]!\n", gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn);
        return E_INV_ARG;
    }

    // Pause GPU
    status = _gpu_pause(&g_gpu_pdev);
    if (SA_FAIL(status))
    {
        DRV_ERR("Pause GPU failed: Unable to pause GPU [%02X:%02X:%02X]! status:%lu!\n",
                gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn, status);
        return E_INTERNAL_ERROR;
    }

    // GPU is paused
    DRV_INFO("Paused GPU [%02X:%02X:%02X]\n", gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn);
    return OS_SUCCESS;
}

OSSTATUS do_CMD_IOSM_FB_RESUME_GPU(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    union pci_bdf gpu_pci_bdf = {.raw = (uint16_t)cmd_packet->arg1}; // arg1 - The BDF of the GPU PCIe device
    struct pci_device *pci = &g_gpu_pdev.pci;

    // The GPU must be probed
    if (!g_gpu_pdev.is_probed || !pci_bdf_equal(gpu_pci_bdf, pci->bdf))
    {
        DRV_ERR("Resume GPU failed: SecAgent has not probed the GPU [%02X:%02X:%02X]!\n", gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn);
        return E_INV_ARG;
    }

    // Resume GPU
    status = _gpu_resume(&g_gpu_pdev);
    if (SA_FAIL(status))
    {
        DRV_ERR("Resume GPU failed: Unable to resume GPU [%02X:%02X:%02X]! status:%lu!\n",
                gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn, status);
        return E_INTERNAL_ERROR;
    }

    // GPU is resumed
    DRV_INFO("Resumed GPU [%02X:%02X:%02X]\n", gpu_pci_bdf.bus, gpu_pci_bdf.dev, gpu_pci_bdf.fn);
    return OS_SUCCESS;
}