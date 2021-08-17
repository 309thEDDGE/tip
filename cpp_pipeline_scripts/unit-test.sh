#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
	set_exit_on_error
	setup

	# A command line argument is either a test directory or a log file to use in place of end-to-end testing
	if [ -f "$1" ]; then
		LOG_FILE=$1
	elif [ -d "$1" ]; then
		E2E_TEST=$1
	elif [ -z "$1" ]; then
		if [ -d /test ]; then
			E2E_TEST=/test
		else
			echo "No file or directory specified and there is no /test directory"
			exit 1
		fi
	else
		echo ""
		echo "'$1' is not a file or directory"
		echo "Usage: unit_test.sh [log_file | test_directory]"
		exit 1
	fi

	declare -A THRESHOLDS
	[ -n "$PARSE_THRESHOLD" ] || PARSE_THRESHOLD=0.0
	[ -n "$TRANSLATE_THRESHOLD" ] || TRANSLATE_THRESHOLD=0.0
	THRESHOLDS=([parse]="$PARSE_THRESHOLD" [translate]="$TRANSLATE_THRESHOLD")


	BASE_DIR=${PWD}
	if [ -z "${CMAKE_BUILD_DIR}" ]; then
		LOCAL=True
		# We are not in the pipeline; set vars for running locally
		if [ -d /app ]; then
			BASE_DIR=/app
		fi
		CMAKE_BUILD_DIR=${BASE_DIR}/build
	fi
	UNITTEST_REPORT_DIR=$BASE_DIR/reports
	CMAKE_BUILD_DIR=$(readlink -f "$CMAKE_BUILD_DIR") # Change to absolute path
	TEST_DIR=${CMAKE_BUILD_DIR}/cpp

	echo ""
	exit $EXIT_CODE
} # main

	function check_time {
		file=$1; shift
		key_word=$1; shift
		threshold=$1; shift
		return_value=0 # Assume success
		# Use the first 5 characters of the type in the pattern (e.g. Parse, Transl)
		pattern=$(echo "${key_word:0:5}.*:.*seconds")

		# Get the line containing the total time
		# and save an array of the words
		words=( $(grep -ie "$pattern" $file) )

		# Get the word before "seconds"
		if [ ${#words[*]} -gt 1 ]; then
			value="${words[-2]}"
		fi

		if [ "$value" = "None" ]; then
			return_value=1
			echo "None"
		else
			# Use python to compare floating point values
			if [ $(python -c "print($value < $threshold)") != "True" ]; then
				return_value=1
				echo "FAIL: $value is not less than $threshold seconds"
			else
				echo "PASS"
			fi
		fi

		return $return_value
	}


# ------------------ RUN MAIN ---------------------
if ! is_test ; then
	main $@
fi
