#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

cd liboauth2

rm -rf build
mkdir build

cd build
  
CFLAGS=${CFLAGS} LDFLAGS=${LDFLAGS} PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig:${PREFIX}/lib64/pkgconfig" ../configure --prefix=${PREFIX} \
--host=amd64-linux \
--enable-static \
--with-apache=no

#make
make -j$(nproc)

make install