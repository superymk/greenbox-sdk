#!/bin/bash

cd libssh2

rm -rf build
mkdir build

cd build


CFLAGS=${CFLAGS} ../configure --prefix=${PREFIX} --disable-tests --disable-examples-build --disable-sshd-tests --enable-static --with-libssl-prefix=${PREFIX} --with-libz-prefix=${PREFIX}



#make
make -j$(nproc)

make install