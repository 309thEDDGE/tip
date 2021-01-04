#!/usr/bin/env bash

main() {
	# In the pipeline the working directory is the root of the project repository.
	# When running from docker, the tip folder is mounted as /app
	test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
	BASE_DIR=$PWD
	BUILD_DIR=$BASE_DIR/build
	DEPS_DIR=$BASE_DIR/deps
	DEPS_SOURCE=/deps
	BUILD_SCRIPT=$0

	# If we are running on a pipeline, always build with Alkemist
	# If not, only include Alkemist if ALKEMIST_LICENSE_KEY is provided
	if [[ -v PIPELINE || -n "$ALKEMIST_LICENSE_KEY" ]]; then
		# Add LFR ALKEMIST build flags
		echo "Building with Alkemist libraries"
		export TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr -L${DEPS_DIR}/alkemist-lfr/lib"
		OLDPATH="${PATH}"
		export PATH="${LFR_ROOT_PATH}/scripts:${PATH}"
	else
		echo "Building WITHOUT Alkemist libraries:"
		echo "   not on a pipeline; ALKEMIST_LICENSE_KEY unset or empty"
	fi

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

	# Get paths to all cached libraries
	BINARIES=( $(find $BUILD_DIR -name \*.a) ) || BINARIES=""
	### Diagnostics
	#  echo "Found ${#BINARIES[*]} cached libraries: ${BINARIES[*]}"
	#  [ ${#BINARIES[*]} -gt 0 ] && ls -lt ${BINARIES[*]}
	### End

	# If no cached libraries exist, skip directly to build
	if [ -z ${BINARIES[0]} ]; then
		echo "No cached libraries; doing a clean build"
	# If variable TIP_REBUILD_ALL is defined, rebuild all
	elif [ -v TIP_REBUILD_ALL ]; then
		echo "Variable TIP_REBUILD_ALL is set; rebuilding all"
		echo rm ${BINARIES[*]}
		rm ${BINARIES[*]}
	# Otherwise, allow CMake to build based on modification times
	else
		echo "Checking for outdated binaries"
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
			[ $BUILD_SCRIPT -nt $file ] && rm $file && echo "...removed outdated $file"
		done
	fi

	echo "Running '$CMAKE'"
	# the pipeline build image has a /deps directory
	# if there is a /deps directory then replace the local deps directory
	if [ -d $DEPS_SOURCE ] ; then
		echo "Restoring 3rd party dependencies from $DEPS_SOURCE"
		rm -rf $DEPS_DIR
		mv $DEPS_SOURCE $DEPS_DIR
	fi

	mkdir -p $BUILD_DIR
	cd $BUILD_DIR
	$CMAKE -DLIBIRIG106=ON -DVIDEO=ON ..

	echo "Running '$MAKE'"
	$MAKE install
	# move bin folder to build for use in later pipeline stages
	cd $BASE_DIR
	if [ -d bin ] ; then 
		rm -rf build/bin
		mv bin build/
	fi

	PATH=$"{OLDPATH}"
} # main

setup() {
	if ! is_set TIP_SETUP_COMPLETED; then
		BASE_DIR=$PWD
		if [[ -d /app ]]; then
			BASE_DIR=/app
		fi
		BUILD_DIR=$BASE_DIR/build
		DEPS_DIR=$BASE_DIR/deps
		TEST_DIR=$BASE_DIR/cpp
		BUILD_SCRIPT=$BASE_DIR/cpp_pipeline_scripts/build.sh
		DEPS_SOURCE=/deps
		E2E_TEST=/test

		# Set default values if unset
		CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-$BUILD_DIR}
		UNITTEST_REPORT_DIR=${UNITTEST_REPORT_DIR:-$BASE_DIR/reports}

		# Set variable to prevent setup from running again
		TIP_SETUP_COMPLETED=""
	fi
}

set_exit_on_error() {
	# exit when any command fails
	set -e

	# keep track of the last executed command
	trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
	# echo an error message before exiting
	trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR
}

is_set() {
	[[ -v $1 ]]
}

is_test() {
	is_set T_fail
}

is_file() {
	[[ -f "$1" ]]
}

is_absolute_path() {
	[[ "${1:0:1}" = "/" ]] # Make sure the first character is '/'
}

get_absolute_path() {
	readlink -f $1
}


# ------------------ RUN MAIN ---------------------
if ! is_test ; then 
	main "$@" 
fi

# ------------------ TESTS ------------------------
T_basht_works() {
	:
}

T_is_set_fails_when_varable_not_defined() {
	unset ___GOBBLEDY___gook____
	is_set ___GOBBLEDY___gook____
	[[ "$?" != 0 ]]
}

T_is_set_passes_when_varable_defined_to_null() {
	local ___gobbledy___GOOK____=
	is_set ___gobbledy___GOOK____
}

T_is_set_passes_when_variable_not_empty() {
	local ___gobbledy___GOOK____="not empty"
	is_set ___gobbledy___GOOK____
}

T_basht_detected() {
	is_set T_fail
}

T_is_test_passes() {
	is_test
}

T_is_file_fails_when_file_missing() {
	is_file /gobbledy/gook/____
	[[ "$?" != 0 ]]
}

T_is_file_passes_when_file_present() {
	is_file /usr/bin/env
}

T_setup_sets_base_dir_to_absolute_directory() {
	setup
	[[ -d "${BASE_DIR}" ]]
	is_absolute_path ${BASE_DIR}
}

T_setup_runs_only_once() {
	setup
	local real_base=${BASE_DIR}
	local fake="___fake_dir____"
	# Change BASE_DIR to prove a second call to setup doesn't reset it
	BASE_DIR=${fake}

	setup
	new_base=${BASE_DIR}
	BASE_DIR=${real_base} # Set BASE_DIR back to its correct value
	[[ $new_base != $real_base ]]
}

T_setup_base_dir_contains_tip() {
	setup
	[[ -d $BASE_DIR/cpp ]] 				|| $T_fail "BASE_DIR does not contain cpp: '$BASE_DIR'"
	[[ -d $BASE_DIR/tip_scripts ]] 		|| $T_fail "BASE_DIR does not contain tip_scripts: '$BASE_DIR'"
	[[ -f $BASE_DIR/CMakeLists.txt ]]	|| $T_fail "BASE_DIR does not contain CMakeLists.txt: '$BASE_DIR'"
}

T_setup_sets_build_dir_variable() {
	setup
	[[ "$BUILD_DIR" == "$BASE_DIR/build" ]]
}

T_setup_sets_deps_dir_variable() {
	setup
	[[ "$DEPS_DIR" == "$BASE_DIR/deps" ]]
}

T_setup_sets_test_dir_variable() {
	setup
	[[ "$TEST_DIR" == "$BASE_DIR/cpp" ]]
}

T_setup_sets_deps_source_variable() {
	setup
	[[ "$DEPS_SOURCE" == /deps ]]
}

T_setup_sets_e2e_test_variable() {
	setup
	[[ "$E2E_TEST" == /test ]]
}

T_setup_sets_build_script_variable() {
	setup
	[[ -f "$BUILD_SCRIPT" ]]
	[[ $(basename ${BUILD_SCRIPT}) == "build.sh" ]]
}

T_setup_sets_cmake_build_dir_default() {
	setup
	[[ "$CMAKE_BUILD_DIR" == "$BUILD_DIR" ]]
}

T_setup_sets_test_reports_default() {
	setup
	[[ "$UNITTEST_REPORT_DIR" == "$BASE_DIR/reports" ]]
}

T_is_absolute_path_passes_on_absolute() {
	is_absolute_path "/anything"
}

T_is_absolute_path_fails_on_relative() {
	is_absolute_path .
	[[ "$?" != 0 ]]
}

T_get_absolute_path() {
	local absolute=$(get_absolute_path .)
	is_absolute_path $absolute
}

T_set_exit_on_error_sets_traps() {
	set_exit_on_error

	[[ -n "$(trap -p DEBUG)" ]]
	[[ -n "$(trap -p ERR)" ]]
}

T_set_exit_on_error_sets_option() {
	set_exit_on_error
	# Clear traps
	trap - ERR DEBUG

	shopt -oq errexit
}
