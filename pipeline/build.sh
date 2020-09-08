# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
[ -d /app/cpp ] && cd /app # if /app/cpp exists cd to /app

# custom build command which will run in the pipeline
# in the pipeline the working directory is the root of the project repository
#   # CPP_BUILD_TOOL: "cmake"
# CC: gcc
# CXX: g++
# GCOV: gcov
# CMAKE_ARGS: -DBUILD_SSL=OFF -DBUILD_TESTS=ON
# CMAKE_BUILD_TARGETS: all
# CMAKE_BUILD_ARGS: ''
#   CC=${CC} CXX=${CXX} cmake ${CMAKE_ARGS} -DCMAKE_MAKE_PROGRAM=make ..
#   make -j ${CMAKE_BUILD_TARGETS} ${CMAKE_BUILD_ARGS} VERBOSE=1
BUILD_DIR=build

if [ ! -f deps/arrow_library_dependencies/lib/libarrow.a ] ; then vendor/build.sh ; fi

mkdir -p $BUILD_DIR \
    && cd $BUILD_DIR \
    && cmake -G Ninja .. -DLIBIRIG106=OFF \
    && ninja


# cmake --version
# gcc --version
# g++ --version
# ctest --version
# ninja --version
# gcovr --version
