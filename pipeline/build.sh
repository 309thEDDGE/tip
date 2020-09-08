# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
test -d /app/cpp && cd /app # if /app/cpp exists cd to /app

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

# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR

CMAKE="cmake -G Ninja"
MAKE=ninja
# CMAKE="cmake"
# MAKE=make

BUILD_DIR=build

echo -n "Checking for dependencies..."
if [ -f deps/arrow_library_dependencies/lib/libarrow.a ] ; then 
	echo "found deps/arrow_library_dependencies/lib/libarrow.a"
else
	echo "not found; building dependencies"
	vendor/build.sh
fi

mkdir -p $BUILD_DIR \
    && cd $BUILD_DIR \
    && $CMAKE .. -DLIBIRIG106=ON -DVIDEO=ON \
    && $MAKE


# cmake --version
# gcc --version
# g++ --version
# ctest --version
# ninja --version
# gcovr --version
