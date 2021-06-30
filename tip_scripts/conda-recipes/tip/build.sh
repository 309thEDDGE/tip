#!/bin/bash

mkdir build-release
cd build-release

cmake ${CMAKE_ARGS} .. \
    -G Ninja \
    -DCONTAINER=ON \
    -DCONDA=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX

cmake --build . --target install -j2
