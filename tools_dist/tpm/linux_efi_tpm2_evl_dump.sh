#!/bin/bash

# /*
#  * -------------------------------------------------------------------------------------
#  * Carnegie Mellon University
#  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
#  * 
#  * This software is the proprietary information of Carnegie Mellon University.
#  * Use is subject to license terms.
#  * 
#  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
#  * written consent of Carnegie Mellon University. is strictly prohibited.
#  * -------------------------------------------------------------------------------------
#  * 
#  * Authors: Miao Yu
#  *          Virgil Gligor
#  */

# Dump entire EFI TPM2.X event logs.
# Usage: linux_efi_tpm2_evl_dump.sh <filepath of result event logs>

sudo cat /sys/kernel/security/tpm0/binary_bios_measurements > $1
sudo chmod 666 $1