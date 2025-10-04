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

#ifndef IOSM_PCIE_PUBLIC_INTF_H
#define IOSM_PCIE_PUBLIC_INTF_H

#include <pci_ids.h>
#include <pci_regs.h>
#include <pci.h>

/// @brief PCI BAR selectors
#define PCI_DEV_BAR_0                   (0)
#define PCI_DEV_BAR_1                   (1)
#define PCI_DEV_BAR_2                   (2)
#define PCI_DEV_BAR_3                   (3)
#define PCI_DEV_BAR_4                   (4)
#define PCI_DEV_BAR_5                   (5)

/// @brief Number of pages of any Inter-SecProcess channel to the IOSM-PCIE service.
#define IOSM_PCIE_INCOMMING_COMM_ISP_PAGES          (1)




/********* Sub-Commands of <SP_MESSAGE> *********/
#define IOSM_PCIE_SUBCMD_INVALID                                        (0)

/// @brief Register the owner SecProcess of the given device.
/// Caller SecProcess must send a message to invoke this function. 
/// Format: <SP_MESSAGE->data> = bdf
/// Caller SecProcess must read response from the Inter-SecProcess shared memory channels. This operation does not 
/// request SecOS to send any response messages.
/// Format: <IOSM_PCIE_ISP_MSG->header.is_response> = true
///			<IOSM_PCIE_ISP_MSG->response.status1> = IOSM_PCIE_STATUS1_SUCCESS on success. Or IOSM_PCIE_STATUS1_FAIL on failure
///			<IOSM_PCIE_ISP_MSG->response.status2> = internal status
#define IOSM_PCIE_SUBCMD_REGISTER_DEV_OWNER                             (1)

/// @brief Unregister the owner SecProcess of the given device.
/// Caller SecProcess must send a message to invoke this function. 
/// Format: <SP_MESSAGE->data> = bdf
/// Caller SecProcess must read response from the Inter-SecProcess shared memory channels. This operation does not 
/// request SecOS to send any response messages.
/// Format: <IOSM_PCIE_ISP_MSG->header.is_response> = true
///			<IOSM_PCIE_ISP_MSG->response.status1> = IOSM_PCIE_STATUS1_SUCCESS on success. Or IOSM_PCIE_STATUS1_FAIL on failure
///			<IOSM_PCIE_ISP_MSG->response.status2> = internal status
#define IOSM_PCIE_SUBCMD_UNREGISTER_DEV_OWNER                           (2)

/// @brief Read the given PCI(e) device's common 256 bytes configuration space.
/// Caller SecProcess must send a message to invoke this function. 
/// Format: <SP_MESSAGE->data> = bdf
/// Caller SecProcess must read response from the Inter-SecProcess shared memory channels. This operation does not 
/// request SecOS to send any response messages.
/// Format: <IOSM_PCIE_ISP_MSG->header.is_response> = true
///			<IOSM_PCIE_ISP_MSG->response.status1> = IOSM_PCIE_STATUS1_SUCCESS on success. Or IOSM_PCIE_STATUS1_FAIL on failure
///			<IOSM_PCIE_ISP_MSG->response.status2> = internal status
///			<IOSM_PCIE_ISP_MSG->response.data_sz> = sizeof(struct pci_device)
///			<IOSM_PCIE_ISP_MSG->response.data> = value of <struct pci_device>
#define IOSM_PCIE_SUBCMD_READ_PCI_CFG_SPACE                             (10)
#define IOSM_PCIE_SUBCMD_WRITE_PCI_CFG_SPACE                            (11)

/// @brief Deny p2p memory accesses to the given device from all all devices (include the device itself).
/// [NOTE] If not denying a device's memory accesses to itself, IOSM modules cannot reliable access the device's MMIO registers. 
#define IOSM_PCIE_SUBCMD_DENY_MEMACCESS_TO_DEV                          (20)
#define IOSM_PCIE_SUBCMD_RESTORE_MEMACCESS_TO_DEV                       (21)

#define IOSM_PCIE_SUBCMD_IOMMU_DMA_MMAP                                 (30)
#define IOSM_PCIE_SUBCMD_IOMMU_DMA_MUNMAP                               (31)


// #define IOSM_PCIE_SUBCMD_SHARE_NS_OS_KERNEL_R                           (50)
// #define IOSM_PCIE_SUBCMD_UNSHARE_NS_OS_KERNEL                           (51)



#define IOSM_PCIE_SUBCMD_DEBUG_PCI_DUMP                                 (1000)




/********* Status of <IOSM_PCIE_ISP_MSG_RESPONSE> *********/
#define IOSM_PCIE_STATUS1_INVALID                   (0)
#define IOSM_PCIE_STATUS1_SUCCESS                   (1)
#define IOSM_PCIE_STATUS1_FAIL                      (2)


#ifndef __ASSEMBLY__

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;
} __attribute__ ((packed)) IOSM_PCIE_ISP_MSG_HEADER;

#define IOSM_PCIE_ISP_MSG_REQUEST_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    info1;
    uint32_t    info2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_PCIE_ISP_MSG_REQUEST_MAX_DATA_SIZE];
} IOSM_PCIE_ISP_MSG_REQUEST;

#define IOSM_PCIE_ISP_MSG_RESPONSE_MAX_DATA_SIZE     (3072)
typedef struct
{
    uint32_t    status1;
    uint32_t    status2;
    uint32_t    info3;
    uint32_t    info4;
    size_t      data_sz;

    char        data[IOSM_PCIE_ISP_MSG_RESPONSE_MAX_DATA_SIZE];
} IOSM_PCIE_ISP_MSG_RESPONSE;

typedef struct
{
    IOSM_PCIE_ISP_MSG_HEADER    header;
    
    union{
        IOSM_PCIE_ISP_MSG_REQUEST   request;
        IOSM_PCIE_ISP_MSG_RESPONSE  response;
    };
} IOSM_PCIE_ISP_MSG;




/********* Input Parameters of Sub-Commands *********/
typedef enum
{
    /// @brief Invalid type of IOMMU PT.
    IOMMU_PT_TYPE_INVALID = 0,

    /// @brief Support DMA accesses to secure domain's memory only.
    /// [NOTE] This type of IOMMU PT allows DMA destinations to be paddrs of the secure domain.
    IOMMU_PT_TYPE_EXCLUSIVE_DEV = 1,

    /// @brief Support DMA accesses to both secure domain's memory and non-secure domain's memory.
    /// [NOTE] This type of IOMMU PT requires that DMA destinations must be spaddrs instead of paddrs of 
    /// secure/non-secure domains.
    IOMMU_PT_TYPE_SHARED_DEV = 2
} IOMMU_PT_TYPE;

typedef struct
{
    union pci_bdf   bdf;
    IOMMU_PT_TYPE   iommu_pt_type;
} PCI_DEV_OWNER_REGISTER_PARAMS;

typedef struct
{
    union pci_bdf   bdf;
    IOMMU_PT_CONF   conf;
} DMA_MMAP_MUNMAP_PARAMS;

typedef struct
{
    shandle_t h_os_kernel_obj;
    spaddr_t os_kernel_base;
    size_t    os_kernel_size;
} NS_OS_KERNEL_OBJ_RESP;

typedef struct
{
    union pci_bdf bdf;
    uint32_t cfg_space_offset;
    uint32_t new_value;
    uint8_t write_size;
    bool read_after_write;
} PCI_CFG_SPACE_WRITE_PARAMS;

typedef struct
{
    uint32_t value;
} PCI_CFG_SPACE_READ_RESP;

static inline union pci_bdf pci_to_bdf(int bus, int dev, int fn) 
{
    union pci_bdf result;
    result.fn = fn;
    result.dev = dev;
    result.bus = bus;
    return result;
}

__attribute__((unused)) static pio_t pci_device_bar_to_pio(struct pci_device* pci, uint32_t bar_sel, bool* out_pio_addr)
{
    pci_bar_t val = 0;

    // Check: params must be valid
    if(!pci)
        return INVALID_SPADDR;
    if(bar_sel > PCI_DEV_BAR_5)
        return INVALID_SPADDR;

    val = pci->pci_bar[bar_sel];

    if((val & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO)
    {
        pio_t io_port_base = val & PCI_BASE_ADDRESS_IO_MASK;
        *out_pio_addr = true;

        return io_port_base;
    }
    
    // Otherwise, it is not a PIO addr
    *out_pio_addr = false;
    return 0;
}

/// @brief Get the MMIO spaddr from the given PCI BAR registers.
/// @param pci 
/// @param bar_sel 
/// @return 
__attribute__((unused)) static spaddr_t pci_device_bar_to_spaddr(struct pci_device* pci, uint32_t bar_sel, bool* out_64bit_mmio_addr)
{
    spaddr_t result = INVALID_SPADDR;
    pci_bar_t val = 0;

    // Check: params must be valid
    if(!pci)
        return INVALID_SPADDR;
    if(bar_sel > PCI_DEV_BAR_5)
        return INVALID_SPADDR;

    val = pci->pci_bar[bar_sel];

    if((val & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO)
    {
        // The BAR describes Port I/O, not MMIO
        return INVALID_SPADDR;
    }
    else if((val & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64)
    {
        // BAR contains a 64-bit address.
        //// A 64-bit base address register consumes 2 of the base address registers available.
        if(bar_sel > PCI_DEV_BAR_4)
            return INVALID_SPADDR;

        if(out_64bit_mmio_addr)
            *out_64bit_mmio_addr = true;

        result = (spaddr_t)UINT32_TO_64((uint32_t)pci->pci_bar[bar_sel + 1], (uint32_t)(pci->pci_bar[bar_sel] & PCI_BASE_ADDRESS_MEM_MASK));
    }
    else if((val & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_32)
    {
        // BAR contains a 32-bit address.
        if(out_64bit_mmio_addr)
            *out_64bit_mmio_addr = false;

        result = (spaddr_t)(pci->pci_bar[bar_sel] & PCI_BASE_ADDRESS_MEM_MASK);
    }

    return result;
}

#endif // __ASSEMBLY__
#endif // IOSM_PCIE_PUBLIC_INTF_H