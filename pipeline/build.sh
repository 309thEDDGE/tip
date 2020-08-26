# in the pipeline the working directory is the root of the project repository
BUILD_DIR=build

mkdir -p $BUILD_DIR \
    && cd $BUILD_DIR \
    && cmake .. -DCONTAINER=ON \
    && make -j2 VERBOSE=1
