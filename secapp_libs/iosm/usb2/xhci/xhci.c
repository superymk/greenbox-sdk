// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* $OpenBSD: xhci.c,v 1.130 2023/07/20 09:43:00 claudio Exp $ */

/*
 * Copyright (c) 2014-2015 Martin Pieuchot
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <palddk.h>
#include <iosm_usb3_sp_intfs.h>
#include "../lib-iosm-usb-internal.h"

#include "xhci.h"
#include "../util.h"
#include "iosm_usb3_calls.h"

static uint32_t _trb_idx_next = 0;

/********* Private Functions *********/
/// @brief Generate TRB id in a round-robin manner.
/// [NOTE] Round-robin the uint32_t ID space is ok because IOSM-USB3 caches a small number of TRBs only.
static inline uint32_t _trb_id_generate(void)
{
    return _trb_idx_next++;
}

/*
 * Number of packets remaining in the TD after the corresponding TRB.
 *
 * Section 4.11.2.4 of xHCI specification r1.1.
 */
static inline uint32_t
xhci_xfer_tdsize(u16 maxpacket, uint32_t remain, uint32_t len)
{
    uint32_t npkt, mps = maxpacket;

    if (len == 0)
        return XHCI_TRB_TDREM(0);

    npkt = howmany(remain - len, UE_GET_SIZE(mps));
    if (npkt > 31)
        npkt = 31;

    return XHCI_TRB_TDREM(npkt);
}

static int _xhci_wait_trb(struct usb3_pipe *p, uint32_t trb_id, int timeout_us, bool enable_debug)
{
    SAP_STATUS status = SUCCESS;
    uint32_t trb_cc = 0;
    int wait_num = timeout_us / CPU_YIELD_USEC; // Num of CPU yields

    // Check: Parameters must be valid
    if(wait_num <= 0)
    {
        // Already timed out
        return -1;
    }

    // Query status
    for (;;)
    {
        status = iosm_usb3_query_trb_status(p->devaddr, p->ep, trb_id, &trb_cc, enable_debug);
        if (FAIL(status))
        {
            USB_ERR("<_xhci_wait_trb> Failed to read status of TRB (ID:%u)'s status!\n", trb_id);
            return -2;
        }

        if(trb_cc != XHCI_CODE_INVALID)
            break;

        if (wait_num <= 0)
        {
            // Timeout
            USB_ERR("<_xhci_wait_trb> TRB (ID:%u) time out!\n", trb_id);
            // if(enable_debug)
            {
                // Print XHCI registers. No need to handle its return value.
                (void)iosm_usb3_debug_print_xhci();
            }
            return -1;
        }

        usleep(CPU_YIELD_USEC);
        wait_num--;
    }

    if (trb_cc != XHCI_CODE_SUCCESS)
    {
        USB_ERR("<_xhci_wait_trb> TRB (ID:%u) error - status=%u\n", trb_id, trb_cc);
        return -2;
    }

    if(enable_debug)
        USB_INFO("<_xhci_wait_trb> TRB (ID:%u) status=%u\n", trb_id, trb_cc);
    else
        USB_DEBUG("<_xhci_wait_trb> TRB (ID:%u) status=%u\n", trb_id, trb_cc);

    // On success
    return 0;
}

/********* Public Functions *********/
SAP_STATUS usb3_usbdev_alloc_defpipe(struct sp_usbpdev_s *dev)
{
    struct usb3_pipe *pipe = NULL;

    // Check: Parameters must be valid
    if (!dev)
        return EINVAL;

    pipe = smalloc(sizeof(struct usb3_pipe));
    if (!pipe)
    {
        return ENOMEM;
    }

    pipe->ep = 0; // Default control endpoint
    memcpy(&pipe->devaddr, &dev->devaddr, sizeof(USBPDEV_ADDR));
    // pipe->devaddr = dev->devaddr;

    if (dev->ddesc)
        pipe->maxpacket = dev->ddesc->bMaxPacketSize;
    else
        pipe->maxpacket = 8;

    dev->defpipe = pipe;

    // Do nothing on the IOSM-USB3. IOSM-USB3 always opens pipes to all endpoints of USB devices.

    // On success
    return SUCCESS;
}

SAP_STATUS usb3_usbdev_free_defpipe(struct sp_usbpdev_s *dev)
{
    if (dev->defpipe)
    {
        sfree(dev->defpipe, sizeof(struct usb3_pipe));
        dev->defpipe = NULL;
    }

    // Do nothing on the IOSM-USB3. IOSM-USB3 always opens pipes to all endpoints of USB devices.
    return SUCCESS;
}

#define XHCI_CONTROL_TRBS_MAX_NUM (3)

SAP_STATUS xhci_send_control(
    struct usb3_pipe *p,
    int dir,
    const void *cmd,
    size_t cmdsize,
    void *data,
    size_t datasize,
    u16 maxpacket,
    uint32_t td_timeout_ms,
    bool enable_debug)
{
    SAP_STATUS status = SUCCESS;
    struct sp_trb trbs[XHCI_CONTROL_TRBS_MAX_NUM];
    struct sp_trb *trb = NULL;
    uint32_t trb_idx = 0;
    uint32_t flags = 0;
    paddr_t paddr = INVALID_PADDR;
    uint32_t trb_num = 0;
    bool wait_td_success = true;
    struct usb_device_request *setup_cmd = (struct usb_device_request *)cmd;

    // Check: Parameters must be valid
    if (!p || !cmd)
        return EINVAL;

    if (cmdsize > XHCI_TRB_TRANSFER_LENGTH_MAX || datasize > XHCI_TRB_TRANSFER_LENGTH_MAX)
        return EINVAL;

    memset(trbs, 0, sizeof(struct sp_trb) * XHCI_CONTROL_TRBS_MAX_NUM);

    // Setup TRB
    trb = &trbs[trb_idx++];
    flags = XHCI_TRB_TYPE_SETUP | XHCI_TRB_IDT;
    if (datasize != 0)
    {
        if (usb_xfer_isread(dir))
            flags |= XHCI_TRB_TRT_IN;
        else
            flags |= XHCI_TRB_TRT_OUT;
    }
    else
        flags |= XHCI_TRB_TRT_NO_DATA;

    trb->id = _trb_id_generate();
    memcpy(&trb->trb.ptr_low, setup_cmd, sizeof(trb->trb.ptr_low) + sizeof(trb->trb.ptr_high));
    trb->trb.status = htole32(XHCI_TRB_INTR(0) | XHCI_TRB_LEN(8));
    trb->trb.control = htole32(flags);

    // Data TRB
    if (datasize != 0)
    {
        trb = &trbs[trb_idx++];
        flags = XHCI_TRB_TYPE_DATA;

        if (usb_xfer_isread(dir))
            flags |= XHCI_TRB_DIR_IN | XHCI_TRB_ISP;

        paddr = sap_mm_vaddr_to_paddr((vaddr_t)data);
        if (paddr == INVALID_PADDR)
            return EINVAL;

        trb->id = _trb_id_generate();
        trb->trb.ptr_low = LOW32(paddr);
        trb->trb.ptr_high = HIGH32(paddr);
        trb->trb.status = htole32(
            XHCI_TRB_INTR(0) | XHCI_TRB_LEN(datasize) |
            xhci_xfer_tdsize(maxpacket, datasize, datasize));
        trb->trb.control = htole32(flags);
    }

    // Status TRB
    trb = &trbs[trb_idx++];
    flags = XHCI_TRB_TYPE_STATUS | XHCI_TRB_IOC;
    if (datasize == 0 || !usb_xfer_isread(dir))
        flags |= XHCI_TRB_DIR_IN;

    trb->id = _trb_id_generate();
    trb->trb.ptr_low = 0;
    trb->trb.ptr_high = 0;
    trb->trb.status = htole32(XHCI_TRB_INTR(0));
    trb->trb.control = htole32(flags);

    // Submit TRBs
    trb_num = trb_idx;
    status = iosm_usb3_submit_trbs(p, trbs, trb_num, 0, enable_debug);
    if (FAIL(status))
    {
        USB_ERR("<xhci_send_control> Submit TRBs error in pipe for USBPDev (dev addr:%u, hub port:%u, hub addr:%u, bus id:%u)! status:%lu\n", 
            p->devaddr.dev_usbaddr, p->devaddr.hub_port, p->devaddr.hub_usbaddr, p->devaddr.bus_id,
            status);
        return EINTERNAL;
    }
    
    if(enable_debug)
        USB_INFO("<xhci_send_control> Submit TRBs succeed!\n");
    else
        USB_DEBUG("<xhci_send_control> Submit TRBs succeed!\n");

    // Wait for the Status TRB completion
    {
        int ret = 0;
        uint32_t status_trb_idx = trb_num - 1;
        ret = _xhci_wait_trb(p, trbs[status_trb_idx].id, td_timeout_ms * 1000, enable_debug);

        if (!ret)
        {
            wait_td_success = true;
            if(enable_debug)
                USB_INFO("TRB%u is processed!\n", status_trb_idx);
            else
                USB_DEBUG("TRB%u is processed!\n", status_trb_idx);
        }
        else
        {
            wait_td_success = false;
            USB_ERR("TRB%u encounters error! ret:%d\n", status_trb_idx, ret);
        }
    }

    // On success
    if (!wait_td_success)
        return EIO;
    else
        return SUCCESS;
}

#define XHCI_GENERIC_TRBS_MAX_NUM (TRBS_MAX_NUM)

SAP_STATUS xhci_device_generic_start(struct usb3_pipe *p, void *dmabuf, uint32_t xfer_length, uint16_t xfer_flags)
{
    SAP_STATUS status = SUCCESS;
    struct sp_trb trbs[XHCI_GENERIC_TRBS_MAX_NUM];
    struct sp_trb *trb = NULL, *trb0 = NULL;
    uint32_t len, remain, flags;
    uint32_t mps = 0;
    paddr_t paddr = INVALID_PADDR;
    int i, ntrb, zerotd = 0;
    int dir = 0;
    uint32_t trb_idx = 0;
    uint32_t trb_num = 0;

    // Check: Parameters must be valid
    if (!p || !dmabuf)
        return EINVAL;

    if (p->ep == 0)
    {
        // This function is for data endpoints. Communication with default control endpoints should use <xhci_send_control>
        return EINVAL;
    }

    mps = p->maxpacket;
    dir = p->ep;
    paddr = sap_mm_vaddr_to_paddr((vaddr_t)dmabuf);
    if (paddr == INVALID_PADDR)
        return EINVAL;

    /* How many TRBs do we need for this transfer? */
    ntrb = howmany(xfer_length, XHCI_TRB_MAXSIZE);

    /* If the buffer crosses a 64k boundary, we need one more. */
    len = XHCI_TRB_MAXSIZE - (paddr & (XHCI_TRB_MAXSIZE - 1));
    if (len < xfer_length)
        ntrb = howmany(xfer_length - len, XHCI_TRB_MAXSIZE) + 1;
    else
        len = xfer_length;

    /* If we need to append a zero length packet, we need one more. */
    if ((xfer_flags & USBD_FORCE_SHORT_XFER || xfer_length == 0) &&
        (xfer_length % UE_GET_SIZE(mps) == 0))
        zerotd = 1;

    // if (xp->free_trbs < (ntrb + zerotd))
    // 	return (ENOMEM);
    if (XHCI_GENERIC_TRBS_MAX_NUM <= (ntrb + zerotd))
        return (ENOMEM);

    /* We'll toggle the first TRB once we're finished with the chain. */
    // trb0 = xhci_xfer_get_trb(sc, xfer, &toggle, (ntrb == 1));
    trb0 = &trbs[trb_idx++];
    flags = XHCI_TRB_TYPE_NORMAL; //| (toggle ^ 1);
    if (usb_xfer_isread(dir))
        flags |= XHCI_TRB_ISP;
    flags |= (ntrb == 1) ? XHCI_TRB_IOC : XHCI_TRB_CHAIN;

    // trb0->trb_paddr = htole64(DMAADDR(&xfer->dmabuf, 0));
    trb0->id = _trb_id_generate();
    trb0->trb.ptr_low = LOW32(paddr);
    trb0->trb.ptr_high = HIGH32(paddr);

    trb0->trb.status = htole32(
        XHCI_TRB_INTR(0) | XHCI_TRB_LEN(len) |
        xhci_xfer_tdsize(mps, xfer_length, len));
    trb0->trb.control = htole32(flags);
    if(flags & XHCI_TRB_IOC)
        p->last_ioc_trb_id = trb0->id;
    // bus_dmamap_sync(xp->ring.dma.tag, xp->ring.dma.map,
    //     TRBOFF(&xp->ring, trb0), sizeof(struct xhci_trb),
    //     BUS_DMASYNC_PREWRITE);

    remain = xfer_length - len;
    paddr += len;

    /* Chain more TRBs if needed. */
    for (i = ntrb - 1; i > 0; i--)
    {
        len = min(remain, (uint32_t)XHCI_TRB_MAXSIZE);

        /* Next (or Last) TRB. */
        // trb = xhci_xfer_get_trb(sc, xfer, &toggle, (i == 1));
        trb = &trbs[trb_idx++];
        flags = XHCI_TRB_TYPE_NORMAL; // | toggle;
        if (usb_xfer_isread(dir))
            flags |= XHCI_TRB_ISP;
        flags |= (i == 1) ? XHCI_TRB_IOC : XHCI_TRB_CHAIN;

        // trb->trb_paddr = htole64(paddr);
        trb->id = _trb_id_generate();
        trb->trb.ptr_low = LOW32(paddr);
        trb->trb.ptr_high = HIGH32(paddr);
        trb->trb.status = htole32(
            XHCI_TRB_INTR(0) | XHCI_TRB_LEN(len) |
            xhci_xfer_tdsize(mps, remain, len));
        trb->trb.control = htole32(flags);
        if(flags & XHCI_TRB_IOC)
            p->last_ioc_trb_id = trb->id;

        // bus_dmamap_sync(xp->ring.dma.tag, xp->ring.dma.map,
        //     TRBOFF(&xp->ring, trb), sizeof(struct xhci_trb),
        //     BUS_DMASYNC_PREWRITE);

        remain -= len;
        paddr += len;
    }

    /* Do we need to issue a zero length transfer? */
    if (zerotd == 1)
    {
        // trb = xhci_xfer_get_trb(sc, xfer, &toggle, -1);
        // trb->trb_paddr = 0;
        trb = &trbs[trb_idx++];
        trb->id = _trb_id_generate();
        trb->trb.ptr_low = 0;
        trb->trb.ptr_high = 0;
        trb->trb.status = 0;
        trb->trb.control = htole32(XHCI_TRB_TYPE_NORMAL | XHCI_TRB_IOC); // | toggle);

        // bus_dmamap_sync(xp->ring.dma.tag, xp->ring.dma.map,
        //     TRBOFF(&xp->ring, trb), sizeof(struct xhci_trb),
        //     BUS_DMASYNC_PREWRITE);

        if(flags & XHCI_TRB_IOC)
            p->last_ioc_trb_id = trb->id;
    }

    /* First TRB. */
    // trb0->trb_flags ^= htole32(XHCI_TRB_CYCLE);
    // bus_dmamap_sync(xp->ring.dma.tag, xp->ring.dma.map,
    //     TRBOFF(&xp->ring, trb0), sizeof(struct xhci_trb),
    //     BUS_DMASYNC_PREWRITE);

    // Submit TRBs
    trb_num = trb_idx;
    status = iosm_usb3_submit_trbs(p, trbs, trb_num, SP_TRBS_CHAINED, false);
    if (FAIL(status))
    {
        USB_ERR("<xhci_device_generic_start> Submit TRBs error! status:%lu\n", status);
        return EINTERNAL;
    }
    USB_DEBUG("<xhci_device_generic_start> Submit TRBs succeed!\n");

    // On success
    return SUCCESS;
}

struct usb3_pipe *libusb3_usbpdev_alloc_intr_pipe(
    struct sp_usbpdev_s *usbdev,
    struct usb_endpoint_descriptor *epdesc)
{
    SAP_STATUS status = SUCCESS;
    struct usb3_pipe *pipe = NULL;
    unsigned int count = 1;

    // Sanity checks
    if (!usbdev || !epdesc)
        return NULL;

    pipe = smalloc(sizeof(struct usb3_pipe));
    if (!pipe)
    {
        return NULL;
    }

    pipe->ep = epdesc->bEndpointAddress;
    memcpy(&pipe->devaddr, &usbdev->devaddr, sizeof(USBPDEV_ADDR));
    pipe->maxpacket = UGETW(epdesc->wMaxPacketSize);

    pipe->data_buf_size = ROUND_UP_TO_NEXT_POWER_OF_2_uint32(pipe->maxpacket * count);
    pipe->data_buf = usbbuf_alloc(USBBUF_MEM_DMA, pipe->data_buf_size);

    if (!pipe->data_buf)
        goto data_buf_alloc_err;

    // Submit TRBs
    status = xhci_device_generic_start(pipe, pipe->data_buf, pipe->data_buf_size, 0);
    if (FAIL(status))
    {
        USB_ERR("<libusb3_usbpdev_alloc_intr_pipe> Submit TRBs error! status:%lu\n", status);
        goto submit_trbs_err;
    }

    // On success
    return pipe;

submit_trbs_err:
    if (pipe->data_buf)
        usbbuf_dealloc(USBBUF_MEM_DMA, pipe->data_buf);
data_buf_alloc_err:
    if (pipe)
        sfree(pipe, sizeof(struct usb3_pipe));
    return NULL;
}

SAP_STATUS libusb3_poll_intr(struct usb3_pipe *pipe, void *data, size_t data_sz)
{
    SAP_STATUS status = SUCCESS;
    uint32_t trb_cc = 0;

    // Check: Parameters must be valid
    if(!pipe || !data)
		return EINVAL;

    if(!pipe->data_buf || !pipe->data_buf_size)
        return EINVAL;

    // Query interrupt pipe status
    status = iosm_usb3_query_trb_status(pipe->devaddr, pipe->ep, pipe->last_ioc_trb_id, &trb_cc, false);
    if(FAIL(status))
    {
        USB_ERR("<libusb3_poll_intr> Failed to read USBTD (ID:%u)'s status! status:%lu\n", pipe->last_ioc_trb_id, status);
		return EINTERNAL;
    }

    // FIXME: more detailed error handling?
    if(trb_cc == XHCI_CODE_INVALID)
    {
        // USB TD is not processed
        return E_USBTD_IS_ACTIVE;
    }
    else if(trb_cc == XHCI_CODE_SHORT_XFER)
    {
        // Do nothing. Short transfers are fine.
    }
    else if(trb_cc != XHCI_CODE_SUCCESS)
    {
        USB_ERR("Polling Interrupt error! trb_cc:0x%08X\n", trb_cc);
		return E_USBTD_STATUS_ERROR;
    }

    // Copy data.
    size_t maxpacket = pipe->maxpacket;
    size_t copy_bytes = min(data_sz, maxpacket);
    pal_mm_memcpy(data, pipe->data_buf, copy_bytes);

    // Reactivate the interrupt pipe
    status = xhci_device_generic_start(pipe, pipe->data_buf, pipe->data_buf_size, 0);
    if (FAIL(status))
    {
        USB_ERR("<libusb3_poll_intr> Failed to reactivate the interrupt pipe (0x%lX)! status:%lu\n", (vaddr_t)pipe, status);
        return EINTERNAL;
    }

    return SUCCESS;
}

SAP_STATUS libusb3_usbpdev_free_pipe(struct usb3_pipe *pipe)
{
    if (pipe)
    {
        if(pipe->data_buf)
        {
            usbbuf_dealloc(USBBUF_MEM_DMA, pipe->data_buf);
            pipe->data_buf = NULL;
        }

        sfree(pipe, sizeof(struct usb3_pipe));
    }

    return SUCCESS;
}