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

# sudo ./gb_secbase_tpm_seal.sh

#  Invoke SecBase to seal its secrets and save the output file alongside UXMHF64 files (Scan and use the first UXMHF64
#  location only).
cd ../tools/tpmtools
sudo ./gb_tpm --secbase-seal