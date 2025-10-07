#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

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