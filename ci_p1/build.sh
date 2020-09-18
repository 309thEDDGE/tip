#!/usr/bin/env bash

# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
BASE_DIR=$PWD
BUILD_DIR=$BASE_DIR/build
BUILD_TIP_DIR=$BUILD_DIR/build-tip
DEPS_DIR=$BUILD_DIR/deps
THIRD_PARTY=$BASE_DIR/vendor
TIMESTAMP_FILE=$DEPS_DIR/.timestamp


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

# Check whether newest library is newer than cached dependencies
# use a timestamp file as a shortcut for speed
echo -n "Checking for updated dependencies..."
NEWEST=$(cd $THIRD_PARTY ; ls -1t *.gz *.zip *.bz2 | head -1)
NEWEST=$THIRD_PARTY/$NEWEST
ls -lt $NEWEST $TIMESTAMP_FILE
if [ $NEWEST -nt $TIMESTAMP_FILE ] ; then 
	echo "need to rebuild dependencies"
	cd $THIRD_PARTY
	bash ./build.sh # use 'bash' command because of pipeline permissions
	rm -rf $DEPS_DIR
	cp -rf $THIRD_PARTY/deps $(dirname "$DEPS_DIR")
	touch $TIMESTAMP_FILE
else
	echo "cached dependencies are current"
fi

echo "Running '$CMAKE' for TIP"
mkdir -p $BUILD_TIP_DIR
cd $BUILD_TIP_DIR
$CMAKE -DLIBIRIG106=ON -DVIDEO=ON ../..

echo "Running '$MAKE' for TIP"
$MAKE

#######################################
rm -f $BUILD_DIR/deps.tar.gz $BUILD_DIR/.ninja* rm $BASE_DIR/.gitlab-ci.yml.deleteme
ls -alt $BASE_DIR $BUILD_DIR $BUILD_TIP_DIR $DEPS_DIR