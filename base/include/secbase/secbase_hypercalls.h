// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header - Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
 */

#ifndef SECBASE_HC_H
#define SECBASE_HC_H

#ifndef __ASSEMBLY__

#include "uxmhf64_compilers.h"

#define SECBASE_PRESENCE_REQ                0x47424F58    // "GBOX"
#define SECBASE_PRESENCE_RESP                0x4D454F57    // "MEOW"

// The begin number of hypercall functions for each IEE
#define NS_HC_FN_NUM_START                    0
#define S_HC_FN_NUM_START                    100000

///////////////// Hypercalls for entire NS domain (i.e., both Rich OS and TinyOS) /////////////////
#define NS_HC_INIT_SECBASE_ONDEMAND_PER_CPU    (NS_HC_FN_NUM_START + 0)
#define NS_HC_FINI_SECBASE_ONDEMAND_PER_CPU    (NS_HC_FN_NUM_START + 1)

///////////////// Hypercalls for Rich OS (Commodity OS) /////////////////
/// @brief Get the unique ID (32 byte maximum) of the current machine. For example, the ID is UXMHF64's SRK for machines
/// with TPM2.X.
#define NS_HC_GET_UNIQUE_MACHINE_ID         (NS_HC_FN_NUM_START + 5)

#define NS_HC_REGISTER_SECOS                (NS_HC_FN_NUM_START + 10)
#define NS_HC_UNREGISTER_SECOS              (NS_HC_FN_NUM_START + 11)
#define NS_HC_RESUME_SECOS                  (NS_HC_FN_NUM_START + 13)
#define NS_HC_READ_DMESG                    (NS_HC_FN_NUM_START + 20)

/// @brief Return the SecBase's boot state. It is always measured in SECBASE_BOOT_STATE_PCR_SRTM/DRTM if TPM is available.
#define NS_HC_GET_SECBASE_BOOT_STATE (NS_HC_FN_NUM_START + 30)

//// TPM related

/// @brief Return TPM PCRs related to SecBase (i.e., PCR 7 or 17) when SecBase initializes. TPM data is always in 
/// big-endian format when transmitted to or received from the TPM.
/// Both Ring 0 and ring 3 of the NS domain can invoke this hypercall.
/// @param CPU_REG_CX pointer of main memory regions in the NS domain to store the PCR value.
/// @param r 
/// @return 
#define NS_HC_TPM_GET_SECBASE_MEASUREMENT   (NS_HC_FN_NUM_START + 100)

#define NS_HC_TPM_SEAL_HYPAPP_SECRETS       (NS_HC_FN_NUM_START + 101)

#define NS_HC_TPM2_PROBE_SRK                (NS_HC_FN_NUM_START + 110)
#define NS_HC_TPM2_REPLACE_SRK              (NS_HC_FN_NUM_START + 111)
#define NS_HC_TPM2_REPLACE_AIK              (NS_HC_FN_NUM_START + 112)



//// Memory related
/// @brief Assign main memory ("mmem") described by guest's CPU_REG_CX to the S domain.
/// [NOTE] SecBase always clears these memory after returning them to the NS domain.
/// [NOTE] NS domain cannot use this function to assign any platform memory to the S domain.
/// @param CPU_REG_CX pointer of main memory regions in the NS domain.
/// @return Return MHV_SUCCESS on success.
///         Return MHV_* errors on corresponding errors.
#define NS_HC_ASSIGN_MMEM_TO_S               (NS_HC_FN_NUM_START + 1000)

//// Interrupt related
#define NS_HC_GET_LAPIC_ID                   (NS_HC_FN_NUM_START + 3000)

//// Debug related
// [Important] These hypercalls are disabled in production version
#ifdef SECBASE_DEVELOP
    #define OS_DEBUG_ATTACH_DEBUGGER         (NS_HC_FN_NUM_START + 10000)
    #define OS_DEBUG_DETACH_DEBUGGER         (NS_HC_FN_NUM_START + 10001)
    #define OS_DEBUG_ACCESS_VMCS             (NS_HC_FN_NUM_START + 10002)
    #define OS_DEBUG_ACCESS_MEM              (NS_HC_FN_NUM_START + 10003)
    #define OS_DEBUG_ACCESS_REGS             (NS_HC_FN_NUM_START + 10004)
        // Access mode
        #define DEBUG_READ_REG             0
        #define DEBUG_WRITE_REG            1
        // Register selector
        #define    REG_SEL_EAX                1
        #define    REG_SEL_EBX                2
        #define    REG_SEL_ECX                3
        #define    REG_SEL_EDX                4
        #define    REG_SEL_ESI                5
        #define    REG_SEL_EDI                6
        #define    REG_SEL_EBP                7
        #define    REG_SEL_ESP                8
        #define    REG_SEL_EFLAGS             9
        #define    REG_SEL_EIP                10
        #define    REG_SEL_CS                 20
        #define    REG_SEL_DS                 21
        #define    REG_SEL_ES                 22
        #define    REG_SEL_FS                 23
        #define    REG_SEL_GS                 24
        #define    REG_SEL_SS                 25

    PACKED_STRUCT_BEGIN
    union reg_debug
    {
        struct{
            uint32_t   access_mode        :  1,
                reg_sel    :  15,
                cpu_sel :  8,
                __reserved_1    : 8;
        } PACKED_STRUCT;

        uint32_t val;
    };
    PACKED_STRUCT_END

    #define OS_DEBUG_CONTINUE_EXECUTION        (NS_HC_FN_NUM_START + 10005)
        #define CONT_EXEC_OK            1
        #define CONT_EXEC_FAILED        2
    #define OS_DEBUG_SET_TF                    (NS_HC_FN_NUM_START + 10006)
        #define DISABLE_TF              0
        #define ENABLE_TF               1
    #define OS_DEBUG_BREAK                     (NS_HC_FN_NUM_START + 10007)
        #define BREAK_AT_ADDR           0
        #define BREAK_NOW               1
    #define OS_DEBUG_BREAK_DATA                (NS_HC_FN_NUM_START + 10008)
    #define OS_DEBUG_GVADDR_TO_GPADDR          (NS_HC_FN_NUM_START + 10009)
#endif

//// x86 Specific
#define NS_HC_X86_PIO_DUMP_PCI_CFG_SPACE                (NS_HC_FN_NUM_START + 20000)

//// Untrusted OS's interfaces to send commands to TinyOS

/// @brief Get TinyOS's info; e.g., LAPIC id of the TinyOS's CPU, running or not.
/// @param CPU_REG_CX The result buffer of the info.
/// @return 
#define NS_HC_GET_TINYOS_INFO                 (NS_HC_FN_NUM_START + 99800)
#define     TINYOS_IPI_VECTOR           (0x80) // Use legacy rich OSes' system call interrupt vector, because they are
                                            // less likely to configure peripheral devices to use this interrupt vector.

/// @brief Boot TinyOS on the hided AP CPUs. The current CPU sends an INIT signal to those CPUs.                                            
#define NS_HC_BOOT_TINYOS                 (NS_HC_FN_NUM_START + 99801)

/// @brief Send a request from the untrusted rich OS to TinyOS.
/// @param CPU_REG_CX The command to the TinyOS.
/// @param CPU_REG_DX Arg1 of the command.
/// @param CPU_REG_SI Arg2 of the command.
/// @param CPU_REG_DI Arg3 of the command.
/// @return 
#define NS_HC_REQUEST_TINYOS                 (NS_HC_FN_NUM_START + 99805)

/// @brief Receive the request response sent from TinyOS.
/// @param CPU_REG_CX The command to the TinyOS.
/// @param CPU_REG_DX Status1 of the command.
/// @param CPU_REG_SI Status2 of the command.
/// @param CPU_REG_DI Status3 of the command.
/// @return 
#define NS_HC_GET_TINYOS_RESPONSE            (NS_HC_FN_NUM_START + 99806)

///////////////// Hypercalls for TinyOS /////////////////
#define NS_HC_TINYOS_DELIVER_ALL_EXCEPTIONS_TO_TINYOS   (NS_HC_FN_NUM_START + 99900)

/// @brief Set TinyOS's status.
/// @param CPU_REG_CX The new status.
/// @return 
#define NS_HC_TINYOS_SET_STATUS              (NS_HC_FN_NUM_START + 99901)
#define     TINYOS_STATUS_HALT           (0)
#define     TINYOS_STATUS_RUNNING        (1)

/// @brief Receive a request sent from the untrusted rich OS to TinyOS.
/// @param CPU_REG_CX The command to the TinyOS.
/// @param CPU_REG_DX Arg1 of the command.
/// @param CPU_REG_SI Arg2 of the command.
/// @param CPU_REG_DI Arg3 of the command.
/// @return 
#define NS_HC_TINYOS_GET_REQUEST             (NS_HC_FN_NUM_START + 99910)
#define     REQ_TINYOS_CMD_INVALID          (0)
#define     REQ_TINYOS_CMD_BOOT_SECOS       (1)

/// @brief Send a request response from TinyOS to the untrusted rich OS.
/// @param CPU_REG_CX The command handled by TinyOS.
/// @param CPU_REG_DX Status1 of the command.
/// @param CPU_REG_SI Status2 of the command.
/// @param CPU_REG_DI Status3 of the command.
/// @return 
#define NS_HC_TINYOS_SET_RESPONSE            (NS_HC_FN_NUM_START + 99911)
#define     RESP_TINYOS_SUCCESS          (0)
#define     RESP_TINYOS_FAIL             (1)

#define NS_HC_TINYOS_BOOT_SECOS              (NS_HC_FN_NUM_START + 99920)




#define NS_HC_TEST3                          (NS_HC_FN_NUM_START + 99997)
#define NS_HC_TEST2                          (NS_HC_FN_NUM_START + 99998)
#define NS_HC_TEST                           (NS_HC_FN_NUM_START + 99999)

// SecOS only
//// General service
#define SOS_HC_INIT_WK_MHV_COMM               (S_HC_FN_NUM_START + 1)
#define SOS_HC_FINI_WK_MHV_COMM               (S_HC_FN_NUM_START + 2)
#define SOS_HC_DELIVER_ALL_EXCEPTIONS_TO_SOS  (S_HC_FN_NUM_START + 3)
#define WK_HC_READ_MHV_PERF                   (S_HC_FN_NUM_START + 10)
#define WK_HC_CLEAR_MHV_PERF                  (S_HC_FN_NUM_START + 11)
#define SOS_HC_GET_SECBASE_CPU_IDX            (S_HC_FN_NUM_START + 12)

/// @brief Use hardware timer to delay CPU for <rcx/ecx> milli-seconds. The larger the value is, the more accurate for
/// the delay due to overhead to enable the hardware timer and prepare the hypercall.
/// @param vcpu 
/// @param r 
/// @return 
#define SOS_HC_HWTIMER_MDELAY                 (S_HC_FN_NUM_START + 13)

/// @brief Return sensitive platform info (e.g., hardware configurations that typically not exposed to untrusted  
/// ring-3 applications).
/// @param vcpu 
/// @param r 
/// @return 
#define SOS_HC_GET_PLATFORM_INFO_SENSITIVE    (S_HC_FN_NUM_START + 20)

/// @brief Return non-sensitive platform info (e.g., hardware configurations that can be known by untrusted ring-3 
/// applications).
/// @param vcpu 
/// @param r 
/// @return 
#define SOS_HC_GET_PLATFORM_INFO_NON_SENSITIVE  (S_HC_FN_NUM_START + 21)
#define SOS_HC_GET_PRNG_SEEDED_FROM_HW          (S_HC_FN_NUM_START + 25)
    #define PRNG_MAX_LEN                  (1024)

//// SecProcess/uTPM
#define SOS_HC_HASH_SECPROCESS                  (S_HC_FN_NUM_START + 1000)
#define SOS_HC_VERIFY_SECPROCESS                (S_HC_FN_NUM_START + 1001)

/// @brief Set the uTPM's master secret and re-initialize the uTPM.
/// @param vcpu 
/// @param r 
/// @return Return MHV_SUCCESS on success.
///         Return MHV_* errors on corresponding errors.
#define SOS_HC_uTPM_SET_SECRET_AND_REINITIALIZE (S_HC_FN_NUM_START + 1010)


/// @brief Set the given PCR with the given measurement.
/// @param vcpu 
/// @param r 
/// @return Return MHV_SUCCESS on success.
///         Return MHV_* errors on corresponding errors.
#define SOS_HC_uTPM_SET_MEASUREMENT            (S_HC_FN_NUM_START + 1012)

#define SOS_HC_uTPM_GET_QUOTE                  (S_HC_FN_NUM_START + 1013)

//// Security related 
#define SOS_HC_CHECK_SECBASE_VERSION           (S_HC_FN_NUM_START + 5000)
#define     SECBASE_DEVELOPMENT_BUILD      1
#define        SECBASE_RETAIL_BUILD        2
#define SOS_HC_EXIT_SECOS                      (S_HC_FN_NUM_START + 5001)

//// Mem/IO ports related
#define SOS_GPADDR_TO_SPADDR                   (S_HC_FN_NUM_START + 10000)
#define SOS_HC_VIRT_NS_MEM                     (S_HC_FN_NUM_START + 10001)
#define SOS_ASSIGN_MEM_TO_NS                   (S_HC_FN_NUM_START + 10002)
#define SOS_HC_TRAP_ACCESS_INIT                (S_HC_FN_NUM_START + 10005)
#define SOS_HC_TRAP_ACCESS_FINI                (S_HC_FN_NUM_START + 10006)
#define SOS_HC_TRAP_MEMORY_ACCESS              (S_HC_FN_NUM_START + 10007)
#define     MEM_PASSTHROUGH                0
#define        MEM_MEDIATE_READ             (1 << 0) // [NOTE] Execution is treated same as Read permission, but do not support mediation
#define        MEM_MEDIATE_WRITE            (1 << 1)
#define        MEM_BLOCK_ACCESS             (1 << 2)
#define        MEM_NOTIFY_ACCESS            (0 << 2)
#define SOS_HC_TRAP_MEMORY_ACCESS_MULTI        (S_HC_FN_NUM_START + 10008)
#define SOS_HC_TRAP_PIO_ACCESS                 (S_HC_FN_NUM_START + 10009)
#define     PIO_PASSTHROUGH                0
#define        PIO_MEDIATE_READ             (1 << 0)
#define        PIO_MEDIATE_WRITE            (1 << 1)
#define        PIO_BLOCK_ACCESS             (1 << 2)
#define        PIO_NOTIFY_ACCESS            (0 << 2)
#define SOS_HC_ASSIGN_PLATFORM_MEM_TO_S        (S_HC_FN_NUM_START + 10050) // [TODO] Merge with SOS_ASSIGN_MEM_TO_S
#define SOS_HC_ASSIGN_PLATFORM_MEM_TO_NS       (S_HC_FN_NUM_START + 10051) // [TODO] Merge with SOS_ASSIGN_MEM_TO_NS

/// @brief Update the cache policy in the SecBase's page table; i.e., always mapping identically from hva to spa.
/// @param CPU_REG_CX Base spaddr of the memory region.
/// @param CPU_REG_DX Number of 4K pages of the memory region.
/// @param CPU_REG_SI True iff enable cache, false iff disable cache.
/// @return 
#define SOS_HC_SECBASE_PT_UPDATE_CACHE         (S_HC_FN_NUM_START + 10052)

//// Interrupts related
#define SOS_HC_INTR_OS_LAPIC_ID                (S_HC_FN_NUM_START + 11001)
#define SOS_HC_INTR_NMI_DISABLE                (S_HC_FN_NUM_START + 11002)
#define SOS_HC_INTR_NMI_ENABLE                 (S_HC_FN_NUM_START + 11003)

//// IOMMU related
#define SOS_HC_IOMMU_PT_CREATE_S_EXCLUSIVE                 (S_HC_FN_NUM_START + 15000)
#define SOS_HC_IOMMU_PT_CREATE_S_NS_SHARED                 (S_HC_FN_NUM_START + 15001) // [TODO] Merge with SOS_HC_IOMMU_PT_CREATE_S_EXCLUSIVE
#define SOS_HC_IOMMU_PT_DESTROY                            (S_HC_FN_NUM_START + 15002)
#define SOS_HC_IOMMU_PT_MAP_S_EXCLUSIVE                    (S_HC_FN_NUM_START + 15003)
#define        DMA_ACCESS_ALLOW            1
#define     DMA_ACCESS_DENY                2
#define SOS_HC_IOMMU_PT_MAP_S_NS_SHARED                    (S_HC_FN_NUM_START + 15004)
#define SOS_HC_IOMMU_PT_BIND_DEVICE                        (S_HC_FN_NUM_START + 15005)
#define SOS_HC_IOMMU_PT_UNBIND_DEVICE                      (S_HC_FN_NUM_START + 15006)

//// IOMMU for IGFX quirks related
#define SOS_HC_IOMMU_IGFX_ENABLE_VTD                       (S_HC_FN_NUM_START + 15400)
#define SOS_HC_IOMMU_IGFX_DISABLE_VTD                      (S_HC_FN_NUM_START + 15401)

//// IOMMU Debug related
#define SOS_HC_IOMMU_DEBUG_PRINT_AND_CLEAR_FAULT_REGS      (S_HC_FN_NUM_START + 15500)
#define SOS_HC_IOMMU_DEBUG_RESTART_DMA_IOMMU               (S_HC_FN_NUM_START + 15501)
#define SOS_HC_IOMMU_DEBUG_DISABLE_DMA_IOMMU               (S_HC_FN_NUM_START + 15502)

//// x86 Specific
#define SOS_HC_X86_IOAPIC_FORCE_DEFINE_NS_ACCESS           (S_HC_FN_NUM_START + 20000)
#define SOS_HC_X86_IOAPIC_FORCE_UNDEFINE_NS_ACCESS         (S_HC_FN_NUM_START + 20001)

#define SOS_HC_X86_MAP_PLATFORM_MEM_TO_S                   (S_HC_FN_NUM_START + 20100)
#define SOS_HC_UNMAP_PLATFORM_MEM_FROM_S                   (S_HC_FN_NUM_START + 20101)

//// Device Specific
#define SOS_HC_DEV_HPET_GET_INFO                           (S_HC_FN_NUM_START + 23000)

#define SOS_HC_DEV_PCIE_REGISTER                           (S_HC_FN_NUM_START + 24000)
#define SOS_HC_DEV_PCIE_UNREGISTER                         (S_HC_FN_NUM_START + 24001)
#define SOS_HC_DEV_PCIE_ASSIGN_TO_S                        (S_HC_FN_NUM_START + 24002)
#define SOS_HC_DEV_PCIE_ASSIGN_TO_NS                       (S_HC_FN_NUM_START + 24003)

#define SOS_HC_DEV_XHCI_REGISTER_BASE_SPADDR               (S_HC_FN_NUM_START + 25000)

//// General Debug
#define SOS_HC_DEBUG_PRINT_CLEAR_EVENT_COUNTERS            (S_HC_FN_NUM_START + 50000)

#define SOS_HC_TEST1                                       (S_HC_FN_NUM_START + 90000)
#define SOS_HC_TEST2                                       (S_HC_FN_NUM_START + 90001)
#define SOS_HC_TEST3                                       (S_HC_FN_NUM_START + 90002)

typedef struct _sos_hc_params_trap_memory
{
    ulong_t    dest_spa;
    size_t            trap_size;
    uint32_t         mediation_type;
} SOS_HC_PARAMS_TRAP_MEM;


/// @brief Parameters for the hypercall SOS_HC_HASH_SECPROCESS
typedef struct _sos_hc_params_hash_secprocess
{
    ulong_t         sp_code_gvaddr;
    size_t          sp_code_size;
    ulong_t         sp_rodata_gvaddr;
    size_t          sp_rodata_size;
    ulong_t         sp_data_gvaddr;
    size_t          sp_data_size;

    ulong_t         sp_sparams_gvaddr;

    // If not 0, then SecBase hashes the given secure parameters region [sp_sparams_vaddr, sp_sparams_vaddr + sp_sparams_size)
    size_t          sp_sparams_size;  

    ulong_t         out_sp_img_hash_gvaddr;
    ulong_t         out_sp_img_hash_size_gvaddr;
    ulong_t         out_sp_sparams_hash_gvaddr;
    ulong_t         out_sp_sparams_hash_size_gvaddr;
} SOS_HC_PARAMS_HASH_SECPROCESS;

typedef struct
{
    ulong_t    pcie_cfg_space_mmio_base_spaddr;
} SOS_HC_PLATFORM_SENSITIVE_INFO;

#define SECBASE_MACHINE_ID_MAX_LENGTH (32) // Must be <= UXMHF64_MACHINE_ID_MAX_LENGTH because SecBase cannot report 
                            // machine ID longer than <UXMHF64_MACHINE_ID_MAX_LENGTH>.


/// @brief 
/// [NOTE] This struct may not be cache aligned. Packed this struct to prevent padding by different compilers.
PACKED_STRUCT_BEGIN
typedef struct PACKED_STRUCT
{
    uint8_t machine_id[SECBASE_MACHINE_ID_MAX_LENGTH];
    uint8_t machine_id_is_available; // 1 iff the SecBase can retrieve the machine ID. Otherwise 0.
    
    uint32_t enable_tpm:1, // Set iff SecBase initialized the physical TPM device successfully.
             enable_efi_tcg2_protocol:1,    // Set iff SecBase initialized the UEFI TCG2 protocol successfully.
             enable_utpm:1, // Set iff SecBase initialized the uTPM successfully.
             __reserved:29;
    uint64_t cpu_freq_hz;
    uint64_t bus_freq_hz; 
} SOS_HC_PLATFORM_NON_SENSITIVE_INFO;
PACKED_STRUCT_END




/********* uTPM Data Structures *********/
#ifndef GreenBox_uTPM_Definitions
#define GreenBox_uTPM_Definitions 1 // Avoid double definition of uTPM data structures in both SecBase and SecOS

/// @brief Length of each uTPM PCR.
#define uTPM_MEASUREMENT_LENGTH                 (32)

/// @brief SecOS may reset PCR 4 and above.
#define uTPM_SECOS_RESETTABLE_PCR_FROM_INDEX    (4)

/// @brief Number of PCRs in uTPM.
/// [NOTE] This value must be the same as <TPM_PCR_NUM>
#define uTPM_PCR_NUM                            (32)

/// @brief The value must be the same with the <outlen> returned by <utpm_quote>. The value is TPM_RSA_KEY_LEN.
#define TPM_QUOTE_SIGNATURE_LEN             (256)
#define TPM_QUOTE_PUBLIC_KEY_LEN_MAX        (384)

PACKED_STRUCT_BEGIN
struct uTPM_PCR_SELECTION {
    uint16_t sizeOfSelect;            /* The size in bytes of the pcrSelect structure */
    uint8_t pcrSelect[uTPM_PCR_NUM/8]; /* This SHALL be a bit map that indicates if a PCR
                                            is active or not */
} PACKED_STRUCT;
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN
struct utpm_pcr
{
    uint8_t value[uTPM_MEASUREMENT_LENGTH];
} PACKED_STRUCT;
PACKED_STRUCT_END

/// @brief Corresponding to TPM_PCR_COMPOSITE in "8.2 TPM_PCR_COMPOSITE" of TPM 1.2 TPM Structures.
PACKED_STRUCT_BEGIN
struct utpm_pcr_comp
{
    struct uTPM_PCR_SELECTION select;

    /// @brief This SHALL be the size of the pcrValue field (not the number of PCR’s). <valueSize> is big endian.
    uint32_t valueSize;
    struct utpm_pcr pcrValue[uTPM_PCR_NUM];
} PACKED_STRUCT;
PACKED_STRUCT_END

#endif // GreenBox_uTPM_Definitions





/********* TPM Data Structures *********/
#define SECBASE_IMG_PCR_SRTM    (4) // Must be the same with <UXMHF64_IMAGE_PCR_SRTM>
#define SECBASE_IMG_PCR_DRTM    (18) // Must be the same with <UXMHF64_IMAGE_PCR_DRTM>
#define SECBASE_BOOT_STATE_PCR_SRTM    (15) // Must be the same with <UXMHF64_BOOT_STATE_PCR_SRTM>
#define SECBASE_BOOT_STATE_PCR_DRTM    (19) // Must be the same with <UXMHF64_BOOT_STATE_PCR_DRTM>


/// @brief Which root of trust is used by the SecBase.
/// [NOTE] Do not use enum because its length may vary in different compilers.
typedef uint32_t SECBASE_ROOT_OF_TRUST_TYPE;
#define SECBASE_ROT_INVALID     0U
#define SECBASE_ROT_SRTM        1U
#define SECBASE_ROT_DRTM        2U

// Define SecBase's TPM digest algorithms as uint32_t (Do not use enum because its length may vary in different compilers)
typedef uint32_t SECBASE_TPM_DIGEST_ALG;
#define SECBASE_TPM_ALG_INVALID     0U
#define SECBASE_TPM_ALG_SHA1        1U
#define SECBASE_TPM_ALG_SHA256      2U
#define SECBASE_TPM_ALG_SHA384      3U
#define SECBASE_TPM_ALG_SHA512      4U
#define SECBASE_TPM_ALG_SM3_256     5U

// Define SecBase's TPM event types as uint32_t (Do not use enum because its length may vary in different compilers)
typedef uint32_t SECBASE_TPM_EVENT_TYPE;
#define SECBASE_TPM_EV_INVALID                            0U
#define SECBASE_TPM_EV_PREBOOT_CERT                       1U
#define SECBASE_TPM_EV_POST_CODE                          2U
#define SECBASE_TPM_EV_NO_ACTION                          3U
#define SECBASE_TPM_EV_SEPARATOR                          4U
#define SECBASE_TPM_EV_ACTION                             5U
#define SECBASE_TPM_EV_EVENT_TAG                          6U
#define SECBASE_TPM_EV_S_CRTM_CONTENTS                    7U
#define SECBASE_TPM_EV_S_CRTM_VERSION                     8U
#define SECBASE_TPM_EV_CPU_MICROCODE                      9U
#define SECBASE_TPM_EV_PLATFORM_CONFIG_FLAGS              10U
#define SECBASE_TPM_EV_TABLE_OF_DEVICES                   11U
#define SECBASE_TPM_EV_COMPACT_HASH                       12U
#define SECBASE_TPM_EV_NONHOST_CODE                       13U
#define SECBASE_TPM_EV_NONHOST_CONFIG                     14U
#define SECBASE_TPM_EV_NONHOST_INFO                       15U
#define SECBASE_TPM_EV_OMIT_BOOT_DEVICE_EVENTS            16U
#define SECBASE_TPM_EV_EFI_VARIABLE_DRIVER_CONFIG         17U
#define SECBASE_TPM_EV_EFI_VARIABLE_BOOT                  18U
#define SECBASE_TPM_EV_EFI_BOOT_SERVICES_APPLICATION      19U
#define SECBASE_TPM_EV_EFI_BOOT_SERVICES_DRIVER           20U
#define SECBASE_TPM_EV_EFI_RUNTIME_SERVICES_DRIVER        21U
#define SECBASE_TPM_EV_EFI_GPT_EVENT                      22U
#define SECBASE_TPM_EV_EFI_ACTION                         23U
#define SECBASE_TPM_EV_EFI_PLATFORM_FIRMWARE_BLOB         24U
#define SECBASE_TPM_EV_EFI_HANDOFF_TABLES                 25U
#define SECBASE_TPM_EV_EFI_VARIABLE_AUTHORITY             26U

#define TPM_DIGEST_MAX_LENGTH   (64) // Corresponds to the size of <tpm_pcr_value_t>

typedef struct 
{
    SECBASE_TPM_EVENT_TYPE ev_type;

    SECBASE_TPM_DIGEST_ALG digest_algo;
    uint8_t digest[TPM_DIGEST_MAX_LENGTH];
    uint32_t digest_size;
} SECBASE_TPM_EVENT_LOG_ENTRY;

#define SECBASE_TPM_EVENT_ENTRIES_MAX (16) // Must be the same with <TPM_EVENT_ENTRIES_MAX>

typedef struct
{
    uint32_t count;
    SECBASE_TPM_EVENT_LOG_ENTRY entries[SECBASE_TPM_EVENT_ENTRIES_MAX];
} SECBASE_TPM_EVENT_LOG_TRACE;

typedef struct
{
    SECBASE_ROOT_OF_TRUST_TYPE rot_type; /// Type of root of trust used by the SecBase
    uint8_t img_digest[TPM_DIGEST_MAX_LENGTH];
    uint32_t img_digest_size;
    uint8_t boot_state_digest[TPM_DIGEST_MAX_LENGTH];
    uint32_t boot_digest_size;

    uint32_t img_pcr_idx;
    uint32_t boot_state_pcr_idx;

    // History of TPM PCRs
    uint8_t boot_state_pcr_orig[TPM_DIGEST_MAX_LENGTH];
    uint32_t boot_state_pcr_orig_size;

    SECBASE_TPM_EVENT_LOG_TRACE img_state_trace;
} NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT;




/********* NS_HC_TPM_SEAL_HYPAPP_SECRETS *********/
/// @brief Must be compatible with the <SEALED_OBJ_STRUCT> struct.
#define NS_HC_PARAMS_TPM_SEAL_HYPAPP_SECRETS_BUF_MAX_SIZE (2048) 

/// @brief Must be compatible with the <SEALED_OBJ_STRUCT> struct. So the caller can serialize its value to a file and
/// UXMHF64 can then de-serialize it later. 
typedef struct 
{
    uint32_t seal_priv_size;
    uint32_t seal_pub_size;

    uint8_t buf[NS_HC_PARAMS_TPM_SEAL_HYPAPP_SECRETS_BUF_MAX_SIZE];
} NS_HC_RESP_TPM_SEAL_HYPAPP_SECRETS;

/********* NS_HC_GET_TINYOS_INFO *********/
typedef struct
{
    uint32_t tos_lapic_id;
    uint32_t tos_ipi_vector;

    uint32_t running_status;
} NS_HC_RESP_GET_TINYOS_INFO;



/********* SOS_HC_VERIFY_SECPROCESS *********/
/// @brief Parameters for the hypercall SOS_HC_VERIFY_SECPROCESS
typedef struct _sos_hc_params_verify_secprocess
{
    ulong_t         sp_signature;
    size_t          sp_signature_size;
    ulong_t         sp_pubkey;
    size_t          sp_pubkey_size;
    size_t          saltlen;
    ulong_t         sp_img_hash;
    size_t          sp_img_hash_size;
} SOS_HC_PARAMS_VERIFY_SECPROCESS;

#define uTPM_MASTER_SEALING_SECRET_SIZE   (32) // Must be the same with <HW_TPM_MASTER_SEALING_SECRET_SIZE>

PACKED_STRUCT_BEGIN
typedef struct PACKED_STRUCT
{
    uint8_t         secret[uTPM_MASTER_SEALING_SECRET_SIZE];
} SOS_HC_PARAMS_uTPM_SET_SECRET;
PACKED_STRUCT_END

/// @brief Parameters for the hypercall SOS_HC_VERIFY_SECPROCESS
PACKED_STRUCT_BEGIN
typedef struct PACKED_STRUCT
{
    uint32_t        pcr;
    uint8_t         measurement[uTPM_MEASUREMENT_LENGTH];
} SOS_HC_PARAMS_uTPM_SET_MEASUREMENT;
PACKED_STRUCT_END




/********* SOS_HC_uTPM_GET_QUOTE *********/

// SecBase always generates a new uTPM AIK on every machine reboot and measures it into physical TPMs. This allows
// users to verify the authenticity of SecBase even if users lost secrets (e.g., due to upgrade CPUs or other devices) 
// sealed in physical TPMs. Thus, the TPM quote version does not have a value like uTPM_QUOTE_PERSISTENT_KEYS.
//
// The GreenBox verifier should verify both the quotes generated by the uTPMs and the quotes generated by the physical
// TPMs in order to ensure that quotes are generated by benign GreenBox.

#define uTPM_QUOTE_NONCE_LENGTH  (32)

/// @brief uTPM quote version: Invalid version.
#define uTPM_QUOTE_VERSION_INVALID (0)
/// @brief uTPM quote version: uTPM has random AIK on each machine reboot. Thus, verifiers must verify uTPM quotes and  
/// physical TPM1.2 devices' quotes together.
#define uTPM_QUOTE_RANDOM_AIK_USE_TPM1 (1)
/// @brief uTPM quote version: uTPM has random AIK on each machine reboot. Thus, verifiers must verify uTPM quotes and  
/// physical TPM2.0 devices' quotes together.
#define uTPM_QUOTE_RANDOM_AIK_USE_TPM2 (2)

/// @brief Parameters for the hypercall SOS_HC_uTPM_GET_QUOTE
PACKED_STRUCT_BEGIN
typedef struct PACKED_STRUCT
{
    uint8_t                 nonce[uTPM_QUOTE_NONCE_LENGTH];

    /*** Output info ***/
    uint32_t                quote_version;
    uint32_t                quote_fixed;
    struct utpm_pcr_comp    pcr_comp;

    uint8_t                 sig[TPM_QUOTE_SIGNATURE_LEN];

    uint8_t                 public_key[TPM_QUOTE_PUBLIC_KEY_LEN_MAX];
    uint32_t                public_key_size;
} SOS_HC_PARAMS_uTPM_GET_QUOTE;
PACKED_STRUCT_END


//! <cpu_idx> of VCPU in SecBase; i.e., logic processor id in SecBase. 
//! [NOTE] XMHF defines VCPU's cpu_id as lapic_id and defines <cpu_idx> as logic processor id, so SecBase uses the name
//! <cpu_idx>.
typedef uint32_t     sb_cpu_idx_t;

/********* NS_HC_GET_SECBASE_BOOT_STATE *********/
///
#define SECBASE_BOOT_STATE_SIZE (1024) // Must be the same with <MHV_BOOT_STATE_SIZE> defined in UXMHF64

PACKED_STRUCT_BEGIN

typedef struct PACKED_STRUCT
{
    /// @brief Size of the RSA public key to verify uTPM quotes.
    uint32_t pub_key_size;

    /// @brief The RSA public key to verify uTPM quotes.
    uint8_t pub_key[TPM_QUOTE_PUBLIC_KEY_LEN_MAX];

    /// @brief uTPM flags
    uint64_t initialized: 1,
             has_prng:1,
             __reserved1: 62;
} UTPM_BOOT_STATE;

/// @brief 
/// [NOTE] This struct must be the same with <MHV_SENSITIVE_BOOT_STATE> in mHV.
typedef struct PACKED_STRUCT
{
    uint8_t is_running;
    uint8_t __reserved1[3]; // paddings

    // CPU (BSP) Info collected during mHV initialization
    uint32_t mhv_init_bsp_vendor;
    uint32_t mhv_init_bsp_model;
    uint32_t mhv_init_bsp_ucode_rev;
} NS_HC_PARAMS_GET_SECBASE_BOOT_STATE_SENSITIVE;

/// @brief 
/// [NOTE] This struct must be compatible with <MHV_BOOT_STATE> in mHV.
typedef struct PACKED_STRUCT
{
    NS_HC_PARAMS_GET_SECBASE_BOOT_STATE_SENSITIVE sensitive_boot_state;

    UTPM_BOOT_STATE utpm;

    /// @brief Unused field, should be all 0s.
    uint8_t __reserved1[SECBASE_BOOT_STATE_SIZE - sizeof(NS_HC_PARAMS_GET_SECBASE_BOOT_STATE_SENSITIVE) - sizeof(UTPM_BOOT_STATE)];
} NS_HC_PARAMS_GET_SECBASE_BOOT_STATE;
PACKED_STRUCT_END


/********* Device Specific *********/
typedef enum
{
    SECBASE_ADDR_TYPE_INVALID = 0,
    SECBASE_ADDR_TYPE_MEMORY = 1,
    SECBASE_ADDR_TYPE_PIO = 2
} secbase_addr_type_t; // Must be the same with <addr_type_t>

PACKED_STRUCT_BEGIN
typedef struct PACKED_STRUCT
{
    secbase_addr_type_t addr_type;

    ulong_t hpet_base_spa;
    ulong_t hpet_base_pio;
} SOS_HC_RESP_DEV_HPET_GET_INFO;
PACKED_STRUCT_END

#endif // __ASSEMBLY__
#endif // SECBASE_HC_H