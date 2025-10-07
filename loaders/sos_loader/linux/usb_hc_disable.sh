#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# this script disables the XHCI controlled by linux kernel in Ubuntu 14.04 and later

echo -n "$1" | sudo tee /sys/bus/pci/drivers/xhci_hcd/unbind