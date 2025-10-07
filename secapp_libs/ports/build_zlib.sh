#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

cd zlib

rm -rf build
mkdir build

cd build


CFLAGS=${CFLAGS} prefix=${PREFIX} ../configure --static



#make
make -j$(nproc)

make install