#!/bin/bash

# Build and install dynamic library
mkdir build
cd build
cmake ${CMAKE_ARGS} \
  -GNinja \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_BUILD_TYPE=Release \
  ..
ninja install
