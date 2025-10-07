#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# sudo ./gb_secbase_tpm_pcrs.sh <PCR1> <PCR2>

# Get TPM PCRs from the physical TPM device
cd ../tools/tpmtools
sudo ./gb_tpm -pf $1 $2