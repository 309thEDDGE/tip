#!/usr/bin/env bash

# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
BASE_DIR=$PWD
BUILD_DIR=$BASE_DIR/build
DEPS_DIR=$BASE_DIR/deps
DEPS_SOURCE=/deps
BUILD_SCRIPT=$0

# Add LFR ALKEMIST build flags
# TODO: Explain how to remove this later
export TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr -L${DEPS_DIR}/alkemist-lfr/lib"
OLDPATH="${PATH}"
export PATH="${LFR_ROOT_PATH}/scripts:${PATH}"

# custom build command which will run in the pipeline
# in the pipeline the working directory is the root of the project repository

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

BINARIES=( $(find $BUILD_DIR -name \*.a) )
### Diagnostics
#  echo "Found ${#BINARIES[*]} libraries: ${BINARIES[*]}"
#  [ ${#BINARIES[*]} -gt 0 ] && ls -lt ${BINARIES[*]}
### End

# If no libraries exist skip directly to build
if [ -z ${BINARIES[0]} ]; then
	echo "No cached libraries; doing a clean build"
# If variable TIP_REBUILD_ALL exists rebuild all
elif [ -v TIP_REBUILD_ALL ]; then
	echo "Variable TIP_REBUILD_ALL is set; rebuilding all"
	echo rm ${BINARIES[*]}
	rm ${BINARIES[*]}
# Otherwise build based on modification times
else
	echo "Checking for outdated binaries"
	# Get paths to all libraries
	echo "...setting each source file mod time to its last commit time"
	cd $BASE_DIR
	for FILE in $(git ls-files | grep -e "\.cpp$\|\.h\|\.sh$")
	do
		TIME=$(git log --pretty=format:%cd -n 1 --date=iso -- "$FILE")
		TIME=$(date -d "$TIME" +%Y%m%d%H%M.%S)
		touch -m -t "$TIME" "$FILE"
		echo -n .
	done
	echo "Done"

	# CMake doesn't know about build.sh, so check its dependencies explicitly
	for file in ${BINARIES[*]}; do
		[ $BUILD_SCRIPT -nt $file ] && rm $file && echo "removed outdated $file"
	done
fi

echo "Running '$CMAKE' for TIP"
# the pipeline build image has a /deps directory
# if there is a /deps directory then replace the local deps directory
if [ -d $DEPS_SOURCE ] ; then
	rm -rf $BASE_DIR/deps
	mv $DEPS_SOURCE $BASE_DIR
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
