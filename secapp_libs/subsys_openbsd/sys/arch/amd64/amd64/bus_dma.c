// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*	$OpenBSD: bus_dma.c,v 1.51 2019/06/09 12:52:04 kettenis Exp $	*/
/*	$NetBSD: bus_dma.c,v 1.3 2003/05/07 21:33:58 fvdl Exp $	*/

/*-
 * Copyright (c) 1996, 1997, 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum and by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * The following is included because _bus_dma_uiomove is derived from
 * uiomove() in kern_subr.c.
 */

/*
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sddk.h>

// #define SKIP_OPENBSD_RENAME
#include <subsys_openbsd.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
// #include <sys/mbuf.h>
// #include <sys/proc.h>

#include <machine/bus.h>

// #include <subsys_openbsd.h>

/*
 * Common function for DMA map creation.  May be called by bus-specific
 * DMA map creation functions.
 */
int
bus_dmamap_create(bus_dma_tag_t t, bus_size_t size, int nsegments,
    bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *dmamp)
{
    // [GreenBox-Specific] Do nothing because unnecessary for GreenBox APIs.
    return 0;
}

/*
 * Like _bus_dmamap_load(), but for raw memory allocated with
 * bus_dmamem_alloc().
 */
int
bus_dmamap_load_raw(bus_dma_tag_t t, bus_dmamap_t map, bus_dma_segment_t *segs,
    int nsegs, bus_size_t size, int flags)
{
    // [GreenBox-Specific] Do nothing because unnecessary for GreenBox APIs.
    return 0;
}

/*
 * Common function for DMA map synchronization.  May be called
 * by bus-specific DMA map synchronization functions.
 */
void
bus_dmamap_sync(bus_dma_tag_t t, bus_dmamap_t map, bus_addr_t addr,
    bus_size_t size, int op)
{
	/* Nothing to do here. */
}

/*
 * Common function for mapping DMA-safe memory.  May be called by
 * bus-specific DMA memory map functions.
 */
int
bus_dmamem_map(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs,
    size_t size, caddr_t *kvap, int flags)
{
    // [TODO] Check: We only support nsegs == 1
    if(nsegs != 1)
        return -1;

    /* Always round the size. */
	size = round_page(size);

    // Check: <size> must match the record
    if(size != segs[0].ds_len)
        return -1;

    *kvap = (caddr_t)segs[0]._ds_vaddr;

    return 0;
}

/*
 * Allocate physical memory from the given physical address range.
 * Called by DMA-safe memory allocation methods.
 */
int
_bus_dmamem_alloc_range(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
    bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
    int flags, bus_addr_t low, bus_addr_t high)
{
    paddr_t curaddr, lastaddr;
    int curseg;
    void* buf = NULL;

    // [TODO] Check: We only support nsegs == 1
    if(nsegs != 1)
        return -1;
    (void)flags;(void)low;(void)high;

    /* Always round the size. */
	size = round_page(size);

    segs[0]._ds_boundary = boundary;
	segs[0]._ds_align = alignment;

    buf = smalloc_align(MEM_DMA, alignment, size);
    if(!buf)
        return -1;

    /*
	 * Compute the location, size, and number of segments actually
	 * returned by the VM code.
	 */

    curseg = 0;
	lastaddr = segs[curseg].ds_addr = mm_vaddr_to_paddr((vaddr_t)buf);
	segs[curseg].ds_len = PAGE_SIZE_4K;

    

    // [GreenBox-Specific] <smalloc_align> returns virtual addresses of DMA buffers. So <bus_dmamem_map> only needs to
    // read out the mapping.
    segs[curseg]._ds_paddr = segs[curseg].ds_addr;
    segs[curseg]._ds_vaddr = (vaddr_t)buf;
    // [GreenBox-Specific] End

    (void)curaddr; (void)lastaddr;
    *rsegs = curseg + 1;
	return (0);
}

/*
 * Common function for DMA-safe memory allocation.  May be called
 * by bus-specific DMA memory allocation functions.
 */
int
bus_dmamem_alloc(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
    bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
    int flags)
{
    /*
	 * XXX in the presence of decent (working) iommus and bouncebuffers
	 * we can then fallback this allocation to a range of { 0, -1 }.
	 * However for now  we err on the side of caution and allocate dma
	 * memory under the 4gig boundary.
	 */
	return (_bus_dmamem_alloc_range(t, size, alignment, boundary,
	    segs, nsegs, rsegs, flags, (bus_addr_t)0, (bus_addr_t)0xffffffff));
}


/*
 * Common function for unmapping DMA-safe memory.  May be called by
 * bus-specific DMA memory unmapping functions.
 */
void
bus_dmamem_unmap(bus_dma_tag_t t, caddr_t kva, size_t size)
{
    // [GreenBox-Specific] Do nothing because unnecessary for GreenBox APIs.
    return;
}

/*
 * Common function for freeing DMA-safe memory.  May be called by
 * bus-specific DMA memory free functions.
 */
void
bus_dmamem_free(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs)
{
    // [TODO] Check: We only support nsegs == 1
    if(nsegs != 1)
        return;

    sfree(MEM_DMA, (void*)segs[0]._ds_vaddr, (size_t)segs[0].ds_len);

    // [GreenBox-Specific] dangling pointers in OpenBSD?
    segs[0].ds_addr = 0;
    segs[0].ds_len = 0;
    segs[0]._ds_boundary = 0;
    segs[0]._ds_align = 0;
    segs[0]._ds_paddr = 0;
    segs[0]._ds_vaddr = 0;
    // [GreenBox-Specific] End
}

/*
 * Common function for DMA map destruction.  May be called by bus-specific
 * DMA map destruction functions.
 */
void
bus_dmamap_destroy(bus_dma_tag_t t, bus_dmamap_t map)
{
    // [GreenBox-Specific] Do nothing because unnecessary for GreenBox APIs.
    return;
}

/*
 * Common function for unloading a DMA map.  May be called by
 * bus-specific DMA map unload functions.
 */
void
bus_dmamap_unload(bus_dma_tag_t t, bus_dmamap_t map)
{
	/*
	 * No resources to free; just mark the mappings as
	 * invalid.
	 */
	map->dm_mapsize = 0;
	map->dm_nsegs = 0;
}