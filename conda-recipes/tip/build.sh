#!/bin/bash

cmake ${CMAKE_ARGS} .. \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCI_COMMIT_TAG=$CI_COMMIT_TAG \
    -DCI_COMMIT_SHORT_SHA=$CI_COMMIT_SHORT_SHA

cmake --build . --target install
