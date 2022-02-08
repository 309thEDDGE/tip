#!/usr/bin/bash

CMAKE_DIR=`pwd`/CMakeFiles
if [ ! -d $CMAKE_DIR ]
then
    cmake .. -GNinja -DCONDA_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=Profile -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_INSTALL_LIBDIR=lib
fi

cmake --build . --target install
tests
gcovr -r ../. --exclude-unreachable-branches --exclude-throw-branches --html-details ./overall-coverage.html .
