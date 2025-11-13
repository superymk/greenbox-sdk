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

/// @file System calls of SecOS.
/// [NOTE] Why different subsystems may have their own individual *_INFO system call each, instead of putting them 
/// together into a single one?
/// Separate *_INFO system calls improve the performance of SecProcesses. Otherwise, a bulky *_INFO system call may be
/// slow in collecting some info unnecessary for the current caller.

#ifndef SECOS_SYSCALLS_H
#define SECOS_SYSCALLS_H

#include "secapp_defs.h"
#include "vk_common_intfs.h"

// SecOS Syscall table
// General syscalls
#define SYS_SP_TEST									    0
#define SYS_IDLEPROC_KILLALL_ZOMBIES					1 //! Release resources of all zombie processes.
#define SYS_SP_SELF_EXIT								2
#define SYS_SP_GET_INFO                                 4
#define SYS_SP_QUERY_ID									5

/// @brief Return all runnable SecProcess's IDs. The result does not contain SecProcesses in the zombie state. 
/// [NOTE] Only the INIT SecProcess is allowed to invoke this system call.
///
/// @param arg1 Result buffer of SecProcess' IDs
///
/// @return Return SUCCESS on success.
#define SYS_SP_GET_ALL_RUNNABLE_SP_IDs					6

/// @brief Force switch to the runnable SecProcess <arg1> on the next signal return of the current SecProcess
/// [NOTE] Only the INIT SecProcess is allowed to invoke this system call.
///
/// @param arg1 The ID of the target SecProcess
///
/// @return Return SUCCESS on success. 
///         Return EINVAL if the target SecProcess does not exist or runnable
///         Return EPERM is the caller is not INIT SecProcess.
#define SYS_SP_FORCE_SIGNAL_RETURN_TO_SP				7

#define SYS_SP_PRINT								    8
#define SYS_SP_ACK_MHV_EVENT							10
// #define SYS_MEMOBJ_MEDIATE_ACCESS						13 // [TODO] need it?
#define SYS_SP_YIELD_CPU								12

/// @brief Notify SecOS about the SECSRV-INIT's initialization status.
/// @param arg1 TRUE if SECSRV-INIT is intialized successfully. Otherwise false.
/// @return Always return SUCCESS
#define SYS_SP_SECSRV_INIT_SET_INITIALIZED				16




/********* Syscalls - NS domain *********/
/// @brief Return the information of the OS in the NS domain.
/// @param arg1 The vaddr of the output result.
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
///         Return EINTERNAL on internal errors.
#define SYS_NS_OS_GET_INFO                              40




/********* Syscalls - Time *********/
/// SecOS defines multiple terms for different timers:
/// Global time: The time elapsed after SecOS boots. It includes all the time spent on both the S domain and the NS 
/// domain after SecOS boots. The unit is second (e.g., s/ms/us/ns).
/// Wall clock time: Unix (wall clock) time. It is the seconds that have elapsed since 1970/01/01.
/// NS domain's time: The time spent in the untrusted OS/Apps after SecOS boots.

/// @brief Get the current CPU's frequency recorded at the time SecOS boots. The result is NOT the current CPU frequency
///
/// @param arg1 Pointer to the output buffer of <uint64_t>.
///
/// @return Return SUCCESS on success.
///			Return ETIMER if the time subsystem encounters errors.
#define SYS_GET_CURRENT_CPU_BOOT_FREQ					50

/// @brief Init the wall clock timer in SecOS. Only trusted SecProcesses can issue this system call.
///
/// @param arg1 Pointer to the input struct TIMEVAL
/// @param arg2 Pointer to the input struct TIMEZONE
///
/// @return Return SUCCESS on success.
///			Return ETIMER if the time subsystem encounters errors.
///         Return EPERM if the caller SecProcess is not a trusted SecProcess
#define SYS_TIME_INIT_WALL_CLOCK						51

/// @brief Get the wall clock timer in SecOS.
/// [NOTE] The returned value may be inaccurate and untrustworthy, because the untrusted OS tells SecOS about the initial
/// wall clock value.
///
/// @param arg1 Pointer to the output struct TIMEVAL
/// @param arg2 Pointer to the output struct TIMEZONE
///
/// @return Return SUCCESS on success.
///			Return ETIMER if the time subsystem encounters errors or the wall clock is uninitialized.
#define SYS_TIME_GET_WALL_CLOCK							52


/// @brief Get the current SecOS kernel time.
/// [NOTE] The returned value may be inaccurate and untrustworthy. [TODO][Issue 170].
///
/// @param arg1 Pointer to the output buffer of <sos_timespec_t>.
/// 
/// @return Return SUCCESS on success.
///			Return ETIMER if the time subsystem encounters errors.
///			Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to access the result buffer.
#define SYS_TIME_GET_GLOBAL								53




/********* Syscalls - uTPM *********/
/// @brief Invoke SecBase to set the uTPM's secret and re-initialize uTPM's sealed storage functions based on the new
/// secret. Only a subset of trusted SecProcesses can issue this system call.
/// @param arg1 Caller SecProcess's memory buffer of the new secret.
/// @param arg2 
/// @param arg3 
/// @return Return SUCCESS on success.
///         Return ENOMEM on memory allocation failure.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM set secret errors.
#define SYS_uTPM_SET_SECRET_AND_REINIT_STORAGE_OPS		100

/// @brief Return the information of the uTPM.
/// @param arg1 The vaddr of the output result.
/// @param arg2 
/// @param arg3 
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to access the output memory.
///         Return EINTERNAL on internal errors.
#define SYS_uTPM_GET_INFO		                        101

/// @brief Reset and extend measurements of the given SecApp (i.e., image and secure parameters) into the given Platform 
/// Configuration Registers (PCRs) of the uTPM. Only trusted SecProcesses can issue this system call.
/// [NOTE] This syscall is implemented as setting measurements into the PCR. It is equivilant to resetting and extending 
/// a PCR in security, though the result value is different.
/// @param arg1 ID of the target SecProcess.
/// @param arg2 uTPM PCR for the SecApp's image. The value must be within [28, 32).
/// @param arg3 uTPM PCR for the SecApp's secure parameters. The value must be within [28, 32).
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM extend errors.
#define SYS_uTPM_RESET_AND_EXTEND_SP_HASH			    102

/// @brief Reset the measurements in the given Platform Configuration Registers (PCRs) of the uTPM. Only trusted 
/// SecProcesses can issue this system call.
/// @param arg1 uTPM PCR 1 to reset. The value must be within [28, 32).
/// @param arg2 uTPM PCR 2 to reset. The value must be within [28, 32).
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM extend errors.
#define SYS_uTPM_RESET_SP_HASH                          103

/// @brief Get a quote of all PCRs in the uTPM.
/// @param arg1 32-bit nonce.
/// @param arg2 Caller SecProcess's memory buffer to store the quote report.
/// @param arg3 
/// @return Return SUCCESS on success.
///         Return ENOMEM on memory allocation failure.
/// 		Return EPERM if SecProcess is not allowed to access the destination memory buffer.
///         Return EINTERNAL on uTPM quote errors.
#define SYS_uTPM_GET_QUOTE			                    104




/********* Syscalls - Pseudo-Random Number Generator *********/
/// @brief Return a pseudo-random number seeded by HW.
/// @param arg1 length of the random number. The number must <= 1024.
/// @param arg2 vaddr of the result buffer.
/// @param arg3 
/// @return Return SUCCESS on success.
///         Return other values on errors.
#define SYS_HW_PRNG_GET_RANDOM_NUMBER					150
    #define HW_PRNG_NUMBER_MAX_LEN                          (1024)




/********* Syscalls - Memory Translation/Access *********/
#define SYS_MM_VADDR_TO_SPADDR							1002
#define SYS_MM_VADDR_TO_PADDR							1003

/// @brief Flush a range of current SecProcess's memory, or flush all memory if it fails.
///
/// @param arg1 virtual address of the current SecProcess's memory.
/// @param arg2 memory size.
/// 
/// @return Return SUCCESS on success.
///			Return EINVAL if the current SecProcess is invalid.
/// 		Return EPERM if the current SecProcess is not allowed to access the memory.
#define SYS_MM_FLUSH_CACHE_RANGE						1004


#define SYS_MM_CHECK_ALLOW_RW						1005



/********* Syscalls - Object Management *********/
/// @brief Create a root object; i.e., object without a parent object.
///
/// @param addr Address of physical memory of the S domain, or address of system physical memory (spaddr_t) of the 
///         machine, or address of IRQ objects. Ignored if creating other types of objects.
/// @param size Size of memory and IRQ objects. Ignored if creating other types of objects.
/// @param flags Creation flags. Note: <ns_access_type> is valid only for OBJ_MEM_PLATFORM root object.
///         Creation of any child objects works as <ns_access_type> is <OBJECT_ACCESS_NS_ACCESS_MEDIATED>.
///
/// @return Return the handle of created object.
///			Return INVALID_HANDLE on any errors.
#define SYS_OBJ_CREATE_ROOT_OBJECT						1101

/// @brief Destroy an object.
///
/// @param arg1 Object handle.
///
/// @return Returns SUCCESS if succeed.
///  		Returns E_OBJ_NOTOWNED if <owner_pool> does not own the object <h_obj>.
/// 		Returns EINVAL if any parameters are empty or on any other internal errors.
#define SYS_OBJ_DESTROY_OBJECT							1102

// [NOTE] The object management system intentionally not to provide a "Get object handle" function.
// The SecProcess should manage object handles by themselves.
//#define SYS_OBJ_GET_OBJECT							1103 

/// @brief Get the information of the given object. Note that the caller SecProcess must have registered the object, or
/// the system call returns errors.
///
/// @param arg1 Object handle.
/// @param arg2 Buffer to hold the result info.
///
/// @return Return SUCCESS on success.
///			Return E_OBJ_NOTEXIST if the SecProcess has not registered the given object.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_GET_INFO								1104

/// @brief Take a non-shared object and create a child object from it. All resources not given to the child still
/// belong to the parent.
/// [NOTE] Only the object owner can call this function.
/// [NOTE] The parent object and its ancestors must not be shared.
/// [NOTE] If the parent object is an OBJ_MEM_PLATFORM object with <ns_access_type> flag set to BLOCKED/PASSTHRU on creation, the
/// flag is NOT inherited to the child objects. Caller SecProcesses must make sure the creation of child objects does not
/// break the NS domain's accesses to the objects. Otherwise, the caller must not create child objects and build
/// their own OBJ_MEM_PLATFORM memory/object management for the parent object.
///
/// @param arg1 Object handle.
/// @param arg2 Start address of the child object's resource.
/// @param arg3 Size of the child object's resource.
///
/// @return Return the handle of the child object on success. 
///			Return INVALID_HANDLE on errors.
#define SYS_OBJ_SPLIT_OBJECT							1115

/// @brief Grant access permission of an owned object to a user.
/// [NOTE] Only the object owner can grant access permission to a user. 
/// [NOTE] A SecProcess cannot share an object with a same SecProcess twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param arg1 Object handle.
/// @param arg2 ID of the target SecProcess.
/// @param arg3 Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EINVAL if the target SecProcess does not have an object pool. In other words, the target SecProcess is malformed.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_SHARE_OBJECT_SP							1118 // [TODO] merge it

/// @brief Grant access permission of an owned object to the non-secure partition.
/// [NOTE] Only the object owner can grant access permission to the non-secure partition. 
/// [NOTE] A SecProcess cannot share an object with the non-secure partition twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param arg1 Object handle.
/// @param arg2 Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_SHARE_OBJECT_NS							1121 // [TODO] merge it

/// @brief Grant access permission of multiple owned objects to the non-secure partition.
/// [NOTE] Only the objects' owner can grant access permission to the non-secure partition.
/// [NOTE] A SecProcess cannot share an object with the non-secure partition twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param arg1 The SYS_PARAMS_OBJECT_SHARE_MULTI parameter of the objects.
/// @param arg2 Number of the objects. The number must be <= SYS_PARAMS_OBJECT_SHARE_MULTI_MAX_NUM.
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_SHARE_OBJECT_NS_MULTI					1125

/// @brief Remove access permission of an owned object from a user.
/// [NOTE] Only the object owner can grant access permission to a user.
/// [NOTE] A SecProcess cannot share an object with a same SecProcess twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param arg1 Object handle.
/// @param arg2 ID of the target SecProcess.
/// @param arg3 Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EINVAL if the target SecProcess does not have an object pool. In other words, the target SecProcess is malformed.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_UNSHARE_OBJECT_SP						1126 // [TODO] merge it

/// @brief Reference a resource object in the SecProcess, so it can be directly addressed later.
///
/// @param arg1 Object handle.
/// @param arg2 Virtual address of the SecProcess to be mapped.
/// @param arg3 Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EPERM if the desired permission is not allowed.
///			Return EINTERNAL on other errors.
#define SYS_OBJ_REFERENCE_OBJECT						1133

/// @brief Unreference a resource object in the SecProcess.
///
/// @param arg1 Object handle.
///
/// @return Return SUCCESS.
#define SYS_OBJ_UNREFERENCE_OBJECT						1134

// [NOTE] The object management system intentionally not to provide a "transfer ownership" (SYS_OBJ_TRANSFER_OWNERSHIP) 
// function. Otherwise, child memory objects may have different owners than their parent objects.




/********* Syscalls - SecProcess Communication *********/
/// @brief Register the signal handler of a SecProcess
///
/// @param arg1 Virtual address of signal handler
///
/// @return Always return SUCCESS.
#define SYS_SIGNAL_INSTALL_HANDLER						1400

/// @brief Finish signal handling
///
/// @param arg1 current_stack_addr of the current SecProcess
///
/// @return Always return SUCCESS.
#define SYS_SIGNAL_RETURN								1401

/// @brief Register the SecBase signal handler of a SecProcess
///
/// @param arg1 Virtual address of signal handler
///
/// @return Always return SUCCESS.
#define SYS_MHV_SIGNAL_INSTALL_HANDLER					1410

/// @brief Finish SecBase signal handling
///
/// @param arg1 current_stack_addr of the current SecProcess
///
/// @return Always return SUCCESS.
#define SYS_MHV_SIGNAL_RETURN							1411

/// @brief Switch to the target SecProcess's event handling.
/// [NOTE] This system call returns nothing, because SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status. 
///
/// @param arg1 ID of the target SecProcess.
///
/// @return Nothing.
#define SYS_SWITCH_TO_SP_EVENT_HANDLING					1412

/// @brief Notify the AppLoader of the current SecProcess is running.
///
/// @param arg1 Additional info to be sent to the AppLoader.
///
/// @return Nothing.
#define SYS_NOTIFY_APPLOADER_SP_RUNNING					1420

/// @brief Notify the given process in the non-secure domain with the given data.
///
/// @param arg1 is the address of the sending data.
/// @param arg2 is the size of the sending data.
/// @param arg3 is the ID of the target process in the non-secure domain.
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL otherwise.
// #define SYS_COMM_APP_NOTIFY								1420 //! Use the services provided by untrusted user application (AppLoader)

/// @brief Create an Inter-SecProcess shared memory channel. The caller SecProcess allocates memory and allow the 
/// target SecProcess to access the memory.
///
/// @param arg1 Virtual address of input parameters <COMM_ISP_CONF>.
///
/// @return Return the actual handle on success.
///			Return INVALID_HANDLE on any errors.
#define SYS_COMM_ISP_CREATE								1430

/// @brief Destroy an Inter-SecProcess shared memory channel. 
///
/// @param arg1 Handle of the communication channel.
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL otherwise.
#define SYS_COMM_ISP_DESTROY							1431

/// @brief Send the SecProcess with an event. This function does not wait for the response.
/// [NOTE] This system call returns nothing, SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status.
///
/// @param arg1 Target SecProcess's ID.
/// @param arg2 Data buffer send to the target SecProcess.
/// @param arg3 Size of the data buffer.
///
/// @return Return SUCCESS on success.
/// 		Return EINVAL if cannot find the target SecProcess.
///			Return EINTERNAL on other errors.
#define SYS_COMM_ISP_SEND								1432

/// @brief Call and switch to the target SecProcess.
/// [NOTE] This system call returns nothing, SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status. 
///
/// @param arg1 Target SecProcess's ID.
/// @param arg2 Data buffer send to the target SecProcess.
/// @param arg3 Size of the data buffer.
///
/// @return The function does not return on success.
/// 		Return EINVAL if cannot find the target SecProcess.
///			Return EINTERNAL on other errors.
#define SYS_COMM_ISP_CALL								1433




/********* Syscalls - I/O Accesses *********/
/// @brief Enable all interrupts. 
/// Only SecServices are allowed to invoke this system call
///
/// @return Return SUCCESS if the operation succeed.
///			Return EPERM if the SecProcess has no permission.
///			Return EINVAL if the SecProcesses is invalid
#define SYS_IO_ENABLE_INTR								2006

/// @brief Disable all interrupts. 
/// Only SecServices are allowed to invoke this system call
///
/// @return Return SUCCESS if the operation succeed.
///			Return EPERM if the SecProcess has no permission.
///			Return EINVAL if the SecProcesses is invalid
#define SYS_IO_DISABLE_INTR								2007

/// @brief Read the given I/O port. 
///
/// @param arg1 Target I/O port.
/// @param arg2 Read size (PIO_SIZE_BYTE, PIO_SIZE_WORD or PIO_SIZE_DWORD).
/// @param arg3 Destination buffer in SecProcess to store the readed value.
///
/// @return Return SUCCESS if the SecProcess has permission.
///			Return EPERM if the SecProcess has no permission.
///			Return EINVAL if the SecProcesses is not used to access PCIE config space
///         Return E_NOT_IMPLEMENTED if the port is not reserved for trusted SecProcesses.
#define SYS_PORTIO_READ									2008
#define		PIO_SIZE_BYTE									0x1
#define 	PIO_SIZE_WORD									0x2
#define 	PIO_SIZE_DWORD									0x4

/// @brief Write the given I/O port. 
///
/// @param arg1 Target I/O port.
/// @param arg2 Value to write.
/// @param arg3 Read size (PIO_SIZE_BYTE, PIO_SIZE_WORD or PIO_SIZE_DWORD).
///
/// @return Return SUCCESS if the SecProcess has permission.
///			Return EPERM if the SecProcess has no permission.
///			Return EINVAL if the SecProcesses is not used to access PCIE config space
///         Return E_NOT_IMPLEMENTED if the port is not reserved for trusted SecProcesses.
#define SYS_PORTIO_WRITE								2009




/********* Syscalls - IOMMU *********/
/// @brief Create an empty IOMMU page table for devices exclusively owned by the secure domain.
/// This page table translates paddrs in the secure domain to spaddrs to ease the implementation of drivers in secure 
/// domain
/// @return Return the handle of created empty IOMMU page table on success, or IOMMU_PT_INVALID on errors
#define SYS_IOMMU_EXCLUSIVE_DEV_CREATE_PT				2201

/// 
#define SYS_IOMMU_SHARED_DEV_CREATE_PT					2202

/// @brief Destroy an IOMMU page table
/// @return Return SUCCESS on success. 
///         Return EPERM if the caller SecProcess is not allowed to access the IOMMU.
///         Return EINTERNAL on any other errors.
#define SYS_IOMMU_DESTROY_PT							2203

#define SYS_IOMMU_BIND_PT								2204
#define SYS_IOMMU_UNBIND_PT								2205

/// @brief Modify the given IOMMU PT for a device owned by secure domain only and map the given paddr memory region in
/// the PT
/// @return Return SUCCSS on success.
///         Return EPERM if the caller SecProcess is not allowed to access the IOMMU.
///         Return ENOMEM if SecOS cannot allocate memory to copy the parameters.
///         Return EINVAL on any other errors.
#define SYS_IOMMU_EXCLUSIVE_DEV_ALLOW_PADDRS			2206

/// @brief Modify the given IOMMU PT for a device owned by secure domain only and unmap the given paddr memory region in
/// the PT
/// @return Return SUCCSS on success.
///         Return EPERM if the caller SecProcess is not allowed to access the IOMMU.
///         Return ENOMEM if SecOS cannot allocate memory to copy the parameters.
///         Return EINVAL on any other errors.
#define SYS_IOMMU_EXCLUSIVE_DEV_DENY_PADDRS				2207



#define SYS_IOMMU_SHARED_DEV_ALLOW_PADDRS				2208
#define SYS_IOMMU_SHARED_DEV_DENY_PADDRS				2209




/********* Syscalls - Hypercalls for I/O devices *********/
/// @brief Allow SecProcesses to issue hypercalls to SecBase to read device info and configure device emulation in SecBase.
#define SYS_HYPERCALL_IO_DEVICE							2500
	// Hypercalls must be the same with the ones defined in <secbase_hypercalls.h>
	#define HC_FN_NUM_START									100000
    #define HC_GET_PLATFORM_INFO_SENSITIVE                  (HC_FN_NUM_START + 20)
	#define HC_DEV_PCIE_REGISTER							(HC_FN_NUM_START + 24000)
	#define HC_DEV_PCIE_UNREGISTER							(HC_FN_NUM_START + 24001)
	#define HC_DEV_PCIE_ASSIGN_TO_S							(HC_FN_NUM_START + 24002)
	#define HC_DEV_PCIE_ASSIGN_TO_NS						(HC_FN_NUM_START + 24003)
	#define HC_DEV_XHCI_REGISTER_BASE_SPADDR				(HC_FN_NUM_START + 25000)

#define SYS_PLATFORM_GET_NONSENSITIVE_INFO				2501




/********* Syscalls - IOMMU for HW quirks *********/
#ifdef IOMMU_ALLOW_DISABLE_FOR_HW_QUIRKS
	#define SYS_IOMMU_ENABLE_FOR_IGFX						4500
	#define SYS_IOMMU_DISABLE_FOR_IGFX						4501
#endif // IOMMU_ALLOW_DISABLE_FOR_HW_QUIRKS




/********* Syscalls - System quirks *********/
#ifdef GREENBOX_USE_UXMHF64_V1
//// [TODO] These quirks should be fixed in the future
#define SYS_QUIRKS_GET_INFO								4800
#endif // GREENBOX_USE_UXMHF64_V1



/********* Syscalls - Debug *********/
#ifdef DEBUG
	#define SYS_DEBUG_IOMMU_PRINT_AND_CLEAR_FAULT_REGS		5000
	#define SYS_DEBUG_IOMMU_RESTART_DMA_IOMMU				5001
#endif // DEBUG





#ifndef __ASSEMBLY__


/********* Syscalls Parameters - General syscalls *********/
typedef struct {
	spid_t ids[SOS_MAX_SPs];     /* The IDs of SecProcess */
	uint32_t num_entries;
} SP_IDS;

typedef struct
{
    /// @brief SecProcess name.
	char 						sp_name[SP_NAME_MAX_SIZE];

    // Flags
	/// @brief True iff the SecProcess is signed by the 1st party (i.e., signed by us (e.g., CMU).).
	bool						is_signed_by_1stparty;
    /// @brief Is this SecProcess a trusted process?
	bool						is_trusted;
    /// @brief Is this SecProcess the "Init" process? SecOS regards the init SecProcess as a trusted process.
	bool						is_init_sp;

    // Crypto-related
    /// @brief Hash of SecProcess's CODE, RODATA, and DATA.
	uint8_t						img_hash[SECPROCESS_HASH_LEN_BYTES];
    /// @brief Hash of secure parameter memory.
	uint8_t						sparams_hash[SECPROCESS_HASH_LEN_BYTES];
} SOS_SP_GET_INFO_RESP;




/********* Syscalls - NS domain *********/
typedef enum 
{
    OS_INVALID = 0,
    OS_LINUX = 1,
    OS_WINDOWS = 2,
    OS_MACOS = 3,
} NS_OS_TYPE;

typedef enum 
{
    INVALID_ISA = 0,
    I386 = 1,
    AMD64 = 2,
} NS_OS_ISA;

typedef struct
{
	uint32_t            ns_os_type:3,
                        /// @brief the micro-architecture of the OS kernel in the NS domain.
                        ns_os_isa:2,
                        __reserved:29;
} SOS_NS_OS_INFO_RESP;




/********* Syscalls Parameters - Time *********/
typedef union 
{
	struct{
		uint32_t tv_sec;                 /* seconds */
		uint32_t tv_nsec;                /* nanoseconds */
	};
	uint64_t val;
} sos_timespec_t;

typedef struct {
	int tz_minuteswest;     /* minutes west of Greenwich */
	int tz_dsttime;         /* type of DST correction */
} TIMEZONE;

typedef struct {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
} TIMEVAL;




/********* uTPM Data Structures (Must be same with the ones defined in <secbase_hypercalls.h>) *********/
#ifndef GreenBox_uTPM_Definitions
#define GreenBox_uTPM_Definitions 1 // Avoid double definition of uTPM data structures in both SecBase and SecOS

/// @brief Length of each uTPM PCR.
#define uTPM_MEASUREMENT_LENGTH                 (32)

/// @brief SecOS may reset PCR 4 and above.
#define uTPM_SECOS_RESETTABLE_PCR_FROM_INDEX	(4)

/// @brief Number of PCRs in uTPM.
/// [NOTE] This value must be the same as <TPM_PCR_NUM>
#define uTPM_PCR_NUM                            (32)

/// @brief The value must be the same with the <outlen> returned by <utpm_quote>. The value is TPM_RSA_KEY_LEN.
#define TPM_QUOTE_SIGNATURE_LEN             (256)
#define TPM_QUOTE_PUBLIC_KEY_LEN_MAX        (384)

struct uTPM_PCR_SELECTION {
    uint16_t sizeOfSelect;            /* The size in bytes of the pcrSelect structure */
    uint8_t pcrSelect[uTPM_PCR_NUM/8]; /* This SHALL be a bit map that indicates if a PCR
                                            is active or not */
} __attribute__((packed));

struct utpm_pcr
{
    uint8_t value[uTPM_MEASUREMENT_LENGTH];
} __attribute__((packed));

/// @brief Corresponding to TPM_PCR_COMPOSITE in "8.2 TPM_PCR_COMPOSITE" of TPM 1.2 TPM Structures.
struct utpm_pcr_comp
{
    struct uTPM_PCR_SELECTION select;

    /// @brief This SHALL be the size of the pcrValue field (not the number of PCR’s). <valueSize> is big endian.
    uint32_t valueSize;
    struct utpm_pcr pcrValue[uTPM_PCR_NUM];
} __attribute__((packed));

#endif // GreenBox_uTPM_Definitions




/********* Syscalls Parameters - uTPM *********/
typedef struct
{
    uint32_t is_enabled:1,
            __reserved: 31;
} SOS_uTPM_INFO;

#define uTPM_MASTER_SEALING_SECRET_SIZE   (32)

typedef struct
{
    uint8_t secret[uTPM_MASTER_SEALING_SECRET_SIZE];
} SOS_uTPM_SET_SECRET_REQ;

#define uTPM_QUOTE_NONCE_LENGTH  (32)

#define UTPM_QUOTE_NONCE_STRING_MAX_LENGTH   (20)
#define UTPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH   (12)

typedef struct
{
    char nonce_str[UTPM_QUOTE_NONCE_STRING_MAX_LENGTH];
    uint8_t nonce_freshness[UTPM_QUOTE_NONCE_FRESHNESS_MAX_LENGTH];
} SOS_uTPM_GET_QUOTE_REQ;

/// @brief 
/// [NOTE] Prevent paddings by the compiler. Both SecProcesses and the mobile verifier app uses this struct.
typedef struct
{
    uint8_t                 nonce[uTPM_QUOTE_NONCE_LENGTH];
    uint32_t                quote_version;
    uint32_t                quote_fixed;
    struct utpm_pcr_comp    pcr_comp;

    uint8_t                 sig[TPM_QUOTE_SIGNATURE_LEN];

    uint8_t                 public_key[TPM_QUOTE_PUBLIC_KEY_LEN_MAX];
    uint32_t                public_key_size;
} __attribute__((packed)) SOS_uTPM_GET_QUOTE_RESP;




/********* Syscalls Parameters - Objects Management *********/
//! @brief Types of resource objects. SecOS/SecProcesses access system resources
//! via objects. And each of these objects must belong to exactly one type
typedef enum _object_type
{
	OBJ_MEM_SDOM = 1,
	OBJ_MEM_PLATFORM = 2,
	OBJ_PIO = 3,

	/// @brief IRQ objects
	/// Note: IRQ maps to interrupt vectors. The mappings are different across machines. For example, IRQ1 may map to 
	/// interrupt vector 49 in some systems.
	OBJ_IRQ = 4,

	/// @brief Interrupt vector objects.
	OBJ_INTR = 5,

	//! @brief IOMMU resources.
	//! Only the trusted SecProcess who owns the OBJ_IOMMU object can call SecOS to configure IOMMU.
	OBJ_IOMMU = 6, 

	OBJ_TYPE_INVALID = 0xFFFFFFF,
} OBJECT_TYPE;

//! @brief Types of accesses permission to resource objects. Different resource
//! objects use different combinations of permissions 
typedef enum
{
	OA_NONE_BLOCK = 0,
	OA_NONE_MEDIATION = (1 << 0),
	OA_R = (1 << 1),
	OA_W = (1 << 2),
	OA_X = (1 << 3),
	OA_NO_CACHE = (1 << 4),
	OA_CACHE_WB = (1 << 5),
	OA_CACHE_WT = (1 << 6),

	// Shortcut of permission combinations
	OA_RX = (OA_R | OA_X),
	OA_RW = (OA_R | OA_W),
	OA_RW_CACHE = (OA_RW | OA_CACHE_WB),
	OA_RW_NO_CACHE = (OA_RW | OA_NO_CACHE),
	OA_RWX = (OA_R | OA_W | OA_X),
	OA_RWX_CACHE = (OA_RWX | OA_CACHE_WB),
	OA_ALL = (OA_RWX | OA_NO_CACHE | OA_CACHE_WB | OA_CACHE_WT),
} OBJ_ACCESS_MODE;

#define OBJECT_ACCESS_NS_ACCESS_MEDIATED (0) // Must be the same with <MEMOBJ_ACCESS_NS_ACCESS_MEDIATED> in object.h 
#define OBJECT_ACCESS_NS_ACCESS_BLOCKED  (1) // Must be the same with <MEMOBJ_ACCESS_NS_ACCESS_BLOCKED> in object.h
#define OBJECT_ACCESS_NS_ACCESS_PASSTHRU (2) // Must be the same with <MEMOBJ_ACCESS_NS_ACCESS_PASSTHRU> in object.h
#define OBJECT_ACCESS_NS_ACCESS_MEDIATED_W (3) // Must be the same with <MEMOBJ_ACCESS_NS_ACCESS_MEDIATED_W> in object.h

/// @brief Flags used when creating memory objects.
typedef union
{
	struct
	{
		uint32_t obj_type:4,
                /// @brief If true, then the NS domain may access the object and the access is not intercepted by the S domain.
				ns_access_type:3, 
				/// @brief [For memory objects only] If true, mHV remaps this page in its page table with cache enabled. 
				/// If false, mHV remaps this page with cache disabled.
				mhv_pt_cache_enable:1, 
				__reserved:24;
	};
	uint32_t raw;
} SYS_PARAMS_OBJECT_CREATE_ROOT_OBJECT_FLAGS;

typedef struct _sys_params_object_share_multi
{
	shandle_t 			h_obj;

	OBJ_ACCESS_MODE 	perm_max; //! Maximum access permission allowed for sharing the object
} SYS_PARAMS_OBJECT_SHARE_MULTI;

/// @brief Max SYS_PARAMS_OBJECT_SHARE_MULTI elements allowed to pass in one syscall.
#define SYS_PARAMS_OBJECT_SHARE_MULTI_MAX_NUM		(64)

typedef struct
{
	paddr_t 			mapped_paddr;
} SYS_OBJ_MEM_INFO;

typedef struct
{
	intr_t				intr;
} SYS_OBJ_IRQ_INFO;

typedef struct 
{
	spid_t 				owner_spid;
	OBJECT_TYPE			obj_type;
	size_t				resource_size;

	/// @brief The max allowed permissions of the caller SecProcess to the object.
	OBJ_ACCESS_MODE 	caller_sp_perm_max;

	union{
		SYS_OBJ_MEM_INFO	mem_info;
		SYS_OBJ_IRQ_INFO	irq_info;
	};
} SYS_OBJ_INFO;




/********* Syscalls Parameters - SecProcess Communication *********/
typedef struct _comm_isp_conf
{
	/// The target SecProcess that is allowed to access this shared communication buffer.
	spid_t 					target_spid;

	/// The parent memory object which will spawn the communication buffer object.
	shandle_t				h_mem;

	/// The SecProcess memory used as this shared communication buffer.
	vaddr_t 				comm_buf;

	/// Number of pages of this shared communication buffer.
	size_t 					num_pages;
} COMM_ISP_CONF;

typedef struct
{
	/// The target SecProcess that is allowed to access this shared communication buffer.
	spid_t 					target_spid;

	///
	uint32_t				req_response:1,
							data_size:16,
							__reserved:15;

	uint32_t				req_id;
	uint32_t				ack_req_id;
	uint32_t				__reserved3:6,
							sub_cmd:26;

	/// 
	vaddr_t					data;
} COMM_ISP_SEND_PARAMS;




/********* Syscall Parameters - Port IO *********/
typedef uint16_t pio_t;




/********* Syscall Parameters - IOMMU *********/
#define IOMMU_PT_CONF_MEMOBJ_MAXNUMS			(16)

typedef uint32_t iommu_pt_t;

typedef struct _iommu_pt_conf
{
	shandle_t 				obj_handles[IOMMU_PT_CONF_MEMOBJ_MAXNUMS];
	uint32_t 				entry_num;
} IOMMU_PT_CONF;




/********* Syscalls - Hypercalls for I/O devices *********/
typedef struct
{
	spaddr_t	pcie_cfg_space_mmio_base_spaddr;
} SECBASE_PLATFORM_INFO;

#define SECBASE_MACHINE_ID_MAX_SIZE (32) // Must be == SECBASE_MACHINE_ID_MAX_LENGTH in SecBase.
typedef struct
{
    uint8_t machine_id[SECBASE_MACHINE_ID_MAX_SIZE];
    uint8_t machine_id_is_available; // 1 iff the SecBase can retrieve the machine ID. Otherwise 0.

	uint32_t enable_tpm:1, // Set iff SecBase initialized the physical TPM device successfully.
             enable_utpm:1, // Set iff SecBase initialized the uTPM successfully.
            __reserved:30;
} SECBASE_PLATFORM_NONSENSITIVE_INFO;



/********* Syscalls - System quirks *********/
typedef struct
{
	bool					quirks_support_ns_dom_intr_interception;
} SECOS_QUIRKS_INFO;




/********* Syscall Parameters - PCIe Config Space *********/
/*union pci_bdf
{
	struct {
		unsigned short	fn:3,
			dev:5,
			bus:8;
	} __attribute__ ((packed));
	unsigned short 	raw;
}; */


#endif //__ASSEMBLY__
#endif //SECOS_SYSCALLS_H