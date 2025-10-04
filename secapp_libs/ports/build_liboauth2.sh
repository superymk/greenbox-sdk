#!/bin/bash

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