# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
BASE_DIR=$PWD
BUILD_DIR=$PWD/build
DEPS_DIR=$BUILD_DIR/deps
THIRD_PARTY=$BASE_DIR/vendor


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

######################
mkdir -p $DEPS_DIR
echo $DEPS_DIR ; ls -lt $DEPS_DIR
echo $THIRD_PARTY ; ls -lt $THIRD_PARTY
######################

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

echo -n "Checking for dependencies..."
if [ -f $DEPS_DIR/arrow_library_dependencies/lib/libarrow.a ] ; then 
	echo "found $DEPS_DIR/arrow_library_dependencies/lib/libarrow.a"
else
	echo "libarrow.a not found; building dependencies"
	bash $THIRD_PARTY/build.sh
	
	echo "Extracting cached dependencies"
	mkdir -p $BUILD_DIR ; cd $BUILD_DIR
	rm -rf $DEPS_DIR
	tar xf $THIRD_PARTY/deps.tar.gz
fi

echo "Running '$CMAKE' for TIP"
mkdir -p $BUILD_DIR ; cd $BUILD_DIR
$CMAKE -DLIBIRIG106=ON -DVIDEO=ON ..

echo "Running '$MAKE' for TIP"
$MAKE

######################
echo $DEPS_DIR ; ls -lt $DEPS_DIR
######################
