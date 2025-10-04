#!/bin/bash

cd zlib

rm -rf build
mkdir build

cd build


CFLAGS=${CFLAGS} prefix=${PREFIX} ../configure --static



#make
make -j$(nproc)

make install