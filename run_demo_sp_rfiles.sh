#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

cd ./output/dist/sap

#sudo ./sl_rfiles
sudo bash -lc 'ulimit -c unlimited; ./sl_rfiles'
sleep 1s