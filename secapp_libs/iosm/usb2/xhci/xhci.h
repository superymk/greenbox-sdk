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

/*-
 * Copyright (c) 2014 Martin Pieuchot. All rights reserved.
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LIBUSB3_XHCI_H
#define LIBUSB3_XHCI_H

#include <palddk.h>
#include <usb_ops.h>

#include "lib-usb3.h"

#define XHCI_TRB_TRANSFER_LENGTH_MAX    (0x1FFFF)

/* Transfer Ring Types */
#define XHCI_TRB_TYPE_NORMAL	(1 << 10)
#define XHCI_TRB_TYPE_SETUP	(2 << 10)	/* Setup stage	(ctrl only) */
#define XHCI_TRB_TYPE_DATA	(3 << 10)	/* Data stage	(ctrl only) */
#define XHCI_TRB_TYPE_STATUS	(4 << 10)	/* Status stage	(ctrl only) */
#define XHCI_TRB_TYPE_ISOCH	(5 << 10)
#define XHCI_TRB_TYPE_LINK	(6 << 10)	/* Link next seg. (all+cmd) */
#define XHCI_TRB_TYPE_EVENT	(7 << 10)	/* Generate event (all) */
#define XHCI_TRB_TYPE_NOOP	(8 << 10)	/* No-Op (all) */

// TRB Structure
#define XHCI_TRB_GET_CODE(x)	(((x) >> 24) & 0xff)
#define XHCI_TRB_TDREM(x)	(((x) & 0x1f) << 17)	/* TD remaining len. */
#define XHCI_TRB_REMAIN(x)	((x) & 0xffffff)	/* Remaining length */
#define XHCI_TRB_LEN(x)		((x) & 0x1ffff)		/* Transfer length */
#define XHCI_TRB_INTR(x)	(((x) & 0x3ff) << 22)	/* MSI-X intr. target */

#define XHCI_TRB_ISP		(1 << 2)	/* Interrupt on short packet */
#define XHCI_TRB_CHAIN		(1 << 4)	/* Chained with next TRB */
#define XHCI_TRB_IOC		(1 << 5)	/* Interrupt On Completion */
#define XHCI_TRB_IDT		(1 << 6)	/* Immediate DaTa */

#define XHCI_TRB_DIR_IN		    (1 << 16)

#define XHCI_TRB_TRT_NO_DATA	(0 << 16)
#define XHCI_TRB_TRT_OUT	    (2 << 16)
#define XHCI_TRB_TRT_IN		    (3 << 16)

/* TRB Completion codes */
#define XHCI_CODE_INVALID	 0	/* Producer didn't update the code. */
#define XHCI_CODE_SUCCESS	 1	/* Badaboum, plaf, plouf, yeepee! */
#define XHCI_CODE_DATA_BUF	 2	/* Overrun or underrun */
#define XHCI_CODE_BABBLE	 3	/* Device is "babbling" */
#define XHCI_CODE_TXERR		 4	/* USB Transaction error */
#define XHCI_CODE_TRB		 5	/* Invalid TRB  */
#define XHCI_CODE_STALL		 6	/* Stall condition */
#define XHCI_CODE_RESOURCE	 7	/* No resource available for the cmd */
#define XHCI_CODE_BANDWIDTH	 8	/* Not enough bandwidth  for the cmd */
#define XHCI_CODE_NO_SLOTS	 9	/* MaxSlots limit reached */
#define XHCI_CODE_STREAM_TYPE	10	/* Stream Context Type value detected */
#define XHCI_CODE_SLOT_NOT_ON	11	/* Related device slot is disabled */
#define XHCI_CODE_ENDP_NOT_ON	12	/* Related endpoint is disabled */
#define XHCI_CODE_SHORT_XFER	13	/* Short packet */
#define XHCI_CODE_RING_UNDERRUN	14	/* Empty ring when transmitting isoc */
#define XHCI_CODE_RING_OVERRUN	15	/* Empty ring when receiving isoc */
#define XHCI_CODE_VF_RING_FULL	16	/* VF's event ring is full */
#define XHCI_CODE_PARAMETER	17	/* Context parameter is invalid */
#define XHCI_CODE_BW_OVERRUN	18 	/* TD exceeds the bandwidth */
#define XHCI_CODE_CONTEXT_STATE	19	/* Transition from illegal ctx state */
#define XHCI_CODE_NO_PING_RESP	20	/* Unable to complete periodic xfer */
#define XHCI_CODE_EV_RING_FULL	21	/* Unable to post an evt to the ring */
#define XHCI_CODE_INCOMPAT_DEV	22	/* Device cannot be accessed */
#define XHCI_CODE_MISSED_SRV	23	/* Unable to service isoc EP in ESIT */
#define XHCI_CODE_CMD_RING_STOP	24 	/* Command Stop (CS) requested */
#define XHCI_CODE_CMD_ABORTED	25 	/* Command Abort (CA) operation */
#define XHCI_CODE_XFER_STOPPED	26 	/* xfer terminated by a stop endpoint */
#define XHCI_CODE_XFER_STOPINV	27 	/* TRB transfer length invalid */
#define XHCI_CODE_XFER_SHORTPKT	28 	/* Stopped before reaching end of TD */
#define XHCI_CODE_MELAT		29	/* Max Exit Latency too large */
#define XHCI_CODE_RESERVED	30
#define XHCI_CODE_ISOC_OVERRUN	31	/* IN data buffer < Max ESIT Payload */
#define XHCI_CODE_EVENT_LOST	32 	/* Internal overrun - impl. specific */
#define XHCI_CODE_UNDEFINED	33 	/* Fatal error - impl. specific */
#define XHCI_CODE_INVALID_SID	34 	/* Invalid stream ID received */
#define XHCI_CODE_SEC_BW	35 	/* Cannot alloc secondary BW Domain */
#define XHCI_CODE_SPLITERR	36 	/* USB2 split transaction */


#ifndef __ASSEMBLY__

// struct sp_usbpdev_s;
// struct usb3_pipe;

#define XHCI_TRB_MAXSIZE	(64 * 1024)

#define usb_xfer_isread(dir)    (dir & UE_DIR_IN)

#define UE_GET_SIZE(a)	((a) & 0x7ff)   // from OpenBSD <usb.h>

/* Macros for counting and rounding. */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif


extern SAP_STATUS usb3_usbdev_alloc_defpipe(struct sp_usbpdev_s* dev);
extern SAP_STATUS usb3_usbdev_free_defpipe(struct sp_usbpdev_s* dev);

extern struct usb3_pipe* libusb3_usbpdev_alloc_intr_pipe(
	struct sp_usbpdev_s *usbdev,
    struct usb_endpoint_descriptor *epdesc
);

extern SAP_STATUS libusb3_poll_intr(struct usb3_pipe *pipe, void *data, size_t data_sz);

/// @brief Free a given pipe
extern SAP_STATUS libusb3_usbpdev_free_pipe(struct usb3_pipe* pipe);

extern SAP_STATUS xhci_send_control(
	struct usb3_pipe* p, 
	int dir, 
	const void *cmd, 
	size_t cmdsize,
    void* data, 
	size_t datasize, 
	u16 maxpacket,
	uint32_t td_timeout_ms,
    bool enable_debug
);

#endif // __ASSEMBLY__
#endif //_PAL_USB_EHCI