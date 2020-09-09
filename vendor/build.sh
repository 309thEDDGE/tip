# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR


if [[ "$(basename $PWD)" != "vendor" ]] ; then cd vendor ; fi
VENDOR=$PWD
cd ..
BASE_DIR=$PWD

echo -n "Checking for ninja..."
if [ -f /usr/local/bin/ninja ] ; then
	echo "yes"
	CMAKE="cmake -G Ninja"
	MAKE=ninja
else
	echo "no.  Using make"
	CMAKE="cmake"
	MAKE="make -j8"
fi

SCRIPT_NAME=$(basename $0)
ARROW_VERSION=apache-arrow-0.14.0
BISON_VERSION=bison-3.6
BISON_EXECUTABLE=$VENDOR/$BISON_VERSION/src/bison
FLEX_VERSION=flex-2.6.4
FLEX_EXECUTABLE=$VENDOR/$FLEX_VERSION/src/flex
BZIP2_VERSION=bzip2-1.0.8
GOOGLE_TEST_VERSION=googletest-release-1.8.1
YAML_CPP_VERSION=yaml-cpp-yaml-cpp-0.6.3

PARQUET_TEST_DATA_SOURCE_PATH=$VENDOR/parquet-testing-master
TIP_DEPS_DIR=$VENDOR/deps
TIP_DEPS_TARBALL=deps.tar.gz
LIBIRIG106_VERSION=libirig106-master

GOOGLE_MOCK_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googlemock/include
GOOGLE_MOCK_INC_DEST=$TIP_DEPS_DIR/gsuite/googlemock
GOOGLE_MOCK_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/include
GOOGLE_MOCK_LIB_DEST=$TIP_DEPS_DIR/gsuite/googlemock/lib

GOOGLE_TEST_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googletest/include/
GOOGLE_TEST_INC_DEST=$TIP_DEPS_DIR/gsuite/googletest/include
GOOGLE_TEST_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/gtest
GOOGLE_TEST_LIB_DEST=$TIP_DEPS_DIR/gsuite/googletest/lib

YAML_CPP_INCLUDE=$VENDOR/$YAML_CPP_VERSION/include
YAML_CPP_INC_DEST=$TIP_DEPS_DIR/yaml-cpp/include
YAML_CPP_LIB=$VENDOR/$YAML_CPP_VERSION/build
YAML_CPP_LIB_DEST=$TIP_DEPS_DIR/yaml-cpp/lib

LIBIRIG106_INCLUDE=$VENDOR/$LIBIRIG106_VERSION/src
LIBIRIG106_INC_DEST=$TIP_DEPS_DIR/libirig106/include
LIBIRIG106_LIB=$VENDOR/$LIBIRIG106_VERSION
LIBIRIG106_LIB_DEST=$TIP_DEPS_DIR/libirig106/lib

ARROW_INCLUDE="$VENDOR/$ARROW_VERSION/cpp/src $ARROW_BUILD_DIR/src"
ARROW_INC_DEST=$TIP_DEPS_DIR/arrow_library_dependencies/src
ARROW_BUILD_DIR=$VENDOR/$ARROW_VERSION/cpp/build
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

FIND_LIBS_CMD="find . -type f -name \*.a -quit" # find at least one .a file

cd $VENDOR
NEWEST=`ls -1t *.tar.* | head -1`
if [[ "$NEWEST" == "$TIP_DEPS_TARBALL" ]] ; then
	echo "TIP dependencies cache is current"
	cd $BASE_DIR
	rm -rf deps
	tar -xvzf $VENDOR/$TIP_DEPS_TARBALL
	exit 0
fi
echo "Building TIP dependencies"

cd $VENDOR

test -d $ARROW_VERSION || ( echo extracting Arrow ; tar -xzf "$ARROW_VERSION.tar.gz" )
test -d $BISON_VERSION || ( echo extracting Bison ; tar -xzf "$BISON_VERSION.tar.gz" )
test -d $FLEX_VERSION || ( echo extracting Flex ; tar -xzf "$FLEX_VERSION.tar.gz" )
test -d $GOOGLE_TEST_VERSION || (echo extracting gtest ; tar -xzf "$GOOGLE_TEST_VERSION.tar.gz")
test -d $YAML_CPP_VERSION || (echo extracting yaml-cpp ; tar -xzf "$YAML_CPP_VERSION.tar.gz")
if [ ! -d $LIBIRIG106 ] ; then
	echo extracting libirig106
	if [ -f $LIBIRIG106.tar.gz ] ; then tar -xzf $LIBIRIG106.tar.gz
	else unzip $LIBIRIG106.zip
	fi
fi

which m4 >& /dev/null || dnf -y install m4

echo -n "Checking for Flex..."
cd $VENDOR/$FLEX_VERSION
if [[ -f $FLEX_EXECUTABLE ]] ; then 
	echo "Flex already built"
	if [[ ! -f /usr/local/bin/flex ]] ; then
		echo "Running '$MAKE install' for Flex"
		$MAKE install
		echo "Installed Flex"
	fi
else
	echo "Building Flex"
	./configure
	$MAKE
	$MAKE install
fi

cd $VENDOR/$BISON_VERSION
echo -n "Checking for Bison..."
if [[ -f $BISON_EXECUTABLE ]] ; then 
	echo "Bison already built"
	if [[ ! -f /usr/local/bin/bison ]] ; then
		echo "Running '$MAKE install for Bison'"
		$MAKE install
		echo "Installed Bison"
	fi
else
	echo "Building Bison"
	./configure
	$MAKE
	$MAKE install
fi

echo -n "Checking for Arrow..."
if [[ -f ${ARROW_LIBRARIES%% *} ]] ; then # Check for first file in library list
	echo "Arrow already built"
else
	echo "Building Arrow"
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
	echo "...Running $CMAKE for Arrow"
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
	echo "...Running $MAKE for Arrow"
	sudo $CMAKE --build . --target install --config Release
	##CMake doesn't return an exit code. Fail if libarrow.a wasn't built
	# cd $VENDOR
	# test -f $ARROW_LIB
fi

# echo
# echo Running Arrow tests

## Need to put data from https:/github.com/apache/parquet-testing (data dir and bad_data dir)
## in a directory and link to that directory with the following env var.
# export PARQUET_TEST_DATA=$ARROW_VERSION/cpp/submodules/parquet-testing/data
# cp -rfn $PARQUET_TEST_DATA_SOURCE_PATH/* cpp/submodules/parquet-testing/data
# ctest --output-on-failure -j2

echo -n "Checking for Google Test..."
cd $GOOGLE_TEST_LIB
if $FIND_LIBS_CMD ; then
	echo "Google Test already built"
else
	echo Building Google Test
	cd $VENDOR/$GOOGLE_TEST_VERSION
	rm -rf build mkdir -p build ; cd build
	$CMAKE ..
	$MAKE
fi

echo -n "Checking for yaml-cpp..."
cd $YAML_CPP_LIB 
if $FIND_LIBS_CMD ; then
	echo "yaml-cpp already built"
else
	echo "Building yaml-cpp"
	cd $VENDOR/$YAML_CPP_VERSION
	mkdir -p build/test/prefix/lib
	cp -n $VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/libgmock.a build/test/prefix/lib
	cd build
	$CMAKE ..
	$MAKE
	test/run-tests
fi

echo -n "Checking for libirig106..."
cd $LIBIRIG106_LIB
if $FIND_LIBS_CMD ; then
	echo "libirig106 already built"
else
	echo Building libirig106
	cd $VENDOR/$LIBIRIG106_VERSION
	$MAKE
fi

# Gather dependencies into deps folder
echo "Gathering dependencies"

GOOGLE_MOCK_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googlemock/include
GOOGLE_MOCK_INC_DEST=$TIP_DEPS_DIR/gsuite/googlemock/include
GOOGLE_MOCK_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock
GOOGLE_MOCK_LIB_DEST=$TIP_DEPS_DIR/gsuite/googlemock/lib

GOOGLE_TEST_INCLUDE=$VENDOR/$GOOGLE_TEST_VERSION/googletest/include/
GOOGLE_TEST_INC_DEST=$TIP_DEPS_DIR/gsuite/googletest/include
GOOGLE_TEST_LIB=$VENDOR/$GOOGLE_TEST_VERSION/build/googlemock/gtest
GOOGLE_TEST_LIB_DEST=$TIP_DEPS_DIR/gsuite/googletest/lib

YAML_CPP_INCLUDE=$VENDOR/$YAML_CPP_VERSION/include
YAML_CPP_INC_DEST=$TIP_DEPS_DIR/yaml-cpp/include
YAML_CPP_LIB=$VENDOR/$YAML_CPP_VERSION/build
YAML_CPP_LIB_DEST=$TIP_DEPS_DIR/yaml-cpp/lib

LIBIRIG106_INCLUDE=$VENDOR/$LIBIRIG106_VERSION/src
LIBIRIG106_INC_DEST=$TIP_DEPS_DIR/libirig106/include
LIBIRIG106_LIB=$VENDOR/$LIBIRIG106_VERSION
LIBIRIG106_LIB_DEST=$TIP_DEPS_DIR/libirig106/lib

ARROW_INCLUDE="$VENDOR/$ARROW_VERSION/cpp/src $ARROW_BUILD_DIR/src"
ARROW_INC_DEST=$TIP_DEPS_DIR/arrow_library_dependencies/src
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



echo "...gmock"
cd $GOOGLE_MOCK_INCLUDE
# find all .h files and copy them, preserving directory structure
find . -type f -name \*.h -exec install -D {} $GOOGLE_MOCK_INC_DEST/{} \;
cd $GOOGLE_MOCK_LIB
find . -type f -name \*.a -exec install -D {} $GOOGLE_MOCK_LIB_DEST/{} \;

echo "...gtest"
cd $GOOGLE_TEST_INCLUDE
find . -type f -name \*.h -exec install -D {} $GOOGLE_TEST_INC_DEST/{} \;
cd $GOOGLE_TEST_LIB
find . -type f -name \*.a -exec install -D {} $GOOGLE_TEST_LIB_DEST/{} \;


echo "...yaml-cpp"
cd $YAML_CPP_INCLUDE
find . -type f -name \*.h -exec install -D {} $YAML_CPP_INC_DEST/{} \;
cd $YAML_CPP_LIB
find . -type f -name \*.a -exec install -D {} $YAML_CPP_LIB_DEST/{} \;

echo "...libirig106"
cd $LIBIRIG106_INCLUDE
find . -type f -name \*.h -exec install -D {} $LIBIRIG106_INC_DEST/{} \;
cd $LIBIRIG106_LIB
find . -type f -name \*.a -exec install -D {} $LIBIRIG106_LIB_DEST/{} \;

echo "...arrow include files"
for source in $ARROW_INCLUDE; do
	cd $source
	find arrow -type f -name \*.h -exec install -D {} $ARROW_INC_DEST/{} \;
	find parquet -type f -name \*.h -exec install -D {} $ARROW_INC_DEST/{} \;
done

echo "...arrow libraries"
mkdir -p $TIP_DEPS_DIR/arrow_library_dependencies/lib
cp -f \
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
	$TIP_DEPS_DIR/arrow_library_dependencies/lib

echo "Finished building TIP dependencies; caching results"
cd $VENDOR
tar czf $TIP_DEPS_TARBALL $(basename $TIP_DEPS_DIR)
echo "Cached TIP dependencies in $TIP_DEPS_TARBALL"

rm -rf $BASE_DIR/deps
mv $TIP_DEPS_DIR $BASE_DIR
