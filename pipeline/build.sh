# custom build command which will run in the pipeline
# in the pipeline the working directory is the root of the project repository
# BUILD_DIR=build

# mkdir -p $BUILD_DIR \
#     && cd $BUILD_DIR \
#     && cmake .. -DCONTAINER=ON \
#     && make -j2 VERBOSE=1

cmake --version
gcc --version
g++ --version
ctest --version
ninja --version
gcovr --version