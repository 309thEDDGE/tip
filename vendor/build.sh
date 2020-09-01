SCRIPT_NAME=$(basename $0)
ARROW_VERSION=apache-arrow-0.14.0
BISON_VERSION=bison-3.6
FLEX_VERSION=flex-2.6.4
BZIP2_VERSION=bzip2-1.0.8
PARQUET_TEST_DATA_SOURCE_PATH=$BASE_DIR/parquet-testing-master

test -d $ARROW_VERSION || tar -xvzf "$ARROW_VERSION.tar.gz"
test -d $BISON_VERSION || tar -xvzf "$BISON_VERSION.tar.gz"
test -d $FLEX_VERSION || tar -xvzf "$FLEX_VERSION.tar.gz"

echo "Building Bison"
cd $BISON_VERSION
./configure && make
cd $BASE_DIR	

echo "Building Flex"
cd $FLEX_PATH
./configure && make
cd $BASE_DIR
exit ############################
# Specify locations of dependencies
export ARROW_BOOST_URL=$BASE_DIR/boost_1_67_0.tar.gz
export ARROW_BROTLI_URL=$BASE_DIR/brotli-1.0.7.tar.gz
export ARROW_CARES_URL=$BASE_DIR/c-ares-1.15.0.tar.gz
export ARROW_DOUBLE_CONVERSION_URL=$BASE_DIR/double-conversion-3.1.4.tar.gz
export ARROW_FLATBUFFERS_URL=$BASE_DIR/flatbuffers-1.11.0.tar.gz
export ARROW_GBENCHMARK_URL=$BASE_DIR/benchmark-1.4.1.tar.gz
export ARROW_GFLAGS_URL=$BASE_DIR/gflags-2.2.0.tar.gz
export ARROW_GLOG_URL=$BASE_DIR/glog-0.3.5.tar.gz
export ARROW_GRPC_URL=$BASE_DIR/grpc-1.20.0.tar.gz
export ARROW_GTEST_URL=$BASE_DIR/googletest-release-1.8.1.tar.gz
export ARROW_JEMALLOC_URL=$BASE_DIR/jemalloc-5.2.1.tar.bz2
export ARROW_LZ4_URL=$BASE_DIR/lz4-1.8.3.tar.gz
export ARROW_PROTOBUF_URL=$BASE_DIR/protobuf-all-3.7.1.tar.gz
export ARROW_RAPIDJSON_URL=$BASE_DIR/rapidjson-2bbd33b33217ff4a73434ebf10cdac41e2ef5e34.tar.gz
export ARROW_RE2_URL=$BASE_DIR/re2-2019-04-01.tar.gz
export ARROW_SNAPPY_URL=$BASE_DIR/snappy-1.1.7.tar.gz
export ARROW_THRIFT_URL=$BASE_DIR/thrift-0.12.0.tar.gz
export ARROW_URIPARSER_URL=$BASE_DIR/uriparser-uriparser-0.9.2.tar.gz
export ARROW_ZLIB_URL=$BASE_DIR/zlib-1.2.11.tar.gz
export ARROW_ZSTD_URL=$BASE_DIR/zstd-1.4.0.tar.gz
export BZIP2_SOURCE_URL=$BASE_DIR/$BZIP2_VERSION.tar.gz

# Set compiler flags
export CC=gcc
export CFLAGS=-pthread
export CXX=g++
export CXXFLAGS=-pthread

echo Running cmake for Arrow
mkdir -p $ARROW_VERSION/cpp/build
cd $BUILD_DIR_NAME

cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DARROW_BUILD_TESTS=ON \
	-DARROW_BUILD_STATIC=ON \
	-DARROW_PARQUET=ON \
	-DARROW_HDFS=ON \
	-DARROW_EXTRA_ERROR_CONTEXT=ON \
	-DARROW_BUILD_UTILITIES=ON \
	-DARROW_BOOST_USE_SHARED=OFF \
	-DPARQUET_BUILD_EXECUTABLES=ON \
	-DTHREADS_PREFER_PTHREAD_FLAG=ON \
	-DARROW_DEPENDENCY_SOURCE=BUNDLED \
	..

echo Building Arrow
sudo cmake --build . --target install --config Release | tee -i cmake_build.out

echo Running tests

# Need to put data from https:/github.com/apache/parquet-testing (data dir and bad_data dir)
# in a directory and link to that directory with the following env var.
export PARQUET_TEST_DATA=$ARROW_VERSION/cpp/submodules/parquet-testing/data
cp -rf $PARQUET_TEST_DATA_SOURCE_PATH/* cpp/submodules/parquet-testing/data

ctest --output-on-failure -j2
