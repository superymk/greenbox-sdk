#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# Dump entire EFI TPM2.X event logs.
# Usage: linux_efi_tpm2_evl_dump.sh <filepath of result event logs>

sudo cat /sys/kernel/security/tpm0/binary_bios_measurements > $1
sudo chmod 666 $1