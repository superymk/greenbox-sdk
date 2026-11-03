// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOCTL_HANDLER_H
#define IOCTL_HANDLER_H

#ifndef __ASSEMBLY__
struct tz_cmd_packet;

/// @brief Handle IOCTL request packet
/// @param cmd_packet Command packet
/// @param copy_retval Output flag for copy back
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS ioctl_request_handler(struct tz_cmd_packet *cmd_packet, int* copy_retval);

/// @brief Probe USB2 EHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB2_EHCI_PROBE(struct tz_cmd_packet *cmd_packet);

/// @brief Unprobe USB2 EHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB2_EHCI_UNPROBE(struct tz_cmd_packet *cmd_packet);

/// @brief Pause USB2 EHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB2_PAUSE_EHCI(struct tz_cmd_packet *cmd_packet);

/// @brief Resume USB2 EHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB2_RESUME_EHCI(struct tz_cmd_packet *cmd_packet);

/// @brief Probe USB3 shared XHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PROBE(struct tz_cmd_packet *cmd_packet);

/// @brief Unprobe USB3 shared XHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_UNPROBE(struct tz_cmd_packet *cmd_packet);

/// @brief Pause USB3 shared XHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PAUSE(struct tz_cmd_packet *cmd_packet);

/// @brief Resume USB3 shared XHCI
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_RESUME(struct tz_cmd_packet *cmd_packet);

/// @brief Probe framebuffer GPU
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_FB_PROBE_GPU(struct tz_cmd_packet *cmd_packet);

/// @brief Pause framebuffer GPU
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_FB_PAUSE_GPU(struct tz_cmd_packet *cmd_packet);

/// @brief Resume framebuffer GPU
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_IOSM_FB_RESUME_GPU(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // IOCTL_HANDLER_H