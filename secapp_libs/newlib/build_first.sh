#!/bin/bash

pushd ./newlib-4.5.0/newlib
aclocal-1.15
automake-1.15
popd

rm -rf build
mkdir build

cd build



../newlib-4.5.0/configure CFLAGS=${CFLAGS} --host=${HOST} --target=${TARGET} --prefix=${PREFIX} --enable-languages=c --with-newlib --disable-multilib --disable-newlib-io-float --enable-newlib-io-long-long --enable-newlib-io-long-double

#make
make -j$(nproc)

make install

