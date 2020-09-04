# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR


if [[ "$(basename $PWD)" != "vendor" ]] ; then cd vendor ; fi
BASE_DIR=$PWD
pwd

SCRIPT_NAME=$(basename $0)
ARROW_VERSION=apache-arrow-0.14.0
BISON_VERSION=bison-3.6
BISON_EXECUTABLE=$BASE_DIR/$BISON_VERSION/src/bison
FLEX_VERSION=flex-2.6.4
FLEX_EXECUTABLE=$BASE_DIR/$FLEX_VERSION/src/flex
BZIP2_VERSION=bzip2-1.0.8
GOOGLE_TEST_VERSION=googletest-release-1.8.1
YAML_CPP_VERSION=yaml-cpp-yaml-cpp-0.6.3

PARQUET_TEST_DATA_SOURCE_PATH=$BASE_DIR/parquet-testing-master
ARROW_BUILD_DIR=$BASE_DIR/$ARROW_VERSION/cpp/build
ARROW_LIB=$ARROW_BUILD_DIR/release/libarrow.a

test -d $ARROW_VERSION || ( echo extracting Arrow ; tar -xzf "$ARROW_VERSION.tar.gz" )
test -d $BISON_VERSION || ( echo extracting Bison ; tar -xzf "$BISON_VERSION.tar.gz" )
test -d $FLEX_VERSION || ( echo extracting Flex ; tar -xzf "$FLEX_VERSION.tar.gz" )
test -d $GOOGLE_TEST_VERSION || (echo extracting gtest ; tar -xzf "$GOOGLE_TEST_VERSION.tar.gz")
test -d $YAML_CPP_VERSION || (echo extracting yaml-cpp ; tar -xzf "$YAML_CPP_VERSION.tar.gz")

which m4 >& /dev/null || dnf -y install m4

cd $BISON_VERSION ; pwd
echo -n "Checking for Bison..."
if [[ -f $BISON_EXECUTABLE ]] ; then 
	echo "Bison already built"
	if [[ ! -f /usr/local/bin/bison ]] ; then
		echo "Running 'make install'"
		make install
	fi
else
	echo "Building Bison"
	./configure
	make
	make install
fi
cd $BASE_DIR ; pwd

echo -n "Checking for Flex..."
cd $FLEX_VERSION ; pwd
if [[ -f $FLEX_EXECUTABLE ]] ; then 
	echo "Flex already built"
	if [[ ! -f /usr/local/bin/flex ]] ; then
		echo "Running 'make install'"
		make install
	fi
else
	echo "Building Flex"
	./configure
	make
	make install
fi
cd $BASE_DIR ; pwd

echo -n "Checking for Arrow..."
if [[ -f $ARROW_LIB ]] ; then
	echo "Arrow already built"
else
	echo "Building Arrow"
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

	echo
	echo Running cmake for Arrow
	mkdir -p $ARROW_BUILD_DIR && cd $ARROW_BUILD_DIR
	pwd

	cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DARROW_BUILD_TESTS=OFF \
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

	echo
	echo Building Arrow
	pwd
	sudo cmake --build . --target install --config Release | tee -i cmake_build.out
	# CMake doesn't return an exit code. Fail if libarrow.a wasn't built
	cd $BASE_DIR ; pwd
	test -f $ARROW_LIB
fi

# echo
# echo Running Arrow tests

## Need to put data from https:/github.com/apache/parquet-testing (data dir and bad_data dir)
## in a directory and link to that directory with the following env var.
# export PARQUET_TEST_DATA=$ARROW_VERSION/cpp/submodules/parquet-testing/data
# cp -rfn $PARQUET_TEST_DATA_SOURCE_PATH/* cpp/submodules/parquet-testing/data
# ctest --output-on-failure -j2
# cd $BASE_DIR ; pwd

echo 
echo Building Google Test
cd $GOOGLE_TEST_VERSION ; pwd
mkdir -p build ; cd build ; pwd
cmake -G Ninja ..
ninja
cd $BASE_DIR ; pwd

echo
echo Building yaml-cpp
cd $YAML_CPP_VERSION
mkdir -p build/test/prefix/lib
cp -n $BASE_DIR/$GOOGLE_TEST_VERSION/build/googlemock/libgmock.a build/test/prefix/lib
cd build
cmake -G Ninja ..
ninja
test/run-tests
cd $BASE_DIR ; pwd

cd .. ; pwd

mkdir -p deps/gsuite/googlemock/
cp -rfn vendor/$GOOGLE_TEST_VERSION/googlemock/include deps/gsuite/googlemock/googlemock/
cp -rfn vendor/$GOOGLE_TEST_VERSION/build/googlemock/ deps/gsuite/googlemock/lib

mkdir -p deps/gsuite/googletest/
cp -rfn vendor/$GOOGLE_TEST_VERSION/googletest/include/ deps/gsuite/googletest/include
cp -rfn vendor/$GOOGLE_TEST_VERSION/build/googlemock/gtest deps/gsuite/googletest/lib

mkdir -p deps/yaml-cpp/build
cp -rfn vendor/$YAML_CPP_VERSION/include deps/yaml-cpp
cp -rfn vendor/$YAML_CPP_VERSION/build/libyaml-cpp.a deps/yaml-cpp

mkdir -p deps/arrow_library_dependencies/lib
cp -rfn vendor/$ARROW_VERSION/cpp/src deps/arrow_library_dependencies/
cp -fn \
	$ARROW_BUILD_DIR/release/libarrow.a \
	$ARROW_BUILD_DIR/boost_ep-prefix/src/boost_ep/stage/lib/libboost_filesystem.a \
	$ARROW_BUILD_DIR/boost_ep-prefix/src/boost_ep/stage/lib/libboost_regex.a \
	$ARROW_BUILD_DIR/boost_ep-prefix/src/boost_ep/stage/lib/libboost_system.a \
	$ARROW_BUILD_DIR/brotli_ep/src/brotli_ep-install/lib/libbrotlicommon-static.a \
	$ARROW_BUILD_DIR/brotli_ep/src/brotli_ep-install/lib/libbrotlidec-static.a \
	$ARROW_BUILD_DIR/brotli_ep/src/brotli_ep-install/lib/libbrotlienc-static.a \
	$ARROW_BUILD_DIR/double-conversion_ep/src/double-conversion_ep/lib/libdouble-conversion.a \
	$ARROW_BUILD_DIR/glog_ep-prefix/src/glog_ep/lib/libglog.a \
	$ARROW_BUILD_DIR/jemalloc_ep-prefix/src/jemalloc_ep/dist/lib/libjemalloc.a \
	$ARROW_BUILD_DIR/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a \
	$ARROW_BUILD_DIR/release/libparquet.a \
	$ARROW_BUILD_DIR/snappy_ep-prefix/src/snappy_ep/libsnappy.a \
	$ARROW_BUILD_DIR/thrift_ep/src/thrift_ep-install/lib/libthrift.a \
	$ARROW_BUILD_DIR/zlib_ep/src/zlib_ep-install/lib/libz.a \
	$ARROW_BUILD_DIR/zstd_ep-install/lib64/libzstd.a \
	deps/arrow_library_dependencies/lib

echo "Finished building TIP dependencies"