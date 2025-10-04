#!/bin/bash

cd libjpeg-turbo

rm -rf build
mkdir build

cd build


cmake -D ENABLE_SHARED=0 -DCMAKE_PREFIX_PATH=${PREFIX} -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_C_FLAGS="${CFLAGS}" -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" ..

make
make install