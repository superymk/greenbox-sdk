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

# Convert the format of the AIK file in <userdata absolute path of tpmtools> from PEM to DER. 
# Usage: linux_tpm2_aik_convert.sh <userdata absolute path of tpmtools>

# Check if the directory exists. If not, create it.
if [ ! -d "$1" ]; then
  mkdir -p "$1"
fi

# Switch directory to <userdata absolute path of tpmtools>.
pushd $1 > /dev/null

sudo openssl rsa -pubin -in aik.pem -outform DER -out aik.der
sudo rm aik.pem
sudo chmod 666 aik.der

# Switch back to the current directory of the script file.
popd > /dev/null