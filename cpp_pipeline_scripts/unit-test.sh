#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline


# exit when any command fails
set -e


# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR

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

declare -A MAX_TIMES
[ -n "$PARSE_THRESHOLD" ] || PARSE_THRESHOLD=0.0
[ -n "$TRANSLATE_THRESHOLD" ] || TRANSLATE_THRESHOLD=0.0
MAX_TIMES=([parse]="$PARSE_THRESHOLD" [translate]="$TRANSLATE_THRESHOLD")


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
echo "-------------------- Check for outdated binaries --------------------"
echo ""
PIPELINE_SCRIPT_DIR=$(dirname $(readlink -f $0))
BUILD_SCRIPT=$PIPELINE_SCRIPT_DIR/build.sh
echo "Build script:"
ls -l $BUILD_SCRIPT
cd $CMAKE_BUILD_DIR
BINARIES=( $(find . -name *.a) )
### Diagnostics
#  echo "Found ${#BINARIES[*]} libraries:"
#  echo "${BINARIES[*]}"
#  ls -lt ${BINARIES[*]}
### End

# Reset build script's modification time to its last commit time
GIT_FILE=$(realpath --relative-to=. $BUILD_SCRIPT)
TIME=$(git log --pretty=format:%cd -n 1 --date=iso -- $GIT_FILE)
TIME=$(date -d "$TIME" +%Y%m%d%H%M.%S)
touch -m -t "$TIME" "$GIT_FILE"

OLDEST=$(ls -t $BINARIES $BUILD_SCRIPT | tail -1)
# Fail if the build script has changed after any binary was built
if [ $OLDEST != $BUILD_SCRIPT ]; then
	echo "Libraries:"
	ls -lt ${BINARIES[*]}
	echo ""
	echo "ERROR:At least one binary file is older than the build script."
	echo "      TIP executables are out of date."
	exit 1
else
	echo "All binaries are newer than the build script"
fi

# Skip unit tests if a log file was specified for testing
if [ -z "$LOG_FILE" ]; then
	echo ""
	echo "-------------------- Unit Tests --------------------"
	echo ""
	# For now, run cpp/tests because it is much faster than ctest
	# In the future we might have to run ctest in order to get coverage statistics
	# If we do, try to make our tests compatible with the --parallel option of ctest
	cd ${TEST_DIR}
	./tests
	cd ${BASE_DIR}

	echo ""
	echo "-------------------- Test Coverage --------------------"

	if which gcovr >& /dev/null; then
		mkdir -p ${UNITTEST_REPORT_DIR}
		echo "Writing coverage reports in ${UNITTEST_REPORT_DIR}"
		GCOV=gcov
		GCOV="${GCOV}" gcovr -j --verbose \
			--exclude-unreachable-branches \
			--exclude-throw-branches \
			--object-directory="${TEST_DIR}" \
			--xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
			--html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
			--sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml \
			--filter "${CPP_COVERAGE_FILTER}" \
			$([ -n "${CPP_COVERAGE_EXCLUDE}" ] && echo -n --exclude="${CPP_COVERAGE_EXCLUDE}")
	else 
		echo "*** No gcovr found. Skipping test coverage ***"
		if [ -z "$LOCAL" ]; then
			exit 1
		fi
	fi

	echo ""
	echo "-------------------- End-to-End Test ---------------------"
	cd $BASE_DIR
	# In the pipeline, only the build directory is present.
	# Restore the bin directory if needed.
	if [ ! -d ./bin ]; then mv build/bin . ; fi

	# If in a test container
	if [ -z "E2E_TEST" ]; then
		echo "No test directory specified and directory /test does not exist"
		EXIT $1
	fi

	# Run end-to-end validator
	python tip_scripts/e2e_validation/run_end_to_end_validator.py --video $E2E_TEST/truth $E2E_TEST/test $E2E_TEST/log

	# get newest log file
	LOG_FILE="$(ls -1t $E2E_TEST/log/* | head -1)"

else
	echo ""
	echo "*** Log file specified on command line.  Skipping tests. ***"
fi


echo ""
echo "-------------------- End-to-End Results ------------------"

if [[ -z "$LOG_FILE" ]]; then
	echo "FAIL: No log file was found."
	exit 1
fi

EXIT_CODE=0 # Start by assuming success
if ! grep "Total raw 1553 data: PASS" "$LOG_FILE" ; then
	echo "ERROR: Parser validation failed"
	EXIT_CODE=1
fi

echo ""
if ! grep "Total translated 1553 data: PASS" "$LOG_FILE" ; then
	echo "ERROR: Translator validation failed"
	# Continue without failing; the pipeline doesn't expect translation to succeed at this point
fi

function check_time {
	file=$1; shift
	key_word=$1; shift
	threshold=$1; shift
	return_value=0 # Assume success
	# Use the first 5 characters of the type in the pattern (e.g. Parse, Transl)
	pattern=$(echo "${key_word:0:5}.*:.*seconds")
	# echo ""
	# echo "File: $file "
	# echo "Pattern: $pattern "
	# echo "Threshold: $threshold"

	# Get the line containing the total time
	# and save an array of the words
	words=( $(grep -ie "$pattern" $file) )
	# echo ""
	# echo "${#words[*]} words "
	# echo "${words[*]} "

	# Get the word before "seconds"
	if [ ${#words[*]} -gt 1 ]; then 
		value="${words[-2]}"
	fi
	# echo "Value: $value"

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

# Check times
for type in ${!MAX_TIMES[@]} ; do
	echo ""
	echo -n "$type time: "
	if ! check_time $LOG_FILE $type ${MAX_TIMES[$type]} ; then
		# For now, do not fail for translate time
		if [ "${type,,}" != "translate" ]; then  # compare lower-case
			EXIT_CODE=1
		fi
	fi
	
done

echo ""
if [ -v PIPELINE -o -v ALKEMIST_LICENSE_KEY ]; then
	cd $BASE_DIR
	echo "-------------- Check for Alkemist presence --------------"
	ldd ./bin/pqcompare
	readelf -x .txtrp ./bin/pqcompare | grep 0x -m3
	ldd ./bin/bincompare
	readelf -x .txtrp ./bin/bincompare | grep 0x -m3
	ldd ./bin/tests
	readelf -x .txtrp ./bin/tests | grep 0x -m3
	ldd ./bin/tip_parse_video
	readelf -x .txtrp ./bin/tip_parse_video | grep 0x -m3
	ldd ./bin/tip_translate
	readelf -x .txtrp ./bin/tip_translate | grep 0x -m3
else
	echo "Skipping Alkemist check: "
	echo "   ALKEMIST_LICENSE_KEY and PIPELINE variables are both undefined"
fi

echo ""
exit $EXIT_CODE