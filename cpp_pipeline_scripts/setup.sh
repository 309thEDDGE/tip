setup() {
	if ! is_set TIP_SETUP_COMPLETED; then
		test -d /app/cpp && cd /app # if /app/cpp exists cd to /app
		BASE_DIR=$PWD
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
	[[ "${1:0:1}" = "/" ]] # Test whether the first character is '/'
}

get_absolute_path() {
	readlink -f $1
}

is_alkemist_included() {
	is_set PIPELINE || is_set ALKEMIST_LICENSE_KEY
}

# ------------------ TESTS ------------------------
T_basht_works() {
	return 0 # We are just verifying this function is called
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
	[[ -d $BASE_DIR/cpp ]] && \
	[[ -d $BASE_DIR/tip_scripts ]] && \
	[[ -f $BASE_DIR/CMakeLists.txt ]]
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

# This test works but causes the test run to abort on any failure
# T_set_exit_on_error() {
# 	set_exit_on_error
# 	shopt -oq errexit
# 	[[ -n "$(trap -p DEBUG)" ]]
# 	[[ -n "$(trap -p ERR)" ]]
# }

T_is_alkemist_included_true_on_pipeline() {
	local PIPELINE=""
	is_alkemist_included
}

T_is_alkemist_included_true_with_license_defined() {
	local ALKEMIST_LICENSE_KEY=""
	is_alkemist_included
}

T_is_alkemist_included_false_w_o_pipeline_or_license() {
	if is_set ALKEMIST_LICENSE_KEY; then local alk="$ALKEMIST_LICENSE_KEY"; fi
	if is_set PIPELINE; then local pl="$PIPELINE"; fi

	unset ALKEMIST_LICENSE_KEY PIPELINE
	is_alkemist_included
	local result = $?

	if is_set $alk; then ALKEMIST_LICENSE_KEY="$alk"
	if is_set $pl; then PIPELINE="$pl"

	[[ $? != 0 ]]
}