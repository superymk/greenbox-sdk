#!/bin/bash
set -e

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# Usage:
# sudo ./gb_secbase_tpm_quote.sh <PCR1> <PCR2> <nonce-base64>

# Check for correct number of arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: sudo $0 <PCR1> <PCR2> <nonce-base64>"
    exit 1
fi

# # Optional: Echo parameters for debugging
# echo "PCR1: $1"
# echo "PCR2: $2"
# echo "Nonce (base64): $3"

# Change to the TPM tools directory
cd ../tools/tpmtools || { echo "Failed to change directory"; exit 1; }

# Execute the TPM quote command with quoted parameters
sudo ./gb_tpm -qnb64 "$1" "$2" "$3"