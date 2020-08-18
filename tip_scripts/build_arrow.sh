#!/usr/bin/env bash
# Using /usr/bin/env to use bash wherever it is found in the user's path

#
# Functions
#

function assert {
# echo "Entering ${FUNCNAME[0]}: '$1', '$2', '$3'"
	if [ $1 != 0 ]; then 
		[ "$2" != "" ] && echo "$SCRIPT_NAME: $2"
		exit 1
	fi
}

function is_installed {
# echo "Entering ${FUNCNAME[0]}: '$1', '$2', '$3'"
	return $(which "$1" >& /dev/null)
}

function assert_installed {
# echo "Entering ${FUNCNAME[0]}: '$1', '$2', '$3'"
	is_installed $1
	assert "$?" "$1 install failed"
}

function assert_success {
# echo "Entering ${FUNCNAME[0]}: '$1', '$2', '$3'"
	assert "$?" "$1"
}

#
# Main
# 

# Variables used by this script
SCRIPT_NAME=$(basename $0)
ARROW_VERSION=apache-arrow-0.14.0

echo "Running script $SCRIPT_NAME"
echo "Starting in directory $PWD"
# Kludge: Get through a cd error early on (container only?).  Subsequent commands succeed.
cd $PWD >& /dev/null
echo "Installing Arrow version: $ARROW_VERSION"

echo -n "Checking for sudo ... "
SUDO_CMD=sudo
is_installed $SUDO_CMD
if [ $? != 0 ]; then
	echo "no.  Suppressing 'sudo' before install commands"
	SUDO_CMD=''
else
	echo yes
fi

# Find a directory in the current path containing the Arrow source
echo -n "Searching for directory: $ARROW_VERSION ... "
MAX_PARENT_LEVEL=5
level=0
while [[ ! -d ./$ARROW_VERSION && $level < $MAX_PARENT_LEVEL ]]; do
	cd .. ####>& /dev/null
	((level++))
done
test "-d ./$ARROW_VERSION" 
assert_success "Directory $ARROW_VERSION not found"

BASE_DIR=$(pwd -P)
echo "found $BASE_DIR/$ARROW_VERSION"

ARROW_DEP_PATH=$BASE_DIR/arrow_build_dependencies
PARQUET_TEST_DATA_SOURCE_PATH=$ARROW_DEP_PATH/parquet-testing-master
ARROW_CPP=$BASE_DIR/apache-arrow-0.14.0/cpp
BUILD_DIR_NAME=build
BZIP2_VERSION=1.0.8
FLEX_PATH=$ARROW_DEP_PATH/flex-2.6.4 # 2.6.4 fails on Ubuntu 18.04; use 2.6.3
BISON_PATH=$ARROW_DEP_PATH/bison-3.6

# Build Bison
is_installed bison
# [ 0 == 1 ] # Force build for testing and debugging
if [ $? != 0 ]; then
	echo
	echo
	echo "Installing Bison from $BISON_PATH.tar.gz"
	cd $ARROW_DEP_PATH
	test -d $BISON_PATH || tar xvzf $BISON_PATH.tar.gz
	cd $BISON_PATH
	./configure && make && $SUDO_CMD make install
	cd $BASE_DIR
fi
assert_installed bison

# Build Flex
# Note: Requires m4 to be installed
is_installed flex
# [ 0 == 1 ] # Force build for testing and debugging
if [ $? != 0 ]; then
	echo
	echo
	echo "Installing Flex from $FLEX_PATH.tar.gz"
	cd $ARROW_DEP_PATH
	test -d $FLEX_PATH || tar xvzf $FLEX_PATH.tar.gz
	cd $FLEX_PATH
	./configure && make && $SUDO_CMD make install
	cd $BASE_DIR
fi
assert_installed flex

#
# Variables used by Arrow CMake
#

# Specify locations of dependencies
export ARROW_BOOST_URL=$ARROW_DEP_PATH/boost_1_67_0.tar.gz
export ARROW_BROTLI_URL=$ARROW_DEP_PATH/brotli-1.0.7.tar.gz
export ARROW_CARES_URL=$ARROW_DEP_PATH/c-ares-1.15.0.tar.gz
export ARROW_DOUBLE_CONVERSION_URL=$ARROW_DEP_PATH/double-conversion-3.1.4.tar.gz
export ARROW_FLATBUFFERS_URL=$ARROW_DEP_PATH/flatbuffers-1.11.0.tar.gz
export ARROW_GBENCHMARK_URL=$ARROW_DEP_PATH/benchmark-1.4.1.tar.gz
export ARROW_GFLAGS_URL=$ARROW_DEP_PATH/gflags-2.2.0.tar.gz
export ARROW_GLOG_URL=$ARROW_DEP_PATH/glog-0.3.5.tar.gz
export ARROW_GRPC_URL=$ARROW_DEP_PATH/grpc-1.20.0.tar.gz
export ARROW_GTEST_URL=$ARROW_DEP_PATH/googletest-release-1.8.1.tar.gz
export ARROW_JEMALLOC_URL=$ARROW_DEP_PATH/jemalloc-5.2.1.tar.gz
export ARROW_LZ4_URL=$ARROW_DEP_PATH/lz4-1.8.3.tar.gz
export ARROW_PROTOBUF_URL=$ARROW_DEP_PATH/protobuf-all-3.7.1.tar.gz
export ARROW_RAPIDJSON_URL=$ARROW_DEP_PATH/rapidjson-2bbd33b33217ff4a73434ebf10cdac41e2ef5e34.tar.gz
export ARROW_RE2_URL=$ARROW_DEP_PATH/re2-2019-04-01.tar.gz
export ARROW_SNAPPY_URL=$ARROW_DEP_PATH/snappy-1.1.7.tar.gz
export ARROW_THRIFT_URL=$ARROW_DEP_PATH/thrift-0.12.0.tar.gz
export ARROW_URIPARSER_URL=$ARROW_DEP_PATH/uriparser-uriparser-0.9.2.tar.gz
export ARROW_ZLIB_URL=$ARROW_DEP_PATH/zlib-1.2.11.tar.gz
export ARROW_ZSTD_URL=$ARROW_DEP_PATH/zstd-1.4.0.tar.gz
export BZIP2_SOURCE_URL=$ARROW_DEP_PATH/bzip2-$BZIP2_VERSION.tar.gz

# Set compiler flags
export CC=gcc
export CFLAGS=-pthread
export CXX=g++
export CXXFLAGS=-pthread

# Build Arrow
echo
echo
echo Running cmake for Arrow
cd $ARROW_CPP
if [ "$1" != "keep" ]; then
	rm -rf $BUILD_DIR_NAME
fi
mkdir -p $BUILD_DIR_NAME
cd $BUILD_DIR_NAME

echo "In directory $PWD"
echo "Running cmake .."
cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DARROW_BUILD_TESTS=ON \
	-DARROW_BUILD_STATIC=ON \
	-DARROW_PARQUET=ON \
	-DARROW_HDFS=ON \
	-DARROW_EXTRA_ERROR_CONTEXT=ON \
	-DARROW_BUILD_UTILITIES=ON \
	-DARROW_BOOST_USE_SHARED=OFF \
	-DARROW_JEMALLOC=ON \
	-DPARQUET_BUILD_EXECUTABLES=ON \
	-DTHREADS_PREFER_PTHREAD_FLAG=ON \
	-DARROW_DEPENDENCY_SOURCE=BUNDLED \
	.. \
	| tee -i cmake.out
assert_success "cmake failed"

#
# Build
#
echo
echo
echo Building Arrow
$SUDO_CMD cmake --build . --target install --config Release | tee -i cmake_build.out
assert_success "build failed"

# Test
echo
echo
echo Running tests

# Need to put data from https:/github.com/apache/parquet-testing (data dir and bad_data dir)
# in a directory and link to that directory with the following env var.
export PARQUET_TEST_DATA=$ARROW_CPP/submodules/parquet-testing/data
cp -rf $PARQUET_TEST_DATA_SOURCE_PATH/* $(dirname $PARQUET_TEST_DATA)

ctest --output-on-failure -j2
