// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Zongwei Zhou (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include "../lib-iosm-usb-internal.h"

#include "ehci.h"
#include "../pal_util.h"
#include "../util.h"
#include "../usb.h"
// #include "pal_usb.h"
#include "iosm_usb2_calls.h"

// Main code for handling USB controllers and devices. (from Seabios)
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

/****************************************************************
 * Forward Declaration
 ****************************************************************/
static struct usb_new_pipe* _ehci_alloc_async_pipe(
	struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
);

static SAP_STATUS _usbpdev_unlink_pipe(struct usb_new_pipe* p);

/****************************************************************
 * Others
 ****************************************************************/

// [Superymk]
// [TODO] Tempory hacks for the SecApp's eEHCI
#define EEHCI_ID        1
#define BUS_ID  		2
static union eEHCI_ID eehci_id = {.e_id = EEHCI_ID, .bus_id = BUS_ID};
// [TODO - Tempory hacks for the SecApp's eEHCI] End

/*static void pal_ehci_reset_pipe(struct ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    mfence();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}*/

/*static int pal_fillTDbuffer(struct ehci_qtd *td, u16 maxpacket, const void *buf, int bytes)
{
    u32 dest = (u32)buf;
	u32 dest_hw = sap_mm_vaddr_to_paddr((vaddr_t)buf);
    u32* pos = td->buf;
	u32* pos_v = td->buf_v = (u32 *)smalloc(5*sizeof(u32));

    while (bytes) {
        if (pos >= &td->buf[C_ARRAY_SIZE(td->buf)])
            // More data than can transfer in a single qtd - only use
            // full packets to prevent a babble error.
            return ALIGN_DOWN(dest - (u32)buf, maxpacket);
        u32 count = bytes;
        u32 max = 0x1000 - (dest & 0xfff);
        if (count > max)
            count = max;
		// ZONGWEI: set qtd->buf hw/virt addrs
        *pos_v = dest;
        *pos = dest_hw;
        bytes -= count;
        dest += count;
		dest_hw += count;
        pos++;
		pos_v++;
    }
    return dest - (u32)buf;
}*/

// [TODO] This function needs to be re-written
static int pal_new_fillTDbuffer(struct ehci_new_qtd *td, u16 maxpacket, const void *buf, size_t bytes)
{
    uintptr_t dest = (uintptr_t)buf;
	paddr_t dest_hw = sap_mm_vaddr_to_paddr((vaddr_t)buf);
    u32* pos = (u32*) td->buf;

	// Step 1. Fill all the five data buffers with valid paddr
	// [NOTE] This is needed for the case that <buf> does not occupy all the five pages
	td->buf[4].buf_pfn = td->buf[3].buf_pfn = td->buf[2].buf_pfn = 
		td->buf[1].buf_pfn = td->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;

	// Step 2. Fill all the five data buffers with correct paddr
    while (bytes) {
        if (pos >= (u32*)&td->buf[C_ARRAY_SIZE(td->buf)])
            // More data than can transfer in a single qtd - only use
            // full packets to prevent a babble error.
            return ALIGN_DOWN(dest - (uintptr_t)buf, maxpacket);
        size_t count = bytes;
        u32 max = 0x1000 - (dest & 0xfff);
        if (count > max)
            count = max;
		// ZONGWEI: set qtd->buf hw/virt addrs
        *pos = dest_hw;
        bytes -= count;
        dest += count;
		dest_hw += count;
        pos++;
    }

    return dest - (uintptr_t)buf;
}

static int pal_ehci_wait_td_new(struct usb_new_pipe *pipe, uint32_t td_slot, int timeout_us)
{
	SAP_STATUS status = SUCCESS;
    uint32_t td_status = 0;
	int wait_num = timeout_us / CPU_YIELD_USEC; // Num of CPU yields

    // Check: Parameters must be valid
    if(wait_num <= 0)
    {
        // Already timed out
        return -1;
    }

    // Query status
    for (;;) {
		status = iosm_usb2_query_usbtd_status(td_slot, &td_status);
		if(FAIL(status))
		{
			USB_ERR("<pal_ehci_wait_td_new> Failed to read USBTD (slot:%u)'s status!\n", td_slot);
			break;
		}
        if (!(td_status & QTD_STS_ACTIVE))
            break;
		
        if (wait_num <= 0) 
		{
			// Timeout
            status = iosm_usb2_ehci_waittick();
			USB_DEBUG("<pal_ehci_wait_td_new> iosm_usb2_ehci_waittick td_slot:%u, td_status:0x%X, status:%lu\n", 
				td_slot, td_status, status);
            return -1;
        }

        usleep(CPU_YIELD_USEC);
		wait_num--;
    }

    if (td_status & QTD_STS_HALT) {
        USB_ERR("<pal_ehci_wait_td> ehci_wait_td error - status=%x\n", td_status);
        //pal_ehci_reset_pipe(pipe);
        return -2;
    }
	
	USB_DEBUG("ehci_wait_td td_status=%x\n", td_status);
    return 0;
}

#define STACKQTDS 4

static struct ehci_new_qtd* _sel_qtd_content(uint32_t i, 
	struct ehci_new_qtd* qtd0_content, struct ehci_new_qtd* qtd1_content, struct ehci_new_qtd* qtd2_content, struct ehci_new_qtd* qtd3_content
)
{
	switch(i)
	{
		case 0:
			return qtd0_content;
		case 1:
			return qtd1_content;
		case 2:
			return qtd2_content;
		case 3:
			return qtd3_content;
		default:
			return NULL;
	}

	return NULL;
}

static uint32_t _sel_qtd_slot(uint32_t i, 
	uint32_t qtd0_slot, uint32_t qtd1_slot, uint32_t qtd2_slot, uint32_t qtd3_slot
)
{
	switch(i)
	{
		case 0:
			return qtd0_slot;
		case 1:
			return qtd1_slot;
		case 2:
			return qtd2_slot;
		case 3:
			return qtd3_slot;
		default:
			return USBTD_SlotID_INVALID;
	}

	return USBTD_SlotID_INVALID;
}

/// @brief This function reuses USB bulk pipes to perform bulk transfers
/// [TODO] FIXME: This function cannot issue bulk transfer so far.
struct usb_new_pipe* pal_ehci_alloc_bulk_pipe_single_qtd(
	struct sp_usbpdev_s *usbdev,
    struct usb_endpoint_descriptor *epdesc,
	int dir, size_t datasize,
	bool active_now, uint32_t timeout_ms
)
{
	SAP_STATUS status = SUCCESS;
	// bool wait_td_success = true;
	void *bulk_data = NULL;
	struct usb_new_pipe* p = NULL;
	int maxpacket = 0;

	// Sanity checks
	if(!usbdev || !epdesc)
		return NULL;

	// if(p->pehci_usbpipe)
	// {
	// 	USB_DEBUG("<usb_ehci_send_bulk> The pipe (%p) must not have an active internal USB pipe!\n", p);
	// 	return EINVAL;
	// }

    if (datasize > 4 * PAGE_SIZE_4K)
	{
        // FIXME: should support larger sizes.
        warn_noalloc();
        return NULL;
    }

	// p = (struct usb_new_pipe*)smalloc(sizeof(struct usb_new_pipe));
	p = libusb2_usbpdev_alloc_bulk_pipe(usbdev, epdesc);
	if(!p)
		return NULL;
	maxpacket = epdesc->wMaxPacketSize;

    // [TODO] Remove the ugly hack (ulong_t) below
	// USB_DEBUG("<usb_ehci_send_bulk> usbdev_pipe:0x%p, dir=0x%x, data of size %lu at addr 0x%lx!\n",
    //       p, dir, (ulong_t)datasize, (vaddr_t)data);

	// Step 1. Allocate 1 QH and 4 QTDs
	uint32_t qtd0_slot = USBTD_SlotID_INVALID;
	uint32_t qh_slot = USBTD_SlotID_INVALID;

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QH32, &qh_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qh_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd0_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}

	// Step 2. Allocate data structures
	struct ehci_new_qtd* qtd0_content = NULL;

	if(datasize < PAGE_SIZE_4K)
	{
		datasize = ROUND_UP_TO_NEXT_POWER_OF_2_uint32(datasize);
		bulk_data = usbbuf_alloc_align(USBBUF_MEM_DMA, datasize, datasize);
	}
	else
	{
		bulk_data = usbbuf_alloc_align(USBBUF_MEM_DMA, PAGE_SIZE_4K, datasize);
	}

	if(!bulk_data)
	{
		warn_noalloc();
		status = ENOMEM;
		goto alloc_ds_fail;
	}

	qtd0_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd0_content));
	if (!qtd0_content)
	{
        warn_noalloc();
		status = ENOMEM;
        goto alloc_ds_fail;
    }

	p->usbpdev_slot = usbdev->usbpdev_slot;
	p->eptype = USB_ENDPOINT_XFER_BULK;

	p->qh_slot = qh_slot;
	p->qtd_slots[0] = qtd0_slot;
	p->qtd_slots[1] = USBTD_SlotID_INVALID;
	p->qtd_slots[2] = USBTD_SlotID_INVALID;
	p->qtd_slots[3] = USBTD_SlotID_INVALID;
	p->p_bulk_info.data_buf = bulk_data;


	// Step 3. Generate QH and QTD contents
	// qH
	p->header_content->qtd_next.next_td_slot = qtd0_slot;
	p->header_content->qtd_next.other_bits = 0;
	//// Explicitly configure <p->qtd_cir_cfg>: def_pipe's qTDs do not ref each other in circular
	p->qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
	p->qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;

	// qTDs
	{
		// u16 maxpacket = p->maxpacket;
		int tdpos = 0;
		int next_tdpos = (tdpos + 1) % STACKQTDS;

		// Init all qTDs' buf pointer to dummy address
		// [NOTE] This is needed for the case that <buf> does not occupy all the five pages of each qTDs
		{
			paddr_t dest_hw = sap_mm_vaddr_to_paddr((vaddr_t)bulk_data);

			qtd0_content->buf[4].buf_pfn = qtd0_content->buf[3].buf_pfn = qtd0_content->buf[2].buf_pfn =
			qtd0_content->buf[1].buf_pfn = qtd0_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;
		}

		// Fill qTDs' buf pointers with the target buffer
		do
		{
			int transfer = 0;
			struct ehci_new_qtd* td = qtd0_content;
			if(!td)
			{
				USB_ERR("<usb_ehci_send_bulk_single_qtd> Failed to select QTD content! tdpos:%u\n", tdpos);
				status = E_USB_LIB_INTERNAL;
				goto fill_tds_error;
			}

			transfer = pal_new_fillTDbuffer(td, maxpacket, bulk_data, datasize);
			if(((size_t)transfer) == datasize)
				td->qtd_next.v = EHCI_PTR_TERM;
			else
			{
				// td->qtd_next.next_td_slot = USBTD_SlotID_INVALID;
				// td->qtd_next.other_bits = 0;

				// Just one qTD, so there is no pointer to any next TDs
				td->qtd_next.v = EHCI_PTR_TERM;
			}

			td->alt_next.v = EHCI_PTR_TERM;


			td->token = (ehci_explen(transfer) |
						((dir & USB_DIR_IN) ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));

			if(active_now)
				td->token |= QTD_STS_ACTIVE;

			//mfence();
			USB_DEBUG("<usb_ehci_send_bulk_single_qtd> next_tdpos:%u, transfer:%u, td->token:0x%08X\n", next_tdpos, transfer, td->token);
			bulk_data += transfer;
			datasize -= transfer;

			// Fill next TD packet
			tdpos = next_tdpos;
			next_tdpos = (tdpos + 1) % STACKQTDS;
		} while (datasize);
	}

	// USB_INFO("Dump QH:\n");
	// pal_usb_dump_usbtd((vaddr_t)p->header_content);
	// USB_INFO("Dump QTD0:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd0_content);


	// Step 4. Submit the QH and QTD
	//// Submit QTD0 first, because it does not reference any other USB TDs
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd0_content, sizeof(*qtd0_content), qtd0_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> Submit qTD0 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk_single_qtd> Submit qTD0 succeed!\n");

	//// Submit qH
	status = iosm_usb2_usbtd_usbmit_and_verify(p->header_content, sizeof(struct ehci_new_qh), qh_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> Submit qH error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk_single_qtd> Submit qH succeed!\n");


	// Step 5. Update eEHCI's USB TD registers
	usbtd_reg_slot_t usbtd_reg_qh = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd0 = usbtd_reg_pick();
	{
		if((usbtd_reg_qh == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd0 == USBTD_REG_SLOT_INVALID))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Reserve USBTD reg in eEHCI error!\n");
			usbtd_reg_return(usbtd_reg_qh);
			usbtd_reg_return(usbtd_reg_qtd0);
			status = EIO;
			goto reserve_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qh, qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qh);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd0, qtd0_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd0);
			status = EIO;
			goto write_eehci_reg_error;
		}

		p->usbtd_reg_qh = usbtd_reg_qh;
		p->usbtd_reg_qtds[0] = usbtd_reg_qtd0;
		p->usbtd_reg_qtds[1] = USBTD_REG_SLOT_INVALID;
		p->usbtd_reg_qtds[2] = USBTD_REG_SLOT_INVALID;
		p->usbtd_reg_qtds[3] = USBTD_REG_SLOT_INVALID;
	}


	// Step 6. Instruct the eEHCI to process these USB TDs
	//status = vkcall_attach_QH_new(p, 0);
	status = iosm_usb2_attach_qh_intr(p->eptype, p->qh_slot, 0, false, 0, 0, &p->pehci_usbpipe);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> vkcall_attach_QH_new error!\n");
		status = EIO;
		goto attach_qh_fail;
	}


	// // Step 7. Wait for qTD completion
	// int ret = pal_ehci_wait_td_new(p, qtd0_slot, timeout_ms * 1000);
	// if(!ret)
	// {
	// 	USB_DEBUG("bulk td 0 is processed!\n");
	// }
	// else
	// {
	// 	USB_ERR("<usb_ehci_send_bulk_single_qtd> Wait TD error! ret:%d\n", ret);
	// 	wait_td_success = false;
	// 	// status = EIO;
	// 	// goto wait_td_err;
	// }

	// // Step 7. Unlink the qH and qTDs from the eEHCI
	// //status = _usbpdev_unlink_pipe(p, qh_slot, qtd0_slot, qtd1_slot, qtd2_slot, qtd3_slot, usbtd_reg_qh, usbtd_reg_qtd0, usbtd_reg_qtd1, usbtd_reg_qtd2, usbtd_reg_qtd3);
	// status = _usbpdev_unlink_pipe(p);
	// if(FAIL(status))
	// {
	// 	USB_ERR("<usb_ehci_send_bulk_single_qtd> _usbpdev_unlink_pipe error!\n");
	// 	status = EIO;
	// 	goto unlink_defpipe_fail;
	// }

	// sfree(qtd0_content, sizeof(struct ehci_new_qtd));

	// // On success
	// if(!wait_td_success)
	// 	return EIO;
	// else
	// 	return SUCCESS;

	USB_DEBUG("ehci_alloc_bulk_pipe succeed!\n");
    return p;




	// PAL-VK interface: clean up qtd list
	/*vkcall_attach_qtd_list(pipe, NULL, (paddr_t)NULL);

bulk_tds_error:
	for (i=0;i<3;i++) {
        struct ehci_qtd *td = &tds[i];
		if (td->buf_v) {
			pal_mm_dealloc(PAL_MEM_ORDINARY, td->buf_v);
		}
	}
    pal_mm_dealloc(PAL_MEM_DMA, tds);
    return ret;*/

// unlink_defpipe_fail:
// wait_td_err:
attach_qh_fail:
write_eehci_reg_error:
reserve_eehci_reg_error:
submit_tds_fail:
fill_tds_error:
alloc_ds_fail:
	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
alloc_qtd_fail:
	iosm_usb2_deallocate_usbtd(qtd0_slot);
	iosm_usb2_deallocate_usbtd(qh_slot);
alloc_qh_fail:
	// sfree(p, sizeof(*p));
	libusb2_usbpdev_free_pipe(p);
    return NULL;
}

SAP_STATUS usb_ehci_send_bulk_single_qtd(struct usb_new_pipe* p, int dir, void *data, size_t datasize, uint32_t timeout_ms)
{
	SAP_STATUS status = SUCCESS;
	bool wait_td_success = true;

	if(!p || !data)
		return EINVAL;

	if(p->pehci_usbpipe)
	{
		USB_DEBUG("<usb_ehci_send_bulk> The pipe (%p) must not have an active internal USB pipe!\n", p);
		return EINVAL;
	}

    if (datasize > 4 * PAGE_SIZE_4K)
	{
        // FIXME: should support larger sizes.
        warn_noalloc();
        return EINVAL;
    }

    // [TODO] Remove the ugly hack (ulong_t) below
	USB_DEBUG("<usb_ehci_send_bulk> usbdev_pipe:0x%p, dir=0x%x, data of size %lu at addr 0x%lx!\n",
          p, dir, (ulong_t)datasize, (vaddr_t)data);

	// Step 1. Allocate 1 QH and 4 QTDs
	uint32_t qtd0_slot = USBTD_SlotID_INVALID;
	uint32_t qh_slot = USBTD_SlotID_INVALID;

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QH32, &qh_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qh_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd0_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}


	// Step 2. Allocate data structures
	struct ehci_new_qtd* qtd0_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd0_content));

	if (!qtd0_content)
	{
        warn_noalloc();
		status = ENOMEM;
        goto alloc_ds_fail;
    }

	p->qh_slot = qh_slot;
	p->qtd_slots[0] = qtd0_slot;
	p->qtd_slots[1] = USBTD_SlotID_INVALID;
	p->qtd_slots[2] = USBTD_SlotID_INVALID;
	p->qtd_slots[3] = USBTD_SlotID_INVALID;
	//p->data_buf = data;


	// Step 3. Generate QH and QTD contents
	// qH
	p->header_content->qtd_next.next_td_slot = qtd0_slot;
	p->header_content->qtd_next.other_bits = 0;
	//// Explicitly configure <p->qtd_cir_cfg>: def_pipe's qTDs do not ref each other in circular
	p->qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
	p->qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;

	// qTDs
	{
		u16 maxpacket = p->maxpacket;
		int tdpos = 0;
		int next_tdpos = (tdpos + 1) % STACKQTDS;

		// Init all qTDs' buf pointer to dummy address
		// [NOTE] This is needed for the case that <buf> does not occupy all the five pages of each qTDs
		{
			paddr_t dest_hw = sap_mm_vaddr_to_paddr((vaddr_t)data);

			qtd0_content->buf[4].buf_pfn = qtd0_content->buf[3].buf_pfn = qtd0_content->buf[2].buf_pfn =
			qtd0_content->buf[1].buf_pfn = qtd0_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;
		}

		// Fill qTDs' buf pointers with the target buffer
		do
		{
			int transfer = 0;
			struct ehci_new_qtd* td = qtd0_content;
			if(!td)
			{
				USB_ERR("<usb_ehci_send_bulk_single_qtd> Failed to select QTD content! tdpos:%u\n", tdpos);
				status = E_USB_LIB_INTERNAL;
				goto fill_tds_error;
			}

			transfer = pal_new_fillTDbuffer(td, maxpacket, data, datasize);
			if(((size_t)transfer) == datasize)
				td->qtd_next.v = EHCI_PTR_TERM;
			else
			{
				// td->qtd_next.next_td_slot = USBTD_SlotID_INVALID;
				// td->qtd_next.other_bits = 0;

				// Just one qTD, so there is no pointer to any next TDs
				td->qtd_next.v = EHCI_PTR_TERM;
			}

			td->alt_next.v = EHCI_PTR_TERM;


			td->token = (ehci_explen(transfer) | QTD_STS_ACTIVE
						| ((dir & USB_DIR_IN) ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
			//mfence();
			USB_DEBUG("<usb_ehci_send_bulk_single_qtd> next_tdpos:%u, transfer:%u, td->token:0x%08X\n", next_tdpos, transfer, td->token);
			data += transfer;
			datasize -= transfer;

			// Fill next TD packet
			tdpos = next_tdpos;
			next_tdpos = (tdpos + 1) % STACKQTDS;
		} while (datasize);
	}

	// USB_INFO("Dump QH:\n");
	// pal_usb_dump_usbtd((vaddr_t)p->header_content);
	// USB_INFO("Dump QTD0:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd0_content);


	// Step 4. Submit the QH and QTD
	//// Submit QTD0 first, because it does not reference any other USB TDs
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd0_content, sizeof(*qtd0_content), qtd0_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> Submit qTD0 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk_single_qtd> Submit qTD0 succeed!\n");

	//// Submit qH
	status = iosm_usb2_usbtd_usbmit_and_verify(p->header_content, sizeof(struct ehci_new_qh), qh_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> Submit qH error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk_single_qtd> Submit qH succeed!\n");


	// Step 5. Update eEHCI's USB TD registers
	usbtd_reg_slot_t usbtd_reg_qh = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd0 = usbtd_reg_pick();
	{
		if((usbtd_reg_qh == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd0 == USBTD_REG_SLOT_INVALID))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Reserve USBTD reg in eEHCI error!\n");
			usbtd_reg_return(usbtd_reg_qh);
			usbtd_reg_return(usbtd_reg_qtd0);
			status = EIO;
			goto reserve_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qh, qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qh);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd0, qtd0_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk_single_qtd> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd0);
			status = EIO;
			goto write_eehci_reg_error;
		}

		p->usbtd_reg_qh = usbtd_reg_qh;
		p->usbtd_reg_qtds[0] = usbtd_reg_qtd0;
		p->usbtd_reg_qtds[1] = USBTD_REG_SLOT_INVALID;
		p->usbtd_reg_qtds[2] = USBTD_REG_SLOT_INVALID;
		p->usbtd_reg_qtds[3] = USBTD_REG_SLOT_INVALID;
	}


	// Step 6. Instruct the eEHCI to process these USB TDs
	//status = vkcall_attach_QH_new(p, 0);
	status = iosm_usb2_attach_qh_intr(p->eptype, p->qh_slot, 0, false, 0, 0, &p->pehci_usbpipe);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> vkcall_attach_QH_new error!\n");
		status = EIO;
		goto attach_qh_fail;
	}


	// Step 7. Wait for qTD completion
	int ret = pal_ehci_wait_td_new(p, qtd0_slot, timeout_ms * 1000);
	if(!ret)
	{
		USB_DEBUG("bulk td 0 is processed!\n");
	}
	else
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> Wait TD error! ret:%d\n", ret);
		wait_td_success = false;
		// status = EIO;
		// goto wait_td_err;
	}

	// Step 7. Unlink the qH and qTDs from the eEHCI
	//status = _usbpdev_unlink_pipe(p, qh_slot, qtd0_slot, qtd1_slot, qtd2_slot, qtd3_slot, usbtd_reg_qh, usbtd_reg_qtd0, usbtd_reg_qtd1, usbtd_reg_qtd2, usbtd_reg_qtd3);
	status = _usbpdev_unlink_pipe(p);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk_single_qtd> _usbpdev_unlink_pipe error!\n");
		status = EIO;
		goto unlink_defpipe_fail;
	}

	sfree(qtd0_content, sizeof(struct ehci_new_qtd));

	// On success
	if(!wait_td_success)
		return EIO;
	else
		return SUCCESS;




	// PAL-VK interface: clean up qtd list
	/*vkcall_attach_qtd_list(pipe, NULL, (paddr_t)NULL);

bulk_tds_error:
	for (i=0;i<3;i++) {
        struct ehci_qtd *td = &tds[i];
		if (td->buf_v) {
			pal_mm_dealloc(PAL_MEM_ORDINARY, td->buf_v);
		}
	}
    pal_mm_dealloc(PAL_MEM_DMA, tds);
    return ret;*/

unlink_defpipe_fail:
// wait_td_err:
attach_qh_fail:
write_eehci_reg_error:
reserve_eehci_reg_error:
submit_tds_fail:
fill_tds_error:
alloc_ds_fail:
	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
alloc_qtd_fail:
	iosm_usb2_deallocate_usbtd(qtd0_slot);
	iosm_usb2_deallocate_usbtd(qh_slot);
alloc_qh_fail:
    return status;
}

SAP_STATUS usb_ehci_send_bulk(struct usb_new_pipe* p, int dir, void *data, size_t datasize, uint32_t timeout_ms)
{
	SAP_STATUS status = SUCCESS;
	bool wait_td_success = true;
    
	if(!p || !data)
		return EINVAL;

	if(p->pehci_usbpipe)
	{
		USB_DEBUG("<usb_ehci_send_bulk> The pipe (%p) must not have an active internal USB pipe!\n", p);
		return EINVAL;
	}

    if (datasize > 4 * PAGE_SIZE_4K) 
	{
        // FIXME: should support larger sizes.
        warn_noalloc();
        return EINVAL;
    }
    
    // [TODO] Remove the ugly hack (ulong_t) below
	USB_DEBUG("<usb_ehci_send_bulk> usbdev_pipe:0x%p, dir=0x%x, data of size %lu at addr 0x%lx!\n",
          p, dir, (ulong_t)datasize, (vaddr_t)data);

	// Step 1. Allocate 1 QH and 4 QTDs
	uint32_t qtd0_slot = USBTD_SlotID_INVALID, qtd1_slot = USBTD_SlotID_INVALID, qtd2_slot = USBTD_SlotID_INVALID, qtd3_slot = USBTD_SlotID_INVALID;
	uint32_t qh_slot = USBTD_SlotID_INVALID;

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QH32, &qh_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qh_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd0_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd1_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd2_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd3_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}


	// Step 2. Allocate data structures
	struct ehci_new_qtd* qtd0_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd0_content));
	struct ehci_new_qtd* qtd1_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd1_content));
	struct ehci_new_qtd* qtd2_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd2_content));
	struct ehci_new_qtd* qtd3_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd3_content));

	if (!qtd0_content || !qtd1_content || !qtd2_content || !qtd3_content) 
	{
        warn_noalloc();
		status = ENOMEM;
        goto alloc_ds_fail;
    }

	p->qh_slot = qh_slot;
	p->qtd_slots[0] = qtd0_slot;
	p->qtd_slots[1] = qtd1_slot;
	p->qtd_slots[2] = qtd2_slot;
	p->qtd_slots[3] = qtd3_slot;
	//p->data_buf = data;
	

	// Step 3. Generate QH and QTD contents
	// qH
	p->header_content->qtd_next.next_td_slot = qtd0_slot;
	p->header_content->qtd_next.other_bits = 0;
	//// Explicitly configure <p->qtd_cir_cfg>: def_pipe's qTDs do not ref each other in circular
	p->qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
	p->qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;

	// qTDs
	{
		u16 maxpacket = p->maxpacket;
		int tdpos = 0;
		int next_tdpos = (tdpos + 1) % STACKQTDS;

		// Init all qTDs' buf pointer to dummy address
		// [NOTE] This is needed for the case that <buf> does not occupy all the five pages of each qTDs
		{
			paddr_t dest_hw = sap_mm_vaddr_to_paddr((vaddr_t)data);

			qtd0_content->buf[4].buf_pfn = qtd0_content->buf[3].buf_pfn = qtd0_content->buf[2].buf_pfn =
			qtd0_content->buf[1].buf_pfn = qtd0_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;

			qtd1_content->buf[4].buf_pfn = qtd1_content->buf[3].buf_pfn = qtd1_content->buf[2].buf_pfn =
			qtd1_content->buf[1].buf_pfn = qtd1_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;

			qtd2_content->buf[4].buf_pfn = qtd2_content->buf[3].buf_pfn = qtd2_content->buf[2].buf_pfn =
			qtd2_content->buf[1].buf_pfn = qtd2_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;

			qtd3_content->buf[4].buf_pfn = qtd3_content->buf[3].buf_pfn = qtd3_content->buf[2].buf_pfn =
			qtd3_content->buf[1].buf_pfn = qtd3_content->buf[0].buf_pfn = dest_hw >> PAGE_SHIFT_4K;
		}

		// Fill qTDs' buf pointers with the target buffer
		do
		{
			int transfer = 0;
			struct ehci_new_qtd* td = _sel_qtd_content(tdpos, qtd0_content, qtd1_content, qtd2_content, qtd3_content);
			if(!td)
			{
				USB_ERR("<usb_ehci_send_bulk> Failed to select QTD content! tdpos:%u\n", tdpos);
				status = E_USB_LIB_INTERNAL;
				goto fill_tds_error; 
			}
			
			transfer = pal_new_fillTDbuffer(td, maxpacket, data, datasize);
			if(((size_t)transfer) == datasize)
				td->qtd_next.v = EHCI_PTR_TERM;
			else
			{
				td->qtd_next.next_td_slot = _sel_qtd_slot(next_tdpos, qtd0_slot, qtd1_slot, qtd2_slot, qtd3_slot);
				td->qtd_next.other_bits = 0;
			}

			td->alt_next.v = EHCI_PTR_TERM;


			td->token = (ehci_explen(transfer) | QTD_STS_ACTIVE
						| ((dir & USB_DIR_IN) ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
			//mfence();
			USB_DEBUG("<usb_ehci_send_bulk> next_tdpos:%u, transfer:%u, td->token:0x%08X\n", next_tdpos, transfer, td->token);
			data += transfer;
			datasize -= transfer;

			// Fill next TD packet
			tdpos = next_tdpos;
			next_tdpos = (tdpos + 1) % STACKQTDS;
		} while (datasize);
	}

	// USB_INFO("Dump QTD3:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd3_content);
	// USB_INFO("Dump QTD2:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd2_content);
	// USB_INFO("Dump QTD1:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd1_content);
	// USB_INFO("Dump QTD0:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd0_content);


	// Step 4. Submit the QH and QTD
	//// Submit QTD3 first, because it does not reference any other USB TDs
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd3_content, sizeof(*qtd3_content), qtd3_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> Submit qTD3 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk> Submit qTD3 succeed!\n");

	//// Submit qTD2, qTD1 and qTD0
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd2_content, sizeof(*qtd2_content), qtd2_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> Submit qTD2 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk> Submit qTD2 succeed!\n");

	status = iosm_usb2_usbtd_usbmit_and_verify(qtd1_content, sizeof(*qtd1_content), qtd1_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> Submit qTD1 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk> Submit qTD1 succeed!\n");

	status = iosm_usb2_usbtd_usbmit_and_verify(qtd0_content, sizeof(*qtd0_content), qtd0_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> Submit qTD0 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk> Submit qTD0 succeed!\n");

	//// Submit qH
	status = iosm_usb2_usbtd_usbmit_and_verify(p->header_content, sizeof(struct ehci_new_qh), qh_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> Submit qH error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<usb_ehci_send_bulk> Submit qH succeed!\n");


	// Step 5. Update eEHCI's USB TD registers
	usbtd_reg_slot_t usbtd_reg_qh = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd0 = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd1 = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd2 = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd3 = usbtd_reg_pick();
	{
		if((usbtd_reg_qh == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd0 == USBTD_REG_SLOT_INVALID) || 
			(usbtd_reg_qtd1 == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd2 == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd3 == USBTD_REG_SLOT_INVALID))
		{
			USB_ERR("<usb_ehci_send_bulk> Reserve USBTD reg in eEHCI error!\n");
			usbtd_reg_return(usbtd_reg_qh);
			usbtd_reg_return(usbtd_reg_qtd0);
			usbtd_reg_return(usbtd_reg_qtd1);
			usbtd_reg_return(usbtd_reg_qtd2);
			usbtd_reg_return(usbtd_reg_qtd3);
			status = EIO;
			goto reserve_eehci_reg_error;
		}
	
		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qh, qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qh);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd0, qtd0_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd0);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd1, qtd1_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd1);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd2, qtd2_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd2);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd3, qtd2_slot);
		if(FAIL(status))
		{
			USB_ERR("<usb_ehci_send_bulk> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd3);
			status = EIO;
			goto write_eehci_reg_error;
		}

		p->usbtd_reg_qh = usbtd_reg_qh;
		p->usbtd_reg_qtds[0] = usbtd_reg_qtd0;
		p->usbtd_reg_qtds[1] = usbtd_reg_qtd1;
		p->usbtd_reg_qtds[2] = usbtd_reg_qtd2;
		p->usbtd_reg_qtds[3] = usbtd_reg_qtd3;
	}


	// Step 6. Instruct the eEHCI to process these USB TDs
	//status = vkcall_attach_QH_new(p, 0);
	status = iosm_usb2_attach_qh_intr(p->eptype, p->qh_slot, 0, false, 0, 0, &p->pehci_usbpipe);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> vkcall_attach_QH_new error!\n");
		status = EIO;
		goto attach_qh_fail;
	}


	// Step 7. Wait for qTD completion
	{
		int i = 0;
		for (i=0; i<STACKQTDS; i++) 
		{
			uint32_t qtd_slot = _sel_qtd_slot(i, qtd0_slot, qtd1_slot, qtd2_slot, qtd3_slot);
			int ret = pal_ehci_wait_td_new(p, qtd_slot, timeout_ms * 1000);
			if (ret)
			{
				USB_ERR("bulk td %d encounters error!\n", i);
				wait_td_success = false;
				// status = EIO;
				// goto wait_td_err;
			}
			USB_DEBUG("bulk td %d is processed!\n", i);
    	}
	}

	// Step 7. Unlink the qH and qTDs from the eEHCI
	//status = _usbpdev_unlink_pipe(p, qh_slot, qtd0_slot, qtd1_slot, qtd2_slot, qtd3_slot, usbtd_reg_qh, usbtd_reg_qtd0, usbtd_reg_qtd1, usbtd_reg_qtd2, usbtd_reg_qtd3);
	status = _usbpdev_unlink_pipe(p);
	if(FAIL(status))
	{
		USB_ERR("<usb_ehci_send_bulk> _usbpdev_unlink_pipe error!\n");
		status = EIO;
		goto unlink_defpipe_fail;
	}

	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
	sfree(qtd1_content, sizeof(struct ehci_new_qtd));
	sfree(qtd2_content, sizeof(struct ehci_new_qtd));
	sfree(qtd3_content, sizeof(struct ehci_new_qtd));

	// On success
	if(!wait_td_success)
		return EIO;
	else
		return SUCCESS;




	// PAL-VK interface: clean up qtd list 
	/*vkcall_attach_qtd_list(pipe, NULL, (paddr_t)NULL);

bulk_tds_error:
	for (i=0;i<3;i++) {
        struct ehci_qtd *td = &tds[i];
		if (td->buf_v) {
			pal_mm_dealloc(PAL_MEM_ORDINARY, td->buf_v);
		}
	}
    pal_mm_dealloc(PAL_MEM_DMA, tds);
    return ret;*/

unlink_defpipe_fail:
// wait_td_err:
attach_qh_fail:
write_eehci_reg_error:
reserve_eehci_reg_error:
submit_tds_fail:
fill_tds_error:
alloc_ds_fail:
	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
	sfree(qtd1_content, sizeof(struct ehci_new_qtd));
	sfree(qtd2_content, sizeof(struct ehci_new_qtd));
	sfree(qtd3_content, sizeof(struct ehci_new_qtd));
alloc_qtd_fail:
	iosm_usb2_deallocate_usbtd(qtd0_slot);
	iosm_usb2_deallocate_usbtd(qtd1_slot);
	iosm_usb2_deallocate_usbtd(qtd2_slot);
	iosm_usb2_deallocate_usbtd(qtd3_slot);
	iosm_usb2_deallocate_usbtd(qh_slot);
alloc_qh_fail:
    return status;
}

/*int usb_ehci_send_bulk(struct usb_pipe *p, int dir, void *data, int datasize)
{
	int ret, i;
	u32 tds_hw;
    struct ehci_pipe *pipe = container_of(p, struct ehci_pipe, pipe);

	if (datasize > 4*4096) {
        // FIXME: should support larger sizes.
        warn_noalloc();
        return -1;
    }

	// Allocate 4 tds on stack (with required alignment)
    struct ehci_qtd *tds = (struct ehci_qtd *)pal_mm_alloc_align(PAL_MEM_DMA, EHCI_QTD_ALIGN, sizeof(struct ehci_qtd) * STACKQTDS);
	tds_hw = sap_mm_vaddr_to_paddr((u32)tds);
    if (!tds) {
        warn_noalloc();
        return -1;
    }
	USB_DEBUG("<usb_ehci_send_bulk> alloc qtds of size %08X at addr %08X, with alignment %08X!\n", 
		sizeof(struct ehci_qtd)*STACKQTDS, tds, EHCI_QTD_ALIGN);

	USB_DEBUG("<usb_ehci_send_bulk> qh=%p dir=%d data=%p size=%d\n"
        , &pipe->qh, dir, data, datasize);
		
    pal_mm_memset(tds, 0, sizeof(*tds) * STACKQTDS);
	
	// Fill TD Packets
	//mfence();
	u16 maxpacket = p->maxpacket;
    int tdpos = 0;
	int next_tdpos = (tdpos + 1) % STACKQTDS;
	struct ehci_qtd *td = tds;
    while (datasize) {
        int transfer = pal_fillTDbuffer(td, maxpacket, data, datasize);
        td->qtd_next = (transfer==datasize ? EHCI_PTR_TERM : (u32)(tds_hw + sizeof(struct ehci_qtd) * next_tdpos));
		td->qtd_next_v = (transfer==datasize ? EHCI_PTR_TERM : (u32)&tds[next_tdpos]);
        td->alt_next = EHCI_PTR_TERM;
		
        
        td->token = (ehci_explen(transfer) | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
		//mfence();
		USB_ERR("<usb_ehci_send_bulk> next_tdpos:%d, transfer:%d, td->token:0x%08X\n", next_tdpos, transfer, td->token);
        data += transfer;
        datasize -= transfer;

		// Fill next TD packet
		tdpos = next_tdpos;
		
		next_tdpos = (tdpos + 1) % STACKQTDS;
		td = &tds[tdpos];
    }

	mfence();
	// PAL-VK interface: attach qtd list
	if (vkcall_attach_qtd_list(pipe, tds, tds_hw)) {
		warn_internalerror();
		goto bulk_tds_error;
	}

	mfence();
	
	// wait for QTD completion 
    for (i=0; i<STACKQTDS; i++) {
        struct ehci_qtd *td = &tds[tdpos++ % STACKQTDS];
        int ret = pal_ehci_wait_td(pipe, td, 500);
        if (ret)
            break;
		USB_DEBUG("bulk td %d is processed!\n", i);
    }

	// PAL-VK interface: clean up qtd list 
	vkcall_attach_qtd_list(pipe, NULL, (paddr_t)NULL);

bulk_tds_error:
	for (i=0;i<3;i++) {
        struct ehci_qtd *td = &tds[i];
		if (td->buf_v) {
			pal_mm_dealloc(PAL_MEM_ORDINARY, td->buf_v);
		}
	}
    pal_mm_dealloc(PAL_MEM_DMA, tds);
    return ret;
}*/


static SAP_STATUS _usbpdev_unlink_pipe(struct usb_new_pipe* p)
{
	SAP_STATUS status = SUCCESS;
	uint32_t qh_slot;
	uint32_t qtd0_slot;
	uint32_t qtd1_slot;
	uint32_t qtd2_slot;
	uint32_t qtd3_slot;
	usbtd_reg_slot_t eehci_usbtd_reg_qh;
	usbtd_reg_slot_t eehci_usbtd_reg_qtd0;
	usbtd_reg_slot_t eehci_usbtd_reg_qtd1;
	usbtd_reg_slot_t eehci_usbtd_reg_qtd2;
	usbtd_reg_slot_t eehci_usbtd_reg_qtd3;

	// Sanity checks
	if(!p)
		return EINVAL;

	qh_slot = p->qh_slot;
	qtd0_slot = p->qtd_slots[0];
	qtd1_slot = p->qtd_slots[1];
	qtd2_slot = p->qtd_slots[2];
	qtd3_slot = p->qtd_slots[3];
	eehci_usbtd_reg_qh = p->usbtd_reg_qh;
	eehci_usbtd_reg_qtd0 = p->usbtd_reg_qtds[0];
	eehci_usbtd_reg_qtd1 = p->usbtd_reg_qtds[1];
	eehci_usbtd_reg_qtd2 = p->usbtd_reg_qtds[2];
	eehci_usbtd_reg_qtd3 = p->usbtd_reg_qtds[3];

	if(qh_slot != USBTD_SlotID_INVALID)
	{
		// Step 1. clean the eEHCI's USB TD registers
		if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qh))
		{
			status = iosm_usb2_eehci_write_usbtd_reg(eehci_usbtd_reg_qh, USBTD_SlotID_INVALID);
			if(FAIL(status))
			{
				USB_ERR("<_usbpdev_unlink_pipe> Write eEHCI's USBTD Reg %u Error!\n", eehci_usbtd_reg_qh);
				status = EIO;
				goto write_eehci_reg_error;
			}
		}

		// Step 2. Deallocate QH and QTDs in the WK
		status = iosm_usb2_deallocate_usbtd(qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate QH Error! qh_slot:%u, status:%lu\n", qh_slot, status);
			status = EIO;
			goto write_eehci_reg_error;
		}
	}

	if(qtd0_slot != USBTD_SlotID_INVALID)
	{
		// Step 1. clean the eEHCI's USB TD registers
		if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd0))
		{
			status = iosm_usb2_eehci_write_usbtd_reg(eehci_usbtd_reg_qtd0, USBTD_SlotID_INVALID);
			if(FAIL(status))
			{
				USB_ERR("<_usbpdev_unlink_pipe> Write eEHCI's USBTD Reg %u Error!\n", eehci_usbtd_reg_qtd0);
				status = EIO;
				goto write_eehci_reg_error;
			}
		}

		// Step 2. Deallocate QH and QTDs in the WK
		status = iosm_usb2_deallocate_usbtd(qtd0_slot);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate QTD0 Error! qtd_slot:%u, status:%lu\n", qtd0_slot, status);
			status = EIO;
			goto deallocate_usbtd_error;
		}
	}

	if(qtd1_slot != USBTD_SlotID_INVALID)
	{
		// Step 1. clean the eEHCI's USB TD registers
		if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd1))
		{
			status = iosm_usb2_eehci_write_usbtd_reg(eehci_usbtd_reg_qtd1, USBTD_SlotID_INVALID);
			if(FAIL(status))
			{
				USB_ERR("<_usbpdev_unlink_pipe> Write eEHCI's USBTD Reg %u Error!\n", eehci_usbtd_reg_qtd1);
				status = EIO;
				goto write_eehci_reg_error;
			}
		}

		// Step 2. Deallocate QH and QTDs in the WK
		status = iosm_usb2_deallocate_usbtd(qtd1_slot);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate QTD0 Error! qtd_slot:%u, status:%lu\n", qtd1_slot, status);
			status = EIO;
			goto deallocate_usbtd_error;
		}
	}

	if(qtd2_slot != USBTD_SlotID_INVALID)
	{
		// Step 1. clean the eEHCI's USB TD registers
		if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd2))
		{
			status = iosm_usb2_eehci_write_usbtd_reg(eehci_usbtd_reg_qtd2, USBTD_SlotID_INVALID);
			if(FAIL(status))
			{
				USB_ERR("<_usbpdev_unlink_pipe> Write eEHCI's USBTD Reg %u Error!\n", eehci_usbtd_reg_qtd2);
				status = EIO;
				goto write_eehci_reg_error;
			}
		}

		// Step 2. Deallocate QH and QTDs in the WK
		status = iosm_usb2_deallocate_usbtd(qtd2_slot);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate QTD0 Error! qtd_slot:%u, status:%lu\n", qtd2_slot, status);
			status = EIO;
			goto deallocate_usbtd_error;
		}
	}

	if(qtd3_slot != USBTD_SlotID_INVALID)
	{
		// Step 1. clean the eEHCI's USB TD registers
		if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd3))
		{
			status = iosm_usb2_eehci_write_usbtd_reg(eehci_usbtd_reg_qtd3, USBTD_SlotID_INVALID);
			if(FAIL(status))
			{
				USB_ERR("<_usbpdev_unlink_pipe> Write eEHCI's USBTD Reg %u Error!\n", eehci_usbtd_reg_qtd3);
				status = EIO;
				goto write_eehci_reg_error;
			}
		}

		// Step 2. Deallocate QH and QTDs in the WK
		status = iosm_usb2_deallocate_usbtd(qtd3_slot);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate QTD0 Error! qtd_slot:%u, status:%lu\n", qtd3_slot, status);
			status = EIO;
			goto deallocate_usbtd_error;
		}
	}

	// Step 3. Notify WK's physical EHCI driver to unlink the internal USB pipe
	if(p->pehci_usbpipe)
	{
		status = iosm_usb2_deattach_qh(p->pehci_usbpipe, p->eptype);
		if(FAIL(status))
		{
			USB_ERR("<_usbpdev_unlink_pipe> Deallocate def pipe Error! status:%lu\n", status);
			status = EIO;
			goto deattach_usbpipe_error;
		}
	}

	// Step 4. Clean the usbpipe <p>'s info set in <ehci_send_control>
	if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qh))
		usbtd_reg_return(eehci_usbtd_reg_qh);
	if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd0))
		usbtd_reg_return(eehci_usbtd_reg_qtd0);
	if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd1))
		usbtd_reg_return(eehci_usbtd_reg_qtd1);
	if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd2))
		usbtd_reg_return(eehci_usbtd_reg_qtd2);
	if(IS_USBTD_REG_SLOT_VALID(eehci_usbtd_reg_qtd3))
		usbtd_reg_return(eehci_usbtd_reg_qtd3);

	p->qh_slot = USBTD_SlotID_INVALID;
	p->qtd_slots[0] = USBTD_SlotID_INVALID;
	p->qtd_slots[1] = USBTD_SlotID_INVALID;
	p->qtd_slots[2] = USBTD_SlotID_INVALID;
	p->qtd_slots[3] = USBTD_SlotID_INVALID;
	p->usbtd_reg_qh = USBTD_REG_SLOT_INVALID;
	p->usbtd_reg_qtds[0] = USBTD_REG_SLOT_INVALID;
	p->usbtd_reg_qtds[1] = USBTD_REG_SLOT_INVALID;
	p->usbtd_reg_qtds[2] = USBTD_REG_SLOT_INVALID;
	p->usbtd_reg_qtds[3] = USBTD_REG_SLOT_INVALID;

	//p->data_buf = NULL;
	p->pehci_usbpipe = INVALID_USBPIPE_HANDLE;

	return SUCCESS;


deattach_usbpipe_error:
deallocate_usbtd_error:
write_eehci_reg_error:
	return status;
}

SAP_STATUS ehci_send_control(
	struct usb_new_pipe* p, 
	int dir, 
	const void *cmd, 
	size_t cmdsize,
    void* data, 
	size_t datasize, 
	u16 maxpacket,
	uint32_t td_timeout_ms
)
{
	SAP_STATUS status = SUCCESS;
	bool wait_td_success = true;

	if(!p || !cmd || !data)
		return EINVAL;

	if(p->pehci_usbpipe)
	{
		USB_DEBUG("<ehci_send_control> The pipe (%p) must not have an active internal USB pipe!\n", p);
		return EINVAL;
	}

    
    if (datasize > 4 * PAGE_SIZE_4K || cmdsize > 4 * PAGE_SIZE_4K) 
	{
        // FIXME: should support larger sizes.
        warn_noalloc();
        return EINVAL;
    }

    // [TODO] Remove the ugly hack (ulong_t) below    
	USB_DEBUG("<ehci_send_control> usbdev_pipe:0x%p, dir=0x%x, cmd of size %lu at addr 0x%lx, data of size %lu at addr 0x%lx!\n",
          p, dir, (ulong_t)cmdsize, (vaddr_t)cmd, (ulong_t)datasize, (vaddr_t)data);

	// Step 1. Allocate 1 QH and 3 QTDs
	uint32_t qtd0_slot = USBTD_SlotID_INVALID, qtd1_slot = USBTD_SlotID_INVALID, qtd2_slot = USBTD_SlotID_INVALID;
	uint32_t qh_slot = USBTD_SlotID_INVALID;

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QH32, &qh_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qh_fail;
	}
	USB_DEBUG("<ehci_send_control> Allocated qH at slot:%u!\n", qh_slot);

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd0_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}
	USB_DEBUG("<ehci_send_control> Allocated qtd0 at slot:%u!\n", qtd0_slot);

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd1_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}
	USB_DEBUG("<ehci_send_control> Allocated qtd1 at slot:%u!\n", qtd1_slot);

	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd2_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		status = EIO;
		goto alloc_qtd_fail;
	}
	USB_DEBUG("<ehci_send_control> Allocated qtd2 at slot:%u!\n", qtd2_slot);
	

	// Step 2. Allocate data structures
	struct ehci_new_qtd* qtd0_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd0_content));
	struct ehci_new_qtd* qtd1_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd1_content));
	struct ehci_new_qtd* qtd2_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd2_content));

	if (!qtd0_content || !qtd1_content || !qtd2_content) 
	{
        warn_noalloc();
		status = ENOMEM;
        goto alloc_ds_fail;
    }

	p->qh_slot = qh_slot;
	p->qtd_slots[0] = qtd0_slot;
	p->qtd_slots[1] = qtd1_slot;
	p->qtd_slots[2] = qtd2_slot;
	p->qtd_slots[3] = USBTD_SlotID_INVALID;
	//p->data_buf = data;
	

	// Step 3. Generate QH and QTD contents
	// qH
	p->header_content->qtd_next.next_td_slot = qtd0_slot;
	p->header_content->qtd_next.other_bits = 0;
	//// Explicitly configure <p->qtd_cir_cfg>: def_pipe's qTDs do not ref each other in circular
	p->qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
	p->qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;
	USB_DEBUG("<ehci_send_control> QH info1:0x%08X, info2:0x%08X\n", p->header_content->info1, p->header_content->info2);

	//// qTD0 - SETUP packet
	qtd0_content->qtd_next.next_td_slot = qtd1_slot;
	qtd0_content->qtd_next.other_bits = 0;
	qtd0_content->alt_next.v = EHCI_PTR_TERM;
	qtd0_content->token = (ehci_explen(cmdsize) | QTD_STS_ACTIVE
                 | QTD_PID_SETUP | ehci_maxerr(3));
	USB_DEBUG("<ehci_send_control> qtd0 token:0x%08X\n", qtd0_content->token);

	if ((size_t)pal_new_fillTDbuffer(qtd0_content, maxpacket, cmd, cmdsize) != cmdsize) {
		warn_internalerror();
		status = EIO;
		goto fill_tds_error;
	}
    
	// qTD1 - DATA packet
	{
		qtd1_content->qtd_next.next_td_slot = qtd2_slot;
		qtd1_content->alt_next.v = EHCI_PTR_TERM;
		qtd1_content->token = (QTD_TOGGLE | ehci_explen(datasize) | QTD_STS_ACTIVE
                     | ((dir & USB_DIR_IN) ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
		USB_DEBUG("<ehci_send_control> qtd1 token:0x%08X\n", qtd1_content->token);

		// The data should be allocated from DMA pool 
        if ((size_t)pal_new_fillTDbuffer(qtd1_content, maxpacket, data, datasize) != datasize) {
			warn_internalerror();
			status = EIO;
			goto fill_tds_error;
		}
    }

	// qTD2 - Status packet
	qtd2_content->qtd_next.v = EHCI_PTR_TERM;
	qtd2_content->alt_next.v = EHCI_PTR_TERM;
	qtd2_content->token = (QTD_TOGGLE | QTD_STS_ACTIVE
                 | ((dir & USB_DIR_IN) ? QTD_PID_OUT : QTD_PID_IN) | ehci_maxerr(3));
	USB_DEBUG("<ehci_send_control> qtd2 token:0x%08X\n", qtd2_content->token);

	qtd2_content->buf[4].buf_pfn = qtd2_content->buf[3].buf_pfn = qtd2_content->buf[2].buf_pfn = 
			qtd2_content->buf[1].buf_pfn = qtd2_content->buf[0].buf_pfn = qtd0_content->buf[0].buf_pfn;

	// USB_INFO("Dump QH:\n");
	// pal_usb_dump_usbtd((vaddr_t)p->header_content);
	// USB_INFO("Dump QTD0:\n");
	// pal_usb_dump_usbtd((vaddr_t)qtd0_content);
	
	// Step 4. Submit the QH and QTD
	//// Submit QTD2 first, because it does not reference any other USB TDs
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd2_content, sizeof(*qtd2_content), qtd2_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<ehci_send_control> Submit qTD2 error, status:%lu!\n", status);
		status = status;
		goto submit_tds_fail;
	}
	USB_DEBUG("<ehci_send_control> Submit qTD2 succeed!\n");

	//// Submit qTD1 and qTD0
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd1_content, sizeof(*qtd1_content), qtd1_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<ehci_send_control> Submit qTD1 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<ehci_send_control> Submit qTD1 succeed!\n");

	status = iosm_usb2_usbtd_usbmit_and_verify(qtd0_content, sizeof(*qtd0_content), qtd0_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<ehci_send_control> Submit qTD0 error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<ehci_send_control> Submit qTD0 succeed!\n");

	//// Submit qH
	status = iosm_usb2_usbtd_usbmit_and_verify(p->header_content, sizeof(struct ehci_new_qh), qh_slot, eehci_id, p->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<ehci_send_control> Submit qH error, status:%lu!\n", status);
		status = EIO;
		goto submit_tds_fail;
	}
	USB_DEBUG("<ehci_send_control> Submit qH succeed!\n");


	// Step 5. Update eEHCI's USB TD registers
	usbtd_reg_slot_t usbtd_reg_qh = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd0 = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd1 = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd2 = usbtd_reg_pick();
	{
		if((usbtd_reg_qh == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd0 == USBTD_REG_SLOT_INVALID) || 
			(usbtd_reg_qtd1 == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd2 == USBTD_REG_SLOT_INVALID))
		{
			USB_ERR("<ehci_send_control> Reserve USBTD reg in eEHCI error!\n");
			usbtd_reg_return(usbtd_reg_qh);
			usbtd_reg_return(usbtd_reg_qtd0);
			usbtd_reg_return(usbtd_reg_qtd1);
			usbtd_reg_return(usbtd_reg_qtd2);
			status = EIO;
			goto reserve_eehci_reg_error;
		}
	
		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qh, qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<ehci_send_control> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qh);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd0, qtd0_slot);
		if(FAIL(status))
		{
			USB_ERR("<ehci_send_control> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd0);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd1, qtd1_slot);
		if(FAIL(status))
		{
			USB_ERR("<ehci_send_control> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd1);
			status = EIO;
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd2, qtd2_slot);
		if(FAIL(status))
		{
			USB_ERR("<ehci_send_control> Write eEHCI's USBTD Reg %u Error!\n", usbtd_reg_qtd2);
			status = EIO;
			goto write_eehci_reg_error;
		}

		p->usbtd_reg_qh = usbtd_reg_qh;
		p->usbtd_reg_qtds[0] = usbtd_reg_qtd0;
		p->usbtd_reg_qtds[1] = usbtd_reg_qtd1;
		p->usbtd_reg_qtds[2] = usbtd_reg_qtd2;
		p->usbtd_reg_qtds[3] = USBTD_REG_SLOT_INVALID;
	}


	// Step 6. Instruct the eEHCI to process these USB TDs
	status = iosm_usb2_attach_qh_intr(p->eptype, p->qh_slot, 0, false, 0, 0, &p->pehci_usbpipe);
	if(FAIL(status))
	{
		USB_ERR("<ehci_send_control> vkcall_attach_QH_new error!\n");
		status = EIO;
		goto attach_qh_fail;
	}


	// Step 7. Wait for qTD completion
	int ret = pal_ehci_wait_td_new(p, qtd0_slot, 500);
	if (!ret)
	{
		USB_DEBUG("qtd0 is processed!\n");
		ret = pal_ehci_wait_td_new(p, qtd1_slot, td_timeout_ms * 1000);
		if (!ret)
		{
			USB_DEBUG("qtd1 is processed!\n");
			ret = pal_ehci_wait_td_new(p, qtd2_slot, 500);
			if (!ret)
			{
				USB_DEBUG("qtd2 is processed!\n");
			}
			else
			{
				USB_ERR("qtd2 encounters error! ret:%d\n", ret);
				wait_td_success = false;
				// status = EIO;
				// goto wait_td_err;
			}
		}
		else
		{
			USB_ERR("qtd1 encounters error! ret:%d\n", ret);
			wait_td_success = false;
			// status = EIO;
			// goto wait_td_err;
		}
	}
	else
	{
		USB_ERR("qtd0 encounters error! ret:%d\n", ret);
		wait_td_success = false;
		// status = EIO;
		// goto wait_td_err;
	}

	// Step 8. Unlink the qH and qTDs from the eEHCI
	status = _usbpdev_unlink_pipe(p);
	if(status)
	{
		USB_ERR("<ehci_send_control> _usbpdev_unlink_pipe error!\n");
		status = EIO;
		goto unlink_defpipe_fail;
	}

	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
	sfree(qtd1_content, sizeof(struct ehci_new_qtd));
	sfree(qtd2_content, sizeof(struct ehci_new_qtd));

	// On success
	if(!wait_td_success)
		return EIO;
	else
		return SUCCESS;


unlink_defpipe_fail:
// wait_td_err:
attach_qh_fail:
write_eehci_reg_error:
reserve_eehci_reg_error:
submit_tds_fail:
fill_tds_error:
alloc_ds_fail:
	sfree(qtd0_content, sizeof(struct ehci_new_qtd));
	sfree(qtd1_content, sizeof(struct ehci_new_qtd));
	sfree(qtd2_content, sizeof(struct ehci_new_qtd));
alloc_qtd_fail:
	iosm_usb2_deallocate_usbtd(qtd0_slot);
	iosm_usb2_deallocate_usbtd(qtd1_slot);
	iosm_usb2_deallocate_usbtd(qtd2_slot);
	iosm_usb2_deallocate_usbtd(qh_slot);
alloc_qh_fail:
    return status;
}




// high bandwidth multiplier, as encoded in highspeed endpoint descriptors
#define hb_mult(wMaxPacketSize) (1 + (((wMaxPacketSize) >> 11) & 0x03))


// Fill "pipe" endpoint info from an endpoint descriptor.
static void usb_desc2pipe(
    struct usb_new_pipe* pipe, 
    struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
)
{
    // pipe->cntl = usbdev->hub->cntl;
   // pipe->type = usbdev->hub->cntl->type;
    pipe->type = USB_TYPE_EHCI;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	memcpy(&pipe->devaddr, &usbdev->devaddr, sizeof(USBPDEV_ADDR));
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
static int usb_getFrameExp(
    struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
)
{
    int period = epdesc->bInterval;
    if (usbdev->speed != USB_HIGHSPEED)
        return (period <= 0) ? 0 : __fls(period);
    return (period <= 4) ? 0 : period - 4;
}

static int pal_ehci_desc2qh(
	struct usb_new_pipe* pipe, 
	struct sp_usbpdev_s* usbdev,
	struct usb_endpoint_descriptor *epdesc,
	uint32_t* qh_04h, 	// Out: Generate QH's dword at 04h-07h
	uint32_t* qh_08h 	// Out: Generate QH's dword at 08h-0Bh
)
{
    usb_desc2pipe(pipe, usbdev, epdesc);

	u32 info1=0, info2=0;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	u16 maxp = (pipe->maxpacket) & 0x7ff;
	uint32_t hub_port = usbdev->devaddr.hub_port;
	uint32_t hub_usbaddr = usbdev->devaddr.hub_usbaddr;
	uint32_t dev_usbaddr = usbdev->devaddr.dev_usbaddr;

    info1 = ((pipe->speed << QH_SPEED_SHIFT)
                      | (pipe->ep << QH_EP_SHIFT)
                      | (dev_usbaddr << QH_DEVADDR_SHIFT));

	switch (pipe->speed) {
	case USB_LOWSPEED:
		/* FALL THROUGH */
	case USB_FULLSPEED:
		if (eptype != USB_ENDPOINT_XFER_INT)
			info1 |= (EHCI_TUNE_RL_TT << QH_RL_SHIFT);
		if (eptype == USB_ENDPOINT_XFER_CONTROL) {
			info1 |= QH_CONTROL;	/* for TT */
			info1 |= QH_TOGGLECONTROL;	/* toggle from qtd */
		}
		info1 |= maxp << QH_MAXPACKET_SHIFT;

		info2 |= (EHCI_TUNE_MULT_TT << QH_MULT_SHIFT);

//		/* Some Freescale processors have an erratum in which the
//		 * port number in the queue head was 0..N-1 instead of 1..N.
//		 */
//		if (ehci_has_fsl_portno_bug(ehci))
//			info2 |= (urb->dev->ttport-1) << 23;
//		else
			info2 |= (hub_port<< QH_HUBPORT_SHIFT);

//		/* set the address of the TT; for TDI's integrated
//		 * root hub tt, leave it zeroed.
//		 */
//		if (tt && tt->hub != ehci_to_hcd(ehci)->self.root_hub)
			info2 |= (hub_usbaddr << QH_HUBADDR_SHIFT);

		/* NOTE:  if (PIPE_INTERRUPT) { scheduler sets c-mask } */

		break;

	case USB_HIGHSPEED:		/* no TT involved */
		if (eptype == USB_ENDPOINT_XFER_CONTROL) {
			info1 |= (EHCI_TUNE_RL_HS << QH_RL_SHIFT);
			info1 |= (64 << QH_MAXPACKET_SHIFT);	/* usb2 fixed maxpacket */
			info1 |= QH_TOGGLECONTROL;	/* toggle from qtd */
			info2 |= (EHCI_TUNE_MULT_HS << QH_MULT_SHIFT);
		} else if (eptype == USB_ENDPOINT_XFER_BULK) {
			info1 |= (EHCI_TUNE_RL_HS << QH_RL_SHIFT);
			/* The USB spec says that high speed bulk endpoints
			 * always use 512 byte maxpacket.  But some device
			 * vendors decided to ignore that, and MSFT is happy
			 * to help them do so.  So now people expect to use
			 * such nonconformant devices with Linux too; sigh.
			 */
			info1 |= (maxp << QH_MAXPACKET_SHIFT);
			info2 |= (EHCI_TUNE_MULT_HS << QH_MULT_SHIFT);
		} else {		/* USB_ENDPOINT_XFER_INT */
			info1 |= (maxp << QH_MAXPACKET_SHIFT);
			info2 |= (hb_mult(maxp) << QH_MULT_SHIFT);
		}
		break;
	default:
		USB_ERR("unrecognized device speed!\n");
		return -1;
	}

	/* FIXME: looks like very rough scheduling */
	if (eptype == USB_ENDPOINT_XFER_INT)
        info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);

	*qh_04h = info1;
	*qh_08h = info2;
	USB_DEBUG("QH info1 %08x, info2 %08x!\n", info1, info2);
	return 0;
}

struct usb_new_pipe* libusb2_usbpdev_alloc_intr_pipe(
	struct sp_usbpdev_s *usbdev,
    struct usb_endpoint_descriptor *epdesc
)
{
	int status = 0;
	paddr_t data_hw;
    struct ehci_new_qh* qh_content = NULL;
    struct ehci_new_qtd* qtd_content = NULL;

	// Sanity checks
	if(!usbdev || !epdesc)
		return NULL;

    int frameexp = usb_getFrameExp(usbdev, epdesc);

    if (frameexp > 10)
        frameexp = 10;
    int ms = 1<<frameexp;

    int maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    //int count = DIV_ROUND_UP(ticks_to_ms(2), ms);
	unsigned int count = 1;
    USB_DEBUG("libusb2_usbpdev_alloc_intr_pipe frameexp %d, interval %d ms, qtd counts %d!\n", frameexp, ms, count);

	// Step 1. Allocate 1 QH and 1 QTD
	uint32_t qh_slot = USBTD_SlotID_INVALID;
	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QH32, &qh_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		goto alloc_qh_fail;
	}

	uint32_t qtd_slot = USBTD_SlotID_INVALID;
	status = iosm_usb2_allocate_usbtd(USBTDs_TYPE_QTD32, &qtd_slot);
	if(FAIL(status))
	{
		warn_internalerror();
		goto alloc_qtd_fail;
	}

	// Step 2. Allocate data structures
	struct usb_new_pipe* intr_pipe = (struct usb_new_pipe*)smalloc(sizeof(*intr_pipe));
	// struct ehci_qtd* intr_tds = (struct ehci_qtd *)smalloc(sizeof(*intr_tds) * count);
	// FIXME: Is it necessary to use maxpacket*count alignment to make sure 
	// that data buffer will not cross page boundary?? 
	unsigned int intr_data_size = ROUND_UP_TO_NEXT_POWER_OF_2_uint32(maxpacket * count);
	void *intr_data = usbbuf_alloc_align(USBBUF_MEM_DMA, intr_data_size, intr_data_size);
	data_hw = sap_mm_vaddr_to_paddr((vaddr_t)intr_data);
	// if (!intr_pipe || !intr_tds || !intr_data) 
	if (!intr_pipe || !intr_data)
	{
        warn_noalloc();
        goto alloc_ds_fail;
    }

	// [TODO] ugly hack. We allocate *_content from DMA memory only for the purpose to get their physical addresses
	qh_content = (struct ehci_new_qh*)smalloc(sizeof(*qh_content)); 
	qtd_content = (struct ehci_new_qtd*)smalloc(sizeof(*qtd_content));

	if (!qh_content || !qtd_content) 
	{
        warn_noalloc();
        goto alloc_ds_fail;
    }

    USB_DEBUG("libusb2_usbpdev_alloc_intr_pipe data_hw %p!\n", (void*)data_hw);

	intr_pipe->usbpdev_slot = usbdev->usbpdev_slot;
	intr_pipe->eptype = USB_ENDPOINT_XFER_INT;

	intr_pipe->qh_slot = qh_slot;
	intr_pipe->qtd_slots[0] = qtd_slot; // [TODO] intr_pipe may need to record multiple qtd slots
	intr_pipe->qtd_slots[1] = USBTD_SlotID_INVALID;
	intr_pipe->qtd_slots[2] = USBTD_SlotID_INVALID;
	intr_pipe->qtd_slots[3] = USBTD_SlotID_INVALID;

    intr_pipe->p_intr_info.data_buf = intr_data;
	intr_pipe->qtd_cir_cfg.src_qtd_slot = qtd_slot;
	intr_pipe->qtd_cir_cfg.dst_qtd_slot = qtd_slot;

	// Step 3. Generate QH and QTD contents
	//// Generate QTD contents
	qtd_content->qtd_next.v = EHCI_PTR_TERM;
	qtd_content->alt_next.v = EHCI_PTR_TERM;
	qtd_content->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
	qtd_content->buf[0].v = data_hw;

	qtd_content->buf[4].buf_pfn = qtd_content->buf[3].buf_pfn = qtd_content->buf[2].buf_pfn = 
		qtd_content->buf[1].buf_pfn = qtd_content->buf[0].buf_pfn;

	//// Generate QH contents
	uint32_t qh_04h = 0;
	uint32_t qh_08h = 0;
	status = pal_ehci_desc2qh(intr_pipe, usbdev, epdesc, &qh_04h, &qh_08h);
    if (status == -1) 
	{
		warn_internalerror();
		goto generate_tds_fail;
	}

	qh_content->info1 = qh_04h;
	qh_content->info2 = qh_08h;

	qh_content->next.v = EHCI_PTR_TERM;
	qh_content->qtd_next.next_td_slot = qtd_slot;
	qh_content->qtd_next.other_bits = 0;
	qh_content->alt_next.v = EHCI_PTR_TERM;

	qh_content->buf[4].buf_pfn = qh_content->buf[3].buf_pfn = qh_content->buf[2].buf_pfn = 
		qh_content->buf[1].buf_pfn = qh_content->buf[0].buf_pfn = qtd_content->buf[0].buf_pfn;

	// Step 4. Submit the QH and QTD
	//// Submit the QTD first, because it does not reference any other USB TDs
	status = iosm_usb2_usbtd_usbmit_and_verify(qtd_content, sizeof(*qtd_content), qtd_slot, eehci_id, intr_pipe->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> Submit qTD error!\n");
		goto submit_tds_fail;
	}
	USB_DEBUG("<libusb2_usbpdev_alloc_intr_pipe> Submit qTD succeed!\n");

	//// Then submit the QH
	status = iosm_usb2_usbtd_usbmit_and_verify(qh_content, sizeof(*qh_content), qh_slot, eehci_id, intr_pipe->usbpdev_slot, 0, 0, 0);
	if(FAIL(status))
	{
		USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> Submit qH error!\n");
		goto submit_tds_fail;
	}
	USB_DEBUG("<libusb2_usbpdev_alloc_intr_pipe> Submit qH succeed!\n");

	// Step 5. Update eEHCI's USB TD registers
	usbtd_reg_slot_t usbtd_reg_qh = usbtd_reg_pick();
	usbtd_reg_slot_t usbtd_reg_qtd = usbtd_reg_pick();
	{
		if((usbtd_reg_qh == USBTD_REG_SLOT_INVALID) || (usbtd_reg_qtd == USBTD_REG_SLOT_INVALID))
		{
			USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> Reserve USBTD reg in eEHCI error!\n");
			usbtd_reg_return(usbtd_reg_qh);
			usbtd_reg_return(usbtd_reg_qtd);
			status = EIO;
			goto reserve_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qh, qh_slot);
		if(FAIL(status))
		{
			USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> Write eEHCI's USBTD Reg %u for QH Error!\n", usbtd_reg_qh);
			goto write_eehci_reg_error;
		}

		status = iosm_usb2_eehci_write_usbtd_reg(usbtd_reg_qtd, qtd_slot);
		if(FAIL(status))
		{
			USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> Write eEHCI's USBTD Reg %u for QTD Error!\n", usbtd_reg_qtd);
			goto write_eehci_reg_error;
		}

		//struct temp_ehci_attach_info temp_attach_info = {.qh_slot = qh_slot, .qtd_slot = qtd_slot};
		//status = vkcall_attach_QH_new(intr_pipe, frameexp);
		status = iosm_usb2_attach_qh_intr(intr_pipe->eptype, intr_pipe->qh_slot, frameexp, true, intr_pipe->qtd_cir_cfg.src_qtd_slot, intr_pipe->qtd_cir_cfg.dst_qtd_slot, &intr_pipe->pehci_usbpipe);
		if(FAIL(status))
		{
			USB_ERR("<libusb2_usbpdev_alloc_intr_pipe> vkcall_attach_QH_new error!\n");
			goto attach_qh_fail;
		}

		intr_pipe->usbtd_reg_qh = usbtd_reg_qh;
		intr_pipe->usbtd_reg_qtds[0] = usbtd_reg_qtd;
		intr_pipe->usbtd_reg_qtds[1] = USBTD_REG_SLOT_INVALID;
		intr_pipe->usbtd_reg_qtds[2] = USBTD_REG_SLOT_INVALID;
		intr_pipe->usbtd_reg_qtds[3] = USBTD_REG_SLOT_INVALID;
	}







	// ZONGWEI: record qh, next_td, tds, data's hw addr
	/*pipe->qh_hw=pipe_hw;
    pipe->next_td = pipe->tds = tds;
	pipe->next_td_hw = pipe->tds_hw = tds_hw;
    pipe->data = data;
	pipe->data_hw = data_hw; 
    pipe->qh.qtd_next = tds_hw & ~EHCI_PTR_BITS;
	pipe->qh.qtd_next_v = (u32)tds;
	pipe->qh.alt_next = EHCI_PTR_TERM; */

	// linking multiple qTDs as a ring buffer
	// FIXME: but why multiple qTDs per frame? 
    /*unsigned int i;
    for (i=0; i<count; i++) {
        struct ehci_qtd *td = &tds[i];
		// ZONGWEI: setup td->qtd_next's virt and hw addr 
        td->qtd_next_v = (i==count-1 ? (u32)tds : (u32)&td[1]);
        td->qtd_next = (sap_mm_vaddr_to_paddr(td->qtd_next_v)) & ~EHCI_PTR_BITS;
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
		// ZONGWEI: setup td->buf's virt and hw addr 
        td->buf[0] = data_hw + maxpacket * i;
		td->buf_v = (u32 *)(pal_mm_alloc(PAL_MEM_ORDINARY, 5*sizeof(u32)));
		*((u32 *)(td->buf_v)) = (u32)data + maxpacket * i;
    } */


	//uint64_t timer1 = 0, timer2 = 0;
	//rdtscll_warm(&timer1, &timer2);

	//timer1 = rdtscll_start();
	//int status = vkcall_attach_QH_Intr(pipe,cntl,frameexp);
	//timer2 = rdtscll_end();
	//USB_ERR("[vkcall_attach_QH_Intr] PERF-Result: %u\n", (timer2-timer1));

	/*if (status) {
		USB_ERR("attach_QH error!\n");
		goto generate_tds_fail;
	} */

	USB_DEBUG("ehci_alloc_intr_pipe succeed!\n");
    return intr_pipe;

attach_qh_fail:
write_eehci_reg_error:
reserve_eehci_reg_error:
submit_tds_fail:
generate_tds_fail:
    /*for (i=0; i<count; i++) {
        struct ehci_qtd *td = &tds[i];
		if (td->buf_v) {
			pal_mm_dealloc(PAL_MEM_ORDINARY, td->buf_v);
			td->buf_v=0;
		}
    }*/
alloc_ds_fail:
	sfree(qh_content, sizeof(struct ehci_new_qh));
	sfree(qtd_content, sizeof(struct ehci_new_qtd));
	sfree(intr_pipe, sizeof(*intr_pipe));
    // sfree(intr_tds, sizeof(*intr_tds) * count);
    usbbuf_dealloc(USBBUF_MEM_DMA, intr_data);
	iosm_usb2_deallocate_usbtd(qtd_slot);
alloc_qtd_fail:
	iosm_usb2_deallocate_usbtd(qh_slot);
alloc_qh_fail:
    return NULL;
}

SAP_STATUS libusb2_poll_intr(struct usb_new_pipe *pipe, void *data, size_t data_sz)
{
	SAP_STATUS status = SUCCESS;
	uint32_t td_status = 0;

	// Sanity checks
	if(pipe->eptype != USB_ENDPOINT_XFER_INT)
		return EINVAL;
	
	if(!data)
		return EINVAL;

	// Read USBTD status
	status = iosm_usb2_query_usbtd_status(pipe->qtd_slots[0], &td_status);
	if(FAIL(status))
	{
		USB_ERR("<pal_ehci_poll_intr> Failed to read USBTD (slot:%u)'s status!\n", pipe->qtd_slots[0]);
		return EINTERNAL;
	}

	// volatile uint64_t c = *(uint64_t*)pipe->p_intr_info.data_buf;
	// USB_ERR("[Superymk] <pal_ehci_poll_intr> td_status:%#X, pipe->qtd_slots[0]:%u, pipe->p_intr_info.data_buf:%llX\n", td_status, pipe->qtd_slots[0], c);


    if (td_status & QTD_STS_ACTIVE)
	{
        // USB TD is not processed
        return E_USBTD_IS_ACTIVE;
	}

    // FIXME: more detailed error handling?
	if (td_status & 0x7f)
	{
		USB_ERR("Polling Interrupt error! td_status:0x%08X\n", td_status);
		return E_USBTD_STATUS_ERROR;
	}

    // Copy data.
    size_t maxpacket = pipe->maxpacket;
	size_t copy_bytes = min(data_sz, maxpacket);
    /*int pos = pipe->next_td - pipe->tds;
    void *tddata = pipe->data + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_LOW, tddata, maxpacket);*/
	pal_mm_memcpy(data, pipe->p_intr_info.data_buf, copy_bytes);

    // Reenable this td.
	/*pipe->next_td = (void *)(orig_td->qtd_next_v & ~EHCI_PTR_BITS);
	pipe->next_td_hw = orig_td->qtd_next;
	orig_td->buf[0]=pipe->data_hw + maxpacket * pos;
	*(orig_td->buf_v)=(u32)tddata;
	mfence(); */
	/* PAL-VK interface */
	status = iosm_usb2_reactivate_qtd(pipe->qtd_slots[0]);
	if(FAIL(status))
	{
		USB_ERR("<pal_ehci_poll_intr> Failed to reactivate QTD (slot:%u)! status:%lu\n", pipe->qtd_slots[0], status);
		return EINTERNAL;
	}

    return SUCCESS;
}

SAP_STATUS pal_ehci_poll_bulk(struct usb_new_pipe *pipe, void *data, size_t data_size)
{
	SAP_STATUS status = SUCCESS;
	uint32_t td_status = 0;

	// Sanity checks
	if(pipe->eptype != USB_ENDPOINT_XFER_BULK)
		return EINVAL;

	if(!data)
		return EINVAL;

	// Read USBTD status
	status = iosm_usb2_query_usbtd_status(pipe->qtd_slots[0], &td_status);
	if(FAIL(status))
	{
		USB_ERR("<pal_ehci_poll_intr> Failed to read USBTD (slot:%u)'s status!\n", pipe->qtd_slots[0]);
		return EINTERNAL;
	}

	// volatile uint64_t c = *(uint64_t*)pipe->p_intr_info.data_buf;
	// USB_ERR("[Superymk] <pal_ehci_poll_intr> td_status:%#X, pipe->qtd_slots[0]:%u, pipe->p_intr_info.data_buf:%llX\n", td_status, pipe->qtd_slots[0], c);


    if (td_status & QTD_STS_ACTIVE)
	{
        // USB TD is not processed
        return E_USBTD_IS_ACTIVE;
	}

    // FIXME: more detailed error handling?
	if (td_status & 0x7f)
	{
		USB_ERR("Polling Interrupt error! td_status:0x%08X\n", td_status);
		return E_USBTD_STATUS_ERROR;
	}

    // Copy data.
    // int maxpacket = pipe->maxpacket;
    /*int pos = pipe->next_td - pipe->tds;
    void *tddata = pipe->data + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_LOW, tddata, maxpacket);*/
	pal_mm_memcpy(data, pipe->p_bulk_info.data_buf, data_size);

    // Reenable this td.
	/*pipe->next_td = (void *)(orig_td->qtd_next_v & ~EHCI_PTR_BITS);
	pipe->next_td_hw = orig_td->qtd_next;
	orig_td->buf[0]=pipe->data_hw + maxpacket * pos;
	*(orig_td->buf_v)=(u32)tddata;
	mfence(); */
	/* PAL-VK interface */
	status = iosm_usb2_reactivate_qtd(pipe->qtd_slots[0]);
	if(FAIL(status))
	{
		USB_ERR("<pal_ehci_poll_intr> Failed to reactivate QTD (slot:%u)! status:%lu\n", pipe->qtd_slots[0], status);
		return EINTERNAL;
	}

    return SUCCESS;
}

struct usb_new_pipe* libusb2_usbpdev_alloc_bulk_pipe(struct sp_usbpdev_s* usbdev, struct usb_endpoint_descriptor* epdesc)
{
	if(!usbdev || !epdesc)
		return NULL;

	return _ehci_alloc_async_pipe(usbdev, epdesc);
}

SAP_STATUS usb2_usbdev_alloc_defpipe(struct sp_usbpdev_s* dev) 
{
	/* reuse the default control pipe if any,
	 * otherwise create a new one */
	u32 maxpacket=0;
	if (!dev)
		return EINVAL;

	if (!dev->defpipe) 
	{
		// If <dev->ddesc> does not exist, the driver should get USB device descriptors right after <usb2_usbdev_alloc_defpipe>
		// to get the <bMaxPacketSize0> field of the device.
		// In Linux, EHCI driver always set <maxpacket = 8> to get USB device descriptors. One possible reason is because
		// OS drivers do not know the correct <bMaxPacketSize0> before running get USB device descriptors, so USB devices
		// must support reporting device descriptors with <maxpacket = 8> in EHCI.

		if (dev->ddesc){
			maxpacket=dev->ddesc->bMaxPacketSize0;
		} else {
			// maxpacket=(dev->speed==USB_LOWSPEED? 8: 64);
			maxpacket = 8;
		}
		USB_DEBUG("<usb2_usbdev_alloc_defpipe> maxpacket=%d!\n", maxpacket);
		struct usb_endpoint_descriptor epdesc = {
			.wMaxPacketSize = maxpacket,
			.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
		};
		dev->defpipe = _ehci_alloc_async_pipe(dev, &epdesc);
		if (!dev->defpipe) {
			warn_internalerror();
			return EINTERNAL;
		}
	}

	// On success
	return SUCCESS;
}

SAP_STATUS usb2_usbdev_free_defpipe(struct sp_usbpdev_s* dev)
{
	if(!dev)
		return EINVAL;

	if(dev->defpipe == NULL) // The default pipe is already free
		return SUCCESS;

	libusb2_usbpdev_free_pipe(dev->defpipe);
	dev->defpipe = NULL;

	return SUCCESS;
}

/// @brief Free a given pipe
SAP_STATUS libusb2_usbpdev_free_pipe(struct usb_new_pipe* pipe)
{
	SAP_STATUS status = SUCCESS;
	//uint32_t qh_slot = pipe->qh_slot;

	if(!pipe)
		return EINVAL;

	status = _usbpdev_unlink_pipe(pipe);
	if(FAIL(status))
		USB_ERR("Free USB Pipe Error: Failed to unlink USB Pipe! status:%lu\n", status);

	// Specific cleaning for interrupt pipes
	if(pipe->eptype == USB_ENDPOINT_XFER_INT)
	{
		// Free data buffer
		void* data_buf = pipe->p_intr_info.data_buf;
		if(data_buf)
		{
			usbbuf_dealloc(USBBUF_MEM_DMA, data_buf);
			pipe->p_intr_info.data_buf = NULL;
		}
	}
	
	// Free <header_content>
	if(pipe->header_content)
	{
		sfree(pipe->header_content, sizeof(struct ehci_new_qh));
		pipe->header_content = NULL;
	}

	return SUCCESS;
}




/********* Private Functions *********/
static struct usb_new_pipe* _ehci_alloc_async_pipe(
	struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
)
{
	//struct pal_utask* this_pal = pal_get_info();
	int status = 0;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	paddr_t usbbuf_base_paddr = INVALID_PADDR;
    struct ehci_new_qh* qh_content = NULL;

    //struct usb_ehci_s* cntl = container_of(
    //    usbdev->hub->cntl, struct usb_ehci_s, usb);
    USB_DEBUG("<_ehci_alloc_async_pipe> ehci_alloc_async_pipe ep type %d\n", eptype);

	// Step 2. Allocate data structures
	// FIXME: disable using previously allocated pipe for now, always
	// use new QH 
	//struct ehci_pipe* pipe = (struct ehci_pipe *)pal_mm_alloc_align(PAL_MEM_DMA, EHCI_QH_ALIGN, sizeof(*pipe));
	struct usb_new_pipe *pipe = (struct usb_new_pipe *)smalloc(sizeof(struct usb_new_pipe));
    if (!pipe) {
        warn_noalloc();
        goto alloc_ds_fail;
    }

    qh_content = (struct ehci_new_qh*)smalloc(sizeof(*qh_content));
	if (!qh_content) 
	{
        warn_noalloc();
        goto alloc_ds_fail;
    }

	// Step 3. Generate QH contents
	//// Generate QH contents
	uint32_t qh_04h = 0;
	uint32_t qh_08h = 0;
	status = pal_ehci_desc2qh(pipe, usbdev, epdesc, &qh_04h, &qh_08h);
    if (status == -1)
	{
		warn_internalerror();
		goto generate_qh_fail;
	}

	qh_content->info1 = qh_04h;
	qh_content->info2 = qh_08h;
	qh_content->next.v = EHCI_PTR_TERM;
	qh_content->qtd_next.v = EHCI_PTR_TERM;
	qh_content->alt_next.v = EHCI_PTR_TERM;

	// Even though this QH does not use any data buffers, the QH needs to set all data buffers point to the SecApp's
	// DMA memory in order to pass USB TD checks
	usbbuf_base_paddr = usbbuf_get_base_paddr();
	qh_content->buf[4].buf_pfn = qh_content->buf[3].buf_pfn = qh_content->buf[2].buf_pfn = 
		qh_content->buf[1].buf_pfn = qh_content->buf[0].buf_pfn = PAGE4K_PADDR_TO_PFN(usbbuf_base_paddr);

	// Expose the <qh_content> in the <pipe>. So later functions can reuse the content of <qh_content>
	pipe->header_content = qh_content;
	pipe->qtd_cir_cfg.src_qtd_slot = USBTD_SlotID_INVALID;
	pipe->qtd_cir_cfg.dst_qtd_slot = USBTD_SlotID_INVALID;

	pipe->usbpdev_slot = usbdev->usbpdev_slot;

	return pipe;

generate_qh_fail:	
alloc_ds_fail:
	if(qh_content)
		sfree(qh_content, sizeof(struct ehci_new_qh));
	if(pipe)
		sfree(pipe, sizeof(struct usb_new_pipe));
	return NULL;
}





/********* Debug Functions *********/
void pal_usb_dump_usbtd(vaddr_t vaddr)
{
	uint32_t* usbtd_mem = (uint32_t*)vaddr;
	USB_INFO("<pal_usb_dump_usbtd> Dump USBTD Content at vaddr:0x%p\n", usbtd_mem);
	USB_INFO("		content[00]:0x%08X \t content[01]:0x%08X \t content[02]:0x%08X \n",
		usbtd_mem[0], usbtd_mem[1], usbtd_mem[2]);
	USB_INFO("		content[03]:0x%08X \t content[04]:0x%08X \t content[05]:0x%08X \n",
		usbtd_mem[3], usbtd_mem[4], usbtd_mem[5]);
	USB_INFO("		content[06]:0x%08X \t content[07]:0x%08X \t content[08]:0x%08X \n",
		usbtd_mem[6], usbtd_mem[7], usbtd_mem[8]);
	USB_INFO("		content[09]:0x%08X \t content[10]:0x%08X \t content[11]:0x%08X \n",
		usbtd_mem[9], usbtd_mem[10], usbtd_mem[11]);
}