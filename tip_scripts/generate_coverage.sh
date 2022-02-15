#!/usr/bin/bash

exit_if_nonzero () { if [ $? -ne 0 ]
    then
        echo ""
        echo "////////////////////////////////////////"
        echo "$1 failed"
        echo "////////////////////////////////////////"
        echo ""
        exit 1
    fi
}

CMAKE_DIR=`pwd`/CMakeFiles
if [ ! -d $CMAKE_DIR ]
then
    cmake .. -GNinja -DCONDA_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=Profile -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_INSTALL_LIBDIR=lib
    exit_if_nonzero "CMake Configure"
fi

cmake --build . --target install
exit_if_nonzero "CMake Build"

tests
exit_if_nonzero "Run tests"

gcovr -r ../. --exclude-unreachable-branches --exclude-throw-branches --html-details ./overall-coverage.html .
exit_if_nonzero "gcovr"

