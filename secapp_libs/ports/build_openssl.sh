#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

cd openssl

rm -rf build
mkdir build

cd build


CFLAGS=${CFLAGS} ../config --prefix=${PREFIX} \
no-asm no-hw no-dso no-md2 no-rc5 no-rfc3779 no-dgram no-shared no-pic no-threads no-egd zlib no-zlib-dynamic no-afalgeng no-apps no-ui-console \
--with-zlib-include=${PREFIX}/include --with-zlib-lib=${PREFIX}/lib \
-static \
-DOPENSSL_NO_DGRAM -DOPENSSL_SYS_GREENBOX

#make
make -j$(nproc)

make install_sw 
make install_ssldirs

# [TODO] Ugly hack. Libssh2 cannot find openssl if we do not copy openssl static libs from lib64 to lib.
cp -f ${PREFIX}/lib64/libcrypto.a ${PREFIX}/lib/libcrypto.a
cp -f ${PREFIX}/lib64/libssl.a ${PREFIX}/lib/libssl.a 