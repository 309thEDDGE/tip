#!/bin/bash

cmake ${CMAKE_ARGS} . \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Profile \
    -DCI_COMMIT_TAG=$CI_COMMIT_TAG \
    -DCI_COMMIT_SHORT_SHA=$CI_COMMIT_SHORT_SHA

cmake --build . --target install

cd $PREFIX/bin
pwd
tree

    # -DCONDA_PREFIX=$CONDA_PREFIX \
    # -DCMAKE_BUILD_TYPE=Profile \
    # -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
    # -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \

