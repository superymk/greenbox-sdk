#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# sudo ./gb_secbase_tpm_seal.sh

#  Invoke SecBase to seal its secrets and save the output file alongside UXMHF64 files (Scan and use the first UXMHF64
#  location only).
cd ../tools/tpmtools
sudo ./gb_tpm --secbase-seal