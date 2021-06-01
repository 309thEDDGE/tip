#!/bin/bash

mkdir build-release
cd build-release

cmake .. -DCONTAINER=True -DCMAKE_PREFIX_PATH=Arrow
cmake --build . --target install


