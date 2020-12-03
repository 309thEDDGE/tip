#!/usr/bin/env bash

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

echo "Setting each source file mod time to its last commit time"
cd $BASE_DIR
for FILE in $(git ls-files | grep -e "\.cpp$\|\.h\|\.sh$")
do
    TIME=$(git log --pretty=format:%cd -n 1 --date=iso -- "$FILE")
    TIME=$(date -d "$TIME" +%Y%m%d%H%M.%S)
    touch -m -t "$TIME" "$FILE"
	echo -n .
done
echo ""
echo "Done"

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

export PATH=${OLDPATH}
