#!/bin/bash

cd curl

rm -rf build
mkdir build

cd build
  
  
CFLAGS=${CFLAGS} LDFLAGS=${LDFLAGS} ../configure --prefix=${PREFIX} \
--host=amd64-linux \
--disable-shared \
--with-openssl=${PREFIX} --with-zlib=${PREFIX} \
--without-libpsl --disable-threaded-resolver

# build only the library (skip the curl tool)
make -j$(nproc) -C lib

# (optional) install just the library and headers
make -j$(nproc) -C lib install
make -j$(nproc) -C include install

make install-am