#!/usr/bin/env bash

# Add RunSafe repo to list of those yum will check for packages
tee /etc/yum.repos.d/runsafesecurity.repo> /dev/null <<- EOM
[RunSafeSecurity]
name=RunSafeSecurity
baseurl=https://runsafesecurity.jfrog.io/artifactory/rpm-alkemist-lfr
enabled=1
gpgcheck=0
gpgkey=https://runsafesecurity.jfrog.io/artifactory/rpm-alkemist-lfr/repodata/repomd.xml.key
repo_gpgcheck=1
EOM
# Install alkemist-lfr
yum -y install alkemist-lfr
source /etc/profile.d/alkemist-lfr.sh

# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
BASE_DIR=$PWD
BUILD_DIR=$BASE_DIR/build
DEPS_DIR=$BASE_DIR/deps
DEPS_SOURCE=/deps

# TODO: Explain how to remove this later
export TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr -L${DEPS_DIR}/alkemist-lfr/lib"
OLDPATH="${PATH}"
export PATH="${LFR_ROOT_PATH}/scripts:${PATH}"

# custom build command which will run in the pipeline
# in the pipeline the working directory is the root of the project repository
# CPP_BUILD_TOOL: "cmake"
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
if [[ -f /usr/local/bin/ninja ]] ; then
	echo "yes"
	CMAKE="cmake -G Ninja"
	MAKE=ninja
else
	echo "no.  Using make"
	CMAKE="cmake"
	MAKE="make -j8"
fi

echo "Running '$CMAKE' for TIP"
# the pipeline build image has a /deps directory
# if there is a /deps directory then replace the local deps directory
if [ -d $DEPS_SOURCE ] ; then
	rm -rf $BASE_DIR/deps
	mv $DEPS_SOURCE $BASE_DIR
	mkdir -p ${DEPS_DIR}/alkemist-lfr/lib
	mv ${LFR_ROOT_PATH}/lib/run/liblfr.a ${DEPS_DIR}/alkemist-lfr/lib
fi
mkdir -p $BUILD_DIR
cd $BUILD_DIR
$CMAKE -DLIBIRIG106=ON -DVIDEO=ON ..

echo "Running '$MAKE' for TIP"
$MAKE install
# move bin folder to build for use in later pipeline stages
cd $BASE_DIR
if [ -d bin ] ; then 
	rm -rf build/bin
	mv bin build/
fi

export PATH=${OLDPATH}
