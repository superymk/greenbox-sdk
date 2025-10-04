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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sl_headers.h>

#include <elf.h>
#include <secsrvs/init_loader.h>
#include <secsrvs/iosm_pcie_loader.h>
#include <secsrvs/iosm_ps2_loader.h>
#include <secsrvs/iosm_usb2_loader.h>
#include <secsrvs/iosm_usb3_loader.h>
#include <secsrvs/iosm_fb_loader.h>

#include <dev/usb.h>

#include "../hash/hash.h"

/// @brief The CPU that executes <secsrvs_load>
static uint32_t _secsrvs_load_thread_affinity = 0;
static uint32_t _num_booted_components = 1; // SecOS has already been booted before booting any IOSMs. So the initial number
                            // of components (comprising of SecOS and IOSMs) is 1.

static SL_STATUS _secsrvs_to_run(uint32_t component_id)
{
    OSSTATUS sa_status = OS_SUCCESS;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_set(NS_GB_COMPONENTS_BOOTING_ID, component_id);
    if(SA_FAIL(sa_status))
    {
        SL_ERR("Failed to store the component to run! SecAgent status:%d\n", sa_status);
        return SL_EINTERNAL;
    }

    // On success
    return SL_SUCCESS;
}

static SL_STATUS _secsrvs_record_running(void)
{
    OSSTATUS sa_status = OS_SUCCESS;

    _num_booted_components++;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_set(NS_GB_COMPONENTS_BOOTED_NUM, _num_booted_components);
    if(SA_FAIL(sa_status))
    {
        SL_ERR("Failed to store the number of running components! SecAgent status:%d\n", sa_status);
        return SL_EINTERNAL;
    }

    // On success
    return SL_SUCCESS;
}

SL_STATUS ld_secsrvs_init_before_secos_run(SL_CFG *sos_cfg)
{
    SL_STATUS status = SL_SUCCESS;

    if (sos_cfg->need_iosm_usb3)
    {
        uint16_t usb_bus_id = sos_cfg->usb3_info.hid_dev1_bus_id;

        // Disable USB host controller
#if defined(__WINDOWS__)
        status = usb_hc_disable_by_vendor_product_str(sos_cfg->usb3_info.xhci_hc_vendor_product_str);
#elif defined(__LINUX__)
        status = usb_hc_disable(sos_cfg->usb3_info.xhci_hc_pci_addr_str);
#endif
        if (SL_FAIL(status))
        {
            SL_ERR("<secsrvs_init_before_secos_run> Failed to disable USB host controller (bus:%u)! status:%lu\n", usb_bus_id, status);
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}

SL_STATUS ld_secsrvs_fini_after_secos_terminate(SL_CFG *sos_cfg)
{
    SL_STATUS status = SL_SUCCESS;

    if (sos_cfg->need_iosm_usb3)
    {
        // Release device info stored in SecAgent
        uint16_t usb_bus_id = sos_cfg->usb3_info.hid_dev1_bus_id;
        // iosm_usb3_unprobe_resc_info();

#if defined(__WINDOWS__)
        status = usb_hc_reset_by_vendor_product_str(sos_cfg->usb3_info.xhci_hc_vendor_product_str);
#elif defined(__LINUX__)
        status = usb_hc_reset(sos_cfg->usb3_info.xhci_hc_pci_addr_str);
#endif
        if (SL_FAIL(status))
        {
            SL_ERR("<secsrvs_unload> Failed to reset USB host controller (bus:%u)! status:%lu\n", usb_bus_id, status);
        }
    }

    // On success
    return SL_SUCCESS;
}

// [TODO][Issue 30] Currently we split <secsrvs_load> from <secsrvs_run>, because hypercalls may cause disk read and
// write incorrect
int secsrvs_load(SL_CFG *sos_cfg, uint32_t cpu_affinity)
{
    int ret = 0;

    _secsrvs_load_thread_affinity = cpu_affinity;
    app_set_cpu_affinity(_secsrvs_load_thread_affinity);

    ret = _init_sp_load(sos_cfg);
    if (ret)
    {
        SL_ERR("<secsrvs_load> Load Init SecProcess error!\n");
        return ret;
    }

    if (sos_cfg->need_iosm_pcie)
    {
        ret = _iosm_pcie_load(sos_cfg);
        if (ret)
        {
            SL_ERR("<secsrvs_load> Load IOSM-PCIE error!\n");
            return ret;
        }
    }

    if (sos_cfg->need_iosm_ps2)
    {
        ret = _iosm_ps2_load(sos_cfg);
        if (ret)
        {
            SL_ERR("<secsrvs_load> Load IOSM-PS2 error!\n");
            return ret;
        }
    }

    // if (sos_cfg->need_iosm_usb2)
    // {
    //     ret = _iosm_usb2_load(sos_cfg);
    //     if (ret)
    //     {
    //         SL_ERR("<secsrvs_load> Load IOSM-USB2 error!\n");
    //         return ret;
    //     }
    // }

    if (sos_cfg->need_iosm_usb3)
    {
        // Load IOSM-USB3 (and disable USB host controller)
        ret = _iosm_usb3_load(sos_cfg);
        if (ret)
        {
            SL_ERR("<secsrvs_load> Load IOSM-USB3 error!\n");
            return ret;
        }
    }

    if (sos_cfg->need_iosm_fb)
    {
        ret = _iosm_fb_load(sos_cfg);
        if (ret)
        {
            SL_ERR("<secsrvs_load> Load IOSM-FB error!\n");
            return ret;
        }
    }

    return 0;
}

SL_STATUS secsrvs_run(SL_CFG *sos_cfg)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;

    app_set_cpu_affinity(_secsrvs_load_thread_affinity);

    // Clear <_num_booted_components>
    _num_booted_components = 1;

    // Run IOSM-Init
    {
        status = _secsrvs_to_run(GB_COMPONENT_ID_SECSRV_INIT);
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-Init] Booting ...... Failed (on mark to run)! status:%lu\n", status);
            return status;
        }

        ret = _init_sp_run();
        if (ret)
        {
            SL_ERR("[IOSM-Init] Booting ...... Failed! status:%d\n", ret);
            return SL_EINTERNAL;
        }

        status = _secsrvs_record_running();
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-Init] Booting ...... Failed (on mark running)! status:%lu\n", status);
            return status;
        }

        SL_INFO("[IOSM-Init] Booting ...... Success\n");
    }

    // Run IOSM-PCIE
    if (sos_cfg->need_iosm_pcie)
    {
        status = _secsrvs_to_run(GB_COMPONENT_ID_IOSM_PCIE);
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-PCIE] Booting ...... Failed (on mark to run)! status:%lu\n", status);
            return status;
        }
        
        ret = _iosm_pcie_run();
        if (ret)
        {
            SL_ERR("[IOSM-PCIE] Booting ...... Failed! status:%d\n", ret);
            return SL_EINTERNAL;
        }

        status = _secsrvs_record_running();
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-PCIE] Booting ...... Failed (on mark running)! status:%lu\n", status);
            return status;
        }

        SL_INFO("[IOSM-PCIE] Booting ...... Success\n");
    }

    // Run IOSM-PS2
    if (sos_cfg->need_iosm_ps2)
    {
        status = _secsrvs_to_run(GB_COMPONENT_ID_IOSM_PS2);
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-PS2] Booting ...... Failed (on mark to run)! status:%lu\n", status);
            return status;
        }

        ret = _iosm_ps2_run();
        if (ret)
        {
            SL_ERR("[IOSM-PS2] Booting ...... Failed! status:%d\n", ret);
            return SL_EINTERNAL;
        }

        status = _secsrvs_record_running();
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-PS2] Booting ...... Failed (on mark running)! status:%lu\n", status);
            return status;
        }

        SL_INFO("[IOSM-PS2] Booting ...... Success\n");
    }
    
    // // Run IOSM-USB2
    // if (sos_cfg->need_iosm_usb2)
    // {
    //     status = _secsrvs_to_run(GB_COMPONENT_ID_IOSM_USB2);
    //     if (SL_FAIL(status))
    //     {
    //         SL_ERR("[IOSM-USB2] Booting ...... Failed (on mark to run)! status:%lu\n", status);
    //         return status;
    //     }

    //     ret = _iosm_usb2_run();
    //     if (ret)
    //     {
    //         SL_ERR("[IOSM-USB2] Booting ...... Failed! status:%d\n", ret);
    //         return SL_EINTERNAL;
    //     }

    //     status = _secsrvs_record_running();
    //     if (SL_FAIL(status))
    //     {
    //         SL_ERR("[IOSM-USB2] Booting ...... Failed (on mark running)! status:%lu\n", status);
    //         return status;
    //     }

    //     SL_INFO("[IOSM-USB2] Booting ...... Success\n");
    // }
    
    // Run IOSM-USB3
    if (sos_cfg->need_iosm_usb3)
    {
        status = _secsrvs_to_run(GB_COMPONENT_ID_IOSM_USB3);
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-USB3] Booting ...... Failed (on mark to run)! status:%lu\n", status);
            return status;
        }

        ret = _iosm_usb3_run();
        if (ret)
        {
            SL_ERR("[IOSM-USB3] Booting ...... Failed! status:%d\n", ret);
            return SL_EINTERNAL;
        }

        status = _secsrvs_record_running();
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-USB3] Booting ...... Failed (on mark running)! status:%lu\n", status);
            return status;
        }

        SL_INFO("[IOSM-USB3] Booting ...... Success\n");
    }
    
    // Run IOSM-FB
    if (sos_cfg->need_iosm_fb)
    {
        status = _secsrvs_to_run(GB_COMPONENT_ID_IOSM_FB);
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-FB] Booting ...... Failed (on mark to run)! status:%lu\n", status);
            return status;
        }

        ret = _iosm_fb_run();
        if (ret)
        {
            SL_ERR("[IOSM-FB] Booting ...... Failed! status:%d\n", ret);
            return SL_EINTERNAL;
        }

        status = _secsrvs_record_running();
        if (SL_FAIL(status))
        {
            SL_ERR("[IOSM-FB] Booting ...... Failed (on mark running)! status:%lu\n", status);
            return status;
        }

        SL_INFO("[IOSM-FB] Booting ...... Success\n");
    }

    // On success
    return SL_SUCCESS;
}

//! Unregister and unload all SecServices (except Init SecProcess)
void secsrvs_terminate(SL_CFG *sos_cfg)
{
    SL_INFO("Terminate SecServices...\n");
    app_set_cpu_affinity(_secsrvs_load_thread_affinity);

    if (sos_cfg->need_iosm_fb)
    {
        iosm_fb_terminate();
    }

    // if (sos_cfg->need_iosm_usb2)
    // {
    //     iosm_usb2_terminate();
    // }

    if (sos_cfg->need_iosm_usb3)
    {
        iosm_usb3_terminate();
    }

    if (sos_cfg->need_iosm_ps2)
    {
        iosm_ps2_terminate();
    }

    if (sos_cfg->need_iosm_pcie)
    {
        iosm_pcie_terminate(); // Disable IOSM-PCIe after disable IOSM-USB2, because it is relied by IOSM-USB2
    }

    // [NOTE] The init SecProcess cannot be terminated, unless killing the entire SecOS
}

//! Unload all SecServices (except Init SecProcess)
void secsrvs_unload(SL_CFG *sos_cfg)
{
    int ret = 0;

    app_set_cpu_affinity(_secsrvs_load_thread_affinity);

    if (sos_cfg->need_iosm_fb)
    {
        _iosm_fb_unload();
        if (ret)
        {
            SL_ERR("<secsrvs_unload> Unload IOSM-FB SecProcess error!\n");
        }
    }

    // if (sos_cfg->need_iosm_usb2)
    // {
    //     ret = _iosm_usb2_unload();
    //     if (ret)
    //     {
    //         SL_ERR("<secsrvs_unload> Unload IOSM-USB2 SecProcess error!\n");
    //     }
    // }

    if (sos_cfg->need_iosm_usb3)
    {
        ret = _iosm_usb3_unload();
        if (ret)
        {
            SL_ERR("<secsrvs_unload> Unload IOSM-USB3 SecProcess error!\n");
        }
    }

    if (sos_cfg->need_iosm_ps2)
    {
        ret = _iosm_ps2_unload();
        if (ret)
        {
            SL_ERR("<secsrvs_unload> Unload IOSM-PS2 SecProcess error!\n");
        }
    }

    if (sos_cfg->need_iosm_pcie)
    {
        ret = iosm_pcie_unload();
        if (ret)
        {
            SL_ERR("<secsrvs_unload> Unload IOSM-PCIE SecProcess error!\n");
        }
    }

    // if (sos_cfg->need_iosm_usb2)
    // {
    //     // Release device info stored in SecAgent
    //     iosm_usb2_unprobe_resc_info();
    // }

    // Release resources of Init SecProcess
    ret = init_sp_unload();
    if (ret)
    {
        SL_ERR("<secsrvs_unload> Unload Init SecProcess error!\n");
    }

    _secsrvs_load_thread_affinity = 0;
}