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

#ifndef IOCTL_HANDLER_H
#define IOCTL_HANDLER_H

#ifndef __ASSEMBLY__
struct tz_cmd_packet;

extern OSSTATUS ioctl_request_handler(struct tz_cmd_packet *cmd_packet, int* copy_retval);

extern OSSTATUS do_CMD_IOSM_USB2_EHCI_PROBE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB2_EHCI_UNPROBE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB2_PAUSE_EHCI(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB2_RESUME_EHCI(struct tz_cmd_packet *cmd_packet);

extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PROBE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_UNPROBE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_PAUSE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_USB3_SHARED_XHCI_RESUME(struct tz_cmd_packet *cmd_packet);


extern OSSTATUS do_CMD_IOSM_FB_PROBE_GPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_FB_PAUSE_GPU(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_IOSM_FB_RESUME_GPU(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // IOCTL_HANDLER_H