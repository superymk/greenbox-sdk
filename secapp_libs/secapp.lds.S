// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* gcc -m32 will define i386, but we do not want it */
#ifdef i386
#undef i386
#endif /* i386 */

#ifdef __i386__
    OUTPUT_ARCH(i386)
#elif defined(__amd64__)
    OUTPUT_ARCH(i386:x86-64)
#else /* !defined(__i386__) && !defined(__amd64__) */
    #error "Unsupported Arch"
#endif /* !defined(__i386__) && !defined(__amd64__) */

ENTRY(sp_entry)

SECTIONS
{  
    /* Link the kernel at this address: "." means the current address */
    /* Fixed SecApp start */
    . = 0x08000000;
    
    /* SecApp Code*/
    .text :
    {
        // [NOTE] The entry address of SecProcesses must be the start of the SecProcess text section.
        *(.text.entry);
        *(.text .stub .text.* .gnu.linkonce.t.*);
        . = ALIGN(0x1000);  /* Page align */
    } =0x90909090 /* fill padding space with no-ops */
    
    /* SecApp Data*/
    .data : 
    {
        *(.data .data.*)
	    *(.dynbss .bss .bss.* .gnu.linkonce.b.*)
        *(.got .got.*)
        *(COMMON)
        *(COM)
        . = ALIGN(0x1000);  /* Page align */
    } =0x00000000 /* fill padding space with zeros */
    
    .rodata ALIGN(0x1000): 
    { 
        *(.version_rodata);
        *(.rodata .rodata.* .gnu.linkonce.r.*);
        . = ALIGN(0x1000); 
    }
    
    /* Include debugging information in kernel memory */
    .stab : 
    {
        . = ALIGN(0x1000);
        *(.stab);
        . = ALIGN(0x1000);
    }

    .stabstr : 
    {
        . = ALIGN(0x1000);
        *(.stabstr);
        . = ALIGN(0x1000);
    }
    
    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack)
    }
}