#!/bin/bash

cd "$(dirname "$0")"
cd ../build
#pwd
rm -r CMakeFiles && rm -r cpp
rm CMakeCache.txt && rm cmake_install.cmake && rm Makefile && rm install_manifest.txt
