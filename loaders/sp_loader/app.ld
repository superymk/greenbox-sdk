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

SECTIONS
{  
    /* PAL Code*/
    .PAL_SCODE ALIGN(0x1000):
    {
        PROVIDE(__pal_scode_start = .);
        *(.pal_scode) /* backwards compat */
        *(.PAL_SCODE)  /* backwards compat */
        . = ALIGN(0x1000);
    } =0x90909090 /* fill padding space with no-ops */
    PROVIDE(__pal_scode_end = .);
      
    /* PAL Data*/
    .PAL_SDATA ALIGN(0x1000):
    {
        PROVIDE(__pal_sdata_start = .);
        *(.pal_sdata) /* backwards compat */
        *(.PAL_SDATA) /* backwards compat */
         . = ALIGN(0x1000); 
    } =0x00000000 /* fill padding space with zeros */
    PROVIDE(__pal_sdata_end = .);
    
    
    .PAL_RODATA ALIGN(0x1000):
    {
        PROVIDE(__pal_rodata_start = .);
        *(.PAL_RODATA) 
        . = ALIGN(0x1000); 
    } =0x00000000 
    PROVIDE(__pal_rodata_end = .);
      
    /* Debug Information */
    .stab : 
    {
        . = ALIGN(0x1000);
        __STAB_BEGIN__ = .;
        *(.stab);
        __STAB_END__ = .;
        . = ALIGN(0x1000);
    }

    .stabstr : 
    {
        . = ALIGN(0x1000);
        __STABSTR_BEGIN__ = .;
        *(.stabstr);
        __STABSTR_END__ = .;
        . = ALIGN(0x1000);
    }
		
}
INSERT AFTER .text;