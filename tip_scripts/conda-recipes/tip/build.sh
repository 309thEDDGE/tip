#!/bin/bash

cmake ${CMAKE_ARGS} . \
    -G Ninja \
    -DCONDA_PREFIX=$BUILD_PREFIX \
    -DCMAKE_BUILD_TYPE=Profile \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCI_COMMIT_TAG=$CI_COMMIT_TAG \
    -DCI_COMMIT_SHORT_SHA=$CI_COMMIT_SHORT_SHA

cmake --build . --target install

