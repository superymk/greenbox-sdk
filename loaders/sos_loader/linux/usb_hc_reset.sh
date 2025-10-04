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

# this script resets the XHCI controlled by linux kernel in Ubuntu 14.04 and later

echo -n "$1" | sudo tee /sys/bus/pci/drivers/xhci_hcd/unbind
echo -n "$1" | sudo tee /sys/bus/pci/drivers/xhci_hcd/bind