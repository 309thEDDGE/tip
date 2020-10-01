#!/usr/bin/env bash

#
# Set up error handling 
# Exit on failure, showing which command failed
#

# exit when any command fails
set -e
# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR

#
# Capture base directories
# Allow script to start from within the vendor folder or its parent
#

if [[ "$(basename $PWD)" != "vendor" ]] ; then cd vendor ; fi
VENDOR=$PWD
cd ..
BASE_DIR=$PWD


#
# Variables
#

SCRIPT_NAME=$(basename $0)

# source versions
ARROW_VERSION=apache-arrow-0.14.0
BISON_VERSION=bison-3.6
FLEX_VERSION=flex-2.6.4
BZIP2_VERSION=bzip2-1.0.8
GOOGLE_TEST_VERSION=googletest-release-1.8.1
YAML_CPP_VERSION=yaml-cpp-yaml-cpp-0.6.3
LIBIRIG106_VERSION=libirig106-master
PCAP_VERSION=libpcap-1.9.1
TINS_VERSION=libtins-4.2

# location of arrow test data
PARQUET_TEST_DATA_SOURCE_PATH=$VENDOR/parquet-testing-master
# locations to check for flex and bison executables 
BISON_EXECUTABLE=$VENDOR/$BISON_VERSION/src/bison
FLEX_EXECUTABLE=$VENDOR/$FLEX_VERSION/src/flex
# location of arrow build
ARROW_BUILD_DIR=$VENDOR/$ARROW_VERSION/cpp/build
# where to store built dependencies
TIP_DEPS_DIR=$VENDOR/deps

#
# MAIN
#
# Define a main function to be run at the end.  
# Doing this allows us to define other functions later in the file
function main {

message "Building TIP dependencies"

#
# Check for Ninja
# Set up for ninja if available; otherwise use cmake
#
start_message "Checking for ninja..."
if [ -f /usr/local/bin/ninja ] ; then
	end_message "yes"
	CMAKE="cmake -G Ninja"
	MAKE=ninja
else
	end_message "no.  Using make"
	CMAKE="cmake"
	MAKE="make -j8"
fi

#
# Extract some source files
# - Arrow, gtest, yaml-cpp, and libirig106 are required for TIP
# - Flex and bison must be installed to build some arrow dependencies
# - Arrow will extract its own dependencies
#

# extract source from tarballs if necessary
cd $VENDOR
test -d $ARROW_VERSION || ( message "extracting Arrow" ; tar -xzf "$ARROW_VERSION.tar.gz" )
test -d $BISON_VERSION || ( message "extracting Bison" ; tar -xzf "$BISON_VERSION.tar.gz" )
test -d $FLEX_VERSION || ( message "extracting Flex" ; tar -xzf "$FLEX_VERSION.tar.gz" )
test -d $GOOGLE_TEST_VERSION || ( message "extracting gtest" ; tar -xzf "$GOOGLE_TEST_VERSION.tar.gz" )
test -d $YAML_CPP_VERSION || ( message "extracting yaml-cpp" ; tar -xzf "$YAML_CPP_VERSION.tar.gz" )
test -d $PCAP_VERSION || ( message "extracting pcap" ; tar -xzf "$PCAP_VERSION.tar.gz" )
test -d $TINS_VERSION || ( message "extracting tins" ; tar -xzf "$TINS_VERSION.tar.gz" )

if [ ! -d $LIBIRIG106_VERSION ] ; then
	message "extracting libirig106"
	# extract either .tar.gz or .zip file
	if [ -f $LIBIRIG106_VERSION.tar.gz ] ; then tar -xzf $LIBIRIG106_VERSION.tar.gz
	else unzip $LIBIRIG106_VERSION.zip
	fi
fi

#
# Install m4
# m4 is required to build flex and pcap
dnf -y update
which m4 >& /dev/null || dnf -y install m4

#
# Build and install flex
#

# check if flex is built and installed
start_message "Checking for Flex..."
cd $VENDOR/$FLEX_VERSION
if [[ -f $FLEX_EXECUTABLE ]] ; then 
	end_message "Flex already built"
	# if built but not installed, install flex
	if [[ ! -f /usr/local/bin/flex ]] ; then
		message "Running '$MAKE install' for Flex"
		make install
		message "Installed Flex"
	fi
else
	# build and install flex
	end_message "Building Flex"
	./configure -C
	make # must use regular make command (not ninja)
	make install
fi

#
# Build and install bison
#

# check if bison is built and installed
cd $VENDOR/$BISON_VERSION
start_message "Checking for Bison..."
if [[ -f $BISON_EXECUTABLE ]] ; then 
	end_message "Bison already built"
	# if built but not installed, install bison
	if [[ ! -f /usr/local/bin/bison ]] ; then
		message "Running '$MAKE install for Bison'"
		make install
		message "Installed Bison"
	fi
else
	# build and install bison
	message "Building Bison"
	which makeinfo >& /dev/null || dnf -y install texinfo
	./configure -C
	make # must use regular make command
	make install
fi

#
# Build arrow
#

start_message "Checking for Arrow..."
if [[ -f $ARROW_BUILD_DIR/release/libarrow.a ]] ; then # Check for first file in library list
	end_message "Arrow already built"
else
	end_message "Building Arrow"
	# Specify locations of dependencies
	export ARROW_BOOST_URL=$VENDOR/boost_1_67_0.tar.gz
	export ARROW_BROTLI_URL=$VENDOR/brotli-1.0.7.tar.gz
	export ARROW_CARES_URL=$VENDOR/c-ares-1.15.0.tar.gz
	export ARROW_DOUBLE_CONVERSION_URL=$VENDOR/double-conversion-3.1.4.tar.gz
	export ARROW_FLATBUFFERS_URL=$VENDOR/flatbuffers-1.11.0.tar.gz
	export ARROW_GBENCHMARK_URL=$VENDOR/benchmark-1.4.1.tar.gz
	export ARROW_GFLAGS_URL=$VENDOR/gflags-2.2.0.tar.gz
	export ARROW_GLOG_URL=$VENDOR/glog-0.3.5.tar.gz
	export ARROW_GRPC_URL=$VENDOR/grpc-1.20.0.tar.gz
	export ARROW_GTEST_URL=$VENDOR/googletest-release-1.8.1.tar.gz
	export ARROW_JEMALLOC_URL=$VENDOR/jemalloc-5.2.1.tar.bz2
	export ARROW_LZ4_URL=$VENDOR/lz4-1.8.3.tar.gz
	export ARROW_PROTOBUF_URL=$VENDOR/protobuf-all-3.7.1.tar.gz
	export ARROW_RAPIDJSON_URL=$VENDOR/rapidjson-2bbd33b33217ff4a73434ebf10cdac41e2ef5e34.tar.gz
	export ARROW_RE2_URL=$VENDOR/re2-2019-04-01.tar.gz
	export ARROW_SNAPPY_URL=$VENDOR/snappy-1.1.7.tar.gz
	export ARROW_THRIFT_URL=$VENDOR/thrift-0.12.0.tar.gz
	export ARROW_URIPARSER_URL=$VENDOR/uriparser-uriparser-0.9.2.tar.gz
	export ARROW_ZLIB_URL=$VENDOR/zlib-1.2.11.tar.gz
	export ARROW_ZSTD_URL=$VENDOR/zstd-1.4.0.tar.gz
	export BZIP2_SOURCE_URL=$VENDOR/$BZIP2_VERSION.tar.gz

	# Set compiler flags
	export CC=gcc
	export CFLAGS=-pthread
	export CXX=g++
	export CXXFLAGS=-pthread

	echo
	message "...Running cmake for Arrow"
	mkdir -p $ARROW_BUILD_DIR && cd $ARROW_BUILD_DIR

	$CMAKE \
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
	message "...Running $MAKE for Arrow"
#	sudo $CMAKE --build . --target install --config release
	$MAKE
fi

#
# Build gtest
#

start_message "Checking for Google Test..."
GOOGLE_TEST_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/gtest
mkdir -p $GOOGLE_TEST_LIB ; cd $GOOGLE_TEST_LIB
end_message "Building Google Test"
cd $VENDOR/$GOOGLE_TEST_VERSION
mkdir -p build ; cd build
$CMAKE ..
$MAKE

#
# Build yaml-cpp
#

start_message "Checking for yaml-cpp..."
YAML_CPP_LIB=$VENDOR/$YAML_CPP_VERSION/build
mkdir -p $YAML_CPP_LIB ; cd $YAML_CPP_LIB 
end_message "Building yaml-cpp"
cd $VENDOR/$YAML_CPP_VERSION
mkdir -p build/test/prefix/lib
cp -n $VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/libgmock.a build/test/prefix/lib
cd build
$CMAKE ..
$MAKE

#
# Build libirig106
#

start_message "Checking for libirig106..."
LIBIRIG106_LIB=$VENDOR/$LIBIRIG106_VERSION
mkdir -p $LIBIRIG106_LIB ; cd $LIBIRIG106_LIB
end_message "Building libirig106"
cd $VENDOR/$LIBIRIG106_VERSION
make # must use regular make

#
# Build PCAP
#

PCAP_LIB=$VENDOR/$PCAP_VERSION/build
PCAP_INCLUDE=$VENDOR/$PCAP_VERSION
mkdir -p $PCAP_LIB
message "Building pcap"
cd $VENDOR/$PCAP_VERSION
mkdir -p build ; cd build
$CMAKE ..
$MAKE

#
# Build TINS
#

TINS_LIB=$VENDOR/$TINS_VERSION/build/lib
TINS_INCLUDE=$VENDOR/$TINS_VERSION/include
mkdir -p $TINS_LIB
message "Building tins"
cd $VENDOR/$TINS_VERSION
mkdir -p build ; cd build
$CMAKE -DLIBTINS_ENABLE_CXX11=1 \
	-DLIBTINS_BUILD_SHARED=0 \
	-DPCAP_LIBRARY=$PCAP_LIB/libpcap.a \
	-DPCAP_INCLUDE_DIR=$PCAP_INCLUDE \
	..
$MAKE

#
# Gather dependencies into one folder
#

message "Gathering dependencies"
message "...gmock"
GOOGLE_MOCK_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googlemock/include
GOOGLE_MOCK_INC_DEST=$TIP_DEPS_DIR/gsuite/googlemock/include
GOOGLE_MOCK_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock
GOOGLE_MOCK_LIB_DEST=$TIP_DEPS_DIR/gsuite/googlemock/lib

cd $GOOGLE_MOCK_INCLUDE
# find all .h files and copy them, preserving directory structure
find . -type f -name \*.h -exec install -D {} $GOOGLE_MOCK_INC_DEST/{} \;
cd $GOOGLE_MOCK_LIB
find . -type f -name \*.a -exec install -D {} $GOOGLE_MOCK_LIB_DEST/{} \;

message "...gtest"
GOOGLE_TEST_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googletest/include/
GOOGLE_TEST_INC_DEST=$TIP_DEPS_DIR/gsuite/googletest/include
GOOGLE_TEST_LIB_DEST=$TIP_DEPS_DIR/gsuite/googletest/lib

cd $GOOGLE_TEST_INCLUDE
find . -type f -name \*.h -exec install -D {} $GOOGLE_TEST_INC_DEST/{} \;
cd $GOOGLE_TEST_LIB
find . -type f -name \*.a -exec install -D {} $GOOGLE_TEST_LIB_DEST/{} \;

message "...yaml-cpp"
YAML_CPP_INCLUDE=$VENDOR/$YAML_CPP_VERSION/include
YAML_CPP_INC_DEST=$TIP_DEPS_DIR/yaml-cpp/include
YAML_CPP_LIB_DEST=$TIP_DEPS_DIR/yaml-cpp/lib

cd $YAML_CPP_INCLUDE
find . -type f -name \*.h -exec install -D {} $YAML_CPP_INC_DEST/{} \;
cd $YAML_CPP_LIB
find . -type f -name \*.a -exec install -D {} $YAML_CPP_LIB_DEST/{} \;

message "...libirig106"
LIBIRIG106_INCLUDE=$VENDOR/$LIBIRIG106_VERSION/src
LIBIRIG106_INC_DEST=$TIP_DEPS_DIR/libirig106/include
LIBIRIG106_LIB_DEST=$TIP_DEPS_DIR/libirig106/lib

cd $LIBIRIG106_INCLUDE
find . -type f -name \*.h -exec install -D {} $LIBIRIG106_INC_DEST/{} \;
cd $LIBIRIG106_LIB
find . -type f -name \*.a -exec install -D {} $LIBIRIG106_LIB_DEST/{} \;

message "...pcap"
PCAP_INCLUDE_DEST=$TIP_DEPS_DIR/pcap/include
PCAP_LIB_DEST=$TIP_DEPS_DIR/pcap/lib
mkdir -p $PCAP_INCLUDE_DEST
cd $PCAP_INCLUDE
find . -type f -name \*.h -exec install -D {} $PCAP_INCLUDE_DEST/{} \;
mkdir -p $PCAP_LIB_DEST
cd $PCAP_LIB
find . -type f -name \*.a -exec install -D {} $PCAP_LIB_DEST/{} \;

message "...tins"
TINS_INCLUDE_DEST=$TIP_DEPS_DIR/tins/include
TINS_LIB_DEST=$TIP_DEPS_DIR/tins/lib
mkdir -p $TINS_INCLUDE_DEST
cd $TINS_INCLUDE
find . -type f -name \*.h -exec install -D {} $TINS_INCLUDE_DEST/{} \;
mkdir -p $TINS_LIB_DEST
cd $TINS_LIB
find . -type f -name \*.a -exec install -D {} $TINS_LIB_DEST/{} \;

message "...arrow include files"
# Arrow include files are in cpp/src and cpp/build/src
# (some are built by cmake)
ARROW_INCLUDE="$VENDOR/$ARROW_VERSION/cpp/src $ARROW_BUILD_DIR/src"
ARROW_INC_DEST=$TIP_DEPS_DIR/arrow_library_dependencies/include
for source in $ARROW_INCLUDE; do
	cd $source
	find arrow -type f -name \*.h -exec install -D {} $ARROW_INC_DEST/{} \;
	find arrow -type f -name \*.hpp -exec install -D {} $ARROW_INC_DEST/{} \;
	find parquet -type f -name \*.h -exec install -D {} $ARROW_INC_DEST/{} \;
done

message "...arrow libraries"
# these are the arrow libraries required by TIP
ARROW_LIBRARIES="$ARROW_BUILD_DIR/release/libarrow.a \
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
	$ARROW_BUILD_DIR/zstd_ep-install/lib64/libzstd.a"
ARROW_LIB_DEST=$TIP_DEPS_DIR/arrow_library_dependencies/lib

mkdir -p $TIP_DEPS_DIR/arrow_library_dependencies/lib
cp -f $ARROW_LIBRARIES $TIP_DEPS_DIR/arrow_library_dependencies/lib

cd $TIP_DEPS_DIR
bash $VENDOR/save_timestamp.sh # use 'bash' command because of pipeline permissions

} # main

function incomplete_message {
	echo
	echo -n "$SCRIPT_NAME: $@"
}

function start_message {
	incomplete_message "$@"
	echo -n "..."
}

function end_message {
	echo "$@"
}

function message {
	incomplete_message "$@"
	echo
}

echo RUNNING MAIN
main $@
